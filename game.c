#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "engine.h"
#include "game.h"

_Bool keysPressed[4] = {0};

void GameInit(struct Game* game)
{
    struct Engine* engine = calloc(1, sizeof(*engine));
    EngineInit(engine);

    game->engine = engine;
    game->engine->keyCallback = GameKeyPress;
    game->engine->gameLoopCallback = GameLoop;
    game->engine->userPointer = game;

    game->engine->camera.x = 5.0f;
    game->engine->camera.y = 5.0f;
    game->engine->camera.z = 5.0f;
    game->engine->camera.angle = 0.0f;

    uint32_t terrainSize = 16;
    float* terrainMesh = GameGenerateTerrain(game, terrainSize);
    GameCreateTerrain(
        game,
        terrainMesh,
        terrainSize,
        "assets/textures/ground.jpg"
    );
    free(terrainMesh);

    struct Mesh* tmp = GameGetMesh(
        game,
        "assets/textures/robot-texture.png",
        "assets/models/robot.dae"
    );
    EngineCreateGameObject(game->engine, tmp);

    EngineRun(engine);
    EngineDestroy(engine);

    free(engine);
}

void GameStart(struct Game* game)
{
}

void GameLoop(struct Game* game)
{
    GameProcessInput(game);
    game->engine->gameObjects[0].position[0] = 10.0f;
    game->engine->gameObjects[1].position[0] = 0.0f;
}

void GameUpdate(struct Game* game)
{

}

void GameRender(struct Game* game)
{

}

void GameProcessInput(struct Game* game)
{
    struct Camera* camera = &(game->engine->camera);

    if (camera->angle > (2*M_PI))
        camera->angle -= (2*M_PI);
    if (camera->angle < 0)
        camera->angle += (2*M_PI);

    if (keysPressed[0]) // A
    {
        camera->angle += 0.005f;
    }
    else if (keysPressed[1]) // D
    {
        camera->angle -= 0.005f;
    }
    if (keysPressed[2]) // W
    {
        camera->x += (float)cos(camera->angle) * 0.1;
        camera->y += (float)sin(camera->angle) * 0.1;
    }
    else if (keysPressed[3]) // S
    {
        camera->x -= (float)cos(camera->angle) * 0.1;
        camera->y -= (float)sin(camera->angle) * 0.1;
    }

    camera->xTarget = camera->x + (float)cos(camera->angle);
    camera->yTarget = camera->y + (float)sin(camera->angle);
    camera->zTarget = camera->z;
}

void GameKeyPress(void* userPointer, int key, int action)
{
    struct Game* game = (struct Game*)userPointer;

    if (key == GLFW_KEY_A)
    {
        keysPressed[0] = action;
    }
    else if (key == GLFW_KEY_D)
    {
        keysPressed[1] = action;
    }
    else if (key == GLFW_KEY_W)
    {
        keysPressed[2] = action;
    }
    else if (key == GLFW_KEY_S)
    {
        keysPressed[3] = action;
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

    uint32_t i;
    for (i=0; i<numPoints; i++)
    {
        /*printf("{ %f, %f, %f }\n",
            points[i*3+0],
            points[i*3+1],
            points[i*3+2]
        );*/

        // Generate vertices
        vertices[i].position[0] = points[i * 3 + 0];
        vertices[i].position[1] = points[i * 3 + 1];
        vertices[i].position[2] = points[i * 3 + 2];
        vertices[i].color[0] = 0.0f;
        vertices[i].color[1] = 0.0f;
        vertices[i].color[2] = 0.0f;
        vertices[i].texCoord[0] = 1.0f;
        vertices[i].texCoord[1] = 1.0f;

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

float* GameGenerateTerrain(struct Game* game, uint32_t size)
{
    float* terrain;
    terrain = calloc(3*(size+1)*(size+1), sizeof(*terrain));

    float tileSize = 4;
    float xOffset = 0.0f;
    float yOffset = 0.0f;

    uint32_t i, j;
    for (i=0; i<3*(size+1)*(size+1); i+=3)
    {
        uint32_t currentPoint = ((i/3)+1) % (size+1);

        terrain[i+0] = xOffset;
        terrain[i+1] = yOffset;
        terrain[i+2] = 2 * (float)rand()/(float)RAND_MAX;//0.0f;

        yOffset += tileSize;
        if (currentPoint == 0)
        {
            xOffset += tileSize;
            yOffset = 0;
        }
    }

    return terrain;
}
