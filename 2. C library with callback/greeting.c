//
// Created by Ruud on 25-5-2020.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "greeting.h"

void greeting(void (*message_printer)(char*), char* message)
{
    size_t message_size = strlen(message) + 20;  // Room for extra text
    char* message_to_send = (char*)malloc(message_size * sizeof(char));
    sprintf(message_to_send, "Saying \"%s\" from C", message);
    message_printer(message_to_send);
}
