#include "Global.h"

HMODULE Global::AnimEngineBase = nullptr;
HMODULE Global::NativeAnimInterfaceBase = nullptr;

void Global::SetAnimEngineBase(HMODULE baseAddress)
{
	if (baseAddress == nullptr) {
		return;
	}

	Global::AnimEngineBase = baseAddress;
}

HMODULE Global::GetAnimEngineBase()
{
	if (Global::AnimEngineBase == nullptr) {
		return nullptr;
	}

	return Global::AnimEngineBase;
}

void Global::SetNativeAnimInterfaceBase(HMODULE baseAddress)
{
	if (baseAddress == nullptr) {
		return;
	}

	Global::NativeAnimInterfaceBase = baseAddress;
}

HMODULE Global::GetNativeAnimInterfaceBase()
{
	if (Global::NativeAnimInterfaceBase == nullptr) {
		return nullptr;
	}

	return Global::NativeAnimInterfaceBase;
}
