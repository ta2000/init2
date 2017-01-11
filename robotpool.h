#ifndef ROBOTPOOL_H_
#define ROBOTPOOL_H_

#define ROBOT_COUNT 4

struct RobotPool
{
    struct Robot robots[ROBOT_COUNT];
    uint8_t robotCount;
    struct Robot* head;
    struct Robot* tail;
};
void RobotPoolInit(struct RobotPool* self);
void RobotPoolUpdate(
    struct RobotPool* self,
    uint16_t* keys
);
struct Robot* RobotPoolGet(struct RobotPool* self);
void RobotPoolReturn(struct RobotPool* self, struct Robot* robot);

#endif
