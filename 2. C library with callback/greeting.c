//
// Created by Ruud on 25-5-2020.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "greeting.h"

int gDebug;

#if DEBUG
gDebug = 1;
#else
gDebug = 0;
#endif

void greeting(void (*message_printer)(char*), char* message)
{
    size_t message_size = strlen(message) + 40;  // Room for extra text
    char* message_to_send = (char*)malloc(message_size * sizeof(char));
    sprintf(message_to_send, "Saying \"%s\" from C (%s build)", message, (gDebug ? "Debug" : "Release"));
    message_printer(message_to_send);
}
