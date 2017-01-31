#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "engine.h"
#include "terrain.h"
#include "bullet.h"
#include "bulletpool.h"
#include "robot.h"
#include "robotpool.h"

void RobotInit(struct Robot* self, struct RobotPool* pool, struct GameObject* gameObject, struct BulletPool* bulletPool)
{
    self->gameObject = gameObject;
    self->gameObject->rotation[0] = (float)degreesToRadians(270.0f);
    self->gameObject->visible = 0;

    self->pool = pool;
    self->bulletPool = bulletPool;

    self->actionKeyMap[MOVE_FORWARD] = GLFW_KEY_W;
    self->actionKeyMap[MOVE_BACKWARD] = GLFW_KEY_S;
    self->actionKeyMap[TURN_LEFT] = GLFW_KEY_A;
    self->actionKeyMap[TURN_RIGHT] = GLFW_KEY_D;
    self->actionKeyMap[SHOOT] = GLFW_KEY_SPACE;

    self->inUse = 0;
    self->next = NULL;
    self->playerControlled = 0;
}

void RobotCreate(struct Robot* self, float x, float y, float z, uint8_t hp)
{
    self->fireRate = 65.0f;
    self->fireTimer = self->fireRate;
    self->shotSpeed = 0.3f;
    self->shotSpread = 8;
    self->acceleration = 0.0015f;
    self->velocity = 0.0f;
    self->rotation = 0.0f;
    self->rotationSpeed = 0.009f;
    self->friction = 0.989f;

    self->gameObject->position[0] = x;
    self->gameObject->position[1] = y;
    self->gameObject->position[2] = z;
    self->hp = hp;
    self->playerControlled = 0;
    self->gameObject->visible = 1;

    self->target = NULL;
    self->moveX = x;
    self->moveY = y;
    self->moveTimer = 400.0f;
}

_Bool RobotUpdate(struct Robot* self, struct Terrain* terrain, double elapsed, uint16_t* keyStates)
{
    // Not in use
    if (!self->inUse) return 0;

    // Check bullet collisions
    uint8_t i;
    for (i=0; i<self->bulletPool->bulletCount; i++)
    {
        if (!self->bulletPool->bullets[i].inUse)
            continue;

        if (RobotDistanceToPoint(
                self,
                self->bulletPool->bullets[i].gameObject->position[0],
                self->bulletPool->bullets[i].gameObject->position[1]
            ) > 1.0f)
        {
            continue;
        }

        if (self->bulletPool->bullets[i].parent != self)
        {
            BulletPoolReturn(
                self->bulletPool,
                &(self->bulletPool->bullets[i])
            );
            self->hp -= 10;
        }
    }
    if (self->hp <= 0)
        return 0;

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
                self->rotation += 2 * M_PI;
        }
        if (keyStates[self->actionKeyMap[TURN_RIGHT]])
        {
            self->rotation += self->rotationSpeed;
            if (self->rotation > 2 * M_PI)
                self->rotation -= 2 * M_PI;
        }
        if (keyStates[self->actionKeyMap[SHOOT]])
        {
            RobotShoot(self);
        }
    }
    // AI controls
    else
    {
        // Target is dead
        if (self->target != NULL &&
            !self->target->inUse)
                self->target = NULL;

        // Wander to points on terrain
        if (self->target == NULL)
        {
            // Find new target
            uint8_t i;
            for (i=0; i<self->pool->robotCount; i++)
            {
                if (&(self->pool->robots[i]) == self)
                    continue;

                if (RobotDistanceToPoint(
                        self,
                        self->pool->robots[i].gameObject->position[0],
                        self->pool->robots[i].gameObject->position[1]
                    ) < 30.0f)
                {
                    self->target = &(self->pool->robots[i]);
                    break;
                }
            }

            // Pick new points once arrived
            float dist = RobotDistanceToPoint(self, self->moveX, self->moveY);
            if (dist < 1.0f && self->moveTimer <= 0.0f)
            {
                self->moveX =
                    (float)(rand() % terrain->quadsPerSide) * terrain->tileSize;
                self->moveY =
                    (float)(rand() % terrain->quadsPerSide) * terrain->tileSize;

                self->moveTimer = (float)(rand()%1000);
            }
            else if (self->moveTimer > 0.0f)
            {
                self->moveTimer -= elapsed;
            }
            // Slow down as destination approaches
            else if (dist > 1.0f)
            {
                self->velocity -= self->acceleration;
            }

            // Wait until just before moving to turn
            if (self->moveTimer < 80.0f)
            {
                self->targetRotation = atan2(
                    self->gameObject->position[0] - self->moveX,
                    self->gameObject->position[1] - self->moveY
                );
            }

        }
        else
        {
            self->targetRotation = atan2(
                self->gameObject->position[0] -
                        self->target->gameObject->position[0],
                self->gameObject->position[1] -
                        self->target->gameObject->position[1]
            );

            if (self->targetRotation > self->rotation - 0.1f &&
                self->targetRotation < self->rotation + 0.1f)
            {
                RobotShoot(self);
            }

            /*if (RobotDistanceToPoint(
                    self,
                    self->target->gameObject->position[0],
                    self->target->gameObject->position[1]
                ) > 10.0f)
            {*/
                self->velocity -= self->acceleration;
            /*}
            else
            {
                self->velocity += self->acceleration;
                self->target = NULL;
            }*/
        }

        if (self->targetRotation < 0)
            self->targetRotation += 2*M_PI;
        if (self->targetRotation > 2*M_PI)
            self->targetRotation -= 2*M_PI;

        if (self->rotation < 0)
            self->rotation += 2*M_PI;
        if (self->rotation > 2*M_PI)
            self->rotation -= 2*M_PI;

        // Rotate to match rotation target
        float delta = self->targetRotation - self->rotation;
        if (delta > 0.0f)
            delta = fmodf(delta, 2*M_PI);
        else
            delta = fmodf(-1 * delta + M_PI, 2*M_PI);

        if (delta < M_PI)
            self->rotation += self->rotationSpeed;
        else
            self->rotation -= self->rotationSpeed;
    }

    // Handle collision
    clampToTerrain(terrain, self->gameObject);

    // Transform object
    self->gameObject->rotation[2] = self->rotation;
    self->velocity *= self->friction;
    //self->velocity *= self->friction;
    self->gameObject->position[0] += sinf(self->rotation) * self->velocity * elapsed;
    self->gameObject->position[1] += cosf(self->rotation) * self->velocity * elapsed;

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
        self->gameObject->position[2] + 4.0f, // Z starts at feet
        self->rotation + ((float)rand()/(float)(RAND_MAX))/self->shotSpread - 1.0f/(float)self->shotSpread,
        self->shotSpeed
    );

    self->fireTimer = self->fireRate;
}

float RobotDistanceToPoint(struct Robot* self, float x, float y)
{
    float dx = self->gameObject->position[0] - x;
    float dy = self->gameObject->position[1] - y;
    return (float)(sqrt((dx * dx) + (dy * dy)));
}
