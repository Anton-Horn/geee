#pragma once
#include "log.h"

#include <stdint.h>
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <optional>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#ifdef EC_WINDOWS
	
#define EC_ABORT(...) {EC_ERROR(__VA_ARGS__); __debugbreak(); }
#define EC_ASSERT(x) if ( (x) ) {EC_INFO("Assert \"{0}\" passed! [{2}:{1}]", TOSTRING(x), __LINE__, __FILE__); } else {EC_ABORT("Assert \"{0}\" failed! [{2}:{1}]", TOSTRING(x), __LINE__, __FILE__);}
#define VKA(x) EC_ASSERT((x) == VK_SUCCESS);

#else

#endif