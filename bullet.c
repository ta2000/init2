#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "engine.h"

#include "bullet.h"

void BulletInit(struct Bullet* self, struct GameObject* gameObject)
{
    self->gameObject = gameObject;
    self->gameObject->visible = 0;
    self->velocity = 0.0f;
    self->rotation = 0.0f;

    self->inUse = 0;
    self->next = NULL;
}

_Bool BulletUpdate(struct Bullet* self, double elapsed)
{
    // Not in use
    if (!self->inUse) return 0;

    self->gameObject->position[1] -= cosf(self->rotation) * self->velocity * elapsed;
    self->gameObject->position[0] -= sinf(self->rotation) * self->velocity * elapsed;

    self->inUse = (
        self->gameObject->position[0] > 0.0f &&
        self->gameObject->position[0] < 512.0f &&
        self->gameObject->position[1] > 0.0f &&
        self->gameObject->position[1] < 512.0f
    );
    return self->inUse;
}
