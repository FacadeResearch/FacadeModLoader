#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "LightHook.h"
#include "FacadeMod.h"

void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    SetConsoleTitleA("FacadeModLoader (Linux version)");
}

void API_Log(const char* msg) {
    printf("[Mod] %s\n", msg);
}

bool API_Hook(void* target, void* detour, void** original) {
    if (!target || !detour) return false;
    
    HookInformation hook = CreateHook(target, detour);
    if (EnableHook(&hook)) {
        if (original) *original = hook.Trampoline;
        return true;
    }
    return false;
}

void LoadMods() {
    FacadeAPI api;

    api.Log = API_Log;
    api.Hook = API_Hook;

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("mods/*.dll", &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char path[MAX_PATH];
            
            sprintf(path, "mods/%s", findData.cFileName);
            
            HMODULE hMod = LoadLibraryA(path);
            
            if (hMod) {
                tOnLoad onLoad = (tOnLoad)GetProcAddress(hMod, "OnLoad");
                if (onLoad) {
                    printf("[FML] initializing: %s\n", findData.cFileName);
                    onLoad(&api); 
                }
            } else {
                printf("[FML] failed to load mod %s (error: %lu)\n", findData.cFileName, GetLastError());
            }
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    } else {
        printf("[FML] No mods found to load!\n");
    }
}

bool Patch(void* address, void* data, size_t size) {
    DWORD oldProtect;

    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }

    memcpy(address, data, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), address, size);
    
    return true;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    CreateConsole();

    uintptr_t base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    if (base == 0) {
        perror("anim engine dll base address not found!\n");
        return -1;
    } 

    LoadMods();
   
    return 0;
}

__declspec(dllexport) int Stub()
{
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
    }
    return TRUE;
}