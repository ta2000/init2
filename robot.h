#ifndef ROBOT_H_
#define ROBOT_H_

enum RobotAction
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    TURN_LEFT,
    TURN_RIGHT,
    SHOOT,
    ACTIONS_LAST
};

struct Robot
{
    struct GameObject* gameObject;
    struct BulletPool* bulletPool;

    _Bool playerControlled;
    enum RobotAction actions[ACTIONS_LAST];
    uint16_t actionKeyMap[ACTIONS_LAST];

    uint16_t hp;
    double fireRate;
    double fireTimer;
    float shotSpeed;
    float acceleration;
    double velocity;
    float rotation;
    float rotationSpeed;
    float friction;

    struct Robot* next;
    _Bool inUse;
};
void RobotInit(
    struct Robot* self,
    struct GameObject* gameObject,
    struct BulletPool* bulletPool
);
_Bool RobotUpdate(
    struct Robot* self,
    struct Terrain* terrain,
    double elapsed,
    uint16_t* keyStates
);
void RobotShoot(struct Robot* self);

#endif
