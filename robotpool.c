#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "engine.h"
#include "robot.h"
#include "robotpool.h"

void RobotPoolInit(struct RobotPool* self, struct GameObject** gameObjects, uint8_t gameObjectCount)
{
    assert(gameObjectCount < MAX_ROBOTS);
    self->robotCount = gameObjectCount;

    self->head = &(self->robots[0]);
    self->tail = &(self->robots[self->robotCount - 1]);

    uint8_t i;
    for (i=0; i<self->robotCount; i++)
    {
        RobotInit(
            &(self->robots[i]),
            gameObjects[i]
        );

        if (i < self->robotCount - 1)
        {
            self->robots[i].next = &(self->robots[i+1]);
        }
    }
}
void RobotPoolUpdate(struct RobotPool* self, double elapsed, uint16_t* keyStates)
{
    uint8_t i;
    for (i=0; i<MAX_ROBOTS; i++)
    {
        if (!RobotUpdate(&(self->robots[i]), elapsed, keyStates))
        {
            RobotPoolReturn(self, &(self->robots[i]));
        }
    }
}
void RobotPoolCreate(struct RobotPool* self, float x, float y, float z)
{
    if (self->head == NULL)
    {
        return;
    }

    self->head->inUse = 1;

    self->head->gameObject->position[0] = x;
    self->head->gameObject->position[1] = y;
    self->head->gameObject->position[2] = z;
    self->head->hp = 100;
    self->head->playerControlled = 1;
    self->head->gameObject->visible = 1;

    self->head = self->head->next;
}
void RobotPoolReturn(struct RobotPool* self, struct Robot* robot)
{
    if (self->head == NULL)
    {
        self->head = robot;
    }

    robot->inUse = 0;
    self->tail->next = robot;
    self->tail = robot;
    robot->next = NULL;
}
