#include <iomanip>
#include<iostream>
#include<Windows.h>

#include "message.h"

HANDLE open_pipe(const char* pipe_name);
int get_choice();
bool send_requests(HANDLE h_pipe);
bool send_reading_request(HANDLE h_pipe, int code);
bool send_writing_request(HANDLE h_pipe, message& msg);

int main(int argc, char** argv)
{
    const HANDLE h_pipe = open_pipe(argv[0]);
    if (h_pipe == nullptr)
        return 1;

    int return_code = 0;
    if (!send_requests(h_pipe))
        return_code = 2;

    CloseHandle(h_pipe);
    return return_code;
}

bool send_requests(HANDLE h_pipe)
{
    while (true)
    {
        const int code = get_choice();
        if (code == 2)
            break;
        if (code != 1 && code != 0)
        {
            std::cout << "You enter incorrect code; please, repeat";
            continue;
        }

        employee emp;
        message msg;

        send_reading_request(h_pipe, code);

        if (!ReadFile(h_pipe, &emp, sizeof(employee), nullptr, nullptr))
        {
            const auto error_code = GetLastError();
            if (error_code == ERROR_BROKEN_PIPE)
                break;
            else
            {
                std::cout << "error of reading pipe";
                return false;
            }
        }
        std::cout << "Employer's ID" << std::setw(23) << "name   " << std::setw(9) << "hours" << "\n";
        std::cout << std::setw(13) << emp.num << std::setw(20) << emp.name << std::setw(12) << emp.hours << "\n";

        if (code == 1)
        {
            if (!send_writing_request(h_pipe, msg))
                return false;
        }

        std::cout << "enter something to release record\n";
        system("pause");
        msg.reading = 4;
        if (!WriteFile(h_pipe, &msg, sizeof(message), nullptr, nullptr))
        {
            std::cout << "error of writing in pipe";
            return false;
        }
    }
    return true;
}

int get_choice()
{
    int code;
    std::cout << "enter code to select action:\n";
    std::cout << "0 to read record\n";
    std::cout << "1 to modify record\n";
    std::cout << "2 to finish process\n";
    std::cin >> code;
    return code;
}

bool send_reading_request(HANDLE h_pipe, int code)
{
    int id;
    std::cout << "enter id of record:\n";
    std::cin >> id;
    message msg;
    msg.id = id;
    msg.reading = code;
    if (!WriteFile(h_pipe, &msg, sizeof(message), nullptr, nullptr))
    {
        std::cout << "error of writing in pipe";
        return false;
    }
    return true;
}

bool send_writing_request(HANDLE h_pipe, message& msg)
{
    std::cout << "enter new id:\n";
    std::cin >> msg.emp.num;
    std::cout << "enter new name:\n";
    std::cin >> msg.emp.name;
    std::cout << "enter new hours:\n";
    std::cin >> msg.emp.hours;
    msg.reading = 2;
    if (!WriteFile(h_pipe, &msg, sizeof(message), nullptr, nullptr))
    {
        std::cout << "error of writing in pipe";
        return false;
    }
    return true;
}

HANDLE open_pipe(const char* pipe_name)
{
    const HANDLE h_pipe = CreateFile(pipe_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (h_pipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "cannot open pipe";
        return nullptr;
    }
    return h_pipe;
}