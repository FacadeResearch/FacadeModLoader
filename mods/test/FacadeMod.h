#ifndef FACADEMOD_H
#define FACADEMOD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void (*Log)(const char* msg);
    bool (*Hook)(void* target, void* detour, void** original);
} FacadeAPI;

typedef void (*tOnLoad)(FacadeAPI* api);

#endif