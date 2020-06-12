#pragma once
#include <vector>
#include <map>
#include <list>
#include <string>
#include <functional>
#include <random>
#include <iostream>
#include <ctime>
#include <tuple>
#include <bitset>
#include "dbconn.h"
namespace pee {

inline SQLite db("pee.db", "pee");

void modifyCurrency(int64_t qq, int64_t c);
void modifyBoxCount(int64_t qq, int64_t c);
void modifyDrawTime(int64_t qq, time_t c);
void modifyKeyCount(int64_t qq, int64_t c);

enum class commands: size_t {
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
    {"��ͨ", commands::��ͨ��ʾ},
    {"�_ͨ", commands::��ͨ��ʾ},  //���w��
    {"��ͨ����", commands::��ͨ��ʾ},
    {"�_ͨ����", commands::��ͨ��ʾ},  //���w��
    {"���ҿ�ͨ����", commands::��ͨ��ʾ},
    {"�o���_ͨ����", commands::��ͨ��ʾ},  //���w��
    {"ע��", commands::��ͨ��ʾ},
    {"�]��", commands::��ͨ��ʾ},  //���w��
    {"ע�Ფ��", commands::��ͨ��ʾ},
    {"�]�Բ���", commands::��ͨ��ʾ},  //���w��
    {"��Ҫע�Ფ��", commands::��ͨ��ʾ},
    {"��Ҫ�]�Բ���", commands::��ͨ��ʾ},  //���w��
    {"��Ҫ��ͨ����", commands::��ͨ},
    {"��Ҫ�_ͨ����", commands::��ͨ},  //���w��
    {"���", commands::���},
    {"�N�~", commands::���},  //���w��
    {"����", commands::����},  //���w���wһ��
    {"����", commands::����},
    {"����", commands::����},  //���w��
    {"����", commands::����},  //����������д��
    {"��", commands::���},  //���w���wһ��
    {"����", commands::����},
    {"�I��", commands::����},  //���w��
    //{"��������", commands::����},
    //{"����", commands::����},
    //{"����", commands::����},
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

command smokeIndicator(const char* msg);
extern std::map<std::string, int64_t> qqid_str;


command msgDispatcher(const char* msg);
std::tuple<bool, int, time_t> updateStamina(int64_t qq, int cost = 0, bool extra = false);
std::tuple<bool, int, time_t> testStamina(int64_t qq, int cost = 0);
////////////////////////////////////////////////////////////////////////////////
// p
struct pdata
{
    int64_t currency;
    int64_t opened_box_count;
    time_t  last_draw_time;
    int64_t keys;

    // flags
    time_t  freeze_assets_expire_time;
    bool    meteor_shield;
    int     air_pump_count;
    int     air_ignore_count;
    time_t  adrenaline_expire_time;
    bool    chaos;

    // counter
    //int     event_drop_count;

};
inline std::map<int64_t, pdata> plist;
void peeCreateTable();
void peeLoadFromDb();

const int INITIAL_BALANCE = 200;

inline std::map<int64_t, int64_t> prevUser;

////////////////////////////////////////////////////////////////////////////////
// �ӽ�
int64_t nosmoking(int64_t group, int64_t target, int duration);
std::string nosmokingWrapper(int64_t qq, int64_t group, int64_t target, int64_t cost);
std::string unsmoke(int64_t qq);
inline std::map<int64_t, std::map<int64_t, time_t>> smokeGroups;


////////////////////////////////////////////////////////////////////////////////
// ��p
const int FREE_BALANCE_ON_NEW_DAY = 10;
const int NEW_DAY_TIME_HOUR = 11;
const int NEW_DAY_TIME_MIN = 0;


inline int extra_tomorrow = 0;

inline time_t daily_refresh_time;   // ��������ʱ�䣬�������ʱ�����ҪС�����ʱ��
inline std::tm daily_refresh_tm;
inline std::tm daily_refresh_tm_auto;
const int DAILY_BONUS = 100;
inline int remain_daily_bonus;

void flushDailyTimep(bool autotriggered = false);

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

inline const std::vector<std::vector<case_detail>> CASE_DETAILS
{
    // �ڶ�
    {
        {0, "[CQ:emoji,id=128201][CQ:emoji,id=128371]", -10},
        {0, "һ����ʲ���", -50},
        {0, "Dropbox(R)", -99},
        {0, "L9 | ��Ȥ�Ѩ", -200},
        {0, "�����", -500},
        {0, "���ҷɵ�������", -1024},
        {0, "�ڶ�", -9999}
    },

    /*
    // ����
    {
        {"", 10000}
    },
    */

    // ����
    {
        {1, "צ�ӵ���� | ������ (95% Blue)", 50000},
        {1, "���צ����� | ������ (Ruby)", 15000},
        {1, "M9�̵���� | ٤�������� (Emerald)", 13000},
        {1, "��������� | ������ (Black Pearl)", 7500},
        {1, "צ�ӵ���� | �Զ���", 6888},
        {1, "��������� | �������ʯ", 5930},
        {1, "���צ����� | ����", 5200},
        {1, "�ܵ���� | ����֮ɫ", 4360},
        {1, "M9 �̵���� | ٤�������", 3180},
        {1, "צ�ӵ���� | ������", 3100},
        {1, "�̽���� | ������", 2580},
        {1, "���צ�����", 2500},
        {1, "���������", 2500},
        {1, "צ�ӵ����", 2250},
        {1, "�̵���� | ����", 2160},
        {1, "�̵����", 1880},
        {1, "�䵶��� | �������ʯ", 1850},
        {1, "�۵���� | ������", 1780},
        {1, "�ܵ����", 1760},
        {1, "M9�̵����", 1700},
        {1, "�����Ե���� | ����", 1490},
        {1, "��������� | �ݽ�֮ɫ", 1360},
        {1, "���צ����� | ����αװ", 1270},
        {1, "M9 �̵���� | �γ�֮ˮ", 1210},
        {1, "�̽����", 1150},
        {1, "�̵���� | ����", 1100},
        {1, "��ɱ��ذ�ף��", 1070},
        {1, "��������� | �Զ���", 980},
        {1, "M9 �̵���� | �ݽ�֮ɫ", 860},
        {1, "�����Ե����", 850},
        {1, "�۵������", 830},
        {1, "�����Ե���� | ����ʿ���", 750},
        {1, "�̽���� | ҹɫ", 700},
        {1, "��Ӱ˫ذ���", 680},
        {1, "��������� | ����", 660},
        {1, "�䵶���", 650},
        {1, "�䵶��� | ����", 630},
        {1, "��������� | ���֮��", 620},
        {1, "�۵����", 610},
        {1, "�̽���� | ����ɭ��", 600},
        {1, "�۵������ | ����αװ ", 580},
        {1, "���������", 560},
        {1, "�����Ե���� | ��������", 560},
        {1, "��������� | ����֮��", 560},
        {1, "��Ӱ˫ذ��� | ����ʿ���", 530},
        {1, "�۵���� | �˹�Ⱦɫ", 510},
        {1, "�䵶��� | �ݽ�֮ɫ ", 500},
        {1, "��������� | ����������", 460},
        {1, "��������� | �ݽ�֮ɫ", 430},
        {1, "��Ӱ˫ذ��� | �������� ", 430},

    },

    // ����
    {
        {2, "AK-47 | ����", 2910},
        {2, "M4A4 | �ʵ�", 672},
        {2, "AWP | �׻�", 600},
        {2, "AK-47 | �ʺ�", 495},
        {2, "M4A4 | ����Ī��", 483},
        {2, "AK-47 | ����Ī��", 481},
        {2, "AWP | ������", 457},
        {2, "AK-47 | ����", 454},
        {2, "M4A4 | ��ɫ��Ӱ", 448},
        {2, "AK-47 | ȼ��������", 446},
        {2, "AWP | ��ɫ��Ӱ", 396},
        {2, "USP ������ | ǹ������", 395},
        {2, "AK-47 | Ѫ���˶�", 370},
        {2, "AK-47 | �޺���ʿ", 369},
        {2, "AWP | ����Ī��", 288},
        {2, "AWP | ��ŭҰ��", 272},
        {2, "P90 | ��֮��ɱ", 271},
        {2, "AK-47 | �����", 243},
        {2, "USP ������ | ��ɫ��Ӱ", 243},
        {2, "AK-47 | ��Ұ����", 235},
        {2, "AK-47 | �޺����", 235},
        {2, "M4A1 ������ | Ů����֮����", 222},
        {2, "FN57 | ��ŭҰ��", 216},
        {2, "ɳĮ֮ӥ | ��ɫ����", 205},
        {2, "M4A4 | �ʼ�ʥ��ʿ", 183},
        {2, "M4A1 ������ | ���߲���", 167},
        {2, "AK-47 | �����ͻ�", 159},
        {2, "������ AR | ��������", 158},
        {2, "SSG 08 | ˮ��֮Ѫ", 145},
        {2, "M4A1 ������ | ��ŭҰ��", 145},
        {2, "M4A4 | ����ɱ¾", 133},
        {2, "M4A1 ������ | ��е��ҵ", 127},
        {2, "SSG 08 | ����֮��", 122},
        {2, "P90 | ����Ī��", 97},
        {2, "ɳĮ֮ӥ | �ƽ����", 90},
        {2, "M4A4 | ����", 73},
        {2, "AWP | ��ηս��", 73},
        {2, "FN57 | ŭå", 69},
        {2, "P250 | ����������", 69},
        {2, "����� 18 �� | ��Ұ����", 68},
        {2, "M4A1 ������ | ��ʱ��", 64},
        {2, "P2000 | ����", 56},
        {2, "����˹ | ������", 55},
        {2, "M4A4 | ս��֮��", 54},
        {2, "CZ75 �Զ���ǹ | ά������", 53},
        {2, "MAC-10 | �޺���ʿ", 51},
        {2, "MP7 | Ѫ���˶�", 46},
        {2, "PP-Ұţ | ��Ŭ��˹֮����", 46},
        {2, "R8 ������ǹ | ����֮ɫ", 40},
        {2, "M4A4 | ɳĮ��Ӣ", 39},
        {2, "AUG | ��ɫ��", 39},
        {2, "�ض�����ǹ | ��������", 33},
    },

    // ����
    {
        {3, "AK-47 | �����ʺ", 343},
        {3, "P2000 | ��֮��ĭ", 300},
        {3, "AWP | ʯī��", 300},
        {3, "P90 | ���֮��", 288},
        {3, "AK-47 | �������", 188},
        {3, "AWP | *��*", 164},
        {3, "AK-47 | ǰ������", 153},
        {3, "AUG | ����", 135},
        {3, "AK-47 | ��ɫ��ѹ��", 125},
        {3, "XM1014 | �ٽ�֮��", 119},
        {3, "M4A4 | ��������", 118},
        {3, "M4A4 | ���ſռ�", 105},
        {3, "M4A4 | ����", 86},
        {3, "ɳĮ֮ӥ | �ƻ�֮ɫ", 85},
        {3, "AWP | ����", 80},
        {3, "AK-47 | ����", 80},
        {3, "P90 | ��Ѫɱ��", 79},
        {3, "FN57 | ˣ���Ϸ", 76},
        {3, "M4A1 ������ | ����", 70},
        {3, "AUG | �ϼ����ͻ�", 69},
        {3, "ɳĮ֮ӥ | ������", 69},
        {3, "SG 553 | ��ʱ��", 66},
        {3, "������ AR | ~�����~", 60},
        {3, "AUG | ϯ��.�׵�", 60},
        {3, "USP ������ | Ѫ��", 59},
        {3, "AUG | ��Ⱥ", 58},
        {3, "˫�ֱ����� | ����Ϯ��", 54},
        {3, "ɳĮ֮ӥ | ��е��ҵ", 51},
        {3, "MP5-SD | �׹���", 48},
        {3, "UMP-45 | ����", 47},
        {3, "M4A1 ������ | ������ 2000", 47},
        {3, "����� 18 �� | ˮ��", 46},
        {3, "P250 | ������", 44},
        {3, "AK-47 | ���ض�", 44},
        {3, "AWP | ��������", 44},
        {3, "P250 | ����", 43},
        {3, "M4A1 ������ | �ػ���", 42},
        {3, "USP ������ | ������", 40},
        {3, "AWP | ��Ӣ֮��", 40},
        {3, "G3SG1 | ������", 40},
        {3, "G3SG1 | ��¬��˹", 39},
        {3, "P90 | ����", 38},
        {3, "AWP | ����", 38},
        {3, "M4A1 ������ | ����Ǧ��", 38},
        {3, "���� | ӣ��֮Ѥ��", 37},
        {3, "XM1014 | ����", 37},
        {3, "SSG 08 | ����", 37},
        {3, "CZ75 �Զ���ǹ | ÷��ʱ��", 36},
        {3, "���� | ��ŭҰ��", 36},
        {3, "AUG | ������", 36},
        {3, "SCAR-20 | ��ʱ��", 35},
        {3, "AWP | ɺ����", 35},
        {3, "P90 | ����ľ��", 35},
        {3, "CZ75 �Զ���ǹ | ����", 35},
        {3, "PP-Ұţ | ������", 35},
        {3, "�ض�����ǹ | ����", 35},
        {3, "CZ75 �Զ���ǹ | �Ƽп�", 34},
        {3, "����˹ | �ŵ���֮��", 34},
        {3, "UMP-45 | Ұ�����ݻ�", 34},
        {3, "M4A1 ������ | ԭ�ӺϽ�", 32},
        {3, "R8 ������ǹ | ��������", 31},
        {3, "���� | �Ŷ�ǹ", 30},
        {3, "SCAR-20 | ������", 29},
        {3, "MP7 | ������", 29},
        {3, "P250 | ���ض�", 26},
        {3, "����˹ | ��е��ҵ", 26},
        {3, "������ AR | �����á���³��", 25},
        {3, "SCAR-20 | Ѫ���˶�", 25},
        {3, "Tec-9 | ȼ��������", 25},
        {3, "����˹ | ����", 24},
        {3, "MP9 | ����", 24},
        {3, "P250 | �����ֻ�", 23},
        {3, "ɳĮ֮ӥ | ��ı��", 23},
        {3, "P90 | ǳ��", 23},
        {3, "P2000 | ��������", 23},
        {3, "�ض�����ǹ | ��Ұ����", 22},
        {3, "P2000 | ɺ����", 21},
        {3, "R8 ������ǹ | ͷ�Ƿ�����", 21},
        {3, "FN57 | ��������", 20},
        {3, "P250 | ����Ī��", 20},
        {3, "USP ������ | �Զ���", 8},
    },

    // ����
    {
        {4, "ɳĮ֮ӥ | ���֮��", 86},
        {4, "AK-47 | ��� Mk01", 73},
        {4, "USP ������ | �������", 70},
        {4, "USP ������ | ��ˮ", 60},
        {4, "M4A1 ������ | ��ˮ", 60},
        {4, "M4A4 | �Ǽ�", 48},
        {4, "MAC-10 | ����ĥ��", 48},
        {4, "M4A1 ������ | �γ�֮ˮ", 45},
        {4, "�ض�����ǹ | �Ȼ� DDPAT", 42},
        {4, "P250 | ��Ұ�ɽ�", 42},
        {4, "����� 18 �� | ��������", 40},
        {4, "AWP | ����", 35},
        {4, "SG 553 | ��������", 34},
        {4, "FN57 | �����ʺ", 33},
        {4, "SG 553 | ���ƶ�", 31},
        {4, "������ AR | �Ȼ� DDPAT", 30},
        {4, "M4A4 | ʨ��", 29},
        {4, "M4A4 | ɱ�����", 28},
        {4, "MP9 | ��Ѫõ��", 26},
        {4, "�ڸ�� | ����װ�ػ�", 26},
        {4, "AWP | èè����", 25},
        {4, "����˹ | ��������", 20},
        {4, "M4A1 ������ | ����", 20},
        {4, "USP ������ | ��ʱ��", 20},
        {4, "����� 18 �� | ������¹�", 20},
        {4, "AUG | ����", 18},
        {4, "SSG 08 | �������", 15},
        {4, "���� | ��������", 15},
        {4, "MP9 | �ƻ�֮ɫ", 15},
        {4, "ɳĮ֮ӥ | �Ų�", 15},
        {4, "AWP | ������", 15},
        {4, "P2000 | �ּ�����", 14},
        {4, "P90 | ����֮��", 14},
        {4, "ɳĮ֮ӥ | ���", 14},
        {4, "FN57 | ��ǹϦʰ", 14},
        {4, "˫�ֱ����� | ����ʿ", 13},
        {4, "P250 | ����", 12},
        {4, "����˹ | ��ʿ", 12},
        {4, "˫�ֱ����� | Ѫ�쵰��", 11},
        {4, "USP ������ | �ػ���", 11},
        {4, "MP7 | ��֮��ĭ", 10},
        {4, "USP ������ | ����", 10},
        {4, "M4A1 ������ | ����", 10},
        {4, "ɳĮ֮ӥ | ��������", 10},
        {4, "UMP-45 | ����֮ʱ", 10},
        {4, "AUG | ����", 10},
        {4, "MAC-10 | ����", 9},
        {4, "R8 ������ǹ | ��������", 9},
        {4, "PP-Ұţ | �Ŷ�ǹ", 8},
        {4, "CZ75 �Զ���ǹ | ���Ƹְ�", 8},
        {4, "Tec-9 | ��Ƭ", 8},
        {4, "FN57 | ͭɫ��ϵ", 8},
        {4, "MAC-10 | ��ȸʯ", 8},
        {4, "�ض�����ǹ | ���֮��", 8},
        {4, "MP5-SD | ��˹", 8},
        {4, "P250 | ����", 8},
        {4, "XM1014 | �������", 7},
        {4, "MAC-10 | �վ�", 7},
        {4, "PP-Ұţ | ˮ������", 7},
        {4, "����� 18 �� | ��������", 7},
        {4, "˫�ֱ����� | ���г��", 7},
        {4, "P2000 | ����", 7},
        {4, "����˹ | Ԫ������", 7},
        {4, "FN57 | ��λ����", 7},
        {4, "AWP | ���֮��", 7},
        {4, "P90 | ������", 7},
        {4, "�ض�����ǹ | �۹��", 7},
        {4, "MAG-7 | SWAG-7", 7},
        {4, "���� | ʯī��", 6},
        {4, "P90 | ����Σ��", 6},
        {4, "G3SG1 | ������", 6},
        {4, "������ AR | �źŵ�", 6},
        {4, "MAC-10 | ����", 6},
        {4, "P250 | Ӱħ", 6},
        {4, "����� 18 �� | ������", 6},
        {4, "Tec-9 | ̮ѩ����", 6},
        {4, "�ڸ�� | ʨ����", 6},
        {4, "���� | ��Ұ����", 6},
        {4, "UMP-45 | ����", 6},
        {4, "CZ75 �Զ���ǹ | ս������", 6},
        {4, "CZ75 �Զ���ǹ | ����", 6},
        {4, "����� 18 �� | ����", 6},
        {4, "CZ75 �Զ���ǹ | �ͻ�", 5},
        {4, "Tec-9 | ����αװ", 5},
        {4, "������ AR | ��ʯ", 5},
        {4, "����� 18 �� | �ʼҾ���", 5},
        {4, "MAG-7 | ������", 5},
        {4, "MP7 | �۹�", 5},
        {4, "������ AR | ��캣Х", 5},
        {4, "M249 | ���̼�����", 5},
        {4, "UMP-45 | ֧��", 5},
        {4, "SG 553 | ��Ӱ", 5},
        {4, "MP7 | Ȩ��֮��", 5},
        {4, "CZ75 �Զ���ǹ | ��ӥ", 5},
        {4, "SSG 08 | ����սʿ", 5},
        {4, "ɳĮ֮ӥ | ָ��", 5},
        {4, "�ض�����ǹ | ·��", 4},
        {4, "PP-Ұţ | ����������", 4},
        {4, "P250 | ������", 4},
        {4, "M249 | ����ʮ�־�", 4},
        {4, "MP7 | �ٵ�", 4},
        {4, "MAG-7 | ����", 4},
        {4, "PP-Ұţ | ��ȼ�ϰ�", 4},
        {4, "˫�ֱ����� | ���Ұ���", 4},
        {4, "MP9 | ���Ͳ���", 4},
        {4, "XM1014 | ��ʰ߲�", 4},
        {4, "������ AR | ���߳��", 4},
        {4, "Tec-9 | ս����ȼ", 4},
        {4, "MAG-7 | ʯ��", 4},
        {4, "SG 553 | ����ͷ", 4},
        {4, "���� | ����", 3},
        {4, "����˹ | ����", 3},
        {4, "MP9 | �챦ʯ����", 3},
        {4, "�ڸ�� | �����", 3},
        {4, "CZ75 �Զ���ǹ | ����", 3},
        {4, "UMP-45 | ����", 3},
        {4, "XM1014 | �ؿ�¬������", 3},
        {4, "G3SG1 | ����", 3},
        {4, "���� | ����", 3},
        {4, "MAC-10 | ����֮��", 3},
        {4, "XM1014 | �ļ�", 3},
        {4, "UMP-45 | x����", 3},
        {4, "G3SG1 | ����", 3},
        {4, "���� | ���ʿ��", 3},
        {4, "XM1014 | ��װ����", 3},
        {4, "SCAR-20 | Ȩ��֮��", 3},
    },

    // ����
    {
        {5, "USP ������ | �����", 29},
        {5, "SG 553 | ����������", 27},
        {5, "USP ������ | ��ͼ", 25},
        {5, "MP7 | ��������", 18},
        {5, "AUG | ӥ��", 18},
        {5, "˫�ֱ����� | �����ľ", 15},
        {5, "M4A1 ������ | ��", 15},
        {5, "SG 553 | �˻�����", 12},
        {5, "M4A4 | �������", 12},
        {5, "M4A1 ������ | Ѫ��", 12},
        {5, "AK-47 | ��Ӣ֮��", 12},
        {5, "USP ������ | Ǧ��", 10},
        {5, "CZ75 �Զ���ǹ | ���֮��", 10},
        {5, "AK-47 | �����ؾ�", 10},
        {5, "UMP-45 | �׹�֮��", 9},
        {5, "SCAR-20 | ���֮��", 9},
        {5, "������ AR | ֧������", 9},
        {5, "G3SG1 | ����Ů��", 9},
        {5, "���� | �˷�����", 8},
        {5, "Tec-9 | ����", 8},
        {5, "P250 | �쳲", 8},
        {5, "������ AR | ɳ����", 8},
        {5, "USP ������ | Ѫ��", 7},
        {5, "R8 ������ǹ | ���֮��", 7},
        {5, "�ڸ�� | *������*", 7},
        {5, "MAG-7 | ������Ƭ", 7},
        {5, "MAC-10 | ����������", 7},
        {5, "����� 18 �� | ��ɫ����", 7},
        {5, "����˹ | ����", 7},
        {5, "SSG 08 | �޾��", 6},
        {5, "ɳĮ֮ӥ | ����˹�Ų�", 6},
        {5, "M4A4 | þԪ��", 6},
        {5, "����˹ | ����֮��", 6},
        {5, "Tec-9 | ��ͭ����", 5},
        {5, "SSG 08 | �Ѻ�", 5},
        {5, "SSG 08 | ��ˮ", 5},
        {5, "ɳĮ֮ӥ | ��ͭװ��", 5},
        {5, "AUG | ��������", 5},
        {5, "USP ������ | ����", 4},
        {5, "UMP-45 | �Ͻ�����", 4},
        {5, "Tec-9 | ������", 4},
        {5, "˫�ֱ����� | �ڱ�", 4},
        {5, "P90 | �������", 4},
        {5, "MAG-7 | Ӳˮ", 4},
        {5, "MAC-10 | ���޹�", 4},
        {5, "M249 | �ҽ�", 4},
        {5, "����˹ | ����֮��", 4},
        {5, "CZ75 �Զ���ǹ | ����", 4},
        {5, "XM1014 | Ѫ�����", 3},
        {5, "Tec-9 | ��ֽ", 3},
        {5, "SSG 08 | ͨ����", 3},
        {5, "ɳĮ֮ӥ | ��ʴ����", 3},
        {5, "PP-Ұţ | �������", 3},
        {5, "P90 | ��Ӣ֮��", 3},
        {5, "P250 | Ԫ������", 3},
        {5, "������ AR | ����", 3},
        {5, "����� 18 �� | ��ʴ����", 3},
        {5, "����� 18 �� | ��֮��", 3},
        {5, "����� 18 �� | ������", 3},
        {5, "Tec-9 | ɳ����", 2},
        {5, "˫�ֱ����� | ��֮˫��", 2},
        {5, "P2000 | ���ߵۻ�", 2},
        {5, "P2000 | ��ɫ��Ƭ�Բ�", 2},
        {5, "P2000 | ��������", 2},
        {5, "MP9 | �ɳ�", 2},
        {5, "MP7 | ����", 2},
        {5, "MP7 | ����Σ��", 2},
        {5, "MAG-7 | ����֮��", 2},
        {5, "������ AR | �������", 2},
        {5, "����� 18 �� | սӥ", 2},
        {5, "����� 18 �� | ��������", 2},
        {5, "����� 18 �� | ����֮��", 2},
        {5, "����� 18 �� | ��Ӱڤ��", 2},
        {5, "G3SG1 | �ڰ�����", 2},
        {5, "FN57 | ��ɫ��Ӧ", 2},
        {5, "FN57 | ����", 2},
        {5, "FN57 | ����Σ��", 2},
        {5, "FN57 | �������", 2},
        {5, "CZ75 �Զ���ǹ | ����Ťת", 2},
        {5, "AUG | �������", 2},
        {5, "AUG | ����ս��", 2},
        {5, "XM1014 | ��ʴ����", 1},
        {5, "XM1014 | ˹�����", 1},
        {5, "XM1014 | �綾ˮ��", 1},
        {5, "XM1014 | ����", 1},
        {5, "���� | Exo", 1},
        {5, "���� | ��ľ", 1},
        {5, "���� | ����", 1},
        {5, "UMP-45 | ��ʿ", 1},
        {5, "UMP-45 | ��֮��", 1},
        {5, "UMP-45 | �Ի�", 1},
        {5, "UMP-45 | ��", 1},
        {5, "UMP-45 | ����", 1},
        {5, "Tec-9 | �鵰��ʯ", 1},
        {5, "Tec-9 | ��-9", 1},
        {5, "Tec-9 | ��ķ����", 1},
        {5, "Tec-9 | ����", 1},
        {5, "˫�ֱ����� | ͨ���", 1},
        {5, "˫�ֱ����� | ��Ƭ", 1},
        {5, "˫�ֱ����� | ���ض�", 1},
        {5, "SG 553 | Σ�վ���", 1},
        {5, "SG 553 | ������", 1},
        {5, "SG 553 | ���", 1},
        {5, "SG 553 | ���޹�", 1},
        {5, "SCAR-20 | ��ɫ½ս��", 1},
        {5, "SCAR-20 | ��ͼ", 1},
        {5, "SCAR-20 | ����", 1},
        {5, "SCAR-20 | ���ֻ���", 1},
        {5, "SCAR-20 | ���ֱ���", 1},
        {5, "R8 ������ǹ | ��", 1},
        {5, "R8 ������ǹ | ����������", 1},
        {5, "PP-Ұţ | ͸����", 1},
        {5, "PP-Ұţ | �ո��", 1},
        {5, "PP-Ұţ | ��ҹ����", 1},
        {5, "PP-Ұţ | ���ֻ���", 1},
        {5, "P90 | ��������", 1},
        {5, "P90 | ǣ����", 1},
        {5, "P90 | ��Ѫ����", 1},
        {5, "P250 | ����", 1},
        {5, "P250 | ����", 1},
        {5, "P250 | ͭ��", 1},
        {5, "P2000 | �������", 1},
        {5, "P2000 | ����", 1},
        {5, "P2000 | ����Σ��", 1},
        {5, "P2000 | ��Ƥ", 1},
        {5, "�ڸ�� | ѣĿ", 1},
        {5, "�ڸ�� | ��ηս��", 1},
        {5, "�ڸ�� | ɳĮ��Ӣ", 1},
        {5, "�ڸ�� | ���̵���", 1},
        {5, "MP9 | �ж���в", 1},
        {5, "MP9 | ������ҩ", 1},
        {5, "MP9 | ����й©", 1},
        {5, "MP9 | ɰ��", 1},
        {5, "MP9 | ëϸѪ��", 1},
        {5, "MP9 | ��ɰ", 1},
        {5, "MP7 | װ�׺���", 1},
        {5, "MP7 | ���޲���", 1},
        {5, "MP7 | ������", 1},
        {5, "MAG-7 | �ܺ�", 1},
        {5, "MAG-7 | �������", 1},
        {5, "MAG-7 | ����", 1},
        {5, "MAC-10 | Ƚ��", 1},
        {5, "MAC-10 | �����Ƥ", 1},
        {5, "MAC-10 | ����", 1},
        {5, "MAC-10 | ������", 1},
        {5, "MAC-10 | ������", 1},
        {5, "M249 | ϵͳ����", 1},
        {5, "M249 | ��Ӱ", 1},
        {5, "�ض�����ǹ | ҹ�ٺ�", 1},
        {5, "�ض�����ǹ | ����", 1},
        {5, "�ض�����ǹ | ǧֽ��", 1},
        {5, "�ض�����ǹ | ��ͭ����", 1},
        {5, "�ض�����ǹ | Ū��֮­", 1},
        {5, "�ض�����ǹ | ��ɰ", 1},
        {5, "������ AR | ս�����", 1},
        {5, "������ AR | ��ɰ", 1},
        {5, "G3SG1 | ͨ���", 1},
        {5, "G3SG1 | ����", 1},
        {5, "G3SG1 | �ȹ���", 1},
        {5, "FN57 | ˹�����", 1},
        {5, "FN57 | ëϸѪ��", 1},
        {5, "����˹ | �Ҵ��� Z", 1},
        {5, "����˹ | ����ɫ", 1},
        {5, "CZ75 �Զ���ǹ | ӡ����", 1},
        {5, "CZ75 �Զ���ǹ | �ۺ���", 1},
    },
};
const case_detail& draw_case(double p);

////////////////////////////////////////////////////////////////////////////////
// ����
#ifdef _DEBUG
const int MAX_STAMINA = 100;
#else
const int MAX_STAMINA = 10;
#endif
const int STAMINA_TIME = 30 * 60; // 30min
inline std::map<int64_t, time_t> staminaRecoveryTime{};
inline std::map<int64_t, int> staminaExtra{};

////////////////////////////////////////////////////////////////////////////////
// �����趨
const int COST_OPEN_RED = 50;
const int COST_OPEN_YELLOW = 255;

const int COST_OPEN_ENDLESS = 50;

const int COST_OPEN_RED_STAMINA = 6;
const int COST_OPEN_ENDLESS_STAMINA = 7;
}
