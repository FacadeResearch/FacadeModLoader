#pragma once
#include "FML_API.h"

class IFacadeMod {
	public:
		virtual ~IFacadeMod() = default;

		virtual const char* GetName() const = 0;
		
		virtual const char* GetAuthor() const = 0;

		virtual bool Load() = 0;
};