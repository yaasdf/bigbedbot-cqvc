#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace opencase
{

enum class commands : size_t {
	��ͨ��ʾ,
	��ͨ,
	���,
	����,
	���,
	����,
	����,
	����,
	����10,
	������,
	������,
	����endless,

};
typedef std::function<std::string(::int64_t, ::int64_t, std::vector<std::string>&, const char*)> callback;
//typedef std::string(*callback)(::int64_t, ::int64_t, std::vector<std::string>);
struct command
{
	commands c = (commands)0;
	std::vector<std::string> args;
	callback func = nullptr;
};

inline std::map<std::string, commands> commands_str
{
	{"����", commands::����},
	{"�_��", commands::����},  //���w��
	{"����ʮ��", commands::����10},
	{"�_��ʮ�B", commands::����10},  //���w��
	{"������", commands::������},
	{"�_�S��", commands::������},  //���w��
	{"������", commands::������},
	{"�_�t��", commands::������},  //���w��
	{"�������", commands::����endless},
	{"��������", commands::����endless},  //�����FF14�ķ�����[����]
	{"�_�����", commands::����endless},  //���w��
	{"�_������", commands::����endless},  //���w��

};
command msgDispatcher(const char* msg);

////////////////////////////////////////////////////////////////////////////////
// ����
const int FEE_PER_CASE = 17;
class case_type
{
private:
	std::string _name;
	//int _worth;
	double _prob;
public:
	case_type(std::string n, /*int w,*/ double p) : _name(n), /*_worth(w),*/ _prob(p) {}
	std::string name() const { return _name; }
	//int worth() const { return _worth; }
	double prob() const { return _prob; }
	//operator int() const { return worth(); }
};
inline const std::vector<case_type> CASE_POOL
{
	{ "����", /*-255, */  0.003 },
	//{ "����", /*10000,*/  0.0006 },
	{ "����", /*1000, */  0.0045 },
	{ "����", /*200,  */  0.01 },
	{ "����", /*5,    */  0.03 },
	{ "����", /*2,    */  0.17 },
};
inline const case_type CASE_DEFAULT{ "����", /*1,*/ 1.0 };

class case_detail
{
private:
	size_t _type_idx;
	std::string _name;
	int _worth = 0;
public:
	case_detail() : _type_idx(CASE_POOL.size()) {}
	case_detail(size_t ty, std::string n, int w) : _type_idx(ty), _name(n), _worth(w) {}
	size_t type_idx() const { return _type_idx; }
	const case_type& type() const { return (_type_idx < CASE_POOL.size() ? CASE_POOL[_type_idx] : CASE_DEFAULT); }
	std::string name() const { return _name; }
	std::string part() const { using namespace std::string_literals; return !_name.empty() ? _name : type().name(); }
	std::string full() const { using namespace std::string_literals; return type().name() + (!_name.empty() ? (" [ "s + _name + " ]"s) : ""); }
	int worth() const { return _worth; }
	operator int() const { return worth(); }
};

const case_detail& draw_case(double p);

////////////////////////////////////////////////////////////////////////////////
// �����趨
const int COST_OPEN_RED = 50;
const int COST_OPEN_YELLOW = 255;

const int COST_OPEN_ENDLESS = 50;

const int COST_OPEN_RED_STAMINA = 6;
const int COST_OPEN_ENDLESS_STAMINA = 7;

extern const std::vector<std::vector<case_detail>> CASE_DETAILS;
}