#include "Utils.h"
#define NOMINMAX
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <limits>

int Utils::integerCheck(int choice)
{
    while (std::cin.fail())
    {
        std::cerr << "Invalid input. Try again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> choice;
    }
    return choice;
}

double Utils::doubleCheck(double value)
{
    while (std::cin.fail())
    {
        std::cerr << "Invalid input. Try again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> value;
    }
    return value;
}

void Utils::initConsole()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}

std::string Utils::checkmonth(int month)
{
    static const char* names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (month >= 1 && month <= 12)
        return names[month - 1];
    return "";
}

std::string Utils::readMaskedInput()
{
    std::string value;
    char ch;
    while (true)
    {
        ch = _getch();
        if (ch == 13) // Enter
        {
            std::cout << std::endl;
            break;
        }
        else if (ch == 8) // Backspace
        {
            if (!value.empty())
            {
                value.pop_back();
                std::cout << "\b \b";
            }
        }
        else
        {
            value += ch;
            std::cout << "*";
        }
    }
    return value;
}