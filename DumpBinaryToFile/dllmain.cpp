#include <fstream>
#include "PEBStructure.h"

namespace EXT
{
    auto GetCurrentPeb_() -> PPEB
    {
        return (PPEB)__readgsqword(0x60);
    }

    auto GetCurrentImageBase() -> HMODULE
    {
        return (HMODULE)GetCurrentPeb_()->ImageBaseAddress;
    }
}

auto Main() -> void
{
    //Get Current Process Base Address
    auto BaseAddress = EXT::GetCurrentImageBase();

    //Get DOS Header
    auto DosHeader = (PIMAGE_DOS_HEADER)((uintptr_t)BaseAddress);
    //Get NT Header
    auto NTHeader = (PIMAGE_NT_HEADERS)((uintptr_t)DosHeader->e_lfanew + (uintptr_t)BaseAddress);
    //Get Current Image Size
    auto ImageSize = NTHeader->OptionalHeader.SizeOfImage;

    //Allocate Memory
    char* AllocatedMem = new char[ImageSize];
    //Copy entire binary to our allocated memory
    memcpy(AllocatedMem, (const void*)BaseAddress, ImageSize);

    //Get DOS Header of our Allocated Dump
    auto DosHeaderNew = (PIMAGE_DOS_HEADER)((uintptr_t)AllocatedMem);
    //Get NT Header of our Allocated Dump
    auto NtHeaderNew = (PIMAGE_NT_HEADERS)((uintptr_t)DosHeaderNew->e_lfanew + (uintptr_t)AllocatedMem);

    //Get Section Header of our Allocated Dump
    PIMAGE_SECTION_HEADER SectionHeader = IMAGE_FIRST_SECTION(NtHeaderNew);

    //Iterate through each section and fix size and address
    for (int i = 0; i < NtHeaderNew->FileHeader.NumberOfSections; i++, SectionHeader++)
    {
        SectionHeader->SizeOfRawData = SectionHeader->Misc.VirtualSize;
        SectionHeader->PointerToRawData = SectionHeader->VirtualAddress;
    }

    char FileName[MAX_PATH];
    //Get current process file name
    GetModuleFileNameA(0, FileName, MAX_PATH);

    sprintf_s(FileName, "%s_dump.exe", FileName);

    //Write our dumped and fixed executable to a file
    std::ofstream Dump(FileName, std::ios::binary);
    Dump.write((char*)AllocatedMem, ImageSize);
    Dump.close();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    DisableThreadLibraryCalls(hModule);

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        Main();

    return TRUE;
}