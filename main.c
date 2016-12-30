#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "engine.h"
#include "game.h"

int main() {
    srand(time(NULL));

    struct Game* game = calloc(1, sizeof(*game));
    GameInit(game);
    free(game);

    return 0;
}
