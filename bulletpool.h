#ifndef BULLETPOOL_H_
#define BULLETPOOL_H_

#define MAX_BULLETS 200

struct BulletPool
{
    struct Bullet bullets[MAX_BULLETS];
    uint8_t bulletCount;
    struct Bullet* head;
    struct Bullet* tail;
};
void BulletPoolInit(
    struct BulletPool* self,
    struct GameObject** gameObjects,
    uint8_t gameObjectCount
);
void BulletPoolUpdate(
    struct BulletPool* self,
    double elapsed
);
void BulletPoolCreate(
    struct BulletPool* self,
    struct Robot* parent,
    float x, float y, float z,
    float rotation,
    float velocity
);
void BulletPoolReturn(
    struct BulletPool* self,
    struct Bullet* bullet
);

#endif
