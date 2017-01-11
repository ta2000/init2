#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "engine.h"

#include "robot.h"

void RobotInit(struct Robot* self, struct GameObject* gameObject, float* position)
{
    self->gameObject = gameObject;
    memcpy(
        &(self->gameObject->position),
        position,
        sizeof(3 * sizeof(*position))
    );
    self->gameObject->rotation[0] = (float)degreesToRadians(90.0f);

    self->actionKeyMap[MOVE_FORWARD] = GLFW_KEY_W;
    self->actionKeyMap[MOVE_BACKWARD] = GLFW_KEY_S;
    self->actionKeyMap[MOVE_LEFT] = GLFW_KEY_A;
    self->actionKeyMap[MOVE_RIGHT] = GLFW_KEY_D;

    self->hp = 100;
    self->fireRate = 10.0f;
    self->acceleration = 0.0008f;
    self->velocity = 0.0f;
    self->rotation = 270.0f;
    self->friction = 0.2f;
    self->mass = 10.0f;

    self->playerControlled = 1;
}

_Bool RobotUpdate(struct Robot* self, uint16_t* keyStates)
{
    // Not in use
    if (self->hp == 0) return 0;

    // Player control handling
    if (self->playerControlled)
    {
        if (keyStates[self->actionKeyMap[MOVE_FORWARD]])
        {
            self->velocity += self->acceleration;
        }
        if (keyStates[self->actionKeyMap[MOVE_BACKWARD]])
        {
            self->velocity -= self->acceleration;
        }
    }

    // AI control handling


    // Handle collision


    // Transform object
    self->gameObject->rotation[1] = (float)degreesToRadians(self->rotation);
    self->velocity *= self->friction;
    vec3* selfPosition = &(self->gameObject->position);
    *selfPosition[0] += (float)cos(self->rotation) * self->velocity;
    *selfPosition[1] += (float)sin(self->rotation) * self->velocity;

    return self->hp == 0;
}
