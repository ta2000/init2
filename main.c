#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "robot.h"
#include "robotpool.h"
#include "engine.h"
#include "game.h"

int main() {
    srand(time(NULL));

    struct Game* game = calloc(1, sizeof(*game));
    GameInit(game);
    free(game);

    return 0;
}
