#include <iostream>
#include<Windows.h>
#include<string>

bool start_process(HANDLE event, const std::string& file_name, int i);
bool read_message(HANDLE h_file, char*& message);
bool rewrite_file(HANDLE h_file, int number_of_records);
void do_work(HANDLE h_mutex, HANDLE h_file);

const int SIZE_OF_BUFFER = 20;
int number_of_records;

int main()
{
    int number_of_senders;
    std::string file_name;
    std::cout << "enter name of binary file\n";
    std::cin >> file_name;
    std::cout << "enter number of records\n";
    std::cin >> number_of_records;
    std::cout << "enter number of senders\n";
    std::cin >> number_of_senders;

    const HANDLE h_file = CreateFile(LPCSTR(file_name.c_str()),
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     nullptr,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
    const HANDLE h_mutex = CreateMutex(nullptr, FALSE, TEXT("file_mutex"));
    if (h_mutex == nullptr)
    {
        std::cout << "cannot create mutex";
        return 1;
    }

    auto* events = new HANDLE[number_of_senders];           
    for (int i = 0; i < number_of_senders; i++)
    {
        if (!start_process(events[i], file_name, i))
        {
            delete[]events;
            CloseHandle(h_mutex);
            CloseHandle(h_file);
            return 2;
        }          
    }
	
    WaitForMultipleObjects(number_of_senders, events, TRUE, INFINITE);
    do_work(h_mutex, h_file);

    CloseHandle(h_mutex);
    CloseHandle(h_file);
    delete[]events;
    return 0;
}

void do_work(HANDLE h_mutex, HANDLE h_file)
{
    int flag;
    char* message = new char[20];
    while (true) {
        std::cout << "enter 0 to read message or 1 to exit process\n";
        std::cin >> flag;
        if (flag == 0)
        {
            WaitForSingleObject(h_mutex, INFINITE);
            if (!read_message(h_file, message))
                continue;
            std::cout << message << "\n";

            if (!rewrite_file(h_file, number_of_records))
                continue;
            ReleaseMutex(h_mutex);
        }
        else
        {
            return;
        }
    }
}

bool read_message(HANDLE h_file, char*& message)
{
    SetFilePointer(h_file, 0, nullptr, FILE_BEGIN);
    const DWORD file_size = GetFileSize(h_file, nullptr);
    if (file_size == 0ul)
    {
        std::cout << "file is empty\n";
        return false;
    }
    if (!ReadFile(h_file, message, 20, nullptr, nullptr))
    {
        std::cout << "error of reading\n";
        return false;
    }
    return true;
}

bool start_process(HANDLE event, const std::string& file_name, int i)
{

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    if (!CreateProcess("D:\\ОС\\Sender\\Debug\\Sender.exe",
        LPSTR((file_name + " " + std::to_string(number_of_records)).c_str()),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
        nullptr,
        nullptr,
        &si,
        &pi))
    {
        std::cout << "error with " << i + 1 << " sender creating\n";
        std::cout << GetLastError() << "\n";
        return false;
    }
    event = CreateEvent(nullptr, TRUE, FALSE, LPSTR(("event" + std::to_string(pi.dwProcessId)).c_str()));
    ResumeThread(pi.hThread);
    return true;
}

bool rewrite_file(HANDLE h_file, int number_of_records)
{
    char* all_file = new char[SIZE_OF_BUFFER * number_of_records];
    char* temp_line = new char[SIZE_OF_BUFFER];
    int count_of_lines = 0;
    DWORD bytes_read = -1;
    while (true)
    {
        if (!ReadFile(h_file, temp_line, SIZE_OF_BUFFER, &bytes_read, nullptr))
        {
            std::cout << "error of reading";
            return false;
        }
        if (bytes_read == 0)
            break;
        memcpy_s(all_file + SIZE_OF_BUFFER * count_of_lines, SIZE_OF_BUFFER, temp_line, SIZE_OF_BUFFER);
        count_of_lines++;
    }
    SetFilePointer(h_file, 0, nullptr, FILE_BEGIN);
    WriteFile(h_file, all_file, count_of_lines * SIZE_OF_BUFFER, nullptr, nullptr);
    SetEndOfFile(h_file);
    return true;
}