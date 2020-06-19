#pragma once
#include <map>
#include <vector>
#include <functional>

#include "app/common/dbconn.h"

void startTimedEvent();
void stopTimedEvent();

void addTimedEvent(std::function<void()> f, int hour, int min);

void clearTimedEvent();