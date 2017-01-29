#ifndef TERRAIN_H_
#define TERRAIN_H_

struct Terrain
{
    struct GameObject* gameObject;
    float* points;
    uint32_t quadsPerSide;
    float tileSize;
};

void clampToTerrain(struct Terrain* self, struct GameObject* object);

#endif

