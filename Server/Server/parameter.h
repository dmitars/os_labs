#pragma once
#include <string>
#include<Windows.h>

typedef struct parameter{
    std::string file_name;
    HANDLE h_pipe;
    std::map<int, record_data>* records_data;
}parameter;
