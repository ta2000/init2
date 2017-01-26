#ifndef BULLET_H_
#define BULLET_H_

struct Robot; // Forward declaration

struct Bullet
{
    struct GameObject* gameObject;
    struct Robot* parent;
    double velocity;
    float rotation;

    struct BulletPool* bulletPool;

    struct Bullet* next;
    _Bool inUse;
};
void BulletInit(
    struct Bullet* self,
    struct GameObject* gameObject
);
_Bool BulletUpdate(
    struct Bullet* self,
    double elapsed
);

#endif
