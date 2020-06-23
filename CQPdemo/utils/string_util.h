#pragma once
#include <vector>
#include <string>

// str.split(' ', '\t')
std::vector<std::string> msg2args(const char* msg);

// split first keyword without space
// len > 0: get len char of first word from left
// len < 0: get -len char of first word from right
std::vector<std::string> msg2args(const char* msg, int commandLen);

// str.strip(' ', '\r', '\n')
std::string strip(const std::string& s);

// [CQ:at,qq=xxxxxxxxxx]
int64_t stripAt(const std::string& s);

// ?
std::string stripImage(const std::string& s);

// ?
std::string stripFace(const std::string& s);
