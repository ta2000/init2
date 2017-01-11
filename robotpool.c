#include <stdio.h>
#include <stdint.h>

#include "robot.h"
#include "robotpool.h"

void RobotPoolInit(struct RobotPool* self)
{
    self->robotCount = ROBOT_COUNT;

    self->head = &(self->robots[0]);
    self->tail = &(self->robots[ROBOT_COUNT - 1]);

    uint8_t i;
    for (i=0; i<ROBOT_COUNT - 1; i++)
    {
        self->robots[i].next = &(self->robots[i+1]);
        self->robots[i].inUse = 0;
    }
}
void RobotPoolUpdate(struct RobotPool* self, uint16_t* keyStates)
{
    uint8_t i;
    for (i=0; i<ROBOT_COUNT; i++)
    {
        // Update robots and return them to the list
        // if they die that frame (return true)
        if (RobotUpdate(&(self->robots[0]), keyStates))
        {
            RobotPoolReturn(self, &(self->robots[0]));
        }
    }
}
// TODO: Final robot in list cannot be taken
struct Robot* RobotPoolGet(struct RobotPool* self)
{
    // All robots in use
    if (self->head->next == NULL)
    {
        return NULL;
    }

    self->head->inUse = 1;
    struct Robot* nextAvailable = self->head;
    self->head = self->head->next;
    return nextAvailable;
}
void RobotPoolReturn(struct RobotPool* self, struct Robot* robot)
{
    robot->inUse = 0;
    self->tail->next = robot;
    self->tail = robot;
    robot->next = NULL;
}
