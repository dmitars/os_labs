#include<iostream>
#include<windows.h>
#include<vector>
#include <cstdlib>
#include<stdio.h>

typedef struct parameter{
	HANDLE* break_event;
	HANDLE* stop_event;
	int number;
}parameter;

CRITICAL_SECTION cs;
CRITICAL_SECTION cs_console;
HANDLE start_event;

DWORD doMarkerWork(LPVOID);
bool createThreads(std::vector<HANDLE>& threads, std::vector<parameter>& parameters,HANDLE* break_events, HANDLE* stop_events, int size,int number_of_threads);
bool work_with_threads(HANDLE* stop_events, HANDLE* break_events, std::vector<HANDLE>& threads_vector, int number_of_threads, int size);
void init_mass(HANDLE* mass_events,const char* temp_name, int number_of_threads);
bool set_event(HANDLE* h_event, const char* name);

void print(std::vector<int>&);
void change_number(int index, int value);
void free_resources(HANDLE* stop_events, HANDLE* break_events);
int get_thread_index(int number_of_threads, const bool* flags_of_broken_threads);

const int MY_NUMBER = 11;
std::vector<int> numbers;

int main()
{	
	int size;

	std::cout << "enter size of array:\n";
	std::cin >> size;
	numbers.resize(size);
	
	int number_of_threads;
	std::cout << "enter number of threads:\n";
	std::cin >> number_of_threads;
	
	std::vector<HANDLE>threads_vector;
	threads_vector.reserve(number_of_threads);

	const auto break_events = new HANDLE[number_of_threads];
	const auto stop_events = new HANDLE[number_of_threads];

	std::vector<parameter>parameters;
	parameters.reserve(number_of_threads);
	init_mass(break_events, "break_event_", number_of_threads);
	init_mass(stop_events, "stop_event_", number_of_threads);

	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&cs_console);

	start_event = CreateEvent(nullptr, TRUE, FALSE, TEXT("start_event"));

	threads_vector.reserve(number_of_threads);
	if (!createThreads(threads_vector, parameters, break_events, stop_events,size,  number_of_threads))
	{
		free_resources(stop_events, break_events);
		return 1;
	}
	
	
	if (!set_event(&start_event,"start_event"))
	{
		free_resources(stop_events, break_events);
		return 2;
	}

	if(!work_with_threads(stop_events,break_events,threads_vector,number_of_threads,size))
	{
		free_resources(stop_events,break_events);
		return 3;
	}	
	
	free_resources(stop_events, break_events);
	return 0;
}

bool createThreads(std::vector<HANDLE>& threads,std::vector<parameter>&parameters,HANDLE* break_events, HANDLE* stop_events,int size,int number_of_threads)
{
	parameter struct_parameters;
	struct_parameters.number = size;
	for(int i= 0;i<number_of_threads;i++)
	{
		struct_parameters.break_event = break_events+i;
		struct_parameters.stop_event = stop_events + i;
		parameters.push_back(struct_parameters);
		
		threads.push_back(CreateThread(nullptr, NULL, LPTHREAD_START_ROUTINE(doMarkerWork), 
			&parameters.at(i), NULL, nullptr));
		if (!threads[i]) {
			EnterCriticalSection(&cs_console);
			std::cerr << "cannot create thread " << i+1 << "\n";
			LeaveCriticalSection(&cs_console);
			return false;
		}
	}
	return true;
}

bool work_with_threads(HANDLE* stop_events,HANDLE* break_events,std::vector<HANDLE>&threads_vector,int number_of_threads,int size)
{
	bool* flags_of_broken_threads = new bool[number_of_threads]();
	int count = 0;
	while (count < number_of_threads) {
		WaitForMultipleObjects(number_of_threads, stop_events, TRUE, INFINITE);
		print(numbers);

		const int thread_index = get_thread_index(number_of_threads, flags_of_broken_threads);

		flags_of_broken_threads[thread_index - 1] = true;
		if (!set_event(&break_events[thread_index - 1], "break_event"))
		{
			free_resources(stop_events, break_events);
			delete[]flags_of_broken_threads;
			return false;
		}

		WaitForSingleObject(threads_vector.at(thread_index - 1), INFINITE);
		for (int i = 0; i < number_of_threads; i++)
		{
			if (!flags_of_broken_threads[i])
				ResetEvent(stop_events[i]);
		}
		if (!set_event(&start_event, "start_event"))
		{
			free_resources(stop_events, break_events);
			return false;
		}

		count++;
	}
	delete[]flags_of_broken_threads;
	return true;
}

int get_thread_index(int number_of_threads, const bool* flags_of_broken_threads)
{
	int thread_index;
	bool incorrect = true;
	EnterCriticalSection(&cs_console);
	while (incorrect) {
		std::cout << "\nenter index of thread to stop\n";
		std::cin >> thread_index;
		if (thread_index > 0 && thread_index <= number_of_threads && !flags_of_broken_threads[thread_index - 1])
			incorrect = false;
		else
			printf("\nincorrect index");
	}
	LeaveCriticalSection(&cs_console);
	
	return thread_index;
}

void free_resources(HANDLE* stop_events,HANDLE* break_events)
{
	delete[]stop_events;
	delete[]break_events;
	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&cs_console);
}

bool set_event(HANDLE* h_event,const char* name)
{
	if (!SetEvent(*h_event))
	{
		EnterCriticalSection(&cs_console);
		printf("SetEvent of %s failed (%lu)\n",name, GetLastError());
		LeaveCriticalSection(&cs_console);
		return false;
	}
	return true;
}

void init_mass(HANDLE* mass_events,const char* temp_name,int number_of_threads)
{
	char* name = new char[20];
	for (int i = 0; i < number_of_threads; i++)
	{
		sprintf_s(name, 20, "%s%i", temp_name, i);
		mass_events[i] = CreateEvent(nullptr, TRUE, FALSE, LPCWSTR(name));
	}
}


void print(std::vector<int>& numbers)
{
	const int size = numbers.size();
	EnterCriticalSection(&cs_console);
	for (int i = 0; i < size; i++)
		printf("\nindex %i: %i", i, numbers.at(i));
	LeaveCriticalSection(&cs_console);
}

DWORD doMarkerWork(LPVOID pointer)
{

	WaitForSingleObject(start_event, INFINITE);

	srand(MY_NUMBER);

	const auto parameters_pointer = static_cast<parameter*>(pointer);
	const int size = parameters_pointer->number;
	std::vector<int>indexes;

	while(true)
	{
		int index = rand() % size;
		if(numbers.at(index) == 0)
		{
			Sleep(5);
			change_number(index, MY_NUMBER);
			indexes.push_back(index);
			Sleep(5);
		}
		else
		{
			EnterCriticalSection(&cs_console);
			std::cout <<"\n\nMy number: "<<MY_NUMBER;
			std::cout << "\nCount of marked elements: " << indexes.size();
			std::cout << "\nCannot be marked: " << index;
			LeaveCriticalSection(&cs_console);
			
			ResetEvent(start_event);
			SetEvent(*(parameters_pointer->stop_event));

			const auto events = new HANDLE[2];
			events[0] = start_event;
			events[1] = *(parameters_pointer->break_event);
			const DWORD exit_code = WaitForMultipleObjects(2, events, FALSE, INFINITE);

			if (exit_code == WAIT_OBJECT_0+1) {
				const int indexes_size = indexes.size();
				for (int i = 0; i < indexes_size; i++)
					change_number(indexes.at(i), 0);

				delete[]events;
				break;
			}
			else
			{
				ResetEvent(*(parameters_pointer->stop_event));
			}
		}
		
	}
	return 0;
}

void change_number(int index, int value)
{
	EnterCriticalSection(&cs);
	numbers.at(index) = value;
	LeaveCriticalSection(&cs);
}