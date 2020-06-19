#include "string_util.h"
#include <sstream>

//////////////////////////////////
std::vector<std::string> msg2args(const char* msg, int max)
{
	std::vector<std::string> query;
	if (msg == nullptr) return query;
	std::stringstream ss(msg);
	int count = 1;
	while (ss && count < max)
	{
		std::string s;
		ss >> s;
		if (!s.empty())
		{
			query.push_back(s);
			++count;
		}
	}
	if (!ss.eof())
	{
		while (!ss.eof())
		{
			char c = ss.get();
			if (c != ' ' && c != '\t')
			{
				ss.unget();
				break;
			}
		}
		char buf[256];
		ss.getline(buf, sizeof(buf));
		std::string s(buf);
		query.push_back(s);
	}
	return query;
}

std::string strip(const std::string& s)
{
	int end = (int)s.length() - 1;
	int start = 0;
	while (start < (int)s.length() - 1 && (s[start] == ' ')) ++start;
	while (end > start && (s[end] == ' ' || s[end] == '\n' || s[end] == '\r')) --end;
	return s.substr(start, end + 1 - start);
}


int64_t stripAt(const std::string& s)
{
	if (s.find("[CQ:at,qq=") == 0 && s.rfind("]") != -1)
	{
		try {
			return std::stoll(s.substr(10, s.length() - 11));
		}
		catch (std::exception&) {
			return 0;
			//ignore
		}
	}
	else
	{
		return 0;
	}
}