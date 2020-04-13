#include <iostream>
#include<Windows.h>
#include<string>

int main()
{
    int number_of_records,number_of_senders;
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
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    auto* events = new HANDLE[number_of_senders];        
    
    for (int i = 0; i < number_of_senders; i++)
    {
        if (!CreateProcess("D:\\ОС\\Sender\\Debug\\Sender.exe",
            LPSTR((file_name+" "+std::to_string(number_of_records)).c_str()),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE|CREATE_SUSPENDED,
            nullptr,
            nullptr,
            &si,
            &pi))
        {
            std::cout << "error with " << i + 1 << " sender creating\n";
            std::cout << GetLastError()<<"\n";
            return 1;
        }
        events[i] = CreateEvent(NULL, TRUE, FALSE, LPSTR(("event" + std::to_string(pi.dwProcessId)).c_str()));
        ResumeThread(pi.hThread);
        system("pause");
    }
    
	if(h_mutex == NULL)
	{
        std::cout << "cannot create mutex";
        return 2;
	}
    WaitForMultipleObjects(number_of_senders, events, TRUE, INFINITE);
    
    int flag;
    char* message = new char[20];
    char* all_file = new char[20*number_of_records];
    char* temp_line = new char[20];
    DWORD bytes_read;
    while (true) {
        std::cout << "enter 0 to read message or 1 to exit process\n";
        std::cin >> flag;
        if (flag == 0)
        {
            WaitForSingleObject(h_mutex, INFINITE);
            SetFilePointer(h_file, 0, nullptr, FILE_BEGIN);
            const DWORD file_size = GetFileSize(h_file, nullptr);
        	if (file_size == 0ul)
        	{
                std::cout << "file is empty\n";
                continue;
        	}
        	if(!ReadFile(h_file, message, 20, nullptr, nullptr))
        	{
                std::cout << "error of reading\n";
                continue;
        	}
            std::cout << message<<"\n";
            int count_of_lines = 0;
            bytes_read = -1;
        	while (true)
        	{
                if (!ReadFile(h_file, temp_line, 20, &bytes_read, nullptr))
                {
                    std::cout << "error of reading";
                    continue;
                }
                if (bytes_read == 0)
                    break;
                memcpy_s(all_file + 20 * count_of_lines, 20, temp_line, 20);
                count_of_lines++;
        	}
            SetFilePointer(h_file, 0, nullptr, FILE_BEGIN);
            WriteFile(h_file, all_file, count_of_lines * 20, nullptr, nullptr);
            SetEndOfFile(h_file);

            ReleaseMutex(h_mutex);
        }
        else
        {
            CloseHandle(h_mutex);
            return 0;
        }
    }

}
