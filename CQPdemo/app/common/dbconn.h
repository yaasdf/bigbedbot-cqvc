#pragma once
#include <any>
#include <vector>
#include <exception>

struct sqlite3;

#define SQLITE_OK 0

class SQLite
{
private:
    sqlite3* _db;
    char logGrp[32];
    bool inTransaction = false;
public:
    SQLite() = delete;
    SQLite(const char* path, const char* log);
    ~SQLite();
    std::vector<std::vector<std::any>> query(const char* stmt, size_t retSize);
    std::vector<std::vector<std::any>> query(const char* stmt, size_t retSize, std::initializer_list<std::any> args);
	int exec(const char* zsql);
    int exec(const char* zsql, std::initializer_list<std::any> args);
    void transactionStart();
    void transactionStop();
    void commit(bool restart_transaction = false);
    const char* errmsg();
public:
	void timedCommit();
	void startTimedCommit();
};
