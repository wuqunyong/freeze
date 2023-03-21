#include "apie/network/logger.h"

#include <time.h>
#include <map>
#include <sstream>
#include <mutex>

#ifdef WIN32

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "apie/network/ctx.h"
#include "apie/network/command.h"
#include "apie/filesystem/directory.h"
#include "apie/common/string_utils.h"


static std::mutex& GetLogSync() {
	static std::mutex log_sync;
	return log_sync;
}

static std::map<std::string, LogFile*>& GetCacheMap() {
	static std::map<std::string, LogFile*> cacheMap;
	return cacheMap;
}


std::string getLogLevelName(int level)
{
	std::string sLevelName;
	switch(level)
	{
	case PIE_DEBUG:
		{
			sLevelName = "PIE_DEBUG";
			break;
		}
	case PIE_VERBOSE:
		{
			sLevelName = "PIE_VERBOSE";
			break;
		}
	case PIE_NOTICE:
		{
			sLevelName = "PIE_INFO";
			break;
		}
	case PIE_WARNING:
		{
			sLevelName = "PIE_WARNING";
			break;
		}
	case PIE_ERROR:
		{
			sLevelName = "PIE_ERROR";
			break;
		}
	case PIE_PANIC:
	{
		sLevelName = "PIE_PANIC";
		break;
	}
	default:
		{
			char temp[64] = {'\0'};
			sprintf(temp,"Level:%d",level);
			sLevelName = temp;
		}
	}

	return sLevelName;
}

void pieLogRaw(const char* file, int cycle, int level, const char* msg)
{
	// 多线程，同时访问所以要加锁
	assert(file != NULL);

	std::lock_guard<std::mutex> guard(GetLogSync());

	std::string logFileName(file);
	LogFile* ptrFile = ptrFile = getCacheFile(logFileName, cycle);
		
	if (!ptrFile)
	{
		printf("getCacheFile %s error!", logFileName.c_str());
		return;
	}
	
	char timebuf[64]={'\0'};
	time_t now = apie::Ctx::getCurSeconds();
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S",localtime(&now));

	struct timeval tp;
	evutil_gettimeofday(&tp, NULL);
	uint64_t iMilliSecond = ((tp.tv_sec * (uint64_t) 1000000 + tp.tv_usec) / 1000);

	std::string sLevelName = getLogLevelName(level);
	fprintf(ptrFile->pFile, "%s|%llu|%s|%s\n", timebuf, (long long unsigned int)iMilliSecond, sLevelName.c_str(), msg);
	fflush(ptrFile->pFile);

	bool bShowConsole = apie::CtxSingleton::get().getConfigs()->log.show_console;
	if (bShowConsole || level >= PIE_ERROR)
	{
#ifdef WIN32
		switch (level)
		{
		case PIE_DEBUG:
		case PIE_VERBOSE:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		}
		case PIE_NOTICE:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			break;
		}
		case PIE_WARNING:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED);
			break;
		}
		case PIE_ERROR:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
			break;
		}
		case PIE_PANIC:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | BACKGROUND_GREEN);
			break;
		}
		default:
			break;
		}
		printf("%s|%llu|%s|fileName:%s|%s\n", timebuf, (long long unsigned int)iMilliSecond, sLevelName.c_str(), file, msg);

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#else
		if (!apie::CtxSingleton::get().getConfigs()->daemon)
		{
			printf("%s|%llu|%s|fileName:%s|%s\n", timebuf, (long long unsigned int)iMilliSecond, sLevelName.c_str(), file, msg);
		}
#endif
	}
}

//file="directory/filename"
//void pieLog(const char* file, int cycle, int level, const char *fmt, ...)
//{
//	va_list ap;
//	char msg[PIE_MAX_LOGMSG_LEN];
//
//	int iConfigLogLevel = apie::CtxSingleton::get().getConfigs()->log.level;
//	if ((level&0xff) < iConfigLogLevel)
//	{
//		return;
//	}
//
//	va_start(ap, fmt);
//	vsnprintf(msg, sizeof(msg), fmt, ap);
//	va_end(ap);
//
//	pieLogRaw(file,cycle,level,msg,false);
//}

//void asyncPieLog(const char* file, int cycle, int level, const char *fmt, ...)
//{
//	va_list ap;
//	char msg[PIE_MAX_LOGMSG_LEN] = {'\0'};
//
//	int iConfigLogLevel = apie::CtxSingleton::get().getConfigs()->log.level;
//	if ((level&0xff) < iConfigLogLevel)
//	{
//		return;
//	}
//
//	va_start(ap, fmt);
//	vsnprintf(msg, sizeof(msg), fmt, ap);
//	va_end(ap);
//
//	if (NULL == apie::CtxSingleton::get().getLogThread())
//	{
//		pieLogRaw(file, cycle, level, msg, false);
//		return;
//	}
//
//	apie::LogCmd* ptrCmd = new apie::LogCmd;
//	ptrCmd->sFile = file;
//	ptrCmd->iCycle = cycle;
//	ptrCmd->iLevel = level;
//	ptrCmd->sMsg = msg;
//	ptrCmd->bIgnoreMerge = false;
//
//	apie::Command cmd;
//	cmd.type = apie::Command::async_log;
//	cmd.args.async_log.ptrData = ptrCmd;
//	apie::CtxSingleton::get().getLogThread()->push(cmd);
//}

//void asyncPieLogIgnoreMerge(const char* file, int cycle, int level, const char *fmt, ...)
//{
//	va_list ap;
//	char msg[PIE_MAX_LOGMSG_LEN] = { '\0' };
//
//	int iConfigLogLevel = apie::CtxSingleton::get().getConfigs()->log.level;
//	if ((level & 0xff) < iConfigLogLevel)
//	{
//		return;
//	}
//
//	va_start(ap, fmt);
//	vsnprintf(msg, sizeof(msg), fmt, ap);
//	va_end(ap);
//
//	if (NULL == apie::CtxSingleton::get().getLogThread())
//	{
//		pieLogRaw(file, cycle, level, msg, true);
//		return;
//	}
//
//	apie::LogCmd* ptrCmd = new apie::LogCmd;
//	ptrCmd->sFile = file;
//	ptrCmd->iCycle = cycle;
//	ptrCmd->iLevel = level;
//	ptrCmd->sMsg = msg;
//	ptrCmd->bIgnoreMerge = true;
//
//	apie::Command cmd;
//	cmd.type = apie::Command::async_log;
//	cmd.args.async_log.ptrData = ptrCmd;
//	apie::CtxSingleton::get().getLogThread()->push(cmd);
//}

LogFile* openFile(std::string file, int cycle)
{
	LogFile* ptrFile = new LogFile;

	std::string sFile;

	std::string sCurPath = apie::filesystem::Directory::getCWD();
	
	//TODO
	//std::string sLogPrefix = "app";

#ifdef WIN32
	sFile = sCurPath;
	sFile.append("/logs/");
	sFile.append(file);
#else
	sFile = "/usr/local/apie/logs/";
	sFile.append(file);
#endif
	
	apie::ReplaceStrAll(sFile,"\\","/");

	std::string::size_type pos = sFile.rfind ("/");
	if (pos == std::string::npos) 
	{
		delete ptrFile;
		return NULL;
	}
	std::string sPath = sFile.substr(0, pos);
	std::string sFileName = sFile.substr(pos + 1);

	apie::filesystem::Directory::createDirectory(sPath.c_str());
	time_t now = apie::Ctx::getCurSeconds();
	struct tm * timeinfo;
	timeinfo = localtime(&now);

	//char file_postfix[64]={'\0'};
	//strftime(file_postfix, sizeof(file_postfix), "%Y%m%d%H-%M%S",localtime(&now));

	std::string sNewFileName;
	sNewFileName = sFileName;
	//sNewFileName.append(".");
	//sNewFileName.append(file_postfix);

	std::string sNewName;
	sNewName = sPath;
	sNewName.append("/");
	sNewName.append(sNewFileName);

	FILE * pFile = fopen(sNewName.c_str(),"a");
	if (!pFile)
	{
		delete ptrFile;
		return NULL;
	}

	ptrFile->pFile = pFile;
	ptrFile->sFile = sNewName;
	ptrFile->iCycle = cycle;
	ptrFile->iCreateYear = timeinfo->tm_year;
	ptrFile->iCreateMonth = timeinfo->tm_mon;
	ptrFile->iCreateDay = timeinfo->tm_mday;
	ptrFile->iCreateHour = timeinfo->tm_hour;
	ptrFile->iCreateMinute = timeinfo->tm_min;

	return ptrFile;
}

void closeFile(LogFile* ptrFile)
{
	fflush(ptrFile->pFile);
	fclose(ptrFile->pFile);
}

void moveFile(std::string from, std::string to)
{
	time_t now = apie::Ctx::getCurSeconds();
	char curDate[64] = { '\0' };
	strftime(curDate, sizeof(curDate), "%Y-%m-%d", localtime(&now));
	to.append(curDate);
	apie::filesystem::Directory::createDirectory(to.c_str());

	std::string baseName = apie::filesystem::Directory::basename(from.c_str());
	char filePostfix[64] = { '\0' };
	strftime(filePostfix, sizeof(filePostfix), "backup-%Y%m%d-%H%M", localtime(&now));
	std::string sNewFileName;
	sNewFileName = baseName;
	sNewFileName.append(".");
	sNewFileName.append(filePostfix);


	std::string target = to + "/" + sNewFileName;
	int result = rename(from.c_str(), target.c_str());
	if (result == 0)
	{
		asyncPieFmtLog("move/log", PIE_CYCLE_DAY, PIE_DEBUG, "rename successfully|{}  ->  {}", from, target);
	}
	else
	{
		asyncPieFmtLog("move/log", PIE_CYCLE_DAY, PIE_ERROR, "rename error|{}  ->  {}", from, target);
	}
}

void checkRotate()
{
	std::lock_guard<std::mutex> guard(GetLogSync());

	std::map<std::string, LogFile*>::iterator ite = GetCacheMap().begin();
	while (ite != GetCacheMap().end())
	{
		if (isChangeFile(ite->second, ite->second->iCycle))
		{
			std::string fromFile = ite->second->sFile;
			closeFile(ite->second);
			delete ite->second;

			std::map<std::string, LogFile*>::iterator o = ite;
			++ite;
			GetCacheMap().erase(o);

			std::string toDir = apie::CtxSingleton::get().getConfigs()->log.backup;
#ifdef WIN32
			toDir = "D:/backup";
#endif
			moveFile(fromFile, toDir);
			continue;
		}

		++ite;
	}
}

void logFileClose()
{
	std::lock_guard<std::mutex> guard(GetLogSync());

	std::map<std::string, LogFile*>::iterator ite = GetCacheMap().begin();
	while (ite != GetCacheMap().end())
	{
		closeFile(ite->second);
		delete ite->second;

		std::map<std::string, LogFile*>::iterator o = ite;
		++ite;
		GetCacheMap().erase(o);
	}
}

LogFile* getCacheFile(std::string file, int cycle)
{
	std::map<std::string, LogFile*>::iterator ite = GetCacheMap().find(file);
	if (ite != GetCacheMap().end())
	{
		return ite->second;
	}

	LogFile* ptrFile = openFile(file, cycle);
	if (!ptrFile)
	{
		return NULL;
	}
	
	GetCacheMap()[file] = ptrFile;
	return ptrFile;
}

bool isChangeFile(LogFile* ptrFile, int cycle)
{
#ifdef WIN32
	int fileFd = _fileno(ptrFile->pFile);
#else
	int fileFd = fileno(ptrFile->pFile);
#endif
	struct stat statInfo;
	if (0 == fstat(fileFd, &statInfo))
	{
		int32_t iSize = apie::CtxSingleton::get().getConfigs()->log.split_size;
		if (statInfo.st_size > iSize *1240 * 1240)
		{
			return true;
		}
	}

	time_t now = apie::Ctx::getCurSeconds();
	struct tm * timeinfo;
	timeinfo = localtime(&now);

	if (ptrFile->iCreateYear != timeinfo->tm_year || ptrFile->iCreateMonth != timeinfo->tm_mon)
	{
		return true;
	}

	switch (cycle)
	{
	case PIE_CYCLE_DAY:
		if (ptrFile->iCreateDay != timeinfo->tm_mday)
		{
			return true;
		}
		break;
	case PIE_CYCLE_HOUR:
		if (ptrFile->iCreateDay != timeinfo->tm_mday || ptrFile->iCreateHour != timeinfo->tm_hour)
		{
			return true;
		}
		break;
	case PIE_CYCLE_MINUTE:
		if (ptrFile->iCreateDay != timeinfo->tm_mday || ptrFile->iCreateHour != timeinfo->tm_hour || ptrFile->iCreateMinute != timeinfo->tm_min)
		{
			return true;
		}
		break;
	}
	
	return false;
}

//void fatalExit(const char* message)
//{
//	fprintf(stderr, "%s: %s\n", "fatalExit", message);
//	pieLog("Exception/Exception", PIE_CYCLE_HOUR, PIE_ERROR, "%s: %s\n", "fatalExit", message);
//
//	//exit(EXIT_FAILURE);
//	abort();
//}
