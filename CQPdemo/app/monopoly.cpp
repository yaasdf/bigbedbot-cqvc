#include <cmath>
#include <thread>
#include <sstream>
#include "monopoly.h"

#include "utils/rand.h"
#include "utils/string_util.h"

#include "data/user.h"
#include "data/group.h"
#include "private/qqid.h"

#include "user_op.h"
#include "smoke.h"

#include "cqp.h"
#include "cqp_ex.h"

extern time_t banTime_me;

namespace mnp
{
using user::plist;

const std::vector<event_type> EVENT_POOL{
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "����Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "��Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "ͭ��Կ��1��"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "��������˿�����˿�1������"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+1); return "���֣����ҿ�1������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "÷��Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "����Կ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+2); return "��ʯԿ�ף��ܿ�2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+10); return "Կ����װ�����10��Կ��"; }},
	{ 0.05,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+1); return "����·��û��Ҫ�ĵ���ˮ�����1����"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+5); return "�����濵˧��ţ���棬���5����"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+20); return "צ�ӵ����ģ�ͣ����20����"; }},
    { 0.005,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+100); return "�󴲼�����ᣬ���100����"; }},
	{ 0.06,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-1); return "����һ��ȯ�����1������"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "����һ��˫�������ײͣ����2������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-4); return "ԭ��ƿ�����4������"; }},
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+4); return "ԭ�ػ�ƿ������ħ��������4��Կ��"; }},
    { 0.002,[](int64_t group, int64_t qq) { plist[qq].modifyCurrency(+500); return "������BEXͨ�ؼ�����񣬻��500����"; }},
    { 0.02,[](int64_t group, int64_t qq) { plist[qq].modifyKeyCount(+5); return "�������ߣ��ܿ�5������"; }},

    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(1); return "��ҵ������˰�Сʱ��"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "������Һ�����2������"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-2); return "������¥�����2������"; } },
    { 0.01,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(1); return "��Һã��㱻��ʳͼƬ����1������"; } },
    { 0.01,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 2); return "����1+1=12�ײͣ�����2����"; } },
    { 0.005,[](int64_t group, int64_t qq) { plist[qq].modifyStamina(-user::MAX_STAMINA); return "�ظ�ʯ����о�������ˬ������������"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].modifyStamina(-3, true);
        using namespace std::string_literals;
        return "С���������3������"s + (plist[qq].getStamina(true).staminaAfterUpdate > user::MAX_STAMINA ? "������ͻ������������"s : ""s);
    }},

    { 0.008,[](int64_t group, int64_t qq)
    {
        auto [enough, stamina, t] = plist[qq].modifyStamina(0);
        plist[qq].modifyStamina(stamina);
        return "�Ӱ��ʼ������������������";
    }},

    { 0.04,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 1); return "���̶�ͯ�ײͣ�����1����"; }},
    { 0.005,[](int64_t group, int64_t qq) { smoke::nosmoking(group, qq, 5); return "���̰����ײͣ�����5����"; }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        plist[qq].modifyCurrency(-50);
        return "�����ƣ�����ֱ�����������ѣ�����ҽҩ��50��";
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(-10);
        user_op::extra_tomorrow += 10;
        return "���Ʋ��뽱������10��������������"; 
    }},

    { 0.01,[](int64_t group, int64_t qq)
    {
        double p = randReal();
        if (p < 0.5) p = 1.0 + (p / 0.5);
		plist[qq].multiplyCurrency(p);
        using namespace std::string_literals;
        return "һ�Ѳ��ˣ�����������"s + std::to_string(p) + "��"s;
    }},

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (grp::groups[group].haveMember(qqid_dk))
        {
            smoke::nosmoking(group, qqid_dk, 1);
            return "����Ҳ������������";
        }
        else return EVENT_POOL[0].func()(group, qq);
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                auto [enough, stamina, t] = plist[qq].modifyStamina(0);
                plist[qq].modifyStamina(stamina);
                plist[qq].modifyStamina(-2);
            }
        }
        return "���򲭣���Ⱥ�����˵������������2��";
    }},

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                if (plist[qq].meteor_shield)
                {
                    plist[qq].meteor_shield = false;
                    continue;
                }
				plist[qq].multiplyCurrency(0.9);
            }
        }
        return "��ʯ����Ⱥ�����˵�Ǯ������ը��10%";
    }},

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::thread([]() {using namespace std::chrono_literals; std::this_thread::sleep_for(2s); user_op::flushDailyTimep(); }).detach();
        return "F5";
    }},

    { 0.01,[](int64_t group, int64_t qq) { return "��Կ��һ�ѣ��㳢���������䵫��ʧ����"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "��е�����һ��������û��ʲô��"; } },
    { 0.01,[](int64_t group, int64_t qq) { return "�޴�س���һ��������û��ʲô��"; } },
    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HAMMER + "������" + EMOJI_HAMMER; } },
    { 0.01,[](int64_t group, int64_t qq) { return "һ��ϡ�����壬����û��ʲô��"; } },

    { 0.002,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(-500);
        return "JJ�֣���ļұ�ը���ˣ���ʧ500��";
    } },

    { 0.01,[](int64_t group, int64_t qq)
    {
		plist[qq].modifyCurrency(+10);
		user_op::extra_tomorrow -= 10;
        if (-user_op::extra_tomorrow > user_op::DAILY_BONUS) user_op::extra_tomorrow = -user_op::DAILY_BONUS;
        return "����ǯ����֤������������͵��10��";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (plist[qq].getKeyCount() < 1) return EVENT_POOL[0].func()(group, qq);
		plist[qq].modifyKeyCount(-1);
        return "�����ӣ���ʲôҲû�п���������1��Կ��";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                plist[qq].modifyStamina(-user::MAX_STAMINA);
            }
        }
        return "һ��ƽ���ܲ�����Ⱥ�����˵�������������";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
            {
                int bonus = randInt(50, 1000);
				plist[qq].modifyCurrency(+bonus);
            }
        }
        return "��紵����Ⱥ��һ�������Լ������񵽶���Ŷ";
    } },

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        if (grp::groups[group].haveMember(qqid_dk))
        {
            int d = randInt(1, 5);
            smoke::nosmoking(group, qq, d);
            using namespace std::string_literals;
            return "�������ӣ��㱻���ӽ���"s + std::to_string(d) + "����"s;
        }
        else return EVENT_POOL[0].func()(group, qq);
    } },

    { 0.01,[](int64_t group, int64_t qq) { return EMOJI_HORN + "��ɵ��"; } },

    /*
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
        size_t size = 0;
        while (c > 0)
        {
            int tmp = c % 10;
            c /= 10;
            ++size;
            if (tmp != 0) numbers.push_back(tmp);
        }
        if (numbers.empty())
        {
            // 0
            return "�ں�ҩˮ�������ȫ�������һ��������";
        }

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
        */

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        std::vector<int64_t> grouplist;
        for (auto& [qq, pd] : plist)
        {
            if (grp::groups[group].haveMember(qq))
                grouplist.push_back(qq);
        }
        size_t idx = randInt(0, grouplist.size() - 1);
        int64_t target = grouplist[idx];
        smoke::nosmoking(group, target, 1);
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
        int64_t cost = plist[qq].getCurrency() * 0.1;
		plist[qq].modifyCurrency(-cost);
        using namespace std::string_literals;
        return "���˷�������ǿ�Ȳ��˱����г�����С�鷢���ˣ����ɷ���"s + std::to_string(cost) + "��";
    } },

        /*
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
    */

    { 0.01,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        for (auto& c : smoke::smokeTimeInGroups)
            for (auto& g : c.second)
                if (t < g.second)
                    CQ_setGroupBan(ac, g.first, c.first, 0);
        return "���꣬�����˵��̶���������";
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
		plist[qq].modifyKeyCount(+50);
        return "����֮������õ����Ӵ�ʽ����Կ��������һ�ѣ����ÿ�50������"; 
    } },

    { 0.002,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        plist[qq].adrenaline_expire_time = t + 15;
        return "�������أ�������15���ڳ鿨����������";
    } },


    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60;
        return "�����̣�bot������1����";
    } },

    { 0.001,[](int64_t group, int64_t qq) -> std::string
    {
        time_t t = time(nullptr);
        banTime_me = t + 60 * 5;
        return "���в�����bot��㱻�㶾����ֻ����Ϣ5����";
    } },

    { 0.005,[](int64_t group, int64_t qq) -> std::string
    {
        plist[qq].chaos = true;
        return "��������Ԥ������һ�γ鿨�ܿ��ܻ�鵽��ֵĶ���Ŷ";
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
                auto [enough, stamina, rtime] = plist[qq].modifyStamina(1);

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
            bool chaos = plist[qq].chaos;
            plist[qq].chaos = false;

            // ����������Ч
            if (plist[qq].air_pump_count)
            {
                --plist[qq].air_pump_count;
                reward = EVENT_DEFAULT;
                ss << CQ_At(qq) << "����ϲ��鵽��" << reward.func()(group, qq);
                return ss.str();
            }

            // ��������������Ч��
            if (plist[qq].air_ignore_count)
            {
                --plist[qq].air_ignore_count;
                do {
                    reward = chaos ? draw_event_chaos() : draw_event(randReal());
                } while (reward.prob() == 1.0);
            }
            else // ͨ���鿨
            {
                reward = chaos ? draw_event_chaos() : draw_event(randReal());
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

double event_max = 0.0;
void calc_event_max()
{
    event_max = 0.0;
    for (const auto& e : EVENT_POOL)
    {
        event_max += e.prob();
    }
}

const event_type& draw_event(double p)
{
    p = p * event_max;
    if (p < 0.0 || p > event_max) return EVENT_DEFAULT;
    double totalp = 0;
    for (const auto& c : EVENT_POOL)
    {
        if (p <= totalp + c.prob()) return c;
        totalp += c.prob();
    }
    return EVENT_DEFAULT;        // not match any case
}


const event_type& draw_event_chaos()
{
    int cnt = EVENT_POOL.size() - 1 + 1;  // include default
    int evt = randInt(0, cnt);

    if (evt == cnt)
        return EVENT_DEFAULT;
    else
        return EVENT_POOL[evt];
}

}