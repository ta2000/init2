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

    uint32_t terrainSize = 4;
    float* terrainMesh = GameCreateTerrain(game, terrainSize);
    GameCreateMesh(game, terrainMesh, 4 * terrainSize * terrainSize);
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

// Only works with quads for now (%6)
void GameCreateMesh(struct Game* game, float* points, uint32_t numPoints)
{
    struct Vertex* vertices;
    vertices = calloc(numPoints, sizeof(*vertices));

    uint32_t* indices;
    indices = calloc(2 * numPoints, sizeof(*indices));
    uint32_t indexCount = 0;

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

        indices[indexCount] = i;
        indexCount++;

        // Duplicate third vertex
        if (indexCount % 3 == 0 && indexCount % 6 != 0)
        {
            indices[indexCount] = i;
            indexCount++;
        }
        // Close loop
        else if ((indexCount+1) % 6 == 0)
        {
            indices[indexCount] = i-3;
            indexCount++;
        }

        /*printf("{ %f, %f, %f }\n",
            vertices[i].position[0],
            vertices[i].position[1],
            vertices[i].position[2]
        );
        if ((i+1)%4 == 0)
            printf("\n");*/
    }

    /*uint32_t tmp;
    for (tmp=0; tmp<indexCount; tmp++)
    {
        printf("%d, ", indices[tmp]);
    } printf("\n");*/

    EngineCreateMesh(game->engine, vertices, i, indices, indexCount);
    EngineCreateGameObject(game->engine, &(game->engine->meshes[0]));

    free(indices);
    free(vertices);
}

float* GameCreateTerrain(struct Game* game, uint32_t size)
{
    float* terrain;
    // Rect = 4 points
    // Point = 3 floats
    terrain = calloc((3 * 4) * (size * size), sizeof(*terrain));

    float square[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    int offset = 1;
    int tileSize = 3;

    uint32_t i, j;
    // (size * size) tiles
    for (i=0; i<12*size*size; i+=12)
    {
        // 4 points
        for (j=0; j<4; j++)
        {
            // 3 coordinates
            terrain[i + (j*3) + 0] =
                tileSize * square[j * 3 + 0] + tileSize * (offset*(i/12)); // X

            terrain[i + (j*3) + 1] =
                tileSize * square[j * 3 + 1] + tileSize * (offset*(i/12)); // Y

            terrain[i + (j*3) + 2] =
                square[j * 3 + 2]; // Z
        }
    }

    return terrain;
}
