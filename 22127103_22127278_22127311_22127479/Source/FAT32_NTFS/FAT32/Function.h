#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

int ReadSector(LPCWSTR drive, int readPoint, BYTE*& sector, int size);
int Get_Value_At_Offset(BYTE* sector, int offset, int number);
std::string Get_String(BYTE* sector, int offset, int number);