#ifndef ROBOT_H_
#define ROBOT_H_

enum RobotAction
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    TURN_LEFT,
    TURN_RIGHT,
    SHOOT,
    ACTIONS_LAST
};

//union

struct Robot
{
    struct GameObject* gameObject;
    struct RobotPool* pool;
    struct BulletPool* bulletPool;

    // Controls
    _Bool playerControlled;
    enum RobotAction actions[ACTIONS_LAST];
    uint16_t actionKeyMap[ACTIONS_LAST];

    // Base traits
    uint16_t hp;
    double fireRate;
    double fireTimer;
    float shotSpeed;
    int shotSpread;
    float acceleration;
    double velocity;
    float rotation;
    float rotationSpeed;
    float friction;

    // AI only
    float targetRotation;
    struct Robot* target;
    float moveX, moveY;
    float moveTimer;

    // List
    struct Robot* next;
    _Bool inUse;
};
void RobotInit(
    struct Robot* self,
    struct RobotPool* pool,
    struct GameObject* gameObject,
    struct BulletPool* bulletPool
);
void RobotCreate(
    struct Robot* self,
    float x,
    float y,
    float z,
    float hp
);
_Bool RobotUpdate(
    struct Robot* self,
    struct Terrain* terrain,
    double elapsed,
    uint16_t* keyStates
);
void RobotShoot(struct Robot* self);
float RobotDistanceToPoint(
    struct Robot* self,
    float x, float y
);

#endif
