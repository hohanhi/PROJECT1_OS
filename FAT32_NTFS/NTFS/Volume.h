#pragma once

#include "PartitionBootSector.h"
#include "Utils.h"
#include "MFT.h"

class ENTRY {
public:
	DWORD		_entryIndex;
	DWORD		_sector;
	FILENAME	_filename;
public:
	ENTRY();
	ENTRY(DWORD index);
	virtual void setFilename(FILENAME Name);
	virtual void addEntry(ENTRY* e) {}
	virtual void printDirectoryTree(int x = 0) {};
	virtual void printEntryInfo(int x = 0);
	virtual ENTRY* SearchEntry(wstring name) { return nullptr;}
};

class FILE_NTFS :public ENTRY {

public:
	FILE_NTFS(DWORD index) : ENTRY(index){ }
	void printDirectoryTree(int x);
	ENTRY* SearchEntry(wstring name);
};

class FOLDER :public ENTRY {
private:
	vector <ENTRY*>	list;
public:
	void addEntry(ENTRY* e);
	FOLDER(DWORD index) : ENTRY(index) {}
	void printDirectoryTree(int x);
	void printEntryInfo(int x = 0);
	ENTRY* SearchEntry(wstring name);
	~FOLDER();
};

class  VOLUME_NTFS :public VOLUME
{
private:
	HANDLE		_device;
	NTFSPBSector _PBSector;
	WORD		_secPerClus;
	WORD		_bytePerSec;
	DWORD		_fstMTF;
	WORD		_bytePerRecord;
	ENTRY* _rootEntry;
public:
	VOLUME_NTFS();
	~VOLUME_NTFS();
	int InitVolume(LPCWSTR drive);
	int  OpenVolume(LPCWSTR  drive);
	int  ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte = 512);
	void printPartitionBootSector();
	void readDirectoryTree(ENTRY* folder = nullptr);
	void printDirectoryTree();
	void printDetailedDirectoryTree();
	void printEntryData(wstring filename);
	void readIndexEntriesFromMFTRecord(MFT_RECORD& mftRecord, ENTRY*& folder, ENTRY*& entry);
	void readIndexEntriesFromIndexAllocationAttribute(MFT_RECORD& mftRecord, BYTE*& byte, ENTRY*& folder, ENTRY*& entry);
	void printFileContent(ENTRY* entry, wstring filename);
};

void Menu(int& choice, wchar_t name);
void InputVolume(VOLUME*& volume, wchar_t& name);