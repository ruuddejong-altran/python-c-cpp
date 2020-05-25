//
// Created by Ruud on 25-5-2020.
//

#include <stdio.h>

#include "greeting.h"

void callback(char* message)
{
    printf("%s\n", message);
}

int main()
{
    greeting(&callback, "Hi from C main");
    return 0;
}