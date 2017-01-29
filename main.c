#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "engine.h"
#include "terrain.h"
#include "bullet.h"
#include "bulletpool.h"
#include "robot.h"
#include "robotpool.h"
#include "game.h"

int main() {
    srand(time(NULL));

    struct Game* game = calloc(1, sizeof(*game));
    GameInit(game);
    free(game);

    return 0;
}
