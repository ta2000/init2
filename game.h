#ifndef GAME_H_
#define GAME_H_

struct Game
{
    struct Engine* engine;
};

void GameInit(struct Game* game);
void GameStart(struct Game* game);
void GameLoop(struct Game* game);
void GameUpdate(struct Game* game);
void GameRender(struct Game* game);
void GameProcessInput(struct Game* game);
void GameKeyPress(
    void* userPointer, // Passed from engine
    int key,
    int action
);
struct Mesh* GameGetMesh(
    struct Game* game,
    const char* texturePath,
    const char* modelPath
);
void GameCreateTerrain(
    struct Game* game,
    float* points,
    uint32_t numPoints,
    const char* texturePath
);
float* GameGenerateTerrain(
    struct Game* game,
    uint32_t size
);

#endif
