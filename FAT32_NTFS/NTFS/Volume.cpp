#include "Volume.h"

VOLUME_NTFS::VOLUME_NTFS()
{
	_device = NULL;
	_rootEntry = nullptr;
}

VOLUME_NTFS::~VOLUME_NTFS()
{
	CloseHandle(_device);
	delete _rootEntry;
}

int VOLUME_NTFS::InitVolume(LPCWSTR drive)
{
		BYTE sector[512];
		int status = this->OpenVolume(drive);
		if (status != 0) return -1;

		this->ReadSector(0, sector);
		_PBSector.Read(sector);
		_PBSector.getFileSystem();
		if (_PBSector.getFileSystem().find("NTFS") == string::npos)
			return 0;
		_secPerClus = _PBSector.getSecPerClus();
		_bytePerSec = _PBSector.getBytePerSec();
		_fstMTF = _PBSector.getFstMTFSec() * _secPerClus;
		_bytePerRecord = _PBSector.getBytePerRecord();
		_rootEntry = new FOLDER(ROOT_FILE_NAME_INDEX);
		_rootEntry->_sector = _fstMTF + ROOT_FILE_NAME_INDEX * 2;
		readDirectoryTree(_rootEntry);
		return 1;
}

int VOLUME_NTFS::OpenVolume(LPCWSTR drive)
{
	_device = CreateFile(drive,    // Drive to open
		GENERIC_READ,           // Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
		NULL,                   // Security Descriptor
		OPEN_EXISTING,          // How to create
		0,                      // File attributes
		NULL);                  // Handle to template

	if (_device == INVALID_HANDLE_VALUE) // Open Error
		return GetLastError();
	else
		return 0;
}

int VOLUME_NTFS::ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte)
{
	DWORD nReadBytes;
	LONG highOffset = readPoint >> 32;
	SetFilePointer(_device, (DWORD)readPoint, &highOffset, FILE_BEGIN);//Set a Point to Read
	if (!ReadFile(_device, sector, nByte, &nReadBytes, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
		exit(1);
	}
	else
		return nReadBytes;
}

void VOLUME_NTFS::printPartitionBootSector()
{
	_PBSector.printBS();
}


void VOLUME_NTFS::readIndexEntriesFromMFTRecord(MFT_RECORD& mftRecord, ENTRY*& folder, ENTRY*& entry)
{
	// Read entry in root index folder attribute.
	LIST_INDEX list = mftRecord.getIndexEntries();
	for (int i = 0; i < list.size(); i++)
	{
		if (list[i].fname.getFileAttr() & ATTR_FILENAME_FLAG_ENTRYECTORY)
		{
			entry = new FOLDER(list[i].curIndex);
			// Recurion to read sub folder
			readDirectoryTree(entry);
		}
		else if (list[i].fname.getFileAttr() & ATTR_FILENAME_FLAG_ARCHIVE)
		{
			entry = new   FILE_NTFS(list[i].curIndex);
		}
		if (entry != nullptr)
		{
			entry->setFilename(list[i].fname);
			entry->_sector = _fstMTF + list[i].curIndex * 2;
			folder->addEntry(entry);
		}
	}
}

void VOLUME_NTFS::readIndexEntriesFromIndexAllocationAttribute(MFT_RECORD& mftRecord, BYTE*& byte, ENTRY*& folder, ENTRY*& entry)
{
	DATARUNLIST run = mftRecord.getDataRun_INDEX();
	INT64 offset = 0;
	BYTE LIST_US[20];

	for (int i = 0; i < run.size(); i++)
	{
		int US = 0;
		offset += run[i].offset;
		UINT64 pos = offset * _secPerClus * _bytePerSec;
		ReadSector(pos, byte, 1024);
		IndexBlock idb;
		readFile((char*)&idb, byte, sizeof IndexBlock);
		readFile((char*)LIST_US, byte + idb.OffsetOfUS + 2, (idb.SizeOfUS - 1) * 2);
		byte[_bytePerSec - 2] = LIST_US[US++];
		byte[_bytePerSec - 1] = LIST_US[US++];

		UINT64 pos2 = idb.EntryOffset + 0x18;
		if (idb.Magic != INDEX_BLOCK_MAGIC)
		{
			cout << "\nMagic number error!\n";
			exit(1);
		}
		UINT64 total = 0;
		while (total + 66 <= idb.TotalEntrySize)
		{
			if (pos2 > 512)
			{
				pos += 512;
				pos2 -= 512;
				ReadSector(pos, byte, 1024);

				byte[_bytePerSec - 2] = LIST_US[US++];
				byte[_bytePerSec - 1] = LIST_US[US++];
			}

			MFTEntryIndex _entryIndex;
			readFile((char*)&_entryIndex, byte + pos2, 84);

			if (_entryIndex.Flags == INDEX_ENTRY_FLAG_LAST) {
				break;
			}

			FileName fName;
			fName.read(byte + pos2 + 16);

			DWORD idex = _entryIndex.DataOffset;
			ENTRY* entry = nullptr;
			if (fName.getFileAttr() == ATTR_FILENAME_FLAG_ENTRYECTORY)
			{
				entry = new FOLDER(idex);
				// Recurion to read sub folder
				readDirectoryTree(entry);
			}
			else if ((fName.getFileAttr() & ATTR_FILENAME_FLAG_ARCHIVE) != 0)
			{
				entry = new FILE_NTFS(idex);
			}
			if (entry != nullptr)
			{
				entry->setFilename(fName);
				entry->_sector = _fstMTF + idex * 2;
				folder->addEntry(entry);
			}
			pos2 += _entryIndex.Length;
			total += _entryIndex.Length;
		}
	}
}

void VOLUME_NTFS::readDirectoryTree(ENTRY* folder)
{
	BYTE* byte = new BYTE[2048];
	UINT64 pos = (_fstMTF * 512 + 1024 * folder->_entryIndex);
	ENTRY* entry = nullptr;
	FileName fName;
	ReadSector(pos, byte, 1024);

	// Read entry in MFT record.
	MFT_RECORD mftRecord;
	mftRecord.read(byte);

	readIndexEntriesFromMFTRecord(mftRecord, folder, entry);
	readIndexEntriesFromIndexAllocationAttribute(mftRecord, byte, folder, entry);
	delete[] byte;
}

void VOLUME_NTFS::printDirectoryTree()
{
	cout << char(218) << string(40, char(196)) << char(191) << endl;
	cout << char(179) << "             DIRECTORY TREE             " << char(179) << endl;
	cout << char(192) << std::string(40, char(196)) << char(217) << endl;
	if (_rootEntry)
		_rootEntry->printDirectoryTree();
}

void VOLUME_NTFS::printDetailedDirectoryTree()
{
	if (!_rootEntry) return;
	cout << char(218) << string(40, char(196)) << char(191) << endl;
	cout << char(179) << "        DETAILED DIRECTORY TREE         " << char(179) << endl;
	cout << char(192) << std::string(40, char(196)) << char(217) << endl;
	_rootEntry->printEntryInfo();
}

void VOLUME_NTFS::printEntryData(wstring filename)
{
	if (!_rootEntry) return;
	ENTRY* entry = _rootEntry->SearchEntry(filename);

	if (entry == nullptr)
	{
		cout << "File Not Found!" << endl;
		return;
	}

	// For file
	if (dynamic_cast <FILE_NTFS*>(entry) != nullptr)
	{
		cout << "------------------FILE INFORMATION------------------" << endl;
		entry->printEntryInfo(0);
		printFileContent(entry, filename);
	}
	else
	{
		//For folder
		cout << "-----------------FOLDER INFORMATION-----------------" << endl;
		cout << char(218) << string(40, char(196)) << char(191) << endl;
		cout << char(179) << "             DIRECTORY TREE            " << char(179) << endl;
		cout << char(192) << std::string(40, char(196)) << char(217) << endl;
		entry->printDirectoryTree();
		cout << string(40, char(196)) << endl;
		cout << char(218) << string(40, char(196)) << char(191) << endl;
		cout << char(179) << "        DETAILED DIRECTORY TREE         " << char(179) << endl;
		cout << char(192) << std::string(40, char(196)) << char(217) << endl;
		entry->printEntryInfo();
	}
}

void VOLUME_NTFS::printFileContent(ENTRY* entry, wstring filename)
{
	BYTE* sec = new BYTE[1024];
	ULONGLONG sizeFile = entry->_filename.getSize();
	UINT64 pos = (_fstMTF * 512 + 1024 * entry->_entryIndex);
	ReadSector(pos, sec, 1024);
	MFT_RECORD mft_record;
	mft_record.read(sec);
	DATA_ATTRIBUTE* dataAtr = mft_record.getDataAttr();

	transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	if (filename.find(L".txt") == std::string::npos) {
		cout << "Use compatible software to read the content!" << endl;
		delete[] sec;
		return;
	}

	if (dataAtr == nullptr)
	{
		cout << "No Data!";
	}
	else if (dataAtr->isDataRun == false)
	{
		cout << dataAtr->Data;
	}
	else
	{
		cout << "------------------------DATA------------------------" << endl;
		DATARUNLIST run = mft_record.getDataRun_DATA();
		int size = run.size();
		BYTE* byte = new BYTE[_bytePerSec];
		ULONGLONG total = 0;
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < _secPerClus * run[i].length; j++)
			{
				ReadSector(((run[i].offset) * _secPerClus + j) * _bytePerSec, byte, _bytePerSec);
				cout << ByteToString(byte, _bytePerSec);

				if (total > sizeFile)
				{
					delete[] sec;
					delete[] byte;
					return;
				}
				total += _bytePerSec;

			}
		}
		delete[] byte;
	}

	delete[] sec;
}

ENTRY::ENTRY()
{
	_entryIndex = -1;
}

ENTRY::ENTRY(DWORD index)
{
	_entryIndex = index;
}

void ENTRY::setFilename(FileName  Name)
{
	this->_filename = Name;
}

void ENTRY::printEntryInfo(int x)
{
	wcout << setw(x) << " " << L"Name: " << _filename.getName() << endl;
	cout << setw(x) << " " << "Attribute: " << _filename.getAttribute() << endl;
	cout << setw(x) << " " << "Create:	-" << _filename.getCreateTime() << endl;
	cout << setw(x) << " " << "Access:	-" << _filename.getAccessTime() << endl;
	cout << setw(x) << " " << "Modified:	-" << _filename.getModifiedTime() << endl;
	cout << setw(x) << " " << "File Size:	" << _filename.getSize() << " Byte" << endl;
	cout << setw(x) << " " << "Entry Index: " << _entryIndex << endl;
	cout << setw(x) << " " << "Sector: " << _sector << endl;
}

void FILE_NTFS::printDirectoryTree(int x)
{
	wstring a = _filename.getName();
	wcout << setw(x) << char(195) << char(196) << " " << a;
	cout << DataSizeFomat(_filename.fName.DataSize) << endl;
}

ENTRY* FILE_NTFS::SearchEntry(wstring name)
{
	if (IsEquals(name, this->_filename.getName()))
	{
		return this;
	}
	return nullptr;
}

void FOLDER::addEntry(ENTRY* e)
{

	list.push_back(e);
}

void FOLDER::printDirectoryTree(int x)
{
	wcout << setw(x) << char(192) << _filename.getName() << L":" << endl;
	size_t n = list.size();
	for (int i = 0; i < n; i++) {
		list[i]->printDirectoryTree(x + 3);
	}
}
void FOLDER::printEntryInfo(int x)
{
	cout << setw(x) << " " << string(35, 196) << endl;
	wcout << setw(x - 1) << char(195) << " " << _filename.getName() << endl;
	cout << setw(x) << " " << "Attribute: " << _filename.getAttribute() << endl;
	cout << setw(x) << " " << "First Index: " << this->_entryIndex << endl;
	cout << setw(x) << " " << "Sector MFT entry: " << _sector << endl;
	cout << setw(x) << " " << string(35, 196) << endl;

	size_t n = list.size();
	for (int i = 0; i < n; i++) {
		list[i]->printEntryInfo(x + 5);
	}
}

ENTRY* FOLDER::SearchEntry(wstring name)
{
	if (IsEquals(name, this->_filename.getName()))
		return this;

	int n = list.size();
	for (int i = 0; i < n; i++)
	{
		ENTRY* en = list[i]->SearchEntry(name);
		if (en != nullptr)
			return en;
	}
	return nullptr;
}

FOLDER::~FOLDER()
{
	int size = list.size();
	for (int i = 0; i < size; i++)
	{
		if(list[i]!=nullptr)
			delete list[i];
	}
}

void InputVolume(VOLUME*& volume, wchar_t& name)
{
	if (volume) {
		delete volume;
		volume = nullptr;
	}
	WCHAR  driveName[] = L"\\\\.\\E:";

	while (true) {
		system("cls");
		wcout << "Enter volume name (0 to exit): ";
		wcin >> name;

		if (name == L'0') {
			if (volume) delete volume;
			exit(0);
		}
		driveName[4] = name;
		volume = new VOLUME_NTFS;
		int status = volume->InitVolume(driveName);
		if (status != 1)
		{
			cout << "No disk found or access is blocked." << endl;
			cout << "Please try again!" << endl;
			Sleep(1500);
			delete volume;
			volume = nullptr;
			continue;
		}
		break;
	}
}

void Menu(int& choice, wchar_t name)
{
	do {
		cout << "=========================================" << endl;
		cout << setw(40) << left << "|           NTFS VOLUME READER " << right << "|" << endl;
		cout << "=========================================" << endl;
		cout << setw(40) << left << "| Enter: " << right << "|" << endl;
		cout << setw(40) << left << "| 1. Change Volume. " << right << "|" << endl;
		cout << setw(40) << left << "| 2. Print Boot Sector Volume." << right << "|" << endl;
		cout << setw(40) << left << "| 3. Print Directory Tree." << right << "|" << endl;
		cout << setw(40) << left << "| 4. Print Detailed Directory Tree." << right << "|" << endl;
		cout << setw(40) << left << "| 5. Search for File or Folder." << right << "|" << endl;
		cout << setw(40) << left << "| 0. Exit." << right << "|" << endl;
		cout << "=========================================" << endl;
		cout << "Your choice: ";

		cin >> choice;
	} while (choice < 0 || choice > 5);
	cout << "=========================================" << endl;
}
