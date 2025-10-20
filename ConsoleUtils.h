#pragma once
#include <string>
#include "FML_API.h"

class FML_API ConsoleUtils {
	public:
		static void CreateConsole(const char* title);
		static void Log(const std::string& message);
};