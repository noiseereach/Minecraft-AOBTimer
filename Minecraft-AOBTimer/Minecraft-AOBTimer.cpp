#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <string>

using namespace std;

#define on ,

// My text color function. Use it if you wish.
void text(int text_color = 7 on int paper_color = 0)
{
    // defaults to light_gray on black
    int color_total = (text_color + (paper_color * 16));
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_total);
}

char* ScanBasic(const char* pattern, const char* mask, char* begin, intptr_t size)
{
    intptr_t patternLen = strlen(mask);

    for (int i = 0; i < size; i++)
    {
        bool found = true;
        for (int j = 0; j < patternLen; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (begin + i);
        }
    }
    return nullptr;
}

void* ScanEx(const char* pattern, const char* mask, char* begin, intptr_t size, HANDLE hProc) {
    char* match{ nullptr };
    SIZE_T bytesRead;
    DWORD oldprotect;
    char* buffer{ nullptr };
    MEMORY_BASIC_INFORMATION mbi;
    mbi.RegionSize = 0x1000;

    VirtualQueryEx(hProc, (LPCVOID)begin, &mbi, sizeof(mbi));

    for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
    {
        if (!VirtualQueryEx(hProc, curr, &mbi, sizeof(mbi))) continue;
        if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || mbi.Protect != PAGE_EXECUTE_READWRITE) continue;

        delete[] buffer;
        buffer = new char[mbi.RegionSize];

        if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect))
        {
            ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
            VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, oldprotect, &oldprotect);

            char* internalAddr = ScanBasic(pattern, mask, buffer, (intptr_t)bytesRead);

            if (internalAddr != nullptr) {
                match = curr + (internalAddr - buffer);
                break;
            }
        }
    }
    delete[] buffer;
    return match;
}


enum Colors {
    black,          //  0 text color - multiply by 16, for background colors
    dark_blue,      //  1
    dark_green,     //  2
    dark_cyan,      //  3
    dark_red,       //  4
    dark_magenta,   //  5
    dark_yellow,    //  6
    light_gray,     //  7
    dark_gray,      //  8
    light_blue,     //  9
    light_green,    // 10
    light_cyan,     // 11
    light_red,      // 12
    light_magenta,  // 13
    light_yellow,   // 14
    white           // 15
};

int main()
{
    time_t now = time(0);
    tm* ltm = localtime(&now);
    text();
    text(light_red on black);
    HWND hwnd = FindWindowA(("LWJGL"), NULL);
    if (hwnd) {
        Sleep(500);
    }
    else {
        cout << "Minecraft not found!" << endl;
        Sleep(2500);
        exit(-1);
    }

    text();
    text(dark_cyan on black);
    std::cout << "[" << ltm->tm_mday << "." << 1 + ltm->tm_mon << "." << 1900 + ltm->tm_year << "]"  " Activate Timer [HOME]" << std::endl;
    std::cout << "[" << ltm->tm_mday << "." << 1 + ltm->tm_mon << "." << 1900 + ltm->tm_year << "]"  " Deactivate Timer [F8]" << std::endl;

    DWORD procId;
    GetWindowThreadProcessId(hwnd, &procId);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    while (true) {
        if (GetAsyncKeyState(VK_HOME) & 1) {
            void* active = ScanEx("\xC5\xE3\x59\xD2\xC5\xEB\x59\xC0", "xxxxxxxx", (char*)0x1000000, 0x9000000, handle);
            WriteProcessMemory(handle, active, "\x90\x90\x90\x90\xC5\xEB\x59\xC0", 8, NULL);
            text(dark_green on black);
            std::cout << "[" << ltm->tm_mday << "." << 1 + ltm->tm_mon << "." << 1900 + ltm->tm_year << "]"  " Timer Active" << std::endl;
        }
        if (GetAsyncKeyState(VK_F8) & 1) {
            void* deactive = ScanEx("\x90\x90\x90\x90\xC5\xEB\x59\xC0", "xxxxxxxx", (char*)0x1000000, 0x9000000, handle);
            WriteProcessMemory(handle, deactive, "\xC5\xE3\x59\xD2\xC5\xEB\x59\xC0", 8, NULL);
            text(light_red on black);
            std::cout << "[" << ltm->tm_mday << "." << 1 + ltm->tm_mon << "." << 1900 + ltm->tm_year << "]"  " Timer Deactive" << std::endl;
        }
        Sleep(1);
    }
}