#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "game.h"

int main() {
    struct Game* game = calloc(1, sizeof(*game));
    GameInit(game);
    free(game);

    return 0;
}
