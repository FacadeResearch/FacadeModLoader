#include <windows.h>
#include "FacadeMod.h"
#include <stdio.h>

bool bSuperSpeed = false;
bool bDrawFps = false;
bool bNoclip = false;
bool bPos = false;
bool console = false;
bool spacePressed = false;
float verticalVelocity = 0.0f;
float gravity = -0.01f;
float jumpStrength = 0.6f;
float groundY = 0.0f;
float noclipSpeed = 0.5f;
bool isJumping = false;
uintptr_t base = 0;

struct Vector3 {
    float x, y, z;
};

typedef float (__cdecl* tGetSpeedValue)(float val, int flag);
tGetSpeedValue originalGetSpeedValue = NULL;

typedef struct Vector3*(__thiscall* GetPosFn)(void* pThis, struct Vector3* outBuffer);

typedef int (__cdecl* tGetStrLen)(char* str);
tGetStrLen originalGetStrLen = NULL;

typedef int32_t (__thiscall* tControlExpression)(void* arg1, int32_t arg2, char arg3);
tControlExpression ControlExpression = NULL;

typedef void(__thiscall* SetPosFn)(void* pThis, float x, float y, float z, float state);

void* getPlayerPointer() { 
    void* pPlayer = *(void**)(base + 0x979474);
    return pPlayer;
}

uintptr_t* getPlayerObject(void* pPlayer) {
     uintptr_t* vtable = *(uintptr_t**)pPlayer;

     return vtable;
}

struct Vector3 getPlayerPosition() {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);
    GetPosFn getPos = (GetPosFn)vtable[11];

    struct Vector3 currentPos; 
    getPos(pPlayer, &currentPos);

    return currentPos;
}

void updatePlayerState(float x, float y, float z, float state) {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);

    SetPosFn setPos = (SetPosFn)vtable[15];

    setPos(pPlayer, x, y, z, state); //state = -1.0f
}

typedef char*(__thiscall* GetNameFn)(void* pThis);

char* getPlayerName() {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);

    GetNameFn getName = (GetNameFn)vtable[6];

    return getName(pPlayer);
}

typedef uint32_t (__cdecl* tPlaySoundEffect)(int32_t arg1);
tPlaySoundEffect originalPlaySoundEffect = NULL;

typedef HCURSOR (__thiscall* tChangeCursorImage)(void* pThis, HCURSOR arg2);
tChangeCursorImage originalChangeCursorImage = NULL;

HCURSOR __thiscall hookedChangeCursorImage(void* pThis, HCURSOR arg2) {
    printf("[XCursor] Requested ID: %p\n", arg2);

   //arg2 = (HCURSOR)14;

   printf("pThis[0x30]: %p\n", *(void**)((char*)pThis + 0x30));
   
    HCURSOR result = originalChangeCursorImage(pThis, arg2);

    printf("Result %d\n", result);

    //arg2 = 6 (knock)
    return result;
}

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

            if (strstr(str, ".noclip") != NULL) {
                bNoclip = !bNoclip;

                printf("No clip: %s\n", bNoclip == 1 ? "ON" : "OFF");

                memset(str, 0, strlen(str));
            }

            if (strstr(str, ".pos") != NULL) {
                bPos = !bPos;

                printf("POS Render: %s\n", bPos == 1 ? "ON" : "OFF");

                memset(str, 0, strlen(str));
            }
        }
    }

    bool spaceIsDown = GetAsyncKeyState(VK_SHIFT) & 0x8000;

    if (spaceIsDown && !spacePressed && !isJumping) {
        struct Vector3 currentPos = getPlayerPosition();
        groundY = currentPos.y;
        verticalVelocity = jumpStrength;
        isJumping = true;
    }

    spacePressed = spaceIsDown;

    if (isJumping)
    {
        struct Vector3 pos = getPlayerPosition();

        pos.y += verticalVelocity;

        verticalVelocity += gravity;

        if (pos.y <= groundY)
        {
            pos.y = groundY;
            isJumping = false;
            verticalVelocity = 0.0f;
        }

        updatePlayerState(pos.x, pos.y, pos.z, -1.0f);
    }

    if (bNoclip) {
        struct Vector3 pos = getPlayerPosition();
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

    return originalGetStrLen(str);
}

typedef void (__cdecl* tDrawGameString)(char* arg1, float arg2, float arg3, int32_t arg4, float arg5, float arg6, int32_t arg7, int32_t arg8);
tDrawGameString DrawGameString = NULL;

typedef int32_t (__cdecl* tSub_1009d210)();
tSub_1009d210 originalSub_1009d210 = NULL;

uint32_t hookedPlaySoundEffect(int32_t arg1) {
    uint32_t result = originalPlaySoundEffect(arg1);

    printf("Playing sound effect: %d\nResult: %d\n", arg1, result);

    return result;
}

typedef uint32_t (__cdecl* tSub_10091af0)(int32_t arg1, char arg2);
tSub_10091af0 originalSub_10091af0 = NULL;

typedef int32_t* (__thiscall* tFindObject)(void* pWorldManager, const char* name);
tFindObject FindObject = NULL;

typedef int32_t* (__thiscall* tPackCoords)(int32_t* arg1, int32_t arg2, int32_t arg3, int32_t arg4);
tPackCoords PackCoords = NULL;

uint32_t hookedSub_10091af0(int32_t arg1, char arg2) {
    uint32_t result = originalSub_10091af0(arg1, arg2);

    void* worldManager = (void*)(base + 0x83f640);
    if (worldManager) {
        int32_t* pCouch = FindObject(worldManager, "couch");
        if (pCouch) {
            printf("Success! Couch found at %p\n", pCouch);

            typedef void (__thiscall* tSetPos)(void* pThis, int32_t x, int32_t y, int32_t z, int32_t flag);
            
            uintptr_t vtable = *(uintptr_t*)pCouch;
            
            tSetPos setPos = (tSetPos)*(uintptr_t*)(vtable + 0x3c);

            setPos(pCouch, 0x43200000, 0, 0, 0xbf800000);
        }
    }

    return result;
}
//b7a50

int32_t __cdecl hookedSub_1009d210() {
    int32_t result = originalSub_1009d210();

    if (bDrawFps) {        
        int* fpsPtr = (int*)(base + 0x1f6f1c); 
        int currentFps = *fpsPtr;

        char fpsBuffer[32];
        sprintf(fpsBuffer, "FPS: %d", currentFps);
        DrawGameString(fpsBuffer, -10.0f, 0.0f, 200, 5.0f, 0.0f, 7, 0);
    }

    if (bPos) {
        struct Vector3 playerPos = getPlayerPosition();

        char* playerName = getPlayerName();
        char posBuffer[128];

        sprintf(posBuffer, "Player: %s | X: %.2f Y: %.2f Z: %.2f", playerName, playerPos.x, playerPos.y, playerPos.z);
        DrawGameString(posBuffer, -10.0f, 20.0f, 200, 5.0f, 0.0f, 7, 0);
    }
 
    return result;
}

float __cdecl hookedGetSpeedValue(float val, int flag) {
    if (bNoclip || isJumping) {
        return 0.0f; 
    }

    float result = originalGetSpeedValue(val, flag);

    if (bSuperSpeed) {
        if (val == 1.5f) return 15.0f;
        if (val == -1.5f) return -15.0f;
        if (val == -0.75f) return -5.0f;
    }

    return result;
}

void InsertCustomName(int index, const char* newName) {
    uintptr_t nameTableAddr = base + 0x1c6c00; 
    uintptr_t* tableEntry = (uintptr_t*)(nameTableAddr + (index * sizeof(uintptr_t)));
    
    DWORD oldProtect;
    VirtualProtect(tableEntry, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);

    *tableEntry = (uintptr_t)newName;
    VirtualProtect(tableEntry, sizeof(uintptr_t), oldProtect, &oldProtect);
}

__declspec(dllexport) void OnLoad(FacadeAPI* api) {
    base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    DrawGameString = (tDrawGameString)(base + 0x9e370);
    originalPlaySoundEffect = (tPlaySoundEffect)(base + 0xa850);
    FindObject = (tFindObject)(base + 0xa7530);
    PackCoords = (tPackCoords)(base + 0x13c30);

    api->Hook((void*)(base + 0xafd30), (void*)&hookedGetSpeedValue, (void**)&originalGetSpeedValue);
    api->Hook((void*)(base + 0xb6e40), (void*)&hookedGetStrLen, (void**)&originalGetStrLen);
    api->Hook((void*)(base + 0x9d210), (void*)&hookedSub_1009d210, (void**)&originalSub_1009d210);
    //api->Hook((void*)(base + 0xb4de0), (void*)&hookedChangeCursorImage, (void**)&originalChangeCursorImage);
    api->Hook((void*)(base + 0x91af0), (void*)&hookedSub_10091af0, (void**)&originalSub_10091af0);

    //InsertCustomName(0, "Melon");
}