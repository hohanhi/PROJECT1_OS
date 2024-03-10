#include <Windows.h>
#include <vector>
#include "Component.h"

class FAT32 {
private:
    LPCWSTR _drive;                      //ten o dia 
    BYTE* FAT;                           //bytes cua 1 bang FAT
    std::vector<Component> componentList;//danh sach cac tap tin/thu muc trong o dia

    unsigned int bytes_sector;           //so byte 1 sector
    unsigned int sectors_cluster;        //so sector 1 cluster
    unsigned int sectors_bootsector;     //so sector cua boot sector
    unsigned int FAT_tables;             //so bang FAT
    unsigned int volume_size;            //kich thuoc o dia
    unsigned int FAT_size;               //kich thuoc 1 bang FAT
    unsigned int first_cluster_RDET;     //cluster bat dau cua RDET
public:
    FAT32();
    FAT32(LPCWSTR drive);
    ~FAT32();
public:
    void Print_BootSector_Info();
    void Read_RDET();
    void Read_SDET(unsigned int first_cluster, unsigned int last_cluster, int level);
    void Print_RDET();
    void Print_FolderTree();
    void Print_Data();
};