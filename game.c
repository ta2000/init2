#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "bullet.h"
#include "bulletpool.h"
#include "robot.h"
#include "robotpool.h"
#include "engine.h"
#include "game.h"

void GameInit(struct Game* game)
{
    struct Engine* engine = calloc(1, sizeof(*engine));
    EngineInit(engine);

    game->engine = engine;
    game->engine->keyCallback = GameKeyPress;
    game->engine->gameLoopCallback = GameLoop;
    game->engine->userPointer = (void*)game;

    game->engine->camera.x = 5.0f;
    game->engine->camera.y = 5.0f;
    game->engine->camera.z = 5.0f;
    game->engine->camera.angle = 0.0f;

    game->numKeyStates = (uint16_t)GLFW_KEY_LAST + 1;

    uint32_t terrainSize;
    float* terrainMesh = GameGenerateTerrain(
        game,
        &terrainSize,
        "assets/heightmaps/heightmap_small.bmp"
    );
    GameCreateTerrain(
        game,
        terrainMesh,
        terrainSize,
        "assets/textures/grass.jpg"
    );
    free(terrainMesh);

    GameStart(game);
    EngineRun(engine);
    EngineDestroy(engine);

    free(engine);
}

void GameStart(struct Game* game)
{
    // Load bullet mesh
    struct Mesh* bulletMesh;
    bulletMesh = GameGetMesh(
        game,
        "assets/textures/bullet-texture.png",
        "assets/models/bullet.dae"
    );

    // Initialize pool for bullets
    struct GameObject** bulletObjects;
    bulletObjects = malloc(sizeof(struct GameObject*) * GAME_NUM_BULLETS);
    uint8_t i;
    for (i=0; i<GAME_NUM_BULLETS; i++)
    {
        bulletObjects[i] = EngineCreateGameObject(game->engine, bulletMesh);
    }
    BulletPoolInit(&(game->bulletPool), bulletObjects, GAME_NUM_BULLETS);
    free(bulletObjects);

    // Load robot mesh
    struct Mesh* robotMesh;
    robotMesh = GameGetMesh(
        game,
        "assets/textures/robot-texture.png",
        "assets/models/robot.dae"
    );

    // Initialize pool for robots
    struct GameObject** robotObjects;
    robotObjects = malloc(sizeof(struct GameObject*) * GAME_NUM_ROBOTS);
    for (i=0; i<GAME_NUM_ROBOTS; i++)
    {
        robotObjects[i] = EngineCreateGameObject(game->engine, robotMesh);
    }
    RobotPoolInit(
        &(game->robotPool),
        robotObjects,
        GAME_NUM_ROBOTS,
        &(game->bulletPool)
    );
    free(robotObjects);

    // Create robots
    for (i=0; i<GAME_NUM_ROBOTS; i++)
    {
        RobotPoolCreate(&(game->robotPool), (float)i*10, 5.0f, 0.0f);
    }
    game->player = &(game->robotPool.robots[0]);

    // Record starting time
    game->then = (double)clock();
}

void GameLoop(void* gamePointer)
{
    struct Game* game = (struct Game*) gamePointer;

    double currentTime = (double)clock();
    double elapsed = ((double)(currentTime - game->then) / CLOCKS_PER_SEC) * 1000.0f;

    GameUpdate(game, elapsed);
    GameRender(game);

    game->then = currentTime;
}

void GameUpdate(struct Game* game, double elapsed)
{
    RobotPoolUpdate(&(game->robotPool), elapsed, game->keyStates);
    BulletPoolUpdate(&(game->bulletPool), elapsed);
    GameUpdateCamera(game, elapsed);
}

void GameUpdateCamera(struct Game* game, double elapsed)
{
    if (game->player == NULL)
        return;

    struct Camera* camera = &(game->engine->camera);
    float* playerPos = game->player->gameObject->position;

    camera->x = playerPos[0] + (sinf(game->player->rotation - 0.18f) * 8.0f);
    camera->y = playerPos[1] + (cosf(game->player->rotation - 0.18f) * 8.0f);
    camera->z = 7.5f;

    camera->xTarget = camera->x - sinf(game->player->rotation);
    camera->yTarget = camera->y - cosf(game->player->rotation);
    camera->zTarget = camera->z - 0.2f;
}

void GameRender(struct Game* game)
{

}

void GameKeyPress(void* userPointer, int key, int action)
{
    struct Game* game = (struct Game*)userPointer;

    if (key == GLFW_KEY_UNKNOWN)
        return;

    if (action == GLFW_PRESS)
    {
        game->keyStates[key] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        game->keyStates[key] = 0;
    }
}

struct Mesh* GameGetMesh(struct Game* game, const char* texturePath, const char* modelPath)
{
    EngineCreateDescriptor(
        game->engine,
        &(game->engine->meshes[game->engine->meshCount].descriptor),
        texturePath
    );
    EngineLoadModel(
        game->engine,
        modelPath
    );

    return &(game->engine->meshes[game->engine->meshCount-1]);
}

void GameCreateTerrain(struct Game* game, float* points, uint32_t size, const char* texturePath)
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
        game->engine,
        &(game->engine->meshes[game->engine->meshCount].descriptor),
        texturePath
    );
    EngineCreateMesh(
        game->engine,
        vertices, i,
        indices, indexCount
    );
    EngineCreateGameObject(
        game->engine,
        &(game->engine->meshes[game->engine->meshCount-1])
    );

    free(indices);
    free(vertices);
}

float* GameGenerateTerrain(struct Game* game, uint32_t* size, char* heightmap)
{
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

    float tileSize = 2;
    float xOffset = 0.0f;
    float yOffset = 0.0f;

    uint32_t pixelIndex = 0;
    uint32_t i;
    for (i=0; i<3*texWidth*texWidth; i+=3)
    {
        uint32_t currentPoint = ((i/3)+1) % texWidth;

        terrain[i+0] = xOffset;
        terrain[i+1] = yOffset;
        terrain[i+2] = ((float)pixels[pixelIndex])/4.0f;
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
