#pragma once

#include <iostream>
#include <sstream>

#include "spdlog/spdlog.h"

#if defined(_MSC_VER)
#include <intrin.h>
#define DebuggerBreak()		do{ __debugbreak(); } while(0)
#define __SHORT_FORM_OF_FILE__		\
	(strrchr(__FILE__, '\\')		\
	? strrchr(__FILE__, '\\') + 1	\
	: __FILE__						\
	)
#else
#include <signal.h>
#define DebuggerBreak()		do{ raise(SIGINT); }while(0)
#define __SHORT_FORM_OF_FILE__		\
	(strrchr(__FILE__, '/')			\
	? strrchr(__FILE__, '/') + 1	\
	: __FILE__						\
	)
#endif

#define ASSERT_LOG(_a, _b, ...)														\
	do {																			\
		if(!(_a)) {																	\
			spdlog::get("console")->critical(fmt::format("{}:{} : " _b "\n", __SHORT_FORM_OF_FILE__, __LINE__, __VA_ARGS__)); \
			DebuggerBreak();														\
			exit(1);																\
		}																			\
	} while(0)

#define LOG_INFO(_a, ...)															\
	do {																			\
		spdlog::get("console")->info(fmt::format("{}:{} : " _a "\n", __SHORT_FORM_OF_FILE__, __LINE__, __VA_ARGS__)); \
	} while(0)

#define LOG_DEBUG(_a, ...)															\
	do {																			\
		spdlog::get("console")->debug(fmt::format("{}:{} : " _a "\n", __SHORT_FORM_OF_FILE__, __LINE__, __VA_ARGS__)); \
	} while(0)

#define LOG_WARN(_a, ...)															\
	do {																			\
		spdlog::get("console")->warn(fmt::format("{}:{} : " _a "\n", __SHORT_FORM_OF_FILE__, __LINE__, __VA_ARGS__)); \
	} while(0)

#define LOG_ERROR(_a, ...)															\
	do {																			\
		spdlog::get("console")->error(fmt::format("{}:{} : " _a "\n", __SHORT_FORM_OF_FILE__, __LINE__, __VA_ARGS__)); \
	} while(0)
