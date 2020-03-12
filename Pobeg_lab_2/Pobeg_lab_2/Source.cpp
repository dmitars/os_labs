#include<iostream>
#include<windows.h>

int* mass;
int min, max;
double average;

CRITICAL_SECTION cs;

void enter_mass(int);

bool start_thread(HANDLE&,const char*, int, DWORD(*f)(LPVOID));
bool wait_thread(HANDLE,const char*);
void change_data(int);
void print_data(int);

DWORD find_min_and_max(LPVOID);
DWORD find_average(LPVOID);

int main() {
	int number;
	
	std::cout << "enter mass size\n";
	std::cin >> number;
	enter_mass(number);

	InitializeCriticalSection(&cs);
	
	HANDLE min_max;
	if (!start_thread(min_max,"min_max", number, find_min_and_max))
		return -1;
	
	HANDLE average;
	if (!start_thread(average,"average", number, find_average))
		return -2;

	if (!wait_thread(min_max, "min_max"))
		return -3;
	if (!wait_thread(average, "average"))
		return -4;
	
	DeleteCriticalSection(&cs);
	
	change_data(number);
	print_data(number);
	
	return 0;
}


void enter_mass(int number) {
	mass = new int[number];
	for (int i = 0; i < number; i++) {
		std::cout << i+1 << " :";
		std::cin >> mass[i];
	}
}

void change_data(int number)
{
	for (int i = 0; i < number; i++)
	{
		if (mass[i] == min)
			mass[i] = int(average);
		if (mass[i] == max)
			mass[i] = int(average);
	}
}

void print_data(int number)
{
	std::cout << "result: ";
	for (int i = 0; i < number; i++)
		std::cout << mass[i] << " ";
}

DWORD find_min_and_max(LPVOID number_pointer)
{
	const int number = *static_cast<int*>(number_pointer);
	min = mass[0];
	max = mass[0];
	for(int i = 1;i<number;i++)
	{
		if(max<mass[i])
			max = mass[i];
		Sleep(7);
		
		if(min>mass[i])
			min = mass[i];
		Sleep(7);
	}
	EnterCriticalSection(&cs);
	std::cout << "max: " << max << "\n";
	std::cout << "min: " << min << "\n";
	LeaveCriticalSection(&cs);
	return true;
}

DWORD find_average(LPVOID number_pointer)
{
	const int number = *static_cast<int*>(number_pointer);
	int sum = mass[0];
	for(int i = 1;i<number;i++)
	{
		sum += mass[i];
		Sleep(12);
	}
	average = static_cast<double>(sum) / number;
	EnterCriticalSection(&cs);
	std::cout <<"average: "<<average<<"\n";
	LeaveCriticalSection(&cs);
	return true;
}

bool start_thread(HANDLE& h_thread,const char* name, int number,DWORD (*function)(LPVOID))
{
	 h_thread = CreateThread(nullptr, NULL,LPTHREAD_START_ROUTINE(function), &number, NULL, nullptr);
	if (!h_thread) {
		std::cerr << "cannot create thread " << name << "\n";
		return false;
	}
	return true;
}

bool wait_thread(HANDLE my_thread, const char* name)
{
	WaitForSingleObject(my_thread, INFINITE);
	DWORD exit_code;
	GetExitCodeThread(my_thread, &exit_code);
	if (exit_code == NULL) {
		std::cerr << "error on thread " << name << "\n";
		CloseHandle(my_thread);
		return false;
	}
	return true;
}