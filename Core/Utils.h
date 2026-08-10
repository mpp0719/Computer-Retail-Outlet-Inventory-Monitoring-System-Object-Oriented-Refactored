#pragma once
#include <string>

class Utils
{
public:
    static int integerCheck(int value);
    static double doubleCheck(double value);
    static void initConsole();               // sets up UTF-8 + ANSI on Windows console
    static std::string checkmonth(int month); // 1-12 -> "January".."December"
    static std::string readMaskedInput();     // masked password input (was inline in stafflogin)

private:
    Utils() = delete;
};