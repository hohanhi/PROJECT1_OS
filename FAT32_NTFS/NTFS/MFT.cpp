#include "MFT.h"

void INDEX_ROOT_ATTRI::read(BYTE sec[])
{

	readFile((char*)&headAttr, sec, 24);
	readFile((char*)&indexRoot, sec + headAttr.Res.Offset, sizeof MFTRootIndexAttribute);
	DWORD pos = headAttr.Res.Offset + sizeof(MFTRootIndexAttribute);
	while (pos + 84 < headAttr.RecordLength)
	{
		MFTEntryIndex ie;
		readFile((char*)&ie, sec + pos, 16);
		FILENAME f;
		f.read(sec + pos + 16);
		BLOCKFILE block{ f,ie.DataOffset };
		list.push_back(block);
		pos += ie.Length;
	}
}

MFT_RECORD::MFT_RECORD()
{
	mft_head = new MFTEntryHeader;
}
void MFT_RECORD::read(BYTE sec[])
{
	MFTAttributeHeader headAttr, headAttr2;
	readFile((char*)mft_head, sec, sizeof(MFTEntryHeader));
	if (this->mft_head->magic != FILE_RECORD_MAGIC)
		return;

	int o = mft_head->updateOffset;
	sec[510] = sec[o + 3];
	sec[511] = sec[o + 4];
	sec[1022] = sec[o + 5];
	sec[1023] = sec[o + 6];

	DWORD curOffset = mft_head->attributeOffset;
	ATTRIBUTE* attr = nullptr;
	while (true) {
		readFile((char*)&headAttr, sec + curOffset, 16);
		attr = nullptr;

		if (headAttr.TypeCode == $FILE_NAME) {
			attr = new FILENAME_ATTRI;
		}
		else {
			if (headAttr.TypeCode == $INDEX_ROOT) {
				attr = new INDEX_ROOT_ATTRI;
			}
			else if (headAttr.TypeCode == $INDEX_ALLOCATION) {
				attr = new INDEX_ALLOC_ATTRI;
			}
			else if (headAttr.TypeCode == $DATA) {
				attr = new DATA_ATTRIBUTE;
			}
		}

		if (attr) {
			attr->read(sec + curOffset);
			attrList.push_back(attr);

		}
		curOffset += headAttr.RecordLength;
		if (headAttr.TypeCode == $END || curOffset > 1024)
			break;
	}
}

DATARUNLIST MFT_RECORD::getDataRun_INDEX()
{
	INDEX_ALLOC_ATTRI* index = nullptr;
	for (int i = 0; i < attrList.size(); i++)
	{
		index = dynamic_cast<INDEX_ALLOC_ATTRI*> (attrList[i]);
		if (index != nullptr)
			return index->getDataRunList();
	}
	return DATARUNLIST();
}

DATARUNLIST MFT_RECORD::getDataRun_DATA()
{
	DATARUNLIST datarun;
	DATA_ATTRIBUTE* data = nullptr;
	for (int i = 0; i < attrList.size(); i++)
	{
		data = dynamic_cast<DATA_ATTRIBUTE*> (attrList[i]);
		if (data != nullptr)
		{
			DATARUNLIST  run = data->getDataRun();
			datarun.insert(datarun.end(), run.begin(), run.end());
		}
	}
	return datarun;
}

LIST_INDEX MFT_RECORD::getIndexEntries()
{
	INDEX_ROOT_ATTRI* id = nullptr;
	for (int i = 0; i < attrList.size(); i++)
	{
		id = dynamic_cast<INDEX_ROOT_ATTRI*> (attrList[i]);
		if (id != nullptr)
			return id->getEntryIndexList();
	}
	return LIST_INDEX();
}

DATA_ATTRIBUTE* MFT_RECORD::getDataAttr()
{
	DATA_ATTRIBUTE* data = nullptr;
	for (int i = 0; i < attrList.size(); i++)
	{
		data = dynamic_cast<DATA_ATTRIBUTE*> (attrList[i]);
		if (data != nullptr)
			return data;
	}
	return nullptr;
}


MFT_RECORD::~MFT_RECORD()
{
	for (int i = 0; i < attrList.size(); i++)
	{
		if (attrList[i] != nullptr)
			delete attrList[i];
	}
	delete mft_head;
}


vector <MFTDatarun> readDataRun(BYTE datarun[], int size)
{
	int i = 0;
	vector < MFTDatarun> list;
	while (datarun[i] != 0x00)
	{
		int offset_len = datarun[i] >> 4;
		int length_len = datarun[i] & 0xf;
		ULONGLONG length = 0;
		LONGLONG offset = 0;
		MFTDatarun drun;
		readFile((char*)&length, datarun + i + 1, length_len);
		readFile((char*)&offset, datarun + i + 1 + length_len, offset_len);
		drun.length = length;
		drun.offset = offset;
		i += offset_len + length_len + 1;
		list.push_back(drun);
	}
	return list;
}

void INDEX_ALLOC_ATTRI::read(BYTE sec[])
{
	readFile((char*)&headAttr, sec, 64);
	USHORT fstData = headAttr.N_Res.MappingPairsOffset;
	DWORD lengRecord = headAttr.RecordLength;
	_list = readDataRun(sec + fstData, lengRecord - fstData);
}

DATARUNLIST INDEX_ALLOC_ATTRI::getDataRunList()
{
	return _list;
}

void FILENAME::read(BYTE sec[])
{
	wchar_t* buffer = new wchar_t[256];
	readFile((char*)&fName, sec, sizeof MFTFilenameAttribute);
		
	WORD length = fName.NameLength;
	readFile((char*)buffer, sec + 66, length *2);
	buffer[length] = NULL;
	entryName = L"";
	for (int i = 0; i < length; i++)
	{
		if (buffer[i] < 32||buffer[i]>128) continue;
		entryName += buffer[i];
	}
}

ULONGLONG FILENAME::getSize()
{
	return fName.DataSize;
}

string FILENAME::getAttribute() {
	string buffer;
	DWORD Attr = fName.FileAttributes;
	if (Attr & ATTR_FILENAME_FLAG_READONLY)
		buffer += "ReadOnly ";
	if (Attr & ATTR_FILENAME_FLAG_HIDDEN)
		buffer += "Hidden ";
	if (Attr & ATTR_FILENAME_FLAG_SYSTEM)
		buffer += "System ";
	if (Attr & ATTR_FILENAME_FLAG_ENTRYECTORY)
		buffer += "Folder ";
	if (Attr & ATTR_FILENAME_FLAG_ARCHIVE)
		buffer += "File ";
	if (Attr & ATTR_FILENAME_FLAG_DEVICE)
		buffer += "Device ";
	return buffer;
}

wstring FILENAME::getName()
{
	return entryName;
}

FILENAME FILENAME::clone()
{
	return *this;
}

void DATA_ATTRIBUTE::read(BYTE sec[])
{
	readFile((char*)&headAttr, sec, 24);
	if (headAttr.FormCode == RESIDENT)
	{
		isDataRun = false;
		Data= ByteToString(sec+ headAttr.Res.Offset, headAttr.Res.Length);
	}
	else 
	{
		isDataRun = true;
		readFile((char*)&headAttr, sec, 64);
		USHORT fstData = headAttr.N_Res.MappingPairsOffset;
		DWORD lengRecord = headAttr.RecordLength;
		_list = readDataRun(sec + fstData, lengRecord - fstData);
	}
}

DATARUNLIST DATA_ATTRIBUTE::getDataRun()
{
	return _list;
}

