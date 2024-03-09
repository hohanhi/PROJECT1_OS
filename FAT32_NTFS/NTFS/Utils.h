#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <ctime>
#include <math.h>
#include <iomanip>
#include <Vector>
#include <stack>
#include <algorithm>
#include <fcntl.h> //_O_WTEXT
using namespace std;

class VOLUME //volume interface
{
public:
	virtual int InitVolume(LPCWSTR  drive) = 0;
	virtual int OpenVolume(LPCWSTR  drive) = 0;
	virtual int ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte = 512) = 0;
	virtual void printPartitionBootSector() = 0;
	virtual void printDirectoryTree() = 0;
	virtual	void printDetailedDirectoryTree() = 0;
	virtual void printEntryData(wstring name) = 0;
};

//read size byte from byte to buffer
void readFile(char* buffer, BYTE byte[], int size);

//convert lib
string DecToDateTime(ULONGLONG time);
string DectoTime(WORD dec);
string DectoDate(WORD dec);
string ByteToString(BYTE byte[], int size);
//if(mode==true, dec=value(little endian) else bigEndien ||defaut=true)
UINT32 HexToDec(string hexSector, bool mode = true);
UINT StringToDec(BYTE[], int size, bool mode = true);
string DataSizeFomat(UINT64 size);
bool IsEquals(wstring str1, wstring str2);

////Fat
////CHeck Entry;
//bool isDelete(BYTE[32]);
//bool isFolder(BYTE[32]);
//bool isFile(BYTE[32]);
//bool isSUB_ENTRY(BYTE[32]);
//bool isDot(BYTE[32]);
//bool isHiddenEntry(BYTE[32]);
//bool isVOLUME_ENTRY(BYTE[32]);
////NTFS

