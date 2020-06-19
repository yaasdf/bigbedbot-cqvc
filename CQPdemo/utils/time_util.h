#pragma once
#include <ctime>
inline const int TIMEZONE_HR = 8;
inline const int TIMEZONE_MIN = 0;
inline std::tm getLocalTime(int timezone_hr, int timezone_min, time_t offset = 0)
{
	auto t = time(nullptr) + offset;
	t += timezone_hr * 60 * 60 + timezone_min * 60;

	std::tm tm;
	gmtime_s(&tm, &t);
	return tm;
}
