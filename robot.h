#ifndef ROBOT_H_
#define ROBOT_H_

enum RobotAction
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    ACTIONS_LAST
};

struct Robot
{
    struct GameObject* gameObject;

    _Bool playerControlled;
    enum RobotAction actions[ACTIONS_LAST];
    uint16_t actionKeyMap[ACTIONS_LAST];

    uint16_t hp;
    double fireRate;
    float shotSpeed;
    float acceleration;
    double velocity;
    float rotation;
    float rotationSpeed;
    float friction;

    struct BulletPool* bulletPool;

    struct Robot* next;
    _Bool inUse;
};
void RobotInit(
    struct Robot* self,
    struct GameObject* gameObject
);
_Bool RobotUpdate(
    struct Robot* self,
    double elapsed,
    uint16_t* keyStates
);
void RobotShoot(struct Robot* self);

#endif
