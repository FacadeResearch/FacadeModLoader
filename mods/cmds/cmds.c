#include <windows.h>
#include "FacadeMod.h"
#include <stdio.h>

int bSuperSpeed = 0;
int bDebugLogs = 0;

typedef int (__cdecl* tDebugLog)(char* msg);
tDebugLog originalDebugLog = NULL;

int __cdecl hookedDebugLog(char* msg) {
    if (msg != NULL && bDebugLogs) {
        printf("[Facade] %s\n", msg);
    }

    return originalDebugLog(msg);
}

typedef float (__cdecl* tGetSpeedValue)(float val, int flag);
tGetSpeedValue originalGetSpeedValue = NULL;

char strState[256] = {0};

typedef int (__cdecl* tGetStrLen)(char* str);
tGetStrLen originalGetStrLen = NULL;

int __cdecl hookedGetStrLen(char* str) {
    void* caller = __builtin_return_address(0);

    if (str != NULL && str[0] != '\0') {
        if (strcmp(strState, str) != 0) {
            strncpy(strState, str, sizeof(strState) - 1);

            if (bDebugLogs) {
                printf("[STR LENGTH HOOK] \"%s\" | Caller Address: %p\n", strState, caller);
            }
        }

        if ((uintptr_t)caller == 0x1009d9dd) {
           if (strstr(str, ".superspeed") != NULL) {   
                printf("Superspeeeeeed!\n");

                bSuperSpeed = !bSuperSpeed;

                printf("Superspeed: %s\n", bSuperSpeed == 1 ? "ON" : "OFF");
                
                memset(str, 0, strlen(str)); 
            }

            if (strstr(str, ".debug") != NULL) {
                bDebugLogs = !bDebugLogs;

                printf("debug logs: %s\n", bDebugLogs == 1 ? "ON" : "OFF");
                
                memset(str, 0, strlen(str)); 
            }
        }
    }

    return originalGetStrLen(str);
}

float __cdecl hookedGetSpeedValue(float val, int flag) {
    float result = originalGetSpeedValue(val, flag);

    if (bSuperSpeed == 1) {
        if (val == 1.5f) return 15.0f;
        if (val == -1.5f) return -15.0f;
        if (val == -0.75f) return -5.0f;
    }

    return result;
}


__declspec(dllexport) void OnLoad(FacadeAPI* api) {
   uintptr_t base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

   if (API_Hook((void*)(base + 0x53a90), (void*)&hookedDebugLog, (void**)&originalDebugLog)) {
        api->Log("hooked DebugLog.");
    }

    if (api->Hook((void*)(base + 0xafd30), (void*)&hookedGetSpeedValue, (void**)&originalGetSpeedValue)) {
        api->Log("hooked GetSpeedValue");
    }

    if (api->Hook((void*)(base + 0xb6e40), (void*)&hookedGetStrLen, (void**)&originalGetStrLen)) {
        api->Log("hooked GetStrLen!");
    }

    api->Log("Lovely day!");
}