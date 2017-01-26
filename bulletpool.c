#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "engine.h"
#include "bullet.h"
#include "bulletpool.h"

void BulletPoolInit(struct BulletPool* self, struct GameObject** gameObjects, uint8_t gameObjectCount)
{
    assert(gameObjectCount < MAX_BULLETS);
    self->bulletCount = gameObjectCount;

    self->head = &(self->bullets[0]);
    self->tail = &(self->bullets[self->bulletCount - 1]);

    uint8_t i;
    for (i=0; i<self->bulletCount; i++)
    {
        BulletInit(
            &(self->bullets[i]),
            gameObjects[i]
        );

        if (i < self->bulletCount - 1)
        {
            self->bullets[i].next = &(self->bullets[i+1]);
        }
    }
}
void BulletPoolUpdate(struct BulletPool* self, double elapsed)
{
    uint8_t i;
    for (i=0; i<self->bulletCount; i++)
    {
        if (!BulletUpdate(&(self->bullets[i]), elapsed))
        {
            BulletPoolReturn(self, &(self->bullets[i]));
        }
    }
}
void BulletPoolCreate(struct BulletPool* self, struct Robot* parent, float x, float y, float z, float rotation, float velocity)
{
    if (self->head == NULL)
    {
        return;
    }

    self->head->inUse = 1;

    self->head->parent = parent;
    self->head->gameObject->position[0] = x;
    self->head->gameObject->position[1] = y;
    self->head->gameObject->position[2] = z;
    self->head->rotation = rotation;
    self->head->velocity = velocity;
    self->head->gameObject->visible = 1;

    self->head = self->head->next;
}
void BulletPoolReturn(struct BulletPool* self, struct Bullet* bullet)
{
    if (self->head == NULL)
    {
        self->head = bullet;
    }

    bullet->inUse = 0;
    bullet->gameObject->visible = 0;
    self->tail->next = bullet;
    self->tail = bullet;
    bullet->next = NULL;
}
