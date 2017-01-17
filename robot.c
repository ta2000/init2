#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "engine.h"

#include "robot.h"

void RobotInit(struct Robot* self, struct GameObject* gameObject)
{
    self->gameObject = gameObject;
    self->gameObject->rotation[0] = (float)degreesToRadians(270.0f);
    self->gameObject->visible = 0;

    self->actionKeyMap[MOVE_FORWARD] = GLFW_KEY_W;
    self->actionKeyMap[MOVE_BACKWARD] = GLFW_KEY_S;
    self->actionKeyMap[MOVE_LEFT] = GLFW_KEY_A;
    self->actionKeyMap[MOVE_RIGHT] = GLFW_KEY_D;
    self->actionKeyMap[SHOOT] = GLFW_KEY_SPACE;

    self->hp = 100;
    self->fireRate = 10.0f;
    self->shotSpeed = 0.00002f;
    self->acceleration = 0.0004f;
    self->velocity = 0.0f;
    self->rotation = 0.0f;
    self->rotationSpeed = 0.006f;
    self->friction = 0.989f;

    self->inUse = 0;
    self->next = NULL;
    self->playerControlled = 0;
}

_Bool RobotUpdate(struct Robot* self, double elapsed, uint16_t* keyStates)
{
    // Not in use
    if (!self->inUse) return 0;

    // Player control handling
    if (self->playerControlled)
    {
        if (keyStates[self->actionKeyMap[MOVE_FORWARD]])
        {
            self->velocity -= self->acceleration;
        }
        if (keyStates[self->actionKeyMap[MOVE_BACKWARD]])
        {
            self->velocity += self->acceleration;
        }
        if (keyStates[self->actionKeyMap[MOVE_LEFT]])
        {
            self->rotation -= self->rotationSpeed;
            if (self->rotation < 0)
                self->rotation = 2 * M_PI;
        }
        if (keyStates[self->actionKeyMap[MOVE_RIGHT]])
        {
            self->rotation += self->rotationSpeed;
            if (self->rotation > 2 * M_PI)
                self->rotation = 0;
        }
        if (keyStates[self->actionKeyMap[SHOOT]])
        {
            RobotShoot(self);
        }
    }

    // Max velocity
    if (self->velocity > 0.3f)
        self->velocity = 0.3f;
    if (self->velocity < -0.3f)
        self->velocity = -0.3f;

    // Handle collision


    // Transform object
    self->gameObject->rotation[2] = self->rotation;
    self->velocity *= self->friction;
    self->gameObject->position[1] += cosf(self->rotation) * self->velocity * elapsed;
    self->gameObject->position[0] += sinf(self->rotation) * self->velocity * elapsed;

    self->inUse = (self->hp > 0);
    return self->inUse;
}

void RobotShoot(struct Robot* self)
{
    /*BulletPoolCreate(
        self->bulletPool,
        self,
        self->rotation,
        self->velocity + self->shotSpeed
    );*/
}
