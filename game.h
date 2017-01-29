#ifndef GAME_H_
#define GAME_H_

#define GAME_NUM_ROBOTS 2
#define GAME_NUM_BULLETS 100

struct Game
{
    struct Engine* engine;
    struct Terrain terrain;
    uint16_t keyStates[GLFW_KEY_LAST + 1];
    uint16_t numKeyStates;
    double then;
    struct BulletPool bulletPool;
    struct RobotPool robotPool;
    struct Robot* player;
};

void GameInit(struct Game* game);
void GameStart(struct Game* game);
void GameLoop(void* gamePointer);
void GameUpdate(struct Game* game, double elapsed);
void GameUpdateCamera(struct Game* game, double elapsed);
void GameRender(struct Game* game);
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
struct GameObject* GameCreateTerrain(
    struct Game* game,
    float* points,
    uint32_t numPoints,
    const char* texturePath
);
float* GameGenerateTerrain(
    struct Game* game,
    float tileSize,
    uint32_t* size,
    char* heightmap
);

#endif
