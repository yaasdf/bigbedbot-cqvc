// test_tool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <csignal>
#include <string>
#include <Windows.h>
#include "../CQPdemo/cqp.h"

HMODULE hDll = NULL;

int32_t(__stdcall* eventStartup)() = NULL;
int32_t(__stdcall* eventExit)() = NULL;
int32_t(__stdcall* eventEnable)() = NULL;
int32_t(__stdcall* eventDisable)() = NULL;
int32_t(__stdcall* eventPrivateMsg)(int32_t subType, int32_t msgId, int64_t fromQQ, const char* msg, int32_t font) = NULL;
int32_t(__stdcall* eventGroupMsg)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char* fromAnonymous, const char* msg, int32_t font) = NULL;

bool coolQStarted = false;
bool coolQLoaded = false;

using std::cout;
using std::endl;

void signalHandler(int signum);

int main()
{
    hDll = LoadLibraryA("app.dll");
    if (!hDll)
    {
        cout << "DLL loading error with code " << GetLastError() << endl;
        return 1;
    }

    eventStartup = decltype(eventStartup)(GetProcAddress(hDll, "_eventStartup"));
    eventExit = decltype(eventExit)(GetProcAddress(hDll, "_eventExit"));
    eventEnable = decltype(eventEnable)(GetProcAddress(hDll, "_eventEnable"));
    eventDisable = decltype(eventDisable)(GetProcAddress(hDll, "_eventDisable"));
    eventPrivateMsg = decltype(eventPrivateMsg)(GetProcAddress(hDll, "_eventPrivateMsg"));
    eventGroupMsg = decltype(eventGroupMsg)(GetProcAddress(hDll, "_eventGroupMsg"));

    if (!eventStartup || !eventExit || !eventEnable || !eventDisable || !eventPrivateMsg || !eventGroupMsg)
    {
        cout << "Get proc error";
        return 2;
    }

    signal(SIGINT, signalHandler);      //ctrl+c
    signal(SIGTERM, signalHandler);     //end process
    signal(SIGBREAK, signalHandler);    //x

    std::string input;
    enum
    {
        CMD,
        PRIVATE,
        GROUP,
    } procMode = CMD;

    eventStartup();
    coolQStarted = true;
    eventEnable();
    coolQLoaded = true;

    while (true)
    {
        cout << "------------------------------------------------" << endl; 
        switch (procMode)
        {
        case CMD:
            cout << "Mode: CMD (1: Private, 2: Group)" << endl;
            break;

        case PRIVATE:
            cout << "Mode: PRIVATE" << endl;
            break;

        case GROUP:
            cout << "Mode: GROUP" << endl;
            break;
        }
        std::getline(std::cin, input);

        if (input.empty())
            procMode = CMD;
        else switch (procMode)
        {
        case CMD:
            if (input == "1")
                procMode = PRIVATE;
            else if (input == "2")
                procMode = GROUP;
            break;

        case PRIVATE:
            eventPrivateMsg(11, 0, 12345678, input.c_str(), 0);
            break;

        case GROUP:
            eventGroupMsg(0, 0, 11111111, 12345678, "SHABI", input.c_str(), 0);
            break;
        }
    }

    if (coolQLoaded)
    {
        eventDisable();
        coolQLoaded = false;
    }
    if (coolQStarted)
    {
        eventExit();
        coolQStarted = false;
    }

    FreeLibrary(hDll);
    return 0;
}

void signalHandler(int signum)
{
    if (coolQLoaded)
    {
        eventDisable();
        coolQLoaded = false;
    }
    if (coolQStarted)
    {
        eventExit();
        coolQStarted = false;
    }

    if (hDll)
        FreeLibrary(hDll);

    exit(signum);
}