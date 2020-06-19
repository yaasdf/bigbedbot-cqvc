#pragma once
#include <cstdint>
#include <string>

class simple_str
{
private:
	int16_t _len = 0;
	char* _data = NULL;

public:
	int16_t length() const { return _len; }
	const char* c_str() const { return _data; }
	std::string operator()()
	{
		return std::string(_data);
	}

	simple_str() {}
	simple_str(int16_t len, const char* str) : _len(len)
	{
		_data = new char[_len + 1];
		memcpy_s(_data, _len + 1, str, _len + 1);
		_data[_len] = 0;
	}
	simple_str(const char* str) : simple_str(int16_t(strlen(str)), str) {}
	simple_str(const std::string& str) : simple_str(int16_t(str.length()), str.c_str()) {}
	~simple_str() { if (_data) delete _data; }
	simple_str(const simple_str& str) : simple_str(str.length(), str.c_str()) {}
	simple_str& operator=(const simple_str& str)
	{
		if (_data) delete _data;
		_len = str.length();
		_data = new char[_len + 1];
		memcpy_s(_data, _len + 1, str.c_str(), _len + 1);
		_data[_len] = 0;
		return *this;
	}
	operator std::string() const { return std::string(_data); }
};
