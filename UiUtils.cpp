#include "UiUtils.h"
#include "ConsoleUtils.h"
#include "Global.h"
#include "OFfsets.h"
#include <Windows.h>
#include <iostream>

void UiUtils::InsertPlayerName(const char* character_name, const char* gender_letter, int index)
{
	DWORD_PTR Base = (DWORD_PTR)Global::GetAnimEngineBase();

	if (Base == NULL) {
		ConsoleUtils::Log("<UiUtils.InsertPlayerName> Couldn't get a base to animEngineDLL.dll");
		return;
	}

	static const char* name_entry[2]{
		character_name,
		gender_letter
	};

	size_t size = sizeof(const char*) * 2;
	DWORD_PTR nameListAddress = Base + NAME_LIST_RVA + (index * 8);

	memcpy((void*)nameListAddress, name_entry, size);

	ConsoleUtils::Log("<UiUtils.InsertPlayerName> Inserted new player name");
}
