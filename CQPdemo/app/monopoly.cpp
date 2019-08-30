#include "monopoly.h"
#include "cqp.h"
#include "appmain.h"
#include "private/qqid.h"
#include <cmath>

extern time_t banTime_me;

namespace mnp
{
const std::vector<event_type> EVENT_POOL{
    { 0.02,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "����Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "��Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "ͭ��Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "��������˿�����˿�1������"; }},
    { 0.02,[](int64_t group, int64_t qq) { modifyKeyCount(qq, ++plist[qq].keys); return "���֣����ҿ�1������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "÷��Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 2; modifyKeyCount(qq, plist[qq].keys); return "��ʯԿ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 10; modifyKeyCount(qq, plist[qq].keys); return "Կ����װ�����10��Կ��"; }},
    { 0.05,[](int64_t group, int64_t qq) { plist[qq].currency += 1; modifyCurrency(qq, plist[qq].currency); return "����·��û��Ҫ�ĵ���ˮ�����1����"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].currency += 2; modifyCurrency(qq, plist[qq].currency); return "�����濵˧��ţ���棬���5����"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].currency += 5; modifyCurrency(qq, plist[qq].currency); return "צ�ӵ����ģ�ͣ����20����"; }},
    { 0.005,[](int64_t group, int64_t qq) { plist[qq].currency += 100; modifyCurrency(qq, plist[qq].currency); return "�󴲼�����ᣬ���100����"; }},
    { 0.06,[](int64_t group, int64_t qq) { updateStamina(qq, -1); return "����һ��ȯ�����1������"; }},
    { 0.02,[](int64_t group, int64_t qq) { updateStamina(qq, -2); return "����һ��˫�������ײͣ����2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, -4); return "ԭ��ƿ�����4������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].keys += 4; modifyKeyCount(qq, plist[qq].keys); return "ԭ�ػ�ƿ������ħ��������4��Կ��"; }},
    { 0.002,[](int64_t group, int64_t qq) { plist[qq].currency += 500; modifyCurrency(qq, plist[qq].currency); return "������BEXͨ�ؼ�����񣬻��500����"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].keys += 5; modifyKeyCount(qq, plist[qq].keys); return "�������ߣ��ܿ�5������"; }},

    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, 1); return "��ҵ������˰�Сʱ��"; } },
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, -2); return "������Һ�����2������"; } },
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, -2); return "������¥�����2������"; } },
    { 0.01,[](int64_t group, int64_t qq) { updateStamina(qq, 1); return "��Һã��㱻��ʳͼƬ����1������"; } },
    { 0.01,[](int64_t group, int64_t qq) { nosmoking(group, qq, 2); return "����1+1=12�ײͣ�����2����"; } },
    { 0.005,[](int64_t group, int64_t qq) { updateStamina(qq, -pee::MAX_STAMINA); return "�ظ�ʯ����о�������ˬ������������"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        updateStamina(qq, -3, true);
        using namespace std::string_literals;
        return "С���������3������"s + (pee::staminaExtra[qq] > 0 ? "������ͻ������������"s : ""s);
    }},

    { 0.008,[](int64_t group, int64_t qq)
    {
        auto [enough, stamina, t] = updateStamina(qq, 0);
        updateStamina(qq, stamina);
        return "�Ӱ��ʼ������������������";
    }},

    { 0.04,[](int64_t group, int64_t qq) { nosmoking(group, qq, 1); return "���̶�ͯ�ײͣ�����1����"; }},
    { 0.005,[](int64_t group, int64_t qq) { nosmoking(group, qq, 5); return "���̰����ײͣ�����5����"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 50;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "�����ƣ�����ֱ������������ѣ�����ҽҩ��50��";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 10; 
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow += 10;
        return "���Ʋ��뽱������10��������������"; 
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        double p = randReal(0.5, 1.5);
        plist[qq].currency *= p;
        modifyCurrency(qq, plist[qq].currency);
        using namespace std::string_literals;
        return "һ�Ѳ��ˣ�����������"s + std::to_string(p) + "��"s;
    }},

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            nosmoking(group, qqid_dk, 1);
            return "����Ҳ������������";
        }
        else return EVENT_POOL[0].func()(group, qq);
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                auto [enough, stamina, t] = updateStamina(qq, 0);
                updateStamina(qq, stamina);
                updateStamina(qq, -2);
            }
        }
        return "���򲭣���Ⱥ�����˵������������2��";
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                if (plist[qq].meteor_shield)
                {
                    plist[qq].meteor_shield = false;
                    continue;
                }
                plist[qq].currency *= 0.9;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "��ʯ����Ⱥ�����˵�Ǯ������ը��10%";
    }},

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::thread([]() {using namespace std::chrono_literals; std::this_thread::sleep_for(2s); pee::flushDailyTimep(); }).detach();
        return "F5";
    }},

    { 0.01,[](int64_t group, int64_t qq) { return "��Կ��һ�ѣ��㳢���������䵫��ʧ����"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "��е�����һ��������û��ʲô��"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "�޴�س���һ��������û��ʲô��"; } },
    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HAMMER + "������" + EMOJI_HAMMER; } },
    { 0.01,[](int64_t group, int64_t qq) { return "һ��ϡ�����壬����û��ʲô��"; } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        plist[qq].currency -= 500;
        if (plist[qq].currency < 0) plist[qq].currency = 0;
        modifyCurrency(qq, plist[qq].currency);
        return "JJ�֣���ļұ�ը���ˣ���ʧ500��";
    } },

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].currency += 10;
        modifyCurrency(qq, plist[qq].currency);
        pee::extra_tomorrow -= 10;
        if (-pee::extra_tomorrow > pee::DAILY_BONUS) pee::extra_tomorrow = -pee::DAILY_BONUS;
        return "����ǯ����֤������������͵��10��";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (plist[qq].keys < 1) return EVENT_POOL[0].func()(group, qq);
        plist[qq].keys--;
        modifyKeyCount(qq, plist[qq].keys);
        return "�����ӣ���ʲôҲû�п���������1��Կ��";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                updateStamina(qq, -pee::MAX_STAMINA);
            }
        }
        return "һ��ƽ���ܲ�����Ⱥ�����˵�������������";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
            {
                int bonus = randInt(50, 1000);
                plist[qq].currency += bonus;
                modifyCurrency(qq, plist[qq].currency);
            }
        }
        return "��紵����Ⱥ��һ�������Լ������񵽶���Ŷ";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qqid_dk, FALSE);
        if (cqinfo && strlen(cqinfo) > 0)
        {
            int d = randInt(1, 5);
            nosmoking(group, qq, d);
            using namespace std::string_literals;
            return "�������ӣ��㱻���ӽ���"s + std::to_string(d) + "����"s;
        }
        else return EVENT_POOL[0].func()(group, qq);
    } },

    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HORN + "��ɵ��"; } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            unsigned idx = randInt(0, numbers.size() - 1);
            p = p * 10 + numbers[idx];
            numbers.erase(numbers.begin() + idx);
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "����ҩˮ���������������";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        unsigned idx = randInt(0, numbers.size() - 1);
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[idx];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "�ں�ҩˮ�������ȫ�������һ��������";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            numbers.push_back(tmp);
        }
        size_t size = numbers.size();
        int64_t p = 0;
        for (size_t i = 0; i < size; ++i)
        {
            p = p * 10 + numbers[i];
        }
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "����ҩˮ�����������������";
    } },

    { 0.002,[](int64_t group, int64_t qq)
    {
        std::vector<unsigned> numbers;
        auto c = plist[qq].currency;
        int len = 0;
        while (c > 0)
        {
            ++len;
            c /= 10;
        }
        int64_t p = plist[qq].currency;
        p = p % 10 * int64_t(std::pow(10.0, len - 1)) + p / 10;
        plist[qq].currency = p;
        modifyCurrency(qq, plist[qq].currency);
        return "ѭ��ҩˮ����������һλ���ֱ���˵�һλ";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::vector<int64_t> grouplist;
        for (auto& [qq, pd] : plist)
        {
            const char* cqinfo = CQ_getGroupMemberInfoV2(ac, group, qq, FALSE);
            if (cqinfo && strlen(cqinfo) > 0)
                grouplist.push_back(qq);
        }
        size_t idx = randInt(0, grouplist.size() - 1);
        int64_t target = grouplist[idx];
        nosmoking(group, target, 1);
        using namespace std::string_literals;
        return "�����޼ʵ�����"s + CQ_At(target) + "���ұ���1����";
    } },


    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].meteor_shield = true;
        return "��ʯ�����ֳ��ܽ��ң�������һ����ʯ�˺�";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        int64_t cost = plist[qq].currency * 0.1;
        plist[qq].currency -= cost;
        modifyCurrency(qq, plist[qq].currency);
        using namespace std::string_literals;
        return "���˷�������ǿ�Ȳ��˱����г�����С�鷢���ˣ����ɷ���"s + std::to_string(cost) + "��";
    } },

    { 0.003,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].air_pump_count = 5;
        return "�߼����ã�5�γ鿨�ڱس�����";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].air_ignore_count = 10;
        return "��ճ�������10�γ鿨�ڲ�������";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        for (auto& c : pee::smokeGroups)
            for (auto& g : c.second)
                if (t < g.second)
                    CQ_setGroupBan(ac, g.first, c.first, 0);
        return "���꣬�����˵��̶���������";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].keys += 50; 
        modifyKeyCount(qq, plist[qq].keys); 
        return "����֮������õ�����Ӵ�ʽ����Կ��������һ�ѣ����ÿ�50������"; 
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        plist[qq].adrenaline_expire_time = t + 15;
        return "�������أ�������15���ڳ鿨����������";
    } },


    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60;
        return "�����̣�bot������1����";
    } },

    { 0.004,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60 * 5;
        return "���в�����bot��㱻�㶾����ֻ����Ϣ5����";
    } },
};
const event_type EVENT_DEFAULT{ 1.0, [](int64_t group, int64_t qq) { return "������ʲô��ô��"; } };


command msgDispatcher(const char* msg)
{
    command c;
    auto query = msg2args(msg);
    if (query.empty()) return c;

    auto cmd = query[0];
    if (commands_str.find(cmd) == commands_str.end()) return c;

    c.args = query;
    switch (c.c = commands_str[cmd])
    {
    case commands::�鿨:
        c.func = [](::int64_t group, ::int64_t qq, std::vector<std::string> args, std::string raw)->std::string
        {
            if (plist.find(qq) == plist.end()) return std::string(CQ_At(qq)) + "���㻹û�п�ͨ����";

            std::stringstream ss;
            time_t t = time(nullptr);
            if (t > plist[qq].adrenaline_expire_time)
            {
                auto [enough, stamina, rtime] = updateStamina(qq, 1);

                if (!enough)
                {
                    ss << CQ_At(qq) << "������������㣬��������"
                        << rtime / (60 * 60) << "Сʱ" << rtime / 60 % 60 << "����";
                    return ss.str();
                }
                // fall through if stamina is enough
            }
            else
            {
                ss << "����������Ч��";
            }

            event_type reward = EVENT_DEFAULT;

            // ����������Ч
            if (plist[qq].air_pump_count)
            {
                --plist[qq].air_pump_count;
                reward = EVENT_DEFAULT;
            }

            // ��������������Ч��
            if (plist[qq].air_ignore_count)
            {
                --plist[qq].air_ignore_count;
                do {
                    reward = draw_event(randReal());
                } while (reward.prob() == 1.0);
            }
            else // ͨ���鿨
            {
                reward = draw_event(randReal());
            }
            ss << CQ_At(qq) << "����ϲ��鵽��" << reward.func()(group, qq);
            return ss.str();
        };
        break;

    default:
        break;
    }
    return c;
}

const event_type& draw_event(double p)
{
    if (p < 0.0 || p > 1.0) return EVENT_DEFAULT;
    double totalp = 0;
    for (const auto& c : EVENT_POOL)
    {
        if (p <= totalp + c.prob()) return c;
        totalp += c.prob();
    }
    return EVENT_DEFAULT;        // not match any case
}

}