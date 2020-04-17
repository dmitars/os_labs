#include <iostream>
#include<Windows.h>
#include<string>

int get_user_choice();

void doWork(HANDLE h_mutex, HANDLE h_file);
void wait_place(HANDLE h_mutex, HANDLE h_file);

const int SIZE_OF_BUFFER = 20;
int number_of_records;

int main(char*argc, char**argv)
{
    const std::string file_name = argv[0];
    number_of_records = std::strtol(argv[1],nullptr,10);
    
    const auto process_id = GetCurrentProcessId();
    const auto h_event = OpenEvent(EVENT_MODIFY_STATE, FALSE, ("event" + std::to_string(process_id)).c_str());
    if(h_event == nullptr)
    {
        std::cout << "cannot open event";
        return 1;
    }
    SetEvent(h_event);
    const HANDLE h_mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("file_mutex"));
    const HANDLE h_file = CreateFile(file_name.c_str(),
                                     GENERIC_READ|GENERIC_WRITE,
                                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
    if (h_mutex == nullptr)
    {
        std::cout << "some error with mutex";
        return 2;
    }

    doWork(h_mutex, h_file);
    CloseHandle(h_mutex);
    CloseHandle(h_file);
    return 0;
}


void doWork(HANDLE h_mutex, HANDLE h_file)
{
    int flag;
    while (true)
    {
        flag = get_user_choice();
        if (flag == 0)
        {
            char* buff = new char[SIZE_OF_BUFFER];
            std::cout << "enter your message:\n";
            std::cin.getline(buff, SIZE_OF_BUFFER);

            WaitForSingleObject(h_mutex, INFINITE);
            wait_place(h_mutex, h_file);

            SetFilePointer(h_file, 0, nullptr, FILE_END);
            WriteFile(h_file, buff, SIZE_OF_BUFFER, nullptr, nullptr);
            ReleaseMutex(h_mutex);
        }
        else
        {
            return;
        }
    }
}

int get_user_choice()
{
    int flag;
    std::cout << "enter 0 to send message or 1 to close process\n";
    std::cin >> flag;
    std::cin.ignore();
    return flag;
}

void wait_place(HANDLE h_mutex, HANDLE h_file)
{
    int high_size;
    int low_size = GetFileSize(h_file, LPDWORD(&high_size));
    while (low_size == SIZE_OF_BUFFER * number_of_records)
    {
        ReleaseMutex(h_mutex);
        Sleep(1000);
        WaitForSingleObject(h_mutex, INFINITE);
        low_size = GetFileSize(h_file, LPDWORD(&high_size));
    }
}