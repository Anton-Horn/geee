#pragma once
#include "log.h"

#include <stdint.h>
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <optional>
#include <filesystem>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define EC_DEFAULT_CON_DEFAULT_DE_NO_COPY_NO_MOVE(x) x() = default;\
													~x() = default;\
													 x(const x&) = delete;\
													 x& operator=(const x&) = delete;\
													 x(const x&&) = delete;\
													 x& operator=(const x&&) = delete;\

#define LOG_ASSERT_PASSED 
#undef LOG_ASSERT_PASSED

#ifdef EC_WINDOWS
	
#define EC_ABORT(...) {EC_ERROR(__VA_ARGS__); __debugbreak(); }

#ifdef LOG_ASSERT_PASSED

#define EC_ASSERT(x) if ( (x) ) {EC_INFO("Assert \"{0}\" passed! [{2}:{1}]", TOSTRING(x), __LINE__, __FILE__); } else {EC_ABORT("Assert \"{0}\" failed! [{2}:{1}]", TOSTRING(x), __LINE__, __FILE__);}

#else

#define EC_ASSERT(x) if ( !(x) ) {EC_ABORT("Assert \"{0}\" failed! [{2}:{1}]", TOSTRING(x), __LINE__, __FILE__);}

#endif

#define VKA(x) EC_ASSERT((x) == VK_SUCCESS);

#else

#define EC_ABORT(...)
#define EC_ASSERT(x)
#define VKA(x)

#endif