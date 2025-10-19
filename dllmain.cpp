#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    SetConsoleTitleA("FacadeModLoader");

    if (freopen("CONOUT$", "w", stdout) == nullptr) {
        return -1;
    }

    if (freopen("CONOUT$", "w", stderr) == nullptr) {
        return -1;
    }

    if (freopen("CONIN$", "r", stdin) == nullptr) {
        return -1;
    }

    std::cout.clear();
    std::cerr.clear();
    std::clog.clear();

    std::cout << "Great things await." << std::endl;

    return 0;
}

//Just so we can hook the IAT, completely unneeded.
extern "C" __declspec(dllexport)
int Stub()
{
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, MainThread, hModule, 0, NULL);
        return TRUE;
    }

    return FALSE;
}