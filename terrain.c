#include <stdio.h>
#include <stdint.h>

#include "engine.h"
#include "terrain.h"

void clampToTerrain(struct Terrain* self, struct GameObject* object)
{
    float* selfPos = self->gameObject->position;
    float* objPos = object->position;

    // Edge clamp
    if (objPos[0] < selfPos[0])
        objPos[0] = selfPos[0];
    if (objPos[0] > selfPos[0] + (self->quadsPerSide * self->tileSize))
        objPos[0] = selfPos[0] + (self->quadsPerSide * self->tileSize);
    if (objPos[1] < selfPos[1])
        objPos[1] = selfPos[1];
    if (objPos[1] > selfPos[1] + (self->quadsPerSide * self->tileSize))
        objPos[1] = selfPos[1] + (self->quadsPerSide * self->tileSize);

    // Height clamp
    uint32_t index = (
        ( self->quadsPerSide+1 ) *
        ( (int)(objPos[0] / self->tileSize) ) +
        ( (int)(objPos[1] / self->tileSize) )
    );
    uint8_t adjustDelay = 30;
    if (objPos[2] < self->points[index*3+2])
        objPos[2] += (self->points[index*3+2] - objPos[2]) / adjustDelay;
    if (objPos[2] > self->points[index*3+2])
        objPos[2] -= (objPos[2] - self->points[index*3+2]) / adjustDelay;
}
