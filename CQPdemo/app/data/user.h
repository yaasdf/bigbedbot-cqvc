#pragma once
#include <map>
#include <ctime>
#include "../common/dbconn.h"

namespace user {

inline SQLite db("pee.db", "pee");

////////////////////////////////////////////////////////////////////////////////
// p

void peeCreateTable();
void peeLoadFromDb();

class pdata
{
public:
	pdata() {}
	pdata(int64_t q, int64_t c, int64_t bc, time_t dt, int64_t k):
		qq(q), currency(c), opened_box_count(bc), last_draw_time(dt), key_count(k) {}
	~pdata() = default;

protected:
	int64_t qq = -1;
    int64_t currency = 0;
    int64_t opened_box_count = 0;
    time_t  last_draw_time = 0;
    int64_t key_count = 0;
	time_t  stamina_recovery_time = 0;
	int		stamina_extra = 0;

public:
	// flags
    time_t  freeze_assets_expire_time = 0;
    bool    meteor_shield = 0;
    int     air_pump_count = 0;
    int     air_ignore_count = 0;
    time_t  adrenaline_expire_time = 0;
    bool    chaos = false;

    // counter
    //int     event_drop_count = 0;

	friend void peeLoadFromDb();

public:
	struct resultStamina
	{
		bool	enough;
		int		staminaAfterUpdate;
		time_t	fullRecoverTime;
	};
	resultStamina getStamina(bool extra);
	resultStamina modifyStamina(int cost = 0, bool extra = false);
	resultStamina testStamina(int cost = 0);

	int64_t getCurrency() { return currency; }
	int64_t getKeyCount() { return key_count; }
	time_t  getLastDrawTime() { return last_draw_time; }
	void modifyCurrency(int64_t delta);
	void multiplyCurrency(double a);
	void modifyBoxCount(int64_t delta);
	void modifyDrawTime(time_t time);
	void modifyKeyCount(int64_t delta);

	void createAccount(int64_t initialBalance);

};

inline std::map<int64_t, pdata> plist;

const int INITIAL_BALANCE = 200;

////////////////////////////////////////////////////////////////////////////////
// ÃÂ¡¶
#ifdef _DEBUG
const int MAX_STAMINA = 100;
#else
const int MAX_STAMINA = 10;
#endif
const int STAMINA_TIME = 30 * 60; // 30min

}
