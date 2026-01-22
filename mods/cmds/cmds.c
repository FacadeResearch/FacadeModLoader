#include <windows.h>
#include "FacadeMod.h"
#include <stdio.h>

bool bSuperSpeed = false;
bool bDrawFps = false;
bool bNoclip = false;

uintptr_t base = 0;

typedef float (__cdecl* tGetSpeedValue)(float val, int flag);
tGetSpeedValue originalGetSpeedValue = NULL;

typedef int (__cdecl* tGetStrLen)(char* str);
tGetStrLen originalGetStrLen = NULL;

int __cdecl hookedGetStrLen(char* str) {
    void* caller = __builtin_return_address(0);

    if (str != NULL && str[0] != '\0') {
        if ((uintptr_t)caller == 0x1009d9dd) {
           if (strstr(str, ".superspeed") != NULL) {   
                bSuperSpeed = !bSuperSpeed;

                printf("Superspeed: %s\n", bSuperSpeed == 1 ? "ON" : "OFF");
                
                memset(str, 0, strlen(str)); 
            }

            if (strstr(str, ".fps") != NULL) {
                bDrawFps = !bDrawFps;

                printf("FPS Counter: %s\n", bDrawFps == 1 ? "ON" : "OFF");
                
                memset(str, 0, strlen(str)); 
            }
        }
    }

    return originalGetStrLen(str);
}

typedef void (__cdecl* tDrawGameString)(char* arg1, float arg2, float arg3, int32_t arg4, float arg5, float arg6, int32_t arg7, int32_t arg8);
tDrawGameString DrawGameString = NULL;

typedef int32_t (__cdecl* tSub_1009d210)();
tSub_1009d210 originalSub_1009d210 = NULL;

__cdecl hookedSub_1009d210() {
    int32_t result = originalSub_1009d210();

    if (bDrawFps) {        
        int* fpsPtr = (int*)(base + 0x1f6f1c); 
        int currentFps = *fpsPtr;

        char fpsBuffer[32];
        sprintf(fpsBuffer, "FPS: %d", currentFps);
        DrawGameString(fpsBuffer, -10.0f, 0.0f, 200, 5.0f, 0.0f, 7, 0);
    }
 
    return result;
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
    base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    DrawGameString = (tDrawGameString)(base + 0x9e370);

    api->Hook((void*)(base + 0xafd30), (void*)&hookedGetSpeedValue, (void**)&originalGetSpeedValue);
    api->Hook((void*)(base + 0xb6e40), (void*)&hookedGetStrLen, (void**)&originalGetStrLen);
    api->Hook((void*)(base + 0x9d210), (void*)&hookedSub_1009d210, (void**)&originalSub_1009d210);
}