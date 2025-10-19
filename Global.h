#pragma once
#include <Windows.h>

class Global {
	public:
		static void SetAnimEngineBase(HMODULE baseAddress);
		static HMODULE GetAnimEngineBase();
		static void SetNativeAnimInterfaceBase(HMODULE baseAddress);
		static HMODULE GetNativeAnimInterfaceBase();
	private:
		static HMODULE AnimEngineBase;
		static HMODULE NativeAnimInterfaceBase;
};