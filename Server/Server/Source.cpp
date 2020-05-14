#include<iostream>
#include<fstream>
#include <iomanip>
#include <map>
#include<vector>

#include "../Client/message.h"
#include "../Client/record_data.h"
#include "employee.h"
#include "parameter.h"

void fill_file(const std::string& file_name, int number_of_records, std::map<int, record_data>& records_data);
employee enter_struct();
bool create_pipe(const std::string& current_pipe_name, int i, std::vector<HANDLE>& pipes);
bool start_process(int index, const std::string& current_pipe_name);
DWORD _stdcall process_client(LPVOID parameter_struct);
void print_file(const std::string& file_name);
void wait_answer(message msg, parameter current_parameter);
void destroy(HANDLE* threads, std::vector<HANDLE>& pipes, int count_of_clients);

CRITICAL_SECTION cs_console;
CRITICAL_SECTION cs_records_data;


int main()
{
    std::string file_name;
	std::map<int, record_data>records_data;
	int number_of_records;
    std::cout << "enter binary file name:\n";
    std::cin >> file_name;
	std::cout << "enter number of records:\n";
	std::cin >> number_of_records;
	fill_file(file_name, number_of_records, records_data);

	int number_of_clients;
	std::cout << "enter number of clients:\n";
	std::cin >> number_of_clients;

    const std::string pipe_name = R"(\\.\pipe\OS5_client_pipe_)";
	std::vector<HANDLE>pipes(number_of_clients);
	std::vector<parameter>parameters(number_of_clients);
	const auto threads = new HANDLE[number_of_clients];

	InitializeCriticalSection(&cs_console);
	InitializeCriticalSection(&cs_records_data);
	
	for(int i = 0;i<number_of_clients;i++)
	{
		auto current_pipe_name = pipe_name + std::to_string((i + 1));
		if(!create_pipe(current_pipe_name,i,pipes))
		{
			destroy(threads,pipes,number_of_clients);
			return 1;
		}
		if (!start_process(i,current_pipe_name)) {
			destroy(threads, pipes, number_of_clients);
			return 2;
		}
		parameters.at(i).file_name = file_name;
		parameters.at(i).h_pipe = pipes.at(i);
		parameters.at(i).records_data = &records_data;

		threads[i] = CreateThread(nullptr, 0, process_client, &parameters.at(i), 0, nullptr);
		if(threads[i] == nullptr)
		{
			std::cout << "cannot start thread number " << i + 1;
			destroy(threads, pipes, number_of_clients);
			return 3;
		}
	}

	WaitForMultipleObjects(number_of_clients, threads, TRUE, INFINITE);
	print_file(file_name);
	destroy(threads, pipes, number_of_clients);
	return 0;
}


void fill_file(const std::string& file_name, int number_of_records, std::map<int, record_data>& records_data)
{
	std::ofstream binary_file(file_name, std::ios::binary);
	for (int i = 0; i < number_of_records; i++)
	{
		std::cout << "Enter #" << i + 1 << " employee:\n";
		employee emp = enter_struct();
		record_data temp;
		temp.cond_of_blocking = 0;
		temp.offset = binary_file.tellp();
		binary_file.write(reinterpret_cast<char*>(&emp), sizeof(emp));
		records_data.insert({ emp.num, temp });
	}
	binary_file.close();
}

bool create_pipe(const std::string& current_pipe_name,int i,std::vector<HANDLE>& pipes)
{
	pipes.at(i) = CreateNamedPipe(current_pipe_name.c_str(),
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		sizeof(message),
		sizeof(message),
		10000,
		nullptr);

	if (pipes.at(i) == INVALID_HANDLE_VALUE)
	{
		std::cout << "cannot start pipe number " << i + 1;
		return false;
	}
	return true;
}

void destroy(HANDLE* threads, std::vector<HANDLE>& pipes, int count_of_clients)
{
	DeleteCriticalSection(&cs_console);
	DeleteCriticalSection(&cs_records_data);
	for (int i = 0; i < count_of_clients; i++) 
	{
		CloseHandle(threads[i]);
		CloseHandle(pipes.at(i));
	}
	delete[]threads; 
}

bool start_process(int index, const std::string& current_pipe_name)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	if (!CreateProcess("D:\\ня\\Server\\Debug\\Client.exe",
		const_cast<char*>(current_pipe_name.c_str()),
		nullptr,
		nullptr,
		FALSE,
		CREATE_NEW_CONSOLE,
		nullptr,
		nullptr,
		&si,
		&pi))
	{
		std::cout << "error with " << index + 1 << " client creating\n";
		std::cout << GetLastError() << "\n";
		return false;
	}
	return true;
}

void print_file(const std::string& file_name)
{
	std::ifstream inputFile(file_name, std::ios::binary);
	std::cout << "Employer's ID" << std::setw(23) << "name   " << std::setw(9) << "hours\n";
	employee emp;
	while (inputFile.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
		std::cout << std::setw(13) << emp.num << std::setw(20) << emp.name << std::setw(12) << emp.hours << "\n";
	}
	inputFile.close();
}

DWORD _stdcall process_client(LPVOID parameter_struct)
{
    const parameter current_parameter = *static_cast<parameter*>(parameter_struct);
    message msg;
	ConnectNamedPipe(current_parameter.h_pipe, nullptr);
	while(true)
	{
		if (!ReadFile(current_parameter.h_pipe, &msg, sizeof(message), nullptr, nullptr))
		{
			const auto code = GetLastError();
			if (code == ERROR_BROKEN_PIPE)
				break;

			EnterCriticalSection(&cs_console);
			std::cout << code << " " << "error of reading pipe";
			LeaveCriticalSection(&cs_console);

		}
		if (msg.reading<2)
		{
			while (current_parameter.records_data->at(msg.id).cond_of_blocking == msg.reading+1)
				continue;

			EnterCriticalSection(&cs_records_data);
			current_parameter.records_data->at(msg.id).cond_of_blocking = 2 - msg.reading;
			LeaveCriticalSection(&cs_records_data);
			std::ifstream file(current_parameter.file_name);
			file.seekg(current_parameter.records_data->at(msg.id).offset);
			employee emp;
			file.read(reinterpret_cast<char*>(&emp), sizeof(emp));
			if(!WriteFile(current_parameter.h_pipe, &emp, sizeof(employee), nullptr, nullptr))
			{
				const auto code = GetLastError();
				if (code == ERROR_BROKEN_PIPE)
					break;
				else
				{
					EnterCriticalSection(&cs_console);
					std::cout << "error of writing in pipe";
					LeaveCriticalSection(&cs_console);
				}
			}
			file.close();			
		}
		else
		{
			employee emp = msg.emp;
			std::ofstream file(current_parameter.file_name, std::ios::binary);
			file.seekp(current_parameter.records_data->at(msg.id).offset);
			file.write(reinterpret_cast<char*>(&emp), sizeof(emp));
			file.close();
		}
		if (msg.reading != 1)
			wait_answer(msg, current_parameter);
	}
	return 0;
}

void wait_answer(message msg, parameter current_parameter)
{
    while (msg.reading != 4) 
		{
			if (!ReadFile(current_parameter.h_pipe, &msg, sizeof(message), nullptr, nullptr))
			{
				const auto code = GetLastError();
				if (code == ERROR_BROKEN_PIPE)
					break;
				else
				{
					EnterCriticalSection(&cs_console);
					std::cout << "error of reading pipe";
					LeaveCriticalSection(&cs_console);
				}
			}
		}
		EnterCriticalSection(&cs_records_data);
		current_parameter.records_data->at(msg.id).cond_of_blocking = 0;
		LeaveCriticalSection(&cs_records_data);
}

employee enter_struct() {
	employee worker;
	std::cout << "enter identify number: ";
	std::cin >> worker.num;
	std::cout << "enter name: ";
	std::cin >> worker.name;
	std::cout << "enter number of hours: ";
	std::cin >> worker.hours;
	return worker;
}