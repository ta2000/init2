#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "engine.h"
#include "bullet.h"
#include "bulletpool.h"

#include "robot.h"

void RobotInit(struct Robot* self, struct GameObject* gameObject, struct BulletPool* bulletPool)
{
    self->gameObject = gameObject;
    self->gameObject->rotation[0] = (float)degreesToRadians(270.0f);
    self->gameObject->visible = 0;

    self->bulletPool = bulletPool;

    self->actionKeyMap[MOVE_FORWARD] = GLFW_KEY_W;
    self->actionKeyMap[MOVE_BACKWARD] = GLFW_KEY_S;
    self->actionKeyMap[MOVE_LEFT] = GLFW_KEY_Q;
    self->actionKeyMap[MOVE_RIGHT] = GLFW_KEY_E;
    self->actionKeyMap[TURN_LEFT] = GLFW_KEY_A;
    self->actionKeyMap[TURN_RIGHT] = GLFW_KEY_D;
    self->actionKeyMap[SHOOT] = GLFW_KEY_SPACE;

    self->hp = 100;
    self->fireRate = 100.0f;
    self->fireTimer = self->fireRate;
    self->shotSpeed = 0.7f;
    self->acceleration = 0.001f;
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

    // Timer for shooting
    if (self->fireTimer > 0)
        self->fireTimer -= elapsed;

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
        if (keyStates[self->actionKeyMap[TURN_LEFT]])
        {
            self->rotation -= self->rotationSpeed;
            if (self->rotation < 0)
                self->rotation = 2 * M_PI;
        }
        if (keyStates[self->actionKeyMap[TURN_RIGHT]])
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
    if (self->velocity > 0.5f)
        self->velocity = 0.5f;
    if (self->velocity < -0.5f)
        self->velocity = -0.5f;

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
    if (self->fireTimer > 0)
        return;

    BulletPoolCreate(
        self->bulletPool,
        self,
        self->gameObject->position[0],
        self->gameObject->position[1],
        self->gameObject->position[2] + 4.0f,
        self->rotation,
        self->velocity + self->shotSpeed
    );

    self->fireTimer = self->fireRate;
}
