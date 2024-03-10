#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include "FAT32.h"
#include "Function.h"


int main() {
    //nhap ten o dia
    std::wstring disk_name;
    std::cout << "Nhap ten o dia: ";
    std::wcin >> disk_name;
    disk_name = L"\\\\.\\" + disk_name + L":";
    LPCWSTR drive = disk_name.c_str();

    FAT32 f(drive);
    std::cout << "---------------------BOOT SECTOR------------------" << std::endl;
    f.Print_BootSector_Info();
    std::cout << std::endl;
    f.Read_RDET();
    std::cout << "---------------THONG TIN CAC TAP TIN---------------" << std::endl;
    f.Print_RDET();
    std::cout << std::endl;
    std::cout << "-------------------CAY THU MUC---------------------" << std::endl;
    f.Print_FolderTree();
    std::cout << std::endl;
    std::cout << "----------THONG TIN TREN CAY THU MUC---------------" << std::endl;
    f.Print_Data();
    return 0;
}