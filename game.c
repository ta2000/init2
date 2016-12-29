#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "game.h"

void GameInit(struct Game* game)
{
    struct Engine* engine = calloc(1, sizeof(*engine));
    EngineInit(engine);

    game->engine = engine;

    EngineLoadModel(engine, "assets/models/robot.dae");
    EngineCreateGameObject(engine, &(engine->meshes[0]));
    EngineRun(engine);
    EngineDestroy(engine);

    free(engine);
}

void GameLoop(struct Game* game)
{

}

void GameUpdate(struct Game* game)
{

}

void GameRender(struct Game* game)
{

}

void GameCreateMesh(struct Game* game, float* vertices, uint32_t vertexCount)
{
    //EngineCreateMesh(game->engine,
}

float* GameCreateTerrain(struct Game* game, int size)
{
    float* terrain;
    terrain = calloc(size * size, sizeof(vec3));

    //uint32_t i, j;

    return terrain;
}
