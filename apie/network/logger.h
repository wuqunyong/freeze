#pragma once

#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include <string_view>

#include <fmt/core.h>


#include "apie/network/ctx.h"

/* Log levels */
#define PIE_DEBUG 0
#define PIE_VERBOSE 1
#define PIE_NOTICE 2
#define PIE_WARNING 3
#define PIE_ERROR 4
#define PIE_PANIC 5

#define PIE_CYCLE_MINUTE 0
#define PIE_CYCLE_HOUR 1
#define PIE_CYCLE_DAY 2

#define PIE_MAX_LOGMSG_LEN    1024*256 /* Default maximum length of syslog messages */


// Convert the line macro to a string literal for concatenation in log macros.
#define DO_STRINGIZE(x) STRINGIZE(x)
#define STRINGIZE(x) #x
#define LINE_STRING DO_STRINGIZE(__LINE__)
#define LOG_PREFIX "[" __FILE__ ":" LINE_STRING "]"


#ifndef MODULE_NAME
#define MODULE_NAME apie::Ctx::GetLogName().c_str()
#endif

struct LogFile
{
	FILE * pFile;
	std::string sFile;
	int iCycle;
	int iCreateYear;
	int iCreateMonth;
	int iCreateDay;
	int iCreateHour;
	int iCreateMinute;
}; 

std::string getLogLevelName(int level);
void pieLogRaw(const char* file, int cycle, int level, const char* msg);

LogFile* openFile(std::string file, int cycle);
void closeFile(LogFile* ptrFile);
void moveFile(std::string from, std::string to);
void checkRotate();

LogFile* getCacheFile(std::string file, int cycle);
bool isChangeFile(LogFile* ptrFile, int cycle);

void logFileClose();


template <class... Args>
void pieFmtLog(std::string_view fileName, int cycle, int level, std::string_view fmt, Args&&... args)
{
	//std::string msg = std::vformat(fmt, std::make_format_args(args...));
	std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
	
	std::string sFileName(fileName.data(), fileName.size());
	pieLogRaw(sFileName.c_str(), cycle, level, msg.c_str());
}


template <class... Args>
void asyncPieFmtLog(std::string_view fileName, int cycle, int level, std::string_view fmt, Args&&... args)
{
	int iConfigLogLevel = apie::CtxSingleton::get().getConfigs()->log.level;
	if ((level & 0xff) < iConfigLogLevel)
	{
		return;
	}

	//std::string msg = std::vformat(fmt, std::make_format_args(args...));

	std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
	std::string sFileName(fileName.data(), fileName.size());

	if (NULL == apie::CtxSingleton::get().getLogThread())
	{
		pieLogRaw(sFileName.c_str(), cycle, level, msg.c_str());
		return;
	}

	apie::LogCmd* ptrCmd = new apie::LogCmd;
	ptrCmd->sFile = sFileName;
	ptrCmd->iCycle = cycle;
	ptrCmd->iLevel = level;
	ptrCmd->sMsg = msg;
	ptrCmd->bIgnoreMerge = false;

	apie::Command cmd;
	cmd.type = apie::Command::async_log;
	cmd.args.async_log.ptrData = ptrCmd;
	apie::CtxSingleton::get().getLogThread()->push(cmd);
}

template <class... Args>
void asyncPieFmtLogIgnoreMerge(std::string_view fileName, int cycle, int level, std::string_view fmt, Args&&... args)
{
	int iConfigLogLevel = apie::CtxSingleton::get().getConfigs()->log.level;
	if ((level & 0xff) < iConfigLogLevel)
	{
		return;
	}

	//std::string msg = std::vformat(fmt, std::make_format_args(args...));

	std::string msg = fmt::format(fmt, std::forward<Args>(args)...);
	std::string sFileName(fileName.data(), fileName.size());
	if (NULL == apie::CtxSingleton::get().getLogThread())
	{
		pieLogRaw(sFileName.c_str(), cycle, level, msg.c_str());
		return;
	}

	apie::LogCmd* ptrCmd = new apie::LogCmd;
	ptrCmd->sFile = sFileName;
	ptrCmd->iCycle = cycle;
	ptrCmd->iLevel = level;
	ptrCmd->sMsg = msg;
	ptrCmd->bIgnoreMerge = true;

	apie::Command cmd;
	cmd.type = apie::Command::async_log;
	cmd.args.async_log.ptrData = ptrCmd;
	apie::CtxSingleton::get().getLogThread()->push(cmd);
}


#ifdef WIN32
#define PANIC_ABORT(format, ...) do { \
	std::string formatStr(LOG_PREFIX "|"); \
	formatStr = formatStr + format; \
	pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, PIE_PANIC, formatStr, __VA_ARGS__); \
	abort(); \
} while (0);
#else
#define PANIC_ABORT(format, args...) do { \
	std::string formatStr(LOG_PREFIX "|"); \
	formatStr = formatStr + format; \
	pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, PIE_PANIC, formatStr, ##args); \
	abort(); \
} while (0);
#endif


#ifdef WIN32
#define PIE_LOG(level, format, ...) do { \
    bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, formatStr, __VA_ARGS__); \
	} \
	else \
	{ \
		pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, format, __VA_ARGS__); \
	} \
} while (0);
#else
#define PIE_LOG(level, format, args...) do { \
	bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, formatStr, ##args); \
	} \
	else \
	{ \
		pieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, format, ##args); \
	} \
} while (0);
#endif


#ifdef WIN32
#define ASYNC_PIE_LOG(level, format, ...) do { \
	bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		asyncPieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, formatStr, __VA_ARGS__); \
	} \
	else \
	{ \
		asyncPieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, format, __VA_ARGS__); \
	} \
} while (0);
#else
#define ASYNC_PIE_LOG(file, cycle, level, format, args...) do { \
	bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		asyncPieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, formatStr, ##args); \
	} \
	else \
	{ \
		asyncPieFmtLog(MODULE_NAME, PIE_CYCLE_DAY, level, format, ##args); \
	} \
} while (0);
#endif

#ifdef WIN32
#define ASYNC_PIE_LOG_CUSTOM(file, level, format, ...) do { \
	bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		asyncPieFmtLogIgnoreMerge(file, PIE_CYCLE_DAY, level, formatStr, __VA_ARGS__); \
	} \
	else \
	{ \
		asyncPieFmtLogIgnoreMerge(file, PIE_CYCLE_DAY, level, format, __VA_ARGS__); \
	} \
} while (0);
#else
#define ASYNC_PIE_LOG_CUSTOM(file, level, format, args...) do { \
	bool bShowPos = apie::CtxSingleton::get().getConfigs()->log.show_pos; \
	if (bShowPos) \
	{ \
		std::string formatStr(LOG_PREFIX "|"); \
		formatStr = formatStr + format; \
		asyncPieFmtLogIgnoreMerge(file, PIE_CYCLE_DAY, level, formatStr, ##args); \
	} \
	else \
	{ \
		asyncPieFmtLogIgnoreMerge(file, PIE_CYCLE_DAY, level, format, ##args); \
	} \
} while (0);
#endif


template <typename ...Args>
std::string SSImpl(Args&& ...args)
{
	std::ostringstream ss;

	(ss << ... << std::forward<Args>(args));

	return ss.str();
}