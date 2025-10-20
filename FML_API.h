#pragma once

#ifdef FACADEMODLOADER_EXPORTS
#define FML_API __declspec(dllexport)
#else
#define FML_API __declspec(dllimport)
#endif