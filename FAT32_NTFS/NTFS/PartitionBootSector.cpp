#include "PartitionBootSector.h"

void NTFSPBSector::Read(BYTE sec[512])
{
    readFile((char*)&pbs, sec, 512);
}

void NTFSPBSector::printBS() {
    cout << "----------------------------------------------" << endl;
    cout << "|       Partition Boot Sector NTFS           |" << endl;
    cout << "----------------------------------------------" << endl;

    cout << "OEM Name: " << pbs.oemID << endl;
    cout << "Bytes Per Sector: " << pbs.bytePerSec << endl;
    cout << "Sectors Per Cluster: " << (int)pbs.secPerClus << endl;
    cout << "Sectors Per Track: " << (int)pbs.secPerTrack << endl;
    cout << "Number of Heads: " << (int)pbs.headNum << endl;
    cout << "Total sectors in volume: " << pbs.totalSec << DataSizeFomat(pbs.totalSec * 512) << endl;
    cout << "Starting Cluster of MTF: " << pbs.MFTClus << endl;
    cout << "Starting Cluster of MFTMirror : " << pbs.MFTMirrClus << endl;
    cout << "MTF Entry Size: " << this->getBytePerRecord() << " byte" << endl;
    cout << "Number of Bytes in Index Block:" << pbs.clusPerBlock * pbs.secPerClus * pbs.bytePerSec << endl;
    cout << "----------------------------------------------" << endl;
}
