#include <stdio.h>
#include <stdlib.h>

#include "engine.h"

int main() {
    struct Engine* engine = calloc(1, sizeof(*engine));

    EngineInit(engine);
    EngineRun(engine);
    EngineDestroy(engine);

    free(engine);

    return 0;
}

