#pragma comment(lib, "libMinHook.x86.lib")
#include "Minhook.h"
#include "Global.h"
#include "ConsoleUtils.h"
#include "UiUtils.h"
#include <Windows.h>
#include <iostream>

const static int WAIT_DELAY = 5000; //Give the engine a bit to start up before patching anything so we can find RVAs, etc

DWORD WINAPI MainThread(LPVOID lpParam) {
    ConsoleUtils::CreateConsole("FacadeModLoader v1.0 / github.com/facaderesearch");

    if (MH_Initialize() != MH_OK) {
        ConsoleUtils::Log("<dllmain.MainThread> MinHook failed to Initialize.");
        return -1;
    }

    Sleep(WAIT_DELAY);

    HMODULE animEngineHandle = GetModuleHandle(L"animEngineDLL.dll");
    HMODULE nativeAnimInterfaceHandle = GetModuleHandle(L"nativeaniminterface.dll");

    if (!animEngineHandle || !nativeAnimInterfaceHandle) {
        return -1;
    }

    Global::SetAnimEngineBase(animEngineHandle);
    Global::SetNativeAnimInterfaceBase(nativeAnimInterfaceHandle);

    UiUtils::InsertPlayerName("Melon", "m", 0);
    UiUtils::InsertPlayerName("Melon", "f", 1);

    ConsoleUtils::Log("FacadeModLoader has loaded successfully.");

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