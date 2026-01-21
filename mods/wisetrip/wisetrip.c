#include <windows.h>
#include "FacadeMod.h"
#include <stdio.h>

typedef int32_t (__attribute__((fastcall)) *tUpdateNPCFace)(void* ecx, void* edx, float arg2, char arg3, char arg4, char arg5);
tUpdateNPCFace originalUpdateNPCFace = NULL;

int32_t __attribute__((fastcall)) hookedNPCFace(void* ecx, void* edx, float arg2, char arg3, char arg4, char arg5) {
    // 0x02ba0040 is Grace 
    // 0x03430040 is Trip

    if ((uintptr_t)ecx == 0x03430040) {
        return originalUpdateNPCFace(ecx, edx, arg2, 1, 1, 1); 
    }

    return originalUpdateNPCFace(ecx, edx, arg2, arg3, arg4, arg5);
}

__declspec(dllexport) void OnLoad(FacadeAPI* api) {
    uintptr_t base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    api->Hook((void*)(base +  0x1fdf0), (void*)&hookedNPCFace, (void**)&originalUpdateNPCFace);
    api->Log("Wise trip.. is very wise..");
}