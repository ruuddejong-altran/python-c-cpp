//
// Created by Ruud on 25-5-2020.
//

#include <stdio.h>
#include "greeting.h"

int gDebug;

#if DEBUG
gDebug = 1;
#else
gDebug = 0;
#endif

void greeting(char* message)
{
    printf("Saying \"%s\" from C (%s build)\n", message, (gDebug ? "Debug" : "Release"));
}
