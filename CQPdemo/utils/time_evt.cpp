
#include <thread>
#include <memory>
#include <chrono>

#include "time_evt.h"
#include "appmain.h"
#include "time_util.h"

std::vector<std::function<void()>> timedEventQueue[24][60];

bool timedThreadRunning = false;
void timedEventLoop()
{
	using namespace std::chrono_literals;
	int prev_hr = 0, prev_min = 0;
	while (gBotEnabled && timedThreadRunning)
	{
		auto t = getLocalTime(TIMEZONE_HR, TIMEZONE_MIN, 0);
		if (prev_min == t.tm_min)
		{
			std::this_thread::sleep_for(30s);
			continue;
		}

		for (auto& f : timedEventQueue[t.tm_hour][t.tm_min])
		{
			if (f) f();
		}

		prev_hr = t.tm_hour;
		prev_min = t.tm_min;
		std::this_thread::sleep_for(1min);
	}
}

std::shared_ptr<std::thread> timedEventThread = nullptr;

void startTimedEvent()
{
	if (!timedEventThread)
	{
		timedEventThread = std::make_shared<std::thread>(timedEventLoop);
		timedEventThread->detach();
	}
}

void stopTimedEvent()
{
	timedThreadRunning = false;
	timedEventThread = nullptr;
}


void addTimedEvent(std::function<void()> f, int hour, int min)
{
	if (hour < 0 || hour > 23) return;
	if (min < 0  || min > 59)  return;
	timedEventQueue[hour][min].push_back(f);
}

void clearTimedEvent()
{
	for (int h = 0; h < 24; ++h)
		for (int m = 0; m < 60; ++m)
		{
			timedEventQueue[h][m].clear();
		}
}