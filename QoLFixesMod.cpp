#include "IFacadeMod.h"
#include "ConsoleUtils.h"
#include "UiUtils.h"
#include "Global.h"
#include "Offsets.h"
#include <MinHook.h>

using OpenTipJarFn = void (*)();
static OpenTipJarFn originalOpenTipJarFn = nullptr;

class QoLFixesMod : public IFacadeMod {
	public:
		const char* GetName() const override {
			return "QoL Fixes";
		}

		const char* GetAuthor() const override {
			return "FacadeModLoader Team";
		}

		static void openTipJarFix()
		{
			ConsoleUtils::Log("QoLFixesMod: Hooked openTipJar called. Opening URL...");

			ShellExecute(0, 0, TEXT("http://www.interactivestory.net/tipjar"), 0, 0, SW_SHOW);
		}

		bool Load() override {
			OpenTipJarFn openTipJar = (OpenTipJarFn)(Global::GetAnimEngineBase() + OPEN_TIP_JAR_FN_RVA);

			if (MH_CreateHook(openTipJar, &openTipJarFix, nullptr) != MH_OK) {
				ConsoleUtils::Log("Create_Hook for Open Tip Jar failed.");
				return false;
			}

			if (MH_EnableHook(openTipJar) != MH_OK) {
				ConsoleUtils::Log("Enable_Hook for Open Tip Jar failed.");
				return false;
			}

			return true;
		}
};