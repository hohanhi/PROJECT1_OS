#pragma once
#include "MFT.h"
#pragma pack(push,1)
typedef struct PBS
{
    // Jump Instruction
    BYTE        jmpBoot[3];

    // OEM Identifier
    BYTE        oemID[8];

    // BIOS Parameter Block
    WORD        bytePerSec;
    BYTE        secPerClus;
    BYTE        reservedSec[2];
    BYTE        zero0[3];
    BYTE        unused1[2];
    BYTE        media;
    BYTE        zero1[2];
    WORD        secPerTrack;
    WORD        headNum;
    DWORD       HiddenSec;
    BYTE        unused2[8];
    LONGLONG    totalSec;

    // Master File Table (MFT)
    LONGLONG    MFTClus;
    LONGLONG    MFTMirrClus;
    INT8        clusPerRecord;
    BYTE        unused4[3];
    INT8        clusPerBlock;
    BYTE        unused5[3];

    // Other Information
    LONGLONG    serialNum;
    DWORD       checkSum;

    // Boot Code
    BYTE        bootCode[426];

    // End Marker
    BYTE        endMarker[2];
};

class NTFSPBSector {
private:
    PBS pbs;
public:
	void Read(BYTE[512]);
	DWORD getFstMTFSec() { return pbs.MFTClus; }
	UINT16 getBytePerSec() { return pbs.bytePerSec; }
	UINT16 getSecPerClus() { return pbs.secPerClus; }
	UINT16 getBytePerRecord() { return pow(2, abs(pbs.clusPerRecord)); }
	string getFileSystem() {
        return ByteToString(pbs.oemID, 8);
	}
	void printBS();
};
#pragma pack(pop)