#include <iostream>
#include<unistd.h>
#include<fstream>
#include<string>
#include<string.h>
#include <sys/wait.h>
#include <iomanip>
#include "employee.h"


bool wait(std::string);
void printToStream(std::ifstream&,std::ostream&,int*);
void showBinaryFile(std::string);
void showSimpleFile(std::string);
int startThread(std::string,std::string,std::string,int*);

std::string binaryFileName;
int numberOfRecords;

std::string nameReportFile;
int paymentForHour;

int main() {

    std::cout << "enter name of file:\n";
    std::cin >> binaryFileName;
    std::cout << "enter number of records:\n";
    std::cin >> numberOfRecords;


    startThread("Creator",binaryFileName,"",&numberOfRecords);
    showBinaryFile(binaryFileName);

    std::cout << "enter report's file name:\n";
    std::cin >> nameReportFile;
    std::cout << "enter payment for hour\n";
    std::cin >> paymentForHour;

    startThread("Reporter",binaryFileName,nameReportFile,&paymentForHour);
    showSimpleFile(nameReportFile);
    return 0;
}



bool wait(std::string name){
    int status;
    wait(&status);
    if (!WIFEXITED(status)) {
        std::cerr << "error of "<<name;
        return false;
    }
    return true;
}

int startThread(std::string name, std::string nameFile,
        std::string nameReport,int* param)
{
    int pid = fork();
    if (pid == -1) {
        std::cerr << "error of starting "<<name;
        return -1;
    } else if (pid == 0) {
        std::string path = "/home/temp/"+name;
        execl(path.c_str(), nameFile.c_str(),nameReport.c_str(),std::to_string(*param).c_str(),NULL);
        exit(0);
    }else {
        if(!wait(name))
            return -2;
        return 0;
    }
}


void showBinaryFile(std::string binaryFileName){
    std::ifstream binaryFile(binaryFileName, std::ios::binary);
    employee emp;
    std::cout << "\n"<<binaryFileName << ":\n";
    printToStream(binaryFile,std::cout,NULL);
    std::cout << "\n\n";
    binaryFile.close();
}

void showSimpleFile(std::string nameReport){
    std::ifstream reportFile(nameReport);
    std::string temp;
    std::cout << "\n\n" << nameReport << ":\n";
    while (reportFile.peek() != EOF) {
        std::getline(reportFile, temp);
        std::cout << temp << "\n";
    }
    reportFile.close();
}

void printToStream(std::ifstream& binaryFile, std::ostream& stream,int* number){
    employee emp;
    stream << "Employee's number   " << std::setw(20) << "Employee's name   " << std::setw(10) << "hours";
    if(number!=NULL)
        stream<< std::setw(16) << "payment\n";
    stream<<"\n";
    while (binaryFile.read((char *) &emp, sizeof(emp))) {
        stream << std::setw(17) << emp.num << std::setw(20) << emp.name << std::setw(13) << emp.hours;
        if(number!=NULL)
            stream<< std::setw(15) << emp.hours * (*number);
        stream<<"\n";
    }
}