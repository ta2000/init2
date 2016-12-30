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

    uint32_t terrainSize = 3;
    float* terrainMesh = GameCreateTerrain(game, terrainSize);
    GameCreateMesh(game, terrainMesh, terrainSize * terrainSize);
    free(terrainMesh);

    EngineLoadModel(engine, "assets/models/robot.dae");
    EngineCreateGameObject(engine, &(engine->meshes[1]));

    EngineRun(engine);
    EngineDestroy(engine);

    free(engine);
}

void GameLoop(struct Game* game)
{
    GameProcessInput(game);
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

void GameKeyPress(struct Game* game, int key, int action)
{
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

void GameCreateMesh(struct Game* game, float* points, uint32_t numPoints)
{
    printf("%d\n", numPoints);

    struct Vertex* vertices;
    vertices = calloc(numPoints, sizeof(*vertices));

    uint32_t* indices;
    indices = calloc(numPoints, sizeof(*indices));

    uint32_t i;
    for (i=0; i<numPoints; i++)
    {
        vertices[i].position[0] = points[i * 3 + 0];
        vertices[i].position[1] = points[i * 3 + 1];
        vertices[i].position[2] = points[i * 3 + 2];
        vertices[i].color[0] = 0.0f;
        vertices[i].color[1] = 0.0f;
        vertices[i].color[2] = 0.0f;
        vertices[i].texCoord[0] = 0.0f;
        vertices[i].texCoord[1] = 0.0f;

        indices[i] = i;

        printf("{ %f, %f, %f }\n",
            vertices[i].position[0],
            vertices[i].position[1],
            vertices[i].position[2]
        );
    }

    EngineCreateMesh(game->engine, vertices, i, indices, i);
    EngineCreateGameObject(game->engine, &(game->engine->meshes[0]));

    free(indices);
    free(vertices);
}

float* GameCreateTerrain(struct Game* game, uint32_t size)
{
    float* terrain;
    terrain = calloc((3*size) * (3*size), sizeof(vec3));

    uint32_t i;
    for (i=0; i< (3*size) * (3*size); i++)
    {
        terrain[i] = 8 * (float)rand()/(float)RAND_MAX;
    }

    return terrain;
}
