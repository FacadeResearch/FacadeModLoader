#pragma once
#include "IFacadeMod.h"
#include "ConsoleUtils.h"
#include "UiUtils.h"
#include "Global.h"
#include <format>
#include <string>
#include <windows.h>
#include "LightHook.hpp"
#include <iostream>

#define DrawGameStringRVA 0x9e370
#define FindObjectRVA 0xa7530
#define PackCoordsRVA 0x13c30
#define StrLenRVA 0xb6e40
#define GetSpeedRVA 0xafd30
#define QuickDrawLoopRVA 0x9d210
#define LocalPlayerRVA 0x979474
#define FPSRVA 0x1f6f1c

struct Vector3 {
    float x, y, z;
};

typedef void(__cdecl* tDrawGameString)(char* arg1, float arg2, float arg3, int32_t arg4, float arg5, float arg6, int32_t arg7, int32_t arg8);
typedef int32_t* (__thiscall* tFindObject)(void* pWorldManager, const char* name);
typedef int32_t* (__thiscall* tPackCoords)(int32_t* arg1, int32_t arg2, int32_t arg3, int32_t arg4);
typedef float(__cdecl* tGetSpeedValue)(float val, int flag);
typedef Vector3* (__thiscall* GetPosFn)(void* pThis, Vector3* outBuffer);
typedef int(__cdecl* tGetStrLen)(char* str);
typedef int32_t(__cdecl* QuickDrawLoopFn)();
typedef void(__thiscall* SetPosFn)(void* pThis, float x, float y, float z, float state);
typedef char* (__thiscall* GetNameFn)(void* pThis);

static tDrawGameString DrawGameString = NULL;
static tFindObject FindObject = NULL;
static tPackCoords PackCoords = NULL;

static LightHook::HookInformation speedHookInfo;
static LightHook::HookInformation strLenHookInfo;
static LightHook::HookInformation quickDrawHookInfo;

static bool bSuperSpeed = false;
static bool bNoclip = false;
static bool bPos = false;
static bool console = false;
static bool spacePressed = false;
static float verticalVelocity = 0.0f;
static float gravity = -0.01f;
static float jumpStrength = 0.6f;
static float groundY = 0.0f;
static float noclipSpeed = 0.5f;
static bool isJumping = false;

inline void* getPlayerPointer() {
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(Global::GetAnimEngineBase());

    if (baseAddr == 0) {
        return nullptr;
    }

    void** pPlayerPtr = reinterpret_cast<void**>(baseAddr + LocalPlayerRVA);

    if (pPlayerPtr == nullptr || *pPlayerPtr == nullptr) {
        return nullptr;
    }

    return *pPlayerPtr;
}

inline uintptr_t* getPlayerObject(void* pPlayer) {
    if (pPlayer == nullptr) {
        return nullptr;
    }

    return *reinterpret_cast<uintptr_t**>(pPlayer);
}

inline Vector3 getPlayerPosition() {
    void* pPlayer = getPlayerPointer();

    if (pPlayer == nullptr) {
        return Vector3();
    }

    uintptr_t* vtable = getPlayerObject(pPlayer);

    if (vtable != nullptr) {
        GetPosFn getPos = (GetPosFn)vtable[11];

        Vector3 currentPos;
        getPos(pPlayer, &currentPos);

        return currentPos;
    }

    return Vector3();
}

inline void updatePlayerState(float x, float y, float z, float state) {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);
    SetPosFn setPos = (SetPosFn)vtable[15];

    setPos(pPlayer, x, y, z, state);
}

inline char* getPlayerName() {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);
    GetNameFn getName = (GetNameFn)vtable[6];

    return getName(pPlayer);
}

inline float __cdecl hookedGetSpeedValue(float val, int flag) {
    if (bNoclip || isJumping) {
        return 0.0f;
    }

    auto original = reinterpret_cast<tGetSpeedValue>(speedHookInfo.Trampoline);
    float result = original(val, flag);

    if (bSuperSpeed) {
        if (val == 1.5f) return 15.0f;
        if (val == -1.5f) return -15.0f;
        if (val == -0.75f) return -5.0f;
    }

    return result;
}

inline int __cdecl hookedGetStrLen(char* str) {
    void* caller = _ReturnAddress();

    if (str != NULL && str[0] != '\0') {
        if ((uintptr_t)caller == 0x1009d9dd) {
            if (strstr(str, ".superspeed") != NULL) {
                bSuperSpeed = !bSuperSpeed;
                printf("Superspeed: %s\n", bSuperSpeed ? "ON" : "OFF");
                memset(str, 0, strlen(str));
            }
            if (strstr(str, ".noclip") != NULL) {
                bNoclip = !bNoclip;
                printf("No clip: %s\n", bNoclip ? "ON" : "OFF");
                memset(str, 0, strlen(str));
            }
            if (strstr(str, ".pos") != NULL) {
                bPos = !bPos;
                printf("POS Render: %s\n", bPos ? "ON" : "OFF");
                memset(str, 0, strlen(str));
            }
        }
    }

    bool spaceIsDown = GetAsyncKeyState(VK_SHIFT) & 0x8000;

    if (spaceIsDown && !spacePressed && !isJumping) {
        Vector3 currentPos = getPlayerPosition();

        groundY = currentPos.y;

        verticalVelocity = jumpStrength;
        isJumping = true;
    }

    spacePressed = spaceIsDown;

    if (isJumping) {
        Vector3 pos = getPlayerPosition();

        pos.y += verticalVelocity;
        verticalVelocity += gravity;

        if (pos.y <= groundY) {
            pos.y = groundY;
            isJumping = false;
            verticalVelocity = 0.0f;
        }

        updatePlayerState(pos.x, pos.y, pos.z, -1.0f);
    }

    if (bNoclip) {
        Vector3 pos = getPlayerPosition();
        float currentSpeed = noclipSpeed;

        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) currentSpeed *= 3.0f;
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) pos.y += currentSpeed;
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) pos.y -= currentSpeed;
        if (GetAsyncKeyState(VK_UP) & 0x8000) pos.z -= currentSpeed;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) pos.z += currentSpeed;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) pos.x -= currentSpeed;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pos.x += currentSpeed;

        updatePlayerState(pos.x, pos.y, pos.z, -1.0f);
    }

    auto original = reinterpret_cast<tGetStrLen>(strLenHookInfo.Trampoline);
    return original(str);
}

inline int32_t __cdecl hookedQuickDrawLoop() {
    auto original = reinterpret_cast<QuickDrawLoopFn>(quickDrawHookInfo.Trampoline);
    int32_t result = original();

    if (bPos) {
        Vector3 playerPos = getPlayerPosition();
        char* playerName = getPlayerName();
        std::string posStr = std::format("Player: {} | X: {:.2f} Y: {:.2f} Z: {:.2f}",
            playerName ? playerName : "Unknown", playerPos.x, playerPos.y, playerPos.z);
        DrawGameString(posStr.data(), -10.0f, 20.0f, 200, 5.0f, 0.0f, 7, 0);
    }

    return result;
}

class CommandsMod : public IFacadeMod {
public:
    const char* GetName() const override {
        return "Commands Mod";
    }

    const char* GetAuthor() const override {
        return "FacadeModLoader Team";
    }

    bool Load() override {
        uintptr_t baseAddr = (uintptr_t)Global::GetAnimEngineBase();

        DrawGameString = (tDrawGameString)(baseAddr + DrawGameStringRVA);
        FindObject = (tFindObject)(baseAddr + FindObjectRVA);
        PackCoords = (tPackCoords)(baseAddr + PackCoordsRVA);

        void* targetGetSpeed = reinterpret_cast<void*>(baseAddr + GetSpeedRVA);
        void* targetGetStrLen = reinterpret_cast<void*>(baseAddr + StrLenRVA);
        void* targetQuickDraw = reinterpret_cast<void*>(baseAddr + QuickDrawLoopRVA);

        speedHookInfo = LightHook::CreateHook(targetGetSpeed, reinterpret_cast<void*>(&hookedGetSpeedValue));
        strLenHookInfo = LightHook::CreateHook(targetGetStrLen, reinterpret_cast<void*>(&hookedGetStrLen));
        quickDrawHookInfo = LightHook::CreateHook(targetQuickDraw, reinterpret_cast<void*>(&hookedQuickDrawLoop));

        if (!LightHook::EnableHook(&speedHookInfo)) {
            ConsoleUtils::Log("LightHook: Create_Hook for Player GetSpeedValue failed.");
            return false;
        }

        if (!LightHook::EnableHook(&strLenHookInfo)) {
            ConsoleUtils::Log("LightHook: Create_Hook for GetStrLen failed.");
            return false;
        }

        if (!LightHook::EnableHook(&quickDrawHookInfo)) {
            ConsoleUtils::Log("LightHook: Create_Hook for Player originalQuickDrawLoopFn failed.");
            return false;
        }

        return true;
    }
};