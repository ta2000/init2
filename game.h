#ifndef GAME_H_
#define GAME_H_

struct Game
{
    struct Engine* engine;
};

void GameInit(struct Game* game);
void GameLoop(struct Game* game);
void GameUpdate(struct Game* game);
void GameRender(struct Game* game);
void GameProcessInput(struct Game* game);
void GameKeyPress(
    void* userPointer, // Passed from engine
    int key,
    int action
);
void GameCreateMesh(
    struct Game* game,
    float* vertices,
    uint32_t vertexCount
);
float* GameCreateTerrain(
    struct Game* game,
    uint32_t size
);

#endif
