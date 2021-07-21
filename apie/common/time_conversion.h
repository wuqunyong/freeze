#pragma once

#include <stdint.h>
#include <ctime>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace apie {

	inline uint64_t StringToUnixSeconds(const std::string& time_str)
	{
		//#include <iomanip>
		//std::get_time
		const std::string& format_str = "%d-%d-%d %d:%d:%d";

		struct tm* tmp_time = (struct tm*)malloc(sizeof(struct tm));
		//strptime(time_str.c_str(), format_str.c_str(), tmp_time);
		int iResult = sscanf(time_str.c_str(), format_str.c_str(),
			&tmp_time->tm_year, &tmp_time->tm_mon, &tmp_time->tm_mday,
			&tmp_time->tm_hour, &tmp_time->tm_min, &tmp_time->tm_sec);
		if (iResult != 6)
		{
			return 0;
		}

		tmp_time->tm_year -= 1900;
		tmp_time->tm_mon--;
		time_t time = mktime(tmp_time);
		free(tmp_time);
		return (uint64_t)time;
	}

	inline std::string UnixSecondsToString(uint64_t unix_seconds, const std::string& format_str = "%Y-%m-%d %H:%M:%S") 
	{
		//std::put_time

		std::time_t t = unix_seconds;
		//struct tm* pt = std::gmtime(&t);
		struct tm* pt = std::localtime(&t);
		if (pt == nullptr) 
		{
			return std::string("");
		}
		uint32_t length = 64;
		std::vector<char> buff(length, '\0');
		strftime(buff.data(), length, format_str.c_str(), pt);
		return std::string(buff.data());
	}

}


