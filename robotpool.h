#ifndef ROBOTPOOL_H_
#define ROBOTPOOL_H_

#define MAX_ROBOTS 10

struct RobotPool
{
    struct Robot robots[MAX_ROBOTS];
    uint8_t robotCount;
    struct Robot* head;
    struct Robot* tail;
};
void RobotPoolInit(
    struct RobotPool* self,
    struct GameObject** gameObjects,
    uint8_t gameObjectCount
);
void RobotPoolUpdate(
    struct RobotPool* self,
    double elapsed,
    uint16_t* keys
);
void RobotPoolCreate(
    struct RobotPool* self,
    float x, float y, float z
);
void RobotPoolReturn(
    struct RobotPool* self,
    struct Robot* robot
);

#endif
