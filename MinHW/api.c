#include "api.h"
#include <stdint.h>
#include <winternl.h>

#define KERNEL32DLL_HASH 0x536CD652u

typedef struct {
    PVOID Reserved1[2];
    LIST_ENTRY InMemoryOrderLinks;
    PVOID Reserved2[2];
    PVOID DllBase;
    uint8_t pad[116];
    ULONG BaseNameHashValue;
} MY_LDR_DATA_TABLE_ENTRY;

uint32_t Kernel32Hashes[] = {
        0x5FBFF0FBu,
        0x5A153F58u,
        0xB511FC4Du,
        0x348B7545u,
        0xB769339Eu
};

uint32_t User32Hashes[] = {
        0x110F756Fu,
        0xEF1BD604u,
        0x932EB07Eu,
        0x1C82E26Fu,
        0xE870E800u,
        0xCBCEB9ABu,
        0xE5425A58u,
        0xABA0605Bu,
        0x218F96D3u,
        0xDD158C66u,
        0xB05422B2u,
        0x5F0BBD59u,
        0x2C1A4AD8u,
        0x68F05E41u
};

uint32_t Gdi32Hash = 0x6F828843u;

#pragma pack(push, 1)
struct dll_info {
    uint32_t exported_functions;
    uintptr_t address_table_RVA;
    uintptr_t name_pointer_table_RVA;
    uintptr_t ordinal_table_RVA;
};
#pragma pack(pop)

static uint32_t djb2(uint8_t* str) {
    uint32_t hash = 5381;
    uint8_t c;

    while (c = *(str++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

static void findFunc(uintptr_t dllBase, uint32_t* hashes, void** ptrs, size_t size) {
    uintptr_t PE_RVA = *(uintptr_t*)((uint8_t*)dllBase + 0x3Cu);
    uintptr_t PE = dllBase + *(uintptr_t*)((uint8_t*)dllBase + 0x3Cu);

    uintptr_t export_table_RVA = *(uintptr_t*)((uint8_t*)PE + 0x78u);
    struct dll_info *dll = (uint8_t*)dllBase + export_table_RVA + 0x18u;

    uintptr_t name_pointer_table_entry_RVA = dll->name_pointer_table_RVA;
    uint32_t i, j;

    uintptr_t ordinal_function_RVA;
    uint16_t ordinal_function;
    uintptr_t function_RVA;

    for (i = 0; i < dll->exported_functions; i++, name_pointer_table_entry_RVA += 4) {
        uintptr_t function_name_RVA = *(uintptr_t*)((uint8_t*)dllBase + name_pointer_table_entry_RVA);
        char* function_name = (uint8_t*)dllBase + function_name_RVA;
        uint32_t function_hash = djb2(function_name);

        for (j = 0; j < size; j++) {
            if (function_hash == hashes[j]) {
                ordinal_function_RVA = dll->ordinal_table_RVA + i * 2;
                ordinal_function = *(uint16_t*)((uint8_t*)dllBase + ordinal_function_RVA);
                function_RVA = *(uintptr_t*)((uint8_t*)dllBase + dll->address_table_RVA + ordinal_function * 4);

                ptrs[j] = (uint8_t*)dllBase + function_RVA;

                break;
            }
        }
    }
}

void initAPI(API* api) {
    PPEB peb = __readfsdword(0x30);
    uintptr_t kernel32Base = 0;

    for (PLIST_ENTRY ptr = peb->Ldr->InMemoryOrderModuleList.Flink; kernel32Base == 0; ptr = ptr->Flink) {
        MY_LDR_DATA_TABLE_ENTRY *e = CONTAINING_RECORD(ptr, MY_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        
        if (e->BaseNameHashValue == KERNEL32DLL_HASH)
            kernel32Base = (uintptr_t)e->DllBase;
    }

    findFunc(kernel32Base, Kernel32Hashes, &api->_LoadLibraryA, ARRAYSIZE(Kernel32Hashes));

    HMODULE user32 = api->_LoadLibraryA("USER32.DLL");
    findFunc((uintptr_t)user32, User32Hashes, &api->_LoadIconA, ARRAYSIZE(User32Hashes));

    HMODULE gdi32 = api->_LoadLibraryA("GDI32.DLL");
    findFunc((uintptr_t)gdi32, &Gdi32Hash, &api->_SetBkMode, 1);
}