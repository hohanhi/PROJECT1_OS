#include "Function.h"

int ReadSector(LPCWSTR drive, int readPoint, BYTE*& sector, int size) {
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,                     // Drive to open
        GENERIC_READ,                              // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                                      // Security Descriptor
        OPEN_EXISTING,                             // How to create
        0,                                         // File attributes
        NULL);                                     // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        std::cout << "CreateFile: " << GetLastError() << std::endl;
        return 0;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, size, &bytesRead, NULL))
    {
        std::cout << "ReadFile: " << GetLastError() << std::endl;
        return 0;
    }
    else
    {
        //std::cout << "Success!" << std::endl;
        return 1;
    }
}

int Get_Value_At_Offset(BYTE* sector, int offset, int number) {
    int result = 0;
    memcpy(&result, sector + offset, number);
    return result;
}


std::string Get_String(BYTE* sector, int offset, int number) {
    std::string result = "";
    for (int i = 0; i < number; i++) {
        if (sector[offset + i] != 0x00 && sector[offset + i] != 0xFF) {
            result += char(Get_Value_At_Offset(sector, offset + i, 1));
        }
    }
    return result;
}

