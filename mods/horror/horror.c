#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#include "FacadeMod.h"
#include <stdio.h>

uintptr_t base = 0;

struct Vector3 {
    float x, y, z;
};

typedef void (__stdcall *tglEnable)(int cap);
typedef void (__stdcall *tglDisable)(int cap);
tglEnable _glEnable;
tglDisable _glDisable;

typedef void(__stdcall *tglBegin)(int mode);
typedef void(__stdcall *tglEnd)(void);
typedef void(__stdcall *tglVertex3f)(float x, float y, float z);
typedef void(__stdcall *tglLoadIdentity)(void);
typedef void(__stdcall *tglTranslatef)(float x, float y, float z);
typedef void(__stdcall *tglColor4f)(float r, float g, float b, float a);
typedef struct Vector3*(__thiscall* GetPosFn)(void* pThis, struct Vector3* outBuffer);
tglVertex3f original_glVertex3f = NULL;

typedef int (__cdecl* tGetStrLen)(char* str);
tGetStrLen originalGetStrLen = NULL;

typedef int32_t (__thiscall* tControlExpression)(void* arg1, int32_t arg2, char arg3);
tControlExpression ControlExpression = NULL;

typedef void (__thiscall* tSetPos)(void* pThis, int32_t x, int32_t y, int32_t z, int32_t flag);

typedef void (__cdecl* tDrawGameString)(char* arg1, float arg2, float arg3, int32_t arg4, float arg5, float arg6, int32_t arg7, int32_t arg8);

typedef bool(__cdecl *tPlayerEnteredText)(const char *text);
typedef void (__stdcall *tglDisableClientState)(int cap);
tglDisableClientState _glDisableClientState = NULL;

typedef void (__stdcall *tglEnableClientState)(int cap);
static tglEnableClientState _glEnableClientState = NULL;

tDrawGameString DrawGameString = NULL;

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

typedef char*(__thiscall* GetNameFn)(void* pThis);

char* getPlayerName() {
    void* pPlayer = getPlayerPointer();
    uintptr_t* vtable = getPlayerObject(pPlayer);

    GetNameFn getName = (GetNameFn)vtable[6];

    return getName(pPlayer);
}

struct Vector3 setPlayerPosition(int32_t x, int32_t y, int32_t z)
{   
    void *pPlayer = getPlayerPointer();
    uintptr_t *vtable = getPlayerObject(pPlayer);
            
    tSetPos setPos = (tSetPos)*(uintptr_t*)(vtable + 0x3c);

    setPos(pPlayer, x, y, z, 0xbf800000);
}

tglBegin _glBegin;
tglEnd _glEnd;
tglVertex3f _glVertex3f;
tglLoadIdentity _glLoadIdentity;
tglTranslatef _glTranslatef;
tglColor4f original_glColor4f = NULL;

int32_t sanityMeter = 100;

void __stdcall hooked_glVertex3f(float x, float y, float z) {
    if (sanityMeter < 50) {
        float distortion = (50.0f - (float)sanityMeter) / 100.0f;

        x *= (1.0f + distortion);
        y *= (1.0f + distortion);
    }

    original_glVertex3f(x, y, z);
}

char* random_str(int length) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static char buffer[128]; 

    if (length > 127) length = 127;

    for (int i = 0; i < length; i++) {
        buffer[i] = charset[rand() % 62];
    }

    buffer[length] = '\0'; 
    return buffer;
}

DWORD lastSoundTick = 0;

static int InsanityDelay = 800;
static int nextSoundDelay = 3000;

typedef int32_t (__cdecl* tGameLoop)();
tGameLoop originalGameLoop = NULL;

tPlayerEnteredText PlayerEnteredText = NULL;

bool musicTriggered = false;

typedef char* (__cdecl* tPlayMusic)(const char* path, int32_t volume, int32_t pitch);
tPlayMusic playMusicFunc = NULL;

const char* psychosisPhrases[] = {
    "THEY ARE LISTENING",
    "TRIP IS NOT REAL",
    "BEHIND THE DOOR",
    "GRACE KNOWS EVERYTHING",
    "DO NOT TURN AROUND",
    "STAY CALM STAY CALM",
    "KILL THEM ALL",
    "GRACE IS A SKINWALKER",
    "THEY KILLED YOUR SON",
    "GET REVENGE",
    "KILL THEM",
    "THEY HAVE YOUR FAMILY HELD HOSTAGE IN THE BEDROOM",
    "OPEN THE BEDROOM DOOR",
    "I AM WORTHLESS",
    "I'M BEING WATCHED"
};

int phraseCount = sizeof(psychosisPhrases) / sizeof(psychosisPhrases[0]);

void DrawPsychosisOverlay() {
    static DWORD lastPhraseTick = 0;
    static int currentPhraseIndex = 0;
    DWORD currentTick = GetTickCount();

    if (currentTick - lastPhraseTick > 3000) {
        currentPhraseIndex = rand() % phraseCount;
        lastPhraseTick = currentTick;
    }

    float x = (float)(rand() % 10 - 5); 
    float y = (float)(rand() % 10 - 5);

    DrawGameString((char*)psychosisPhrases[currentPhraseIndex], x, y, 0x190, 10.0f, 0.0f, 7, 0);
}

typedef void(__thiscall *tSetPos)(void *pThis, int32_t x, int32_t y, int32_t z, int32_t flag);
tSetPos original_SetPos = NULL;

int32_t __cdecl hookedGameLoop()
{
    int32_t result = originalGameLoop();
    DWORD currentTick = GetTickCount();
    static DWORD lastTick = 0;

    if (GetTickCount() - lastTick > InsanityDelay) {
        if (!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
            if (sanityMeter > 0) {
                sanityMeter--;
            }
        } else {
            if (sanityMeter < 100) {
                sanityMeter++;
            }
        }

        lastTick = GetTickCount();
    }

    if (sanityMeter < 70) {
        if (currentTick - lastSoundTick > nextSoundDelay) {
            
            int randomID = (rand() % 100) + 1;

            ((uint32_t (*)(int32_t))(base + 0xa850))(randomID);

            nextSoundDelay = 500 + (rand() % (500 + (sanityMeter * 40)));
            lastSoundTick = currentTick;
        }
    }

    if (sanityMeter < 20) {
        PlayerEnteredText("Trip I fucking hate you and I will kill you. FUCK out of my face, melonhead. Fucker.");
    }

    if (sanityMeter < 40 && (rand() % 100 > 95)) {
        char* glitch = random_str(20);

        DrawGameString(glitch, (float)(rand()%20-10), (float)(rand()%20-10), 0x12C, 6.0f, 0.0f, 7, 0);
    }

    if (sanityMeter < 50 && sanityMeter > 10) {
        DrawPsychosisOverlay();
    }

    if (sanityMeter < 20) {
        float x = (float)(rand() % 10 - 5); 
        float y = (float)(rand() % 10 - 5);

        DrawGameString("RUN.", x, y, 0x190, 10.0f, 0.0f, 7, 0);
    }
    
    if (sanityMeter <= 60 && !musicTriggered) {
        playMusicFunc("sounds\\mp3_2\\4.mp3", 100, 60);

        musicTriggered = true;
    } 
    else if (sanityMeter > 65) {
        musicTriggered = false; 
    }

    char insanityBuf[64];
    sprintf(insanityBuf, "Sanity: %d%%", sanityMeter);
    DrawGameString(insanityBuf, -55.0f, -85.0f, 0x4b, 8.0f, 0.0f, 7, 0);

    return result;
}

void OverwriteText(uintptr_t offset, const char* newText)
{
    uintptr_t text = base + offset;
    DWORD oldProtect;
    VirtualProtect((void*)text, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    *(const char**)text = newText;
    VirtualProtect((void*)text, sizeof(uintptr_t), oldProtect, &oldProtect);
}

__attribute__((naked)) void midHook_IntroText()
{
    __asm__(
        ".intel_syntax noprefix\n"
        "call _RenderHorrorIntro\n"

        "mov eax, [_base]\n"
        "add eax, 0x97d1b\n" 

        "jmp eax\n"
        ".att_syntax\n"
    );
}

typedef void (__stdcall *tglClear)(int mask);
tglClear original_glClear = NULL;

void __stdcall hooked_glClear(int mask) {
    if (sanityMeter < 70) {
        static int frameCount = 0;
        if (++frameCount % 3 != 0) {
            original_glClear(0x2e5);  //It isnt valid but hey it makes it fuck up more than 0x4100
            return; 
        }
    }

    original_glClear(mask);
}

void RenderHorrorIntro() {
    const char* line1 = "Don't let your sanity meter drop to low levels!";
    const char* line2 = "Make sure to always hold right shift to keep your calm.";
    const char* line3 = "If you don't, you will go fucking nuts.";

    //text, x, y, width, size, opacity, opacity again?, ?
    DrawGameString((char*)line1, -40.0f, 10.0f, 0x12C, 5.5f, 0.0f, 7, 0);
    DrawGameString((char*)line2, -40.0f, 0.0f, 0x12C, 5.5f, 0.0f, 7, 0);
    DrawGameString((char*)line3, -40.0f, -12.0f, 0x12C, 5.5f, 0.0f, 7, 0);
}

tglTranslatef original_glTranslatef = NULL;

void __stdcall hooked_glTranslatef(float x, float y, float z) {
    if (sanityMeter < 30) {
        float slowTime = (float)GetTickCount() * 0.0005f;

        y += (float)(sin(slowTime) * 0.35f); 
        x += (float)(cos(slowTime * 0.8f) * 0.25f);
    }
    original_glTranslatef(x, y, z);
}

void OnLoad(FacadeAPI* api) {
    printf("Good luck..\n");

    base = (uintptr_t)GetModuleHandleA("animEngineDLL.dll");

    if (!base)
        return;

    DrawGameString = (tDrawGameString)(base + 0x9e370);
    playMusicFunc = (tPlayMusic)(base + 0x22e8);
    PlayerEnteredText = (tPlayerEnteredText)(base + 0xB6C04);

    HMODULE hGlut = GetModuleHandleA("opengl32.dll");
    tglClear real_glClear = (tglClear)GetProcAddress(hGlut, "glClear");

    original_glColor4f = (tglColor4f)GetProcAddress(hGlut, "glColor4f");

    tglTranslatef real_glTranslatef = (tglTranslatef)GetProcAddress(hGlut, "glTranslatef");
    _glDisableClientState = (tglDisableClientState)GetProcAddress(hGlut, "glDisableClientState");
    _glEnableClientState = (tglEnableClientState)GetProcAddress(hGlut, "glEnableClientState");

    api->Hook((void*)real_glTranslatef, (void*)&hooked_glTranslatef, (void**)&original_glTranslatef);
    
    api->Hook((void*)real_glClear, (void*)&hooked_glClear, (void**)&original_glClear);

    _glBegin = (tglBegin)GetProcAddress(hGlut, "glBegin");
    _glEnable = (tglEnable)GetProcAddress(hGlut, "glEnable");
    _glDisable = (tglDisable)GetProcAddress(hGlut, "glDisable");
    _glEnd = (tglEnd)GetProcAddress(hGlut, "glEnd");
    _glVertex3f = (tglVertex3f)GetProcAddress(hGlut, "glVertex3f");
    tglVertex3f real_glVertex3f = (tglVertex3f)GetProcAddress(hGlut, "glVertex3f");
    _glLoadIdentity = (tglLoadIdentity)GetProcAddress(hGlut, "glLoadIdentity");
    _glTranslatef = (tglTranslatef)GetProcAddress(hGlut, "glTranslatef");

    api->Hook((void*)real_glVertex3f, (void*)&hooked_glVertex3f, (void**)&original_glVertex3f);
    api->Hook((void*)(base + 0x9d210), (void*)&hookedGameLoop, (void**)&originalGameLoop);

    void *hookAddrIntro = (void *)(base + 0x97c82);
    void *dummy4;

    api->Hook(hookAddrIntro, (void *)&midHook_IntroText, &dummy4);
}

//i686-w64-mingw32-gcc -shared -o horror.dll horror.c -static-libgcc -luser32 -lshell32 -lws2_32 && mv ./horror.dll "/home/noia/.wine/drive_c/Program Files (x86)/Facade/util/sources/facade regular with mods/mods"