#ifndef ROBOT_H_
#define ROBOT_H_

enum RobotAction
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    ACTIONS_LAST
};

struct Robot
{
    struct GameObject* gameObject;

    _Bool playerControlled;
    enum RobotAction actions[ACTIONS_LAST - 1];
    uint16_t actionKeyMap[ACTIONS_LAST - 1];

    uint16_t hp;
    double fireRate;
    float acceleration;
    float velocity;
    float rotation;
    float friction;
    float mass;

    struct Robot* next;
    _Bool inUse;
};
void RobotInit(
    struct Robot* self,
    struct GameObject* gameObject,
    float position[3]
);
_Bool RobotUpdate(
    struct Robot* self,
    uint16_t* keyStates
);

#endif
