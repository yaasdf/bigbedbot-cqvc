#pragma once
#include <vector>
#include <string>

// str.split(' ', '\t')
std::vector<std::string> msg2args(const char* msg, int max = INT_MAX);

// str.strip(' ', '\r', '\n')
std::string strip(const std::string& s);

// [CQ:at,qq=xxxxxxxxxx]
int64_t stripAt(const std::string& s);

// ?
std::string stripImage(const std::string& s);

// ?
std::string stripFace(const std::string& s);
