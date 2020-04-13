#include <iostream>
#include<Windows.h>
#include<string>

int main(char*argc, char**argv)
{
    const std::string file_name = argv[0];
    const int number_of_records = std::strtol(argv[1],nullptr,10);
    system("pause");
    int flag;
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
    while(true)
    {
        std::cout << "enter 0 to send message or 1 to close process\n";
        std::cin >> flag;
        std::cin.ignore();
        if(flag == 0)
        {
            char* buff = new char[20];
            std::cout << "enter your message:\n";
            std::cin.getline(buff,20);
          
            WaitForSingleObject(h_mutex, INFINITE);
            int high_size;
            int low_size = GetFileSize(h_file, LPDWORD(&high_size));
            while(low_size == 20*number_of_records)
            {
                ReleaseMutex(h_mutex);
                Sleep(1000);
                WaitForSingleObject(h_mutex, INFINITE);
                low_size = GetFileSize(h_file, LPDWORD(&high_size));
            }

            SetFilePointer(h_file,0, nullptr, FILE_END);
//            SetEndOfFile(h_file);

            WriteFile(h_file, buff, 20, nullptr, nullptr);
            ReleaseMutex(h_mutex);
        }
        else
        {
            CloseHandle(h_mutex);
            CloseHandle(h_file);
            return 0;
        }
    }
}


void doWork()
{
    
}