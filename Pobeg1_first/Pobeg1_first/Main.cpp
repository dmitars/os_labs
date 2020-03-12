#include"Main.h"
#include"employee.h"
#pragma warning(disable:4996)

bool Main::doMain() {
	std::string first_file;
	PROCESS_INFORMATION pi;
	int numberOfRecords;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	std::cout << "enter name of file:\n";
	std::cin >> first_file;
	std::cout << "enter number of records:\n";
	std::cin >> numberOfRecords;

	std::string firstComLine = first_file + " " + std::to_string(numberOfRecords);
	LPSTR szArgs = new CHAR[firstComLine.length() + 1];
	strcpy(szArgs, firstComLine.c_str());
		if (!CreateProcess("D:\\ОС\\Pobeg1_first\\Debug\\Creator.exe", szArgs, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL,
			NULL, &si, &pi))
		{
			std::cerr << "cannot start Creator";
			return false;
		}
		else
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			DWORD code;
			if (GetExitCodeProcess(pi.hProcess, &code))
				std::cout << "exit creator's code: " << code << "\n";
			if(code != 0)
			{
				std::cerr << "error of Reporter's work\n";
				return -1;
			}
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	

	std::ifstream binaryFile(first_file, std::ios::binary);
	employee emp;
	std::cout << first_file << ":\n";
	setlocale(LC_ALL, "rus");
	std::cout << "Номер сотрудника   " << std::setw(20) << "имя сотрудника   " << std::setw(10) << "часы\n";
	while (binaryFile.read((char*)&emp, sizeof(emp))) {
		std::cout << std::setw(16) << emp.num << std::setw(20) << emp.name << std::setw(12) << emp.hours << "\n";
	}
	std::cout << "\n\n\n";
	binaryFile.close();

	std::string nameReport;
	int paymentForHour;
	std::cout << "enter report's file name:\n";
	std::cin >> nameReport;
	std::cout << "enter payment for hour\n";
	std::cin >> paymentForHour;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);


	delete[]szArgs;
	std::string secondComLine = first_file + " " + nameReport + " " + std::to_string(paymentForHour);
	szArgs = new CHAR[secondComLine.length() + 1];
	strcpy(szArgs, secondComLine.c_str());

	if (!CreateProcess("D:\\ОС\\Pobeg1_first\\Debug\\Reporter.exe", (LPSTR)secondComLine.c_str(), NULL, NULL, FALSE,
	                   CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		std::cerr << "cannot start Reporter";
		return false;
	}
	else
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD code;
		if (GetExitCodeProcess(pi.hProcess, &code))
			std::cout << "exit creator's code: " << code << "\n";
		if (code != 0)
		{
			std::cerr << "error of Reporter's work\n";
			return -1;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	std::ifstream reportFile(nameReport);
	std::string temp;
	std::cout << "\n\n" << nameReport << ":\n";
	while (reportFile.peek() != EOF) {
		std::getline(reportFile, temp);
		std::cout << temp << "\n";
	}
	reportFile.close();
	delete[]szArgs;
	return true;
}