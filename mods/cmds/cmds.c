#include <windows.h>
#include "FacadeMod.h"
#include <stdio.h>

bool bSuperSpeed = false;

typedef float (__cdecl* tGetSpeedValue)(float val, int flag);
tGetSpeedValue originalGetSpeedValue = NULL;

typedef int (__cdecl* tGetStrLen)(char* str);
tGetStrLen originalGetStrLen = NULL;

int __cdecl hookedGetStrLen(char* str) {
    void* caller = __builtin_return_address(0);

    if (str != NULL && str[0] != '\0') {
        if ((uintptr_t)caller == 0x1009d9dd) {
           if (strstr(str, ".superspeed") != NULL) {   
                printf("Superspeeeeeed!\n");

                bSuperSpeed = !bSuperSpeed;

                printf("Superspeed: %s\n", bSuperSpeed == 1 ? "ON" : "OFF");
                
                memset(str, 0, strlen(str)); 
            }
        }
    }

    return originalGetStrLen(str);
}

float __cdecl hookedGetSpeedValue(float val, int flag) {
    float result = originalGetSpeedValue(val, flag);

    if (bSuperSpeed) {
        if (val == 1.5f) return 15.0f;
        if (val == -1.5f) return -15.0f;
        if (val == -0.75f) return -5.0f;
    }

    return result;
}

__declspec(dllexport) void OnLoad(FacadeAPI* api) {
    uintptr_t base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    api->Hook((void*)(base + 0xafd30), (void*)&hookedGetSpeedValue, (void**)&originalGetSpeedValue);
    api->Hook((void*)(base + 0xb6e40), (void*)&hookedGetStrLen, (void**)&originalGetStrLen);
}