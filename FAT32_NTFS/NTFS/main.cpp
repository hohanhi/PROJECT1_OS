#include "Volume.h"

int main()
{
    VOLUME* volume = nullptr;
    wchar_t driveName;
    wstring FileName;

    InputVolume(volume, driveName);
    int choice = 0;
    do
    {
        system("cls");
        Menu(choice, driveName);
        switch (choice)
        {
        case 1:
            InputVolume(volume, driveName);
            cout << "Change volume successfully!" << endl;
            break;
        case 2:
            volume->printPartitionBootSector();
            break;
        case 3:
            volume->printDirectoryTree();
            break;
        case 4:
            volume->printDetailedDirectoryTree();
            break;
        case 5:
            wcin.ignore();
            cout << "Input entry name:";
            getline(wcin, FileName);
            volume->printEntryData(FileName);
            break;
        }
        system("pause");


    } while (choice);

    delete volume;
    return 0;
}