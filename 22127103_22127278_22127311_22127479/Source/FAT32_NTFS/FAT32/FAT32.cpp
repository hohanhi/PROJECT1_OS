#include "FAT32.h"
#include "Function.h"
#include <string>
#include <algorithm>
#include <math.h>

FAT32::FAT32() {
    _drive = L"\\\\.\\F:";
    BYTE* BootSector = new BYTE[512];
    ReadSector(_drive, 0, BootSector, 512);
    bytes_sector = Get_Value_At_Offset(BootSector, 0x0B, 2);
    sectors_cluster = Get_Value_At_Offset(BootSector, 0x0D, 1);
    sectors_bootsector = Get_Value_At_Offset(BootSector, 0x0E, 2);
    FAT_tables = Get_Value_At_Offset(BootSector, 0x10, 1);
    volume_size = Get_Value_At_Offset(BootSector, 0x20, 4);
    FAT_size = Get_Value_At_Offset(BootSector, 0x24, 4);
    first_cluster_RDET = Get_Value_At_Offset(BootSector, 0x2C, 4);
    FAT = new BYTE[512];
    ReadSector(_drive, sectors_bootsector * bytes_sector, FAT, 512);
    delete[] BootSector;
}

FAT32::FAT32(LPCWSTR drive) {
    _drive = drive;
    BYTE* BootSector = new BYTE[512];
    ReadSector(_drive, 0, BootSector, 512);
    bytes_sector = Get_Value_At_Offset(BootSector, 0x0B, 2);
    sectors_cluster = Get_Value_At_Offset(BootSector, 0x0D, 1);
    sectors_bootsector = Get_Value_At_Offset(BootSector, 0x0E, 2);
    FAT_tables = Get_Value_At_Offset(BootSector, 0x10, 1);
    volume_size = Get_Value_At_Offset(BootSector, 0x20, 4);
    FAT_size = Get_Value_At_Offset(BootSector, 0x24, 4);
    first_cluster_RDET = Get_Value_At_Offset(BootSector, 0x2C, 4);
    FAT = new BYTE[512];
    ReadSector(_drive, sectors_bootsector * bytes_sector, FAT, 512);
    delete[] BootSector;
}

FAT32::~FAT32() {
    delete[] FAT;
}

void FAT32::Print_BootSector_Info() {
    std::cout << "Loai FAT: FAT32" << std::endl;
    std::cout << "So byte cho 1 sector: " << bytes_sector << std::endl;
    std::cout << "So sector cho 1 cluster: " << sectors_cluster << std::endl;
    std::cout << "So sector tren Bootsector: " << sectors_bootsector << std::endl;
    std::cout << "So bang FAT: " << FAT_tables << std::endl;
    std::cout << "Tong so sector tren dia: " << volume_size << std::endl;
    std::cout << "So sector 1 bang FAT: " << FAT_size << std::endl;
    std::cout << "Sector dau tien cua bang FAT: " << sectors_bootsector << std::endl;
    std::cout << "Sector dau tien cua vung DATA: " << sectors_bootsector + FAT_tables * FAT_size << std::endl;
    std::cout << "Cluster bat dau cua RDET: " << first_cluster_RDET << std::endl;
}


void FAT32::Read_SDET(unsigned int first_cluster, unsigned int last_cluster, int level) {
    unsigned int size = (last_cluster - first_cluster + 1) * sectors_cluster * bytes_sector;
    BYTE* SDET = new BYTE[bytes_sector];
    unsigned int readPoint = (sectors_bootsector + FAT_tables * FAT_size + (first_cluster - 2) * sectors_cluster) * bytes_sector;
    ReadSector(_drive, readPoint, SDET, bytes_sector);
    size -= bytes_sector;
    std::string filename = "";
    int pCurr = 0;
    while (true) {
        if (pCurr == bytes_sector) {
            if (size == 0)
                break;
            else {
                readPoint += bytes_sector;
                delete[] SDET;
                SDET = new BYTE[bytes_sector];
                ReadSector(_drive, readPoint, SDET, bytes_sector);
                size -= bytes_sector;
            }
        }
        if (Get_Value_At_Offset(SDET, pCurr + 0x0B, 1) == 0x00)
            break;
        //entry "." và ".."
        if (Get_Value_At_Offset(SDET, pCurr, 1) == 0x2E) {
            filename = "";
        }
        else {
            //entry chinh
            if (Get_Value_At_Offset(SDET, pCurr + 0x0B, 1) == 0x10 || Get_Value_At_Offset(SDET, pCurr + 0x0B, 1) == 0x20) {
                if (filename == "")
                    filename = Get_String(SDET, pCurr, 11);
                unsigned int first_cluster = Get_Value_At_Offset(SDET, pCurr + 0x14, 2) * pow(16, 4) + Get_Value_At_Offset(SDET, pCurr + 0x1A, 2);
                unsigned int last_cluster = first_cluster;
                unsigned int size = Get_Value_At_Offset(SDET, pCurr + 0x1C, 4);
                while (Get_Value_At_Offset(FAT, last_cluster * 4, 4) != 0x0FFFFFFF) {
                    last_cluster = Get_Value_At_Offset(FAT, last_cluster * 4, 4);
                }
                //thu muc
                if (Get_Value_At_Offset(SDET, pCurr + 0x0B, 1) == 0x10) {
                    Component p(filename, 1, first_cluster, last_cluster, level, size);
                    componentList.push_back(p);
                    Read_SDET(first_cluster, last_cluster, level + 1);
                }
                //tap tin
                else {
                    Component p(filename, 2, first_cluster, last_cluster, level, size);
                    componentList.push_back(p);
                }
                filename = "";
            }
            //entry phu
            else if (Get_Value_At_Offset(SDET, pCurr + 0x0B, 1) == 0x0F) {
                filename = Get_String(SDET, pCurr + 0x01, 10) + Get_String(SDET, pCurr + 0x0E, 12) + Get_String(SDET, pCurr + 0x1C, 4) + filename;
            }
            else {
                filename = "";
            }
        }
        pCurr += 32;
    }
    delete[] SDET;
}


void FAT32::Read_RDET() {
    //byte dau tien cua RDET
    unsigned int readPoint = (sectors_bootsector + FAT_tables * FAT_size + (first_cluster_RDET - 2) * sectors_cluster) * bytes_sector;
    BYTE* RDET = new BYTE[bytes_sector];          //mang cac bytes trong 1 sector
    std::string filename = "";                    //ten tap tin/ thu muc
    int pCurr = 0;                                //con tro dau moi 32 bytes
    ReadSector(_drive, readPoint, RDET, bytes_sector); //doc 1 sector cua RDET
    while (true) {
        if (pCurr == bytes_sector) {
            readPoint += bytes_sector;;
            delete[] RDET;
            RDET = new BYTE[bytes_sector];
            ReadSector(_drive, readPoint, RDET, bytes_sector);
            pCurr = 0;
        }
        if (Get_Value_At_Offset(RDET, 0x0B + pCurr, 1) == 0x00)
            break;
        //entry chinh
        if (Get_Value_At_Offset(RDET, 0x0B + pCurr, 1) == 0x10 || Get_Value_At_Offset(RDET, 0x0B + pCurr, 1) == 0x20) {
            if (filename == "") {
                filename = Get_String(RDET, pCurr, 11);
            }
            unsigned int first_cluster = Get_Value_At_Offset(RDET, pCurr + 0x14, 2) * pow(16, 4) + Get_Value_At_Offset(RDET, pCurr + 0x1A, 2);
            unsigned int last_cluster = first_cluster;
            unsigned int size = Get_Value_At_Offset(RDET, pCurr + 0x1C, 4);
            while (Get_Value_At_Offset(FAT, last_cluster * 4, 4) != 0x0FFFFFFF) {
                last_cluster = Get_Value_At_Offset(FAT, last_cluster * 4, 4);
            }
            if (Get_Value_At_Offset(RDET, 0x0B + pCurr, 1) == 0x10) {
                Component p(filename, 1, first_cluster, last_cluster, 1, size);
                componentList.push_back(p);
                Read_SDET(first_cluster, last_cluster, 2);
            }
            else {
                Component p(filename, 2, first_cluster, last_cluster, 1, size);
                componentList.push_back(p);
            }
            filename = "";
        }
        //entry phu
        else if (Get_Value_At_Offset(RDET, 0x0B + pCurr, 1) == 0x0F) {
            filename = Get_String(RDET, pCurr + 0x01, 10) + Get_String(RDET, pCurr + 0x0E, 12) + Get_String(RDET, pCurr + 0x1C, 4) + filename;
        }
        else {
            filename = "";
        }
        pCurr += 32;
    }
    delete[] RDET;
}

void FAT32::Print_RDET() {
    for (int i = 0; i < componentList.size(); i++) {
        std::cout << "Ten: " << componentList[i].name() << std::endl;
        std::cout << "Trang thai: ";
        if (componentList[i].status() == 1)
            std::cout << "Thu muc" << std::endl;
        else if (componentList[i].status() == 2)
            std::cout << "Tap tin" << std::endl;
        std::cout << "Chiem cac cluster: ";
        for (int j = componentList[i].firstCluster(); j <= componentList[i].lastCluster(); j++) {
            std::cout << j;
            if (j < componentList[i].lastCluster())
                std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "Tuong ung cac sector: ";
        unsigned int first_sector = sectors_bootsector + FAT_size * FAT_tables + (componentList[i].firstCluster() - 2) * sectors_cluster;
        unsigned int last_sector = sectors_bootsector + FAT_size * FAT_tables + (componentList[i].lastCluster() + 1 - 2) * sectors_cluster;
        for (int i = first_sector; i < last_sector; i++) {
            std::cout << i;
            if (i < last_sector - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "Kich thuoc: " << componentList[i].size() << std::endl << std::endl << std::endl;
    }
}

void FAT32::Print_FolderTree() {
    for (int i = 0; i < componentList.size(); i++) {
        for (int j = 1; j < componentList[i].level(); j++) {
            std::cout << "\t";
        }
        std::cout << "--" << componentList[i].name() << std::endl;
    }
}



void FAT32::Print_Data() {
    for (int i = 0; i < componentList.size(); i++) {
        //tap tin
        std::cout << "--" << componentList[i].name() << std::endl;
        if (componentList[i].status() == 2) {
            std::string filename = componentList[i].name();
            std::string extension = "";
            int pos = filename.find_last_of(".");
            if (pos == -1)
                pos = filename.find_last_of(" ");
            if (pos != -1)
                extension = filename.substr(pos + 1, filename.length() - pos);
            transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension == "txt") {
                unsigned int first_sector = sectors_bootsector + FAT_size * FAT_tables + (componentList[i].firstCluster() - 2) * sectors_cluster;
                unsigned int last_sector = sectors_bootsector + FAT_size * FAT_tables + (componentList[i].lastCluster() + 1 - 2) * sectors_cluster;
                unsigned int size = componentList[i].size();
                std::string data = "";
                unsigned int index = first_sector;

                while (index < last_sector && size > 0) {
                    BYTE* sector = new BYTE[bytes_sector];
                    ReadSector(_drive, index * bytes_sector, sector, bytes_sector);
                    unsigned int readSize = size <= bytes_sector ? size : bytes_sector;
                    data += Get_String(sector, 0, readSize);
                    size -= readSize;
                    index++;
                    delete[] sector;
                }
                std::cout << data << std::endl;
            }
            else if (extension == "docx") {
                std::cout << "Doc noi dung file bang: Word" << std::endl;
            }
            else if (extension == "pptx") {
                std::cout << "Doc noi dung file bang: Powerpoint." << std::endl;
            }
            else if (extension == "xlsx") {
                std::cout << "Doc noi dung file bang: Excel." << std::endl;
            }
            else if (extension == "pdf") {
                std::cout << "Doc noi dung file bang: PDF Reader." << std::endl;
            }
        }
        //thu muc
        else {
            int j = i + 1;
            while (j < componentList.size() && componentList[j].level() > componentList[i].level()) {
                for (int k = 1; k < componentList[j].level(); k++) {
                    std::cout << "\t";
                }
                std::cout << "--" << componentList[j].name() << std::endl;
                j++;
            }
        }
        std::cout << std::endl;
    }
}