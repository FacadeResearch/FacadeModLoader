#define _CRT_SECURE_NO_WARNINGS
#include "ConsoleUtils.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <Windows.h>
#include <chrono>

void ConsoleUtils::CreateConsole(const char* title)
{
    AllocConsole();
    SetConsoleTitleA(title);

    if (freopen("CONOUT$", "w", stdout) == nullptr) {
        return;
    }

    if (freopen("CONOUT$", "w", stderr) == nullptr) {
        return;
    }

    if (freopen("CONIN$", "r", stdin) == nullptr) {
        return;
    }

    std::cout.clear();
    std::cerr.clear();
    std::clog.clear();
}

void ConsoleUtils::Log(const char* message)
{
    time_t time_now = std::time(0);

    std::cout << std::put_time(std::localtime(&time_now), "%y-%m-%d %OH:%OM:%OS") << " [FACADE MOD LOADER] (v1.0): " << message << std::endl;
}
