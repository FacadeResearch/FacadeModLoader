#pragma comment(lib, "libMinHook.x86.lib")
#include "Minhook.h"
#include "Global.h"
#include "ConsoleUtils.h"
#include "UiUtils.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include "IFacadeMod.h"
#include "QoLFixesMod.cpp"

const static int WAIT_DELAY = 1500; //Give the engine a bit to start up before patching anything so we can find RVAs, etc

using CreateModFn = IFacadeMod * (*)();
using DestroyModFn = void(*)(IFacadeMod* instance);

struct LoadedMod {
    HMODULE library_handle;
    IFacadeMod* instance;
    DestroyModFn destroy_func;
};

static std::vector<LoadedMod> Mods;

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

    if (!std::filesystem::exists("Mods")) {
        std::filesystem::create_directory("Mods");
    }

    for (auto& file : std::filesystem::directory_iterator("Mods")) {
        std::filesystem::path path = file.path();
        std::filesystem::path absolute_path = std::filesystem::absolute(path);

        if (!path.has_extension()) {
            continue;
        }

        if (path.string().find(".dll") == std::string::npos) {
            continue;
        }

        std::filesystem::path filename = path.filename();

        ConsoleUtils::Log(std::string("Loading " + filename.string() + "..."));

        HMODULE library = LoadLibraryA(absolute_path.string().c_str());

        if (library == nullptr) {
            ConsoleUtils::Log(std::string("Failed to load " + path.string() + " - skipping.."));
            continue;
        }

        CreateModFn createmod = (CreateModFn)GetProcAddress(library, "CreateMod");

        if (createmod == nullptr) {
            ConsoleUtils::Log(std::string("Failed to load " + path.string() + " - no exported CreateMod - skipping.."));
            FreeLibrary(library);
            continue;
        }

        DestroyModFn destroymod = (DestroyModFn)GetProcAddress(library, "DestroyMod");

        if (destroymod == nullptr) {
            ConsoleUtils::Log(std::string("Failed to load " + path.string() + " - no exported DestroyMod - skipping.."));
            FreeLibrary(library);
            continue;
        }

        IFacadeMod* mod = createmod();

        if (mod) {
            mod->Load();

            Mods.push_back({ library, mod, destroymod });

            ConsoleUtils::Log(std::string("Loaded ") + mod->GetName() + " by " + mod->GetAuthor());
        }
        else {
            FreeLibrary(library);
        }
    }

    QoLFixesMod* qolfixesmod = new QoLFixesMod();

    qolfixesmod->Load();

    ConsoleUtils::Log(std::string("Loaded Core Mod ") + qolfixesmod->GetName() + " by " + qolfixesmod->GetAuthor());

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