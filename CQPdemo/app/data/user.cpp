#include "user.h"
#include "cqp.h"
#include "appmain.h"
#include "utils/time_util.h"

namespace user {

void peeCreateTable()
{
	if (db.exec(
		"CREATE TABLE IF NOT EXISTS pee( \
            qqid    INTEGER PRIMARY KEY NOT NULL, \
            currency INTEGER            NOT NULL, \
            cases   INTEGER             NOT NULL, \
            dailytime INTEGER           NOT NULL,  \
            keys    INTEGER             NOT NULL  \
         )") != SQLITE_OK)
		CQ_addLog(ac, CQLOG_ERROR, "pee", db.errmsg());
}

void peeLoadFromDb()
{
	auto list = db.query("SELECT * FROM pee", 5);
	for (auto& row : list)
	{
		int64_t qq = std::any_cast<int64_t>(row[0]);
		int64_t p1, p2, p4;
		time_t  p3;
		p1 = std::any_cast<int64_t>(row[1]);
		p2 = std::any_cast<int64_t>(row[2]);
		p3 = std::any_cast<time_t> (row[3]);
		p4 = std::any_cast<int64_t>(row[4]);
		plist[qq] = { qq, p1, p2, p3, p4 };
		plist[qq].freeze_assets_expire_time = INT64_MAX;
	}
	char msg[128];
	sprintf_s(msg, "added %u users", plist.size());
	CQ_addLog(ac, CQLOG_DEBUG, "pee", msg);
}

pdata::resultStamina pdata::getStamina(bool extra)
{
	time_t t = time(nullptr);
	time_t last = stamina_recovery_time;
	int stamina = MAX_STAMINA;
	if (last > t) stamina -= (last - t) / STAMINA_TIME + !!((last - t) % STAMINA_TIME);
	return { true, stamina, stamina_recovery_time};
}

pdata::resultStamina pdata::modifyStamina(int cost, bool extra)
{
	int stamina = getStamina(false).staminaAfterUpdate;

    bool enough = false;

    if (cost > 0)
    {
        if (stamina_extra >= cost)
        {
            enough = true;
            stamina_extra -= cost;
        }
        else if (stamina + stamina_extra >= cost)
        {
            enough = true;
            cost -= stamina_extra;
            stamina_extra = 0;
            stamina -= cost;
        }
        else
        {
            enough = false;
        }
    }
    else if (cost < 0)
    {
        enough = true;
        if (stamina - cost <= MAX_STAMINA)
        {
            // do nothing
        }
        else if (extra) // part of cost(recovery) goes to extra
        {
            stamina_extra += stamina - cost - MAX_STAMINA;
        }
    }

	time_t t = time(nullptr);
	time_t last = stamina_recovery_time;
    if (enough)
    {
        if (last > t)
            stamina_recovery_time += STAMINA_TIME * cost;
        else
            stamina_recovery_time = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) stamina_recovery_time = t;
    return { enough, stamina, stamina_recovery_time - t };
}

pdata::resultStamina pdata::testStamina(int cost)
{
	int stamina = getStamina(false).staminaAfterUpdate;

    bool enough = false;

    if (cost > 0)
    {
        if (stamina_extra >= cost)
        {
            enough = true;
        }
        else if (stamina + stamina_extra >= cost)
        {
            enough = true;
        }
        else
        {
            enough = false;
        }
    }

	time_t t = time(nullptr);
	time_t last = stamina_recovery_time;
    if (enough)
    {
        if (last > t)
            last += STAMINA_TIME * cost;
        else
            last = t + STAMINA_TIME * cost;
    }

    if (enough && stamina >= MAX_STAMINA) last = t;
    return { enough, stamina, last - t };
}

void pdata::modifyCurrency(int64_t c)
{
	currency += c;
	if (currency < 0) currency = 0;
	db.exec("UPDATE pee SET currency=? WHERE qqid=?", { currency, qq });
}
void pdata::multiplyCurrency(double a)
{
	currency *= a;
	if (currency < 0) currency = 0;
	db.exec("UPDATE pee SET currency=? WHERE qqid=?", { currency, qq });
}
void pdata::modifyBoxCount(int64_t c)
{
	opened_box_count += c;
	db.exec("UPDATE pee SET cases=? WHERE qqid=?", { opened_box_count, qq });
}
void pdata::modifyDrawTime(time_t c)
{
	last_draw_time = c;
	db.exec("UPDATE pee SET dailytime=? WHERE qqid=?", { last_draw_time, qq });
}
void pdata::modifyKeyCount(int64_t c)
{
	key_count += c;
	if (key_count < 0) key_count = 0;
	db.exec("UPDATE pee SET keys=? WHERE qqid=?", { key_count, qq });
}

void pdata::createAccount(int64_t c)
{
	currency = c;
	db.exec("INSERT INTO pee(qqid, currency, cases, dailytime, keys) VALUES(? , ? , ? , ? , ?)",
		{ qq, currency, 0, 0, 0 });
}

}
