#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "engine.h"
#include "terrain.h"
#include "bullet.h"
#include "bulletpool.h"
#include "robot.h"
#include "robotpool.h"
#include "game.h"

void GameInit(struct Game* self)
{
    struct Engine* engine = calloc(1, sizeof(*engine));
    EngineInit(engine);

    self->engine = engine;
    self->engine->keyCallback = GameKeyPress;
    self->engine->gameLoopCallback = GameLoop;
    self->engine->userPointer = (void*)self;

    self->engine->camera.x = 5.0f;
    self->engine->camera.y = 5.0f;
    self->engine->camera.z = 5.0f;
    self->engine->camera.angle = 0.0f;

    self->numKeyStates = (uint16_t)GLFW_KEY_LAST + 1;

    self->terrain.tileSize = 0.8f;
    self->terrain.points = GameGenerateTerrain(
        self,
        self->terrain.tileSize,
        &(self->terrain.quadsPerSide),
        "assets/heightmaps/heightmap.bmp"
    );
    self->terrain.gameObject = GameCreateTerrain(
        self,
        self->terrain.points,
        self->terrain.quadsPerSide,
        "assets/textures/grass.jpg"
    );

    GameStart(self);
    EngineRun(self->engine);
    EngineDestroy(self->engine);
    free(self->engine);

    free(self->terrain.points);
}

void GameStart(struct Game* self)
{
    // Load bullet mesh
    struct Mesh* bulletMesh;
    bulletMesh = GameGetMesh(
        self,
        "assets/textures/bullet-texture.png",
        "assets/models/bullet.dae"
    );

    // Initialize pool for bullets
    struct GameObject** bulletObjects;
    bulletObjects = malloc(sizeof(struct GameObject*) * GAME_NUM_BULLETS);
    uint8_t i;
    for (i=0; i<GAME_NUM_BULLETS; i++)
    {
        bulletObjects[i] = EngineCreateGameObject(self->engine, bulletMesh);
    }
    BulletPoolInit(&(self->bulletPool), bulletObjects, GAME_NUM_BULLETS);
    free(bulletObjects);

    // Load robot mesh
    struct Mesh* robotMesh;
    robotMesh = GameGetMesh(
        self,
        "assets/textures/robot-texture.png",
        "assets/models/robot.dae"
    );

    // Initialize pool for robots
    struct GameObject** robotObjects;
    robotObjects = malloc(sizeof(struct GameObject*) * GAME_NUM_ROBOTS);
    for (i=0; i<GAME_NUM_ROBOTS; i++)
    {
        robotObjects[i] = EngineCreateGameObject(self->engine, robotMesh);
    }
    RobotPoolInit(
        &(self->robotPool),
        robotObjects,
        GAME_NUM_ROBOTS,
        &(self->bulletPool)
    );
    free(robotObjects);

    // Create robots
    for (i=0; i<GAME_NUM_ROBOTS; i++)
    {
        RobotPoolCreate(&(self->robotPool), (float)i*10, 5.0f, 0.0f);
    }
    self->player = &(self->robotPool.robots[0]);
    self->player->playerControlled = 1;

    // Record starting time
    self->then = (double)clock();
}

void GameLoop(void* gamePointer)
{
    struct Game* self = (struct Game*) gamePointer;

    double currentTime = (double)clock();
    double elapsed = ((double)(currentTime - self->then) / CLOCKS_PER_SEC) * 1000.0f;

    GameUpdate(self, elapsed);
    GameRender(self);

    self->then = currentTime;
}

void GameUpdate(struct Game* self, double elapsed)
{
    RobotPoolUpdate(
        &(self->robotPool),
        &(self->terrain),
        elapsed,
        self->keyStates
    );
    BulletPoolUpdate(
        &(self->bulletPool),
        elapsed
    );
    GameUpdateCamera(self, elapsed);
}

void GameUpdateCamera(struct Game* self, double elapsed)
{
    if (self->player == NULL)
        return;

    struct Camera* camera = &(self->engine->camera);
    float* playerPos = self->player->gameObject->position;

    camera->x = playerPos[0] + (sinf(self->player->rotation - 0.18f) * 8.0f);
    camera->y = playerPos[1] + (cosf(self->player->rotation - 0.18f) * 8.0f);
    camera->z = playerPos[2] + 7.5f;

    camera->xTarget = camera->x - sinf(self->player->rotation);
    camera->yTarget = camera->y - cosf(self->player->rotation);
    camera->zTarget = camera->z - 0.2f;
}

void GameRender(struct Game* self)
{

}

void GameKeyPress(void* userPointer, int key, int action)
{
    struct Game* self = (struct Game*)userPointer;

    if (key == GLFW_KEY_UNKNOWN)
        return;

    if (action == GLFW_PRESS)
    {
        self->keyStates[key] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        self->keyStates[key] = 0;
    }
}

struct Mesh* GameGetMesh(struct Game* self, const char* texturePath, const char* modelPath)
{
    EngineCreateDescriptor(
        self->engine,
        &(self->engine->meshes[self->engine->meshCount].descriptor),
        texturePath
    );
    EngineLoadModel(
        self->engine,
        modelPath
    );

    return &(self->engine->meshes[self->engine->meshCount-1]);
}

struct GameObject* GameCreateTerrain(struct Game* self, float* points, uint32_t size, const char* texturePath)
{
    uint32_t numPoints = (size+1) * (size+1);

    struct Vertex* vertices;
    vertices = calloc(numPoints, sizeof(*vertices));

    uint32_t* indices;
    indices = calloc(6 * (size*size), sizeof(*indices));
    uint32_t indexCount = 0;

    uint32_t row = 0;
    uint32_t i;
    for (i=0; i<numPoints; i++)
    {
        /*printf("{ %f, %f, %f }\n",
            points[i*3+0],
            points[i*3+1],
            points[i*3+2]
        );*/

        if (i % (size+1) == 0)
        {
            row++;
        }

        // Generate vertices
        vertices[i].position[0] = points[i * 3 + 0];
        vertices[i].position[1] = points[i * 3 + 1];
        vertices[i].position[2] = points[i * 3 + 2];
        vertices[i].texCoord[0] = 0.2f * size * ((float)(i%(size+1)) / (float)size);
        vertices[i].texCoord[1] = 0.2f * size * ((float)row / (float)(size+1));
        vertices[i].color[0] = 0.0f;
        vertices[i].color[1] = 0.0f;
        vertices[i].color[2] = 0.0f;

        // Generate indices for all non-degenerate points
        if ((i+1) % (size+1) != 0 && i < numPoints - (size+1))
        {
            indices[indexCount] = i;
            indexCount++;
            indices[indexCount] = i + (size+1);
            indexCount++;
            indices[indexCount] = i + (size+2);
            indexCount++;
            indices[indexCount] = i + (size+2);
            indexCount++;
            indices[indexCount] = i + 1;
            indexCount++;
            indices[indexCount] = i;
            indexCount++;
        }
    }

    EngineCreateDescriptor(
        self->engine,
        &(self->engine->meshes[self->engine->meshCount].descriptor),
        texturePath
    );
    EngineCreateMesh(
        self->engine,
        vertices, i,
        indices, indexCount
    );
    EngineCreateGameObject(
        self->engine,
        &(self->engine->meshes[self->engine->meshCount-1])
    );

    free(indices);
    free(vertices);

    return &(self->engine->gameObjects[self->engine->gameObjectCount-1]);
}

float* GameGenerateTerrain(struct Game* self, float tileSize, uint32_t* size, char* heightmap)
{
    // stb_image takes signed int parameters (for some reason)
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        heightmap,
        &texWidth,
        &texHeight,
        &texChannels,
        STBI_rgb_alpha
    );

    if (!pixels)
    {
        fprintf(stderr, "stb_image failed to load resource %s\n", heightmap);
        exit(-1);
    }

    assert(texWidth == texHeight);
    *size = texWidth - 1;

    float* terrain;
    terrain = calloc(3*texWidth*texWidth, sizeof(*terrain));

    float xOffset = 0.0f;
    float yOffset = 0.0f;

    uint32_t pixelIndex = 0;
    int i;
    for (i=0; i<3*texWidth*texWidth; i+=3)
    {
        uint32_t currentPoint = ((i/3)+1) % texWidth;

        terrain[i+0] = xOffset;
        terrain[i+1] = yOffset;
        terrain[i+2] = ((float)pixels[pixelIndex])/8.0f;
        //terrain[i+2] = 0.0f;//2 * (float)rand()/(float)RAND_MAX;
        pixelIndex+=4;

        yOffset += tileSize;
        if (currentPoint == 0)
        {
            xOffset += tileSize;
            yOffset = 0;
        }
    }

    stbi_image_free(pixels);

    return terrain;
}
