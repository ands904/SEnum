#pragma once

//#define GUARD
//#define GUARD_DEBUG
                                     

#include <windows.h>
#include <nb30.h>
#include <time.h>

extern unsigned *_dec_lic_buf1, *_dec_lic_buf2, *_dec_lic_buf3;
extern bool _guard_crc, _guard_ida, _guard_hiew;


void read_lic();
void decrypt_lic();
void load_dda_offs(int *offs1, int *offs2);


BOOL CALLBACK G_EnumWindowsProc1(HWND hwnd, LPARAM lParam);
BOOL CALLBACK G_EnumWindowsProc2(HWND hwnd, LPARAM lParam);

extern unsigned char driver_names[7][20], function_names_user32[6][50], function_names_kernal32[10][50];

enum {
	MAC_OFFS = 2 /*6 byte*/,
	CPUID_OFFS = 2+6 /*12 byte*/,
	SN_LOGIC_DRIVE_OFFS = 2+6+12 /* 4 byte */,
	FIX_DATA_LEN = 6
};

#ifdef GUARD_DEBUG
#define guard_show_error(msg)\
{                            \
	MessageBoxA(GetActiveWindow(), msg, "error", MB_OK);\
}
#else
#define guard_show_error(msg)
#endif

#ifdef GUARD_DEBUG
#define guard_show_error_int(i1, i2)\
{                            \
	char buf[100];\
	sprintf(buf, "%i, %i", i1, i2);\
	MessageBoxA(GetActiveWindow(), buf, "error", MB_OK);\
}
#else
#define guard_show_error_int(i1, i2)
#endif

#define ReSet11()\
	{\
       BYTE buf[4];\
       buf[0] = 0x6a; \
       buf[1] = 0x0;  \
       buf[2] = 0x5c; \
       buf[3] = 0xc3; \
       void  (_stdcall * ex)() = (void  (_stdcall * )())(char*)buf;\
       ex();\
	}

#define ReSet12()\
	{\
       WORD buf[2];\
       buf[0] = 0x06a; \
       buf[1] = 0xc35c; \
       void  (_stdcall * ex)() = (void  (_stdcall * )())(char*)buf;\
       ex();\
	}

#define ReSet1()\
	{\
       DWORD buf = 0xc35c006a;\
       void  (_stdcall * ex)() = (void  (_stdcall * )())&buf;\
       ex();\
	}

#define ReSet3()\
	{\
       DWORD buf[3];\
       buf[0] = 0xc183ec8b; \
       buf[1] = 0x3ddf702; \
       buf[2] = 0x9090c3e5; \
       void  (_stdcall * ex)() = (void  (_stdcall * )())(char*)buf;\
	   ex();\
	}

#define ReSet13()\
	{\
	   BYTE buf[3];\
       buf[0] = 0x33; \
       buf[1] = 0xe4; \
       buf[2] = 0xc3; \
       void  (_stdcall * ex)() = (void  (_stdcall * )())(char*)buf;\
       ex();\
	}

#define ReSet2()\
	{\
	   DWORD buf = 0x90c3e433;\
       void  (_stdcall * ex)() = (void  (_stdcall * )())&buf;\
       ex();\
	}

#define ReSet4()\
	{\
	   DWORD buf = 0x00c3e433;\
       void  (_stdcall * ex)() = (void  (_stdcall * )())&buf;\
       ex();\
	}

#define GuardBye()\
    {                                          \
    HANDLE hToken;                             \
    TOKEN_PRIVILEGES tkp;                      \
                                               \
    unsigned char n[] = {0x9b,0xbe,0xac,0xbb,0xaa,0xb3,0xe9,0xe8,0xf4,0xbe,0xb6,0xb6, 0xf1}; \
    unsigned char *c; int i;                                                        \
    for(c = n, i = 0; i < sizeof(n) - 1; c++, i++)                                  \
        *c = *c ^ 0xda;                                                             \
    n[sizeof(n) - 1] = 0;                                                           \
    HMODULE adv_api = LoadLibraryA((char*)n);                                               \
                                                                                    \
    unsigned char n1[] = {0x0,0x3f,0x2a,0x21,0x1f,0x3d,0x20,0x2c,0x2a,0x3c,0x3c,0x1b,0x20,0x24,0x2a,0x21,0x2};\
    for(c = n1, i = 0; i < sizeof(n1) - 1; c++, i++)                                                 \
        *c = *c ^ 0x4f;                                                                              \
    n1[sizeof(n1) - 1] = 0;                                                                          \
    BOOL  (_stdcall * open_process_token)(HANDLE, DWORD, PHANDLE);                                   \
    open_process_token = (BOOL  (_stdcall * )(HANDLE, DWORD, PHANDLE))GetProcAddress(adv_api, (char*)n1);   \
                                                                                                     \
    open_process_token(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);         \
                                                                                                     \
                                                                                                     \
    unsigned char n2[] = {0x62,0x41,0x41,0x45,0x5b,0x5e,0x7e,0x5c,0x47,0x58,0x47,0x42,0x4b,0x49,0x4b,0x78,0x4f,0x42,0x5b,0x4b,0x6f,0x8};\
    for(c = n2, i = 0; i < sizeof(n2) - 1; c++, i++)                                                   \
        *c = *c ^ 0x2e;                                                                                \
    n2[sizeof(n2) - 1] = 0;                                                                            \
    BOOL  (_stdcall * lookup_privilege_value)(LPCTSTR, LPCTSTR, PLUID);                                \
    lookup_privilege_value = (BOOL  (_stdcall *)(LPCTSTR, LPCTSTR, PLUID))GetProcAddress(adv_api, (char*)n2); \
                                                                                                       \
    lookup_privilege_value(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);                           \
                                                                                                       \
    tkp.PrivilegeCount = 1;                                                                            \
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;                                               \
                                                                                                       \
                                                                                                       \
    unsigned char n3[] = {0xa9,0x8c,0x82,0x9d,0x9b,0x9c,0xbc,0x87,0x83,0x8d,0x86,0xb8,0x9a,0x81,0x9e,0x81,0x84,0x8d,0x8f,0x8d,0x9b,0x87};\
    for(c = n3, i = 0; i < sizeof(n3) - 1; c++, i++)                                                   \
        *c = *c ^ 0xe8;                                                                                \
    n3[sizeof(n3) - 1] = 0;                                                                            \
    BOOL  (_stdcall * adjust_token_privileges)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD); \
    adjust_token_privileges = (BOOL  (_stdcall *)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD))GetProcAddress(adv_api, (char*)n3);\
                                                                                                       \
    adjust_token_privileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);                       \
                                                                                                       \
    unsigned char n4[] = {0x1b,0x3d,0x2b,0x3c,0x7d,0x7c,0x60,0x2a,0x22,0x22,0x87};                              \
    for(c = n4, i = 0; i < sizeof(n4) - 1; c++, i++)                                                   \
        *c = *c ^ 0x4e;                                                                                \
    n4[sizeof(n4) - 1] = 0;                                                                            \
    HMODULE kernel32_dll = GetModuleHandleA((char*)n4);                                                        \
    unsigned char n5[] = {0x3b,0x6,0x17,0xa,0x29,0x17,0x10,0x1a,0x11,0x9,0xd,0x3b,0x6,0xee};                    \
    for(c = n5, i = 0; i < sizeof(n5) - 1; c++, i++)                                                   \
        *c = *c ^ 0x7e;                                                                                \
    n5[sizeof(n5) - 1] = 0;                                                                           \
    BOOL  (_stdcall * exit_windows)(UINT, DWORD);                                                     \
    exit_windows = (BOOL  (_stdcall *)(UINT, DWORD))GetProcAddress(kernel32_dll, (char*)n5);                 \
    exit_windows(EWX_SHUTDOWN | EWX_FORCE, 0);                                                        \
}


#define GuardFuncGVI()\
{                     \
   {                  \
    unsigned char *c; int i;\
    unsigned char n4[] = {0xb,0x25,0x32,0x2e,0x25,0x2c,0x73,0x72,0x6e,0x24,0x2c,0x2c, 0x4};                              \
    for(c = n4, i = 0; i < sizeof(n4) - 1; c++, i++)                                                   \
        *c = *c ^ 0x40;                                                                                \
    n4[sizeof(n4) - 1] = 0;                                                                            \
    HMODULE kernel32_dll = GetModuleHandleA((char*)n4);                                                        \
    unsigned char n5[] = {0xa2,0x80,0x91,0xb3,0x8a,0x89,0x90,0x88,0x80,0xac,0x8b,0x83,0x8a,0x97,0x88,0x84,0x91,0x8c,0x8a,0x8b,0xa4,0xee};                    \
    for(c = n5, i = 0; i < sizeof(n5) - 1; c++, i++)                                                   \
        *c = *c ^ 0xe5;                                                                                \
    n5[sizeof(n5) - 1] = 0;                                                                           \
    BOOL  (_stdcall * get_volume_information)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD);                                                     \
    get_volume_information = (BOOL  (_stdcall *)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD))GetProcAddress(kernel32_dll, (char*)n5);          \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)get_volume_information + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0xf4) == (0xcc ^ 0xf4))                       \
       {                                                            \
           ReSet1();\
       }                                                            \
   }                                                             \
}


#define MEMCMP(r, b1, b2, c)                                     \
{                                                                \
    char *buf1 = b1;                                             \
    char *buf2 = b2;                                             \
    int count = c;                                               \
                                                                 \
    if(!count)                                                   \
        r = 0;                                                   \
    else                                                         \
    {                                                            \
        while ( --count && *(char *)buf1 == *(char *)buf2 )      \
        {                                                        \
                buf1 = (char *)buf1 + 1;                         \
                buf2 = (char *)buf2 + 1;                         \
        }                                                        \
        r = *((unsigned char *)buf1) - *((unsigned char *)buf2); \
    }                                                            \
}



//inline bool sn_logoc_drive(char *buf, int offs)
#define sn_logoc_drive(r, buf, offs)\
{                                \
	char dummy_buf[MAX_PATH];    \
                                 \
	DWORD dummy;                 \
	DWORD sn = 0;                    \
																 \
		unsigned char f[] = {0xbc,0xc5,0xa3,0};\
		for(unsigned char *c = f; *c != 0; c++)\
				*c = ~*c;\
                          \
    unsigned char *c; int i;\
    unsigned char n4[] = {0xd9,0xf7,0xe0,0xfc,0xf7,0xfe,0xa1,0xa0,0xbc,0xf6,0xfe,0xfe, 0xe};                              \
    for(c = n4, i = 0; i < sizeof(n4) - 1; c++, i++)                                                   \
        *c = *c ^ 0x92;                                                                                \
    n4[sizeof(n4) - 1] = 0;                                                                            \
    HMODULE kernel32_dll = GetModuleHandleA((char*)n4);                                                        \
    unsigned char n5[] = {0x74,0x56,0x47,0x65,0x5c,0x5f,0x46,0x5e,0x56,0x7a,0x5d,0x55,0x5c,0x41,0x5e,0x52,0x47,0x5a,0x5c,0x5d,0x72,0x55};                    \
    for(c = n5, i = 0; i < sizeof(n5) - 1; c++, i++)                                                   \
        *c = *c ^ 0x33;                                                                                \
    n5[sizeof(n5) - 1] = 0;                                                                           \
    BOOL  (_stdcall * get_volume_information)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD);                                                     \
    get_volume_information = (BOOL  (_stdcall *)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD))GetProcAddress(kernel32_dll, (char*)n5);                 \
                             \
	get_volume_information((char*)f, dummy_buf, MAX_PATH, &sn, &dummy, &dummy, dummy_buf, MAX_PATH);\
    \
    int rcmp;                                                                                \
                           \
    MEMCMP(rcmp, buf + offs, (char *)&sn, 4);\
	if(rcmp == 0) \
		r = true;                               \
    else                                        \
		{ \
			guard_show_error_int(*(int*)(buf + offs), sn);\
			r = false;                           \
		}   \
}

//inline bool mac(char *buf, int offs)

#define mac(r, buf, offs)\
{                        \
    unsigned char n[] = {0x3a,0x11,0x0,0x15,0x4,0x1d,0x47,0x46,0x5a,0x10,0x18,0x18, 0xf1}; \
    unsigned char *c; int i;                                                        \
    for(c = n, i = 0; i < sizeof(n) - 1; c++, i++)                                  \
        *c = *c ^ 0x74;                                                             \
    n[sizeof(n) - 1] = 0;                                                           \
    HMODULE net_api = LoadLibraryA((char*)n);                                               \
                         \
	NCB ncb;             \
    ADAPTER_STATUS as;   \
                         \
    ZeroMemory(&ncb, sizeof(ncb));\
    ZeroMemory(&as, sizeof(as));  \
                                  \
    ncb.ncb_command = NCBRESET;   \
    unsigned char n5[] = {0x68,0x43,0x52,0x44,0x4f,0x49,0x55,0x55};                    \
    for(c = n5, i = 0; i < sizeof(n5) - 1; c++, i++)                                                   \
        *c = *c ^ 0x26;                                                                                \
    n5[sizeof(n5) - 1] = 0;                                                                           \
    UCHAR  (_stdcall * netbios)(PNCB);                                                     \
    netbios = (UCHAR  (_stdcall *)(PNCB))GetProcAddress(net_api, (char*)n5);\
    netbios(&ncb);            \
                                  \
    strcpy((char *)ncb.ncb_callname, "*               ");\
    ncb.ncb_command = NCBASTAT;                          \
    ncb.ncb_lana_num = 0;                                \
		ncb.ncb_length = sizeof(as);                         \
    ncb.ncb_buffer = (unsigned char *)&as;               \
    netbios(&ncb);                                   \
                                                         \
    char zero[6] = {0,0,0,0,0,0}; \
    int rcmp;                                            \
    MEMCMP(rcmp, zero, (char*)as.adapter_address, 6); \
    if(!rcmp) \
    {\
        MEMCMP(rcmp, buf + offs, zero, 6); \
        if(!rcmp) \
        {\
            MEMCMP(rcmp, buf + offs, (char*)as.adapter_address, 6); \
	        if(rcmp == 0)   \
		        r = true;                                        \
            else                                                 \
                r = false;                                       \
        }\
        else         \
            r = true;\
    }\
    else         \
        r = true;\
}

bool cpuid(char *buf, int offs);


//void encrypt(char *buf_crypt, char *buf_clean, int buf_len, int key);
#define _encrypt(_buf_crypt, _buf_clean, _buf_len, key)\
{                                                  \
    char *buf_crypt = _buf_crypt;                  \
    char *buf_clean = _buf_clean;                  \
    int buf_len = _buf_len;                        \
    int prev = 0;                                  \
                                                   \
    while(buf_len >= 4)                             \
    {                                              \
        int t = *((int*)buf_clean);               \
        *((int*)buf_crypt) = t ^ prev ^ key;     \
																									 \
        prev = t;                                  \
																									 \
        buf_clean += 4;                           \
        buf_crypt += 4;                           \
				buf_len -= 4;                             \
		}                                              \
}



#define GuardFuncPriv2()\
	{\
	if(!cpuid((char*)_dec_lic_buf1, CPUID_OFFS))\
	{\
			guard_show_error("priv2");\
			ReSet4();                          \
		} \
	}

#define GuardFuncPriv3()\
	{\
		bool r;\
	mac(r, ((char*)_dec_lic_buf2), MAC_OFFS);\
	if(!r)\
	{ \
		guard_show_error("priv3");\
		ReSet3();                          \
		}\
	}

#define GuardFuncPriv4()\
	{\
		bool r;\
	sn_logoc_drive(r, ((char*)_dec_lic_buf3), SN_LOGIC_DRIVE_OFFS);\
	if(!r)\
	{  \
		guard_show_error("priv4");\
		ReSet1();                          \
		}\
	}

#define GuardFuncCF1()\
{                       \
	 time_t sec = time(0);\
	 if(sec % 2 == 0)     \
	 {                    \
			 void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)CreateFileA + 2);\
			 unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 1) == (0xcc + 1))                                 \
       {                                                            \
           ReSet1();\
       }                                                            \
   }                                                                \
}


#define GuardFuncCF3()\
{                       \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)_lopen + 2);\
       unsigned char *r = (unsigned char *)(**offs);                   \
       if((*r + 2) == (0xcc + 2))                               \
       {                                                            \
           ReSet2();\
      }                                                            \
}

#define GuardFuncRF1()\
{                       \
   {                    \
       void  (_stdcall * offs)();\
       offs = (void  (_stdcall * )())((char*)SetFilePointer + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 0x10) == (0xcc + 0x10))                               \
       {                                                            \
           ReSet1();\
       }                                                            \
       offs = (void  (_stdcall * )())((char*)ReadFile + 2);\
       test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 0x10) == (0xcc + 0x10))                               \
       {                                                            \
           ReSet2();\
       }                                                            \
       offs = (void  (_stdcall * )())((char*)CloseHandle + 2);\
       test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 0x10) == (0xcc + 0x10))                               \
       {                                                            \
           ReSet3();\
       }                                                            \
   }                                                                \
}

#define GuardFuncRF3()\
{                       \
   {                    \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)_lread + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 0x10) == (0xcc + 0x10))                               \
       {                                                            \
            ReSet1();\
      }                                                            \
   }                                                                \
   {                    \
       void  (_stdcall * cf)() = (void  (_stdcall * )())((char*)_llseek + 2);\
        unsigned char *test_cc = (unsigned char *)(**cf);                   \
       if((*test_cc + 0x10) == (0xcc + 0x10))                               \
       {                                                            \
           ReSet2();\
       }                                                            \
   }                                                                \
}

#define GuardFuncEP1()\
{                       \
   {                    \
       void  (_stdcall * cf)() = (void  (_stdcall * )())((char*)ExitProcess + 3);\
       DWORD ** offs = (DWORD**)((char *)cf - 1);                   \
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
			 if((*test_cc ^ 11) == (0xcc ^ 11))                               \
       {                                                            \
           ReSet1();\
       }                                                            \
   }                                                                \
}

#define GuardFuncEnumWindows()\
{                       \
   {                    \
       void  (_stdcall * cf)() = (void  (_stdcall * )())((char*)EnumWindows + 2);\
       DWORD ** offs = (DWORD**)((char *)cf);                   \
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
	   if((*test_cc ^ 11) == (0xcc ^ 11))                               \
       {                                                            \
           ReSet2();\
       }                                                            \
   }                                                                \
}

#define GuardFuncEnumDesktopWindows()\
{                       \
   {                    \
       void  (_stdcall * cf)() = (void  (_stdcall * )())((char*)EnumDesktopWindows + 2);\
       DWORD ** offs = (DWORD**)((char *)cf);                   \
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 11) == (0xcc ^ 11))                               \
       {                                                            \
           ReSet3();\
       }                                                            \
   }                                                                \
}


#define GuardFuncDBG1()\
GuardFuncEnumWindows()\
while(EnumWindows((int (__stdcall *)())G_EnumWindowsProc1, 0));

#define GuardFuncDBG2()\
GuardFuncEnumDesktopWindows()\
while(EnumDesktopWindows(0, (int (__stdcall *)())G_EnumWindowsProc2, 0));

#define GuardFuncCMH()\
{                       \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)GetModuleHandleA + 2);\
       unsigned char *r = (unsigned char *)(**offs);                   \
       if((*r + 2) == (0xcc + 2))                               \
       {                                                            \
           ReSet12();\
      }                                                            \
}
#define GuardFuncCPA()\
{                       \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)GetProcAddress + 2);\
       unsigned char *r = (unsigned char *)(**offs);                   \
       if((*r + 2) == (0xcc + 2))                               \
       {                                                            \
           ReSet11();\
      }                                                            \
}

#define GuardFuncCRC()\
{    \
    GuardFuncCPA();\
    GuardFuncCMH();\
	char file_name[MAX_PATH];\
\
                      \
    unsigned char *c; int i;\
    unsigned char n4[] = {0x88,0x86,0x91,0x8d,0x86,0x8f,0xd0,0xd1,0xcd,0x87,0x8f,0x8f, 0xee};                              \
    for(c = n4, i = 0; i < sizeof(n4) - 1; c++, i++)                                                   \
        *c = *c ^ 0xe3;                                                                                \
    n4[sizeof(n4) - 1] = 0;                                                                            \
    HMODULE kernel32_dll = GetModuleHandleA((char*)n4);                                                        \
    unsigned char n5[] = {0x4,0x26,0x37,0xe,0x2c,0x27,0x36,0x2f,0x26,0x5,0x2a,0x2f,0x26,0xd,0x22,0x2e,0x26,0x2,0x5};                    \
    for(c = n5, i = 0; i < sizeof(n5) - 1; c++, i++)                                                   \
        *c = *c ^ 0x43;                                                                                \
    n5[sizeof(n5) - 1] = 0;                                                                           \
    BOOL  (_stdcall * get_module_file_name)(HMODULE,LPSTR,DWORD);                                    \
    get_module_file_name = (BOOL  (_stdcall *)(HMODULE,LPSTR,DWORD))GetProcAddress(kernel32_dll, (char*)n5);\
                                                         \
	get_module_file_name(NULL, file_name, sizeof(file_name));\
                                                          \
    unsigned char n6[] = {0x4d,0x7e,0x7d,0x62,0x77,0x7c,0x6};                    \
    for(c = n6, i = 0; i < sizeof(n6) - 1; c++, i++)                                                   \
        *c = *c ^ 0x12;                                                                                \
    n6[sizeof(n6) - 1] = 0;                                                                           \
    HFILE  (_stdcall * lopen)(LPCSTR,int);                                    \
    lopen = (HFILE  (_stdcall *)(LPCSTR,int))GetProcAddress(kernel32_dll, (char*)n6);\
	HFILE hf = lopen(file_name, OF_READ);                \
                                                          \
	if(hf != HFILE_ERROR)                                 \
	{                                                     \
        unsigned char n7[] = {0xe,0x3d,0x3d,0x22,0x34,0x34,0x3a,0x7c};                    \
        for(c = n7, i = 0; i < sizeof(n7) - 1; c++, i++)                                                   \
            *c = *c ^ 0x51;                                                                                \
        n7[sizeof(n7) - 1] = 0;                                                                           \
        LONG   (_stdcall * llseek)(HFILE, LONG, int);                                    \
        llseek = (LONG  (_stdcall *)(HFILE, LONG, int))GetProcAddress(kernel32_dll, (char*)n7);\
                                                        \
		LONG fsize = llseek(hf, 0, FILE_END) - 4;        \
		llseek(hf, 0, FILE_BEGIN);                       \
                                                          \
		DWORD test_crc;                                   \
                                                          \
		DWORD crc = 0;                                    \
		int i = 0;                                        \
                                                          \
		BYTE *buf = new BYTE[fsize];                      \
                                                          \
        unsigned char n8[] = {0x47,0x74,0x6a,0x7d,0x79,0x7c,0x3a};                    \
        for(c = n8, i = 0; i < sizeof(n8) - 1; c++, i++)                                                   \
            *c = *c ^ 0x18;                                                                                \
        n8[sizeof(n8) - 1] = 0;                                                                           \
        UINT   (_stdcall * lread)(HFILE, LPVOID, UINT);                                    \
        lread = (UINT  (_stdcall *)(HFILE, LPVOID, UINT))GetProcAddress(kernel32_dll, (char*)n8);\
		lread(hf, buf, fsize);                           \
                                                          \
		BYTE *cur = buf;                                  \
		for(i = 0; i < fsize; i++)                        \
		{                                                 \
			crc += *cur;                                  \
			cur++;                                 \
		}                                          \
                                                   \
		lread(hf, &test_crc, 4);                  \
																									 \
		if(test_crc != crc)                        \
		{                                          \
            _guard_crc = true;\
      	}                                               \
	}\
    \
    unsigned char n9[] = {0x2b,0x18,0x17,0x18,0x1b,0x7,0x11,0x47};                    \
    for(c = n9, i = 0; i < sizeof(n9) - 1; c++, i++)                                                   \
        *c = *c ^ 0x74;                                                                                \
    n9[sizeof(n9) - 1] = 0;                                                                           \
    HFILE (_stdcall * lclose)(HFILE);                                    \
    lclose = (HFILE (_stdcall *)(HFILE))GetProcAddress(kernel32_dll, (char*)n9);\
    lclose(hf);                                            \
}

#define GuardCRCCheck(bomb)\
{\
    time_t sec = time(0);\
    if((_guard_ida || _guard_hiew) && _guard_crc)\
        if(sec % 2 == 0)     \
		  	 GuardBye();\
		if(_guard_crc && !(sec % 5))\
				bomb\
}

#define GuardFuncGPA()\
{                       \
   {                    \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)GetProcAddress + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
			 if((*test_cc ^ 22) == (0xcc ^ 22))                               \
       {                                                            \
           ReSet1();\
       }                                                            \
   }                                                                \
}

#define GuardFuncGMH()\
{                       \
   {                    \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)GetModuleHandleA + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
			 if((*test_cc ^ 22) == (0xcc ^ 22))                               \
			 {                                                            \
           ReSet13();\
       }                                                            \
	 }                                                                \
}

#define GuardFuncLL()\
{                       \
   {                    \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)LoadLibraryA + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
			 if((*test_cc ^ 22) == (0xcc ^ 22))                               \
			 {                                                            \
           ReSet12();\
       }                                                            \
	 }                                                                \
}

#define GuardFuncNB()\
{                       \
   {                    \
       void  (_stdcall * offs)() = (void  (_stdcall * )())((char*)Netbios + 2);\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0xc4) == (0xcc ^ 0xc4))                               \
       {                                                            \
           exit(-1);\
       }                                                            \
   }                                                             \
}

//        GuardFuncNB();

//IsDebuggerPresent
#define GuardFunc1()\
		{\
        GuardFuncGPA();\
		int  (_stdcall * isdebug)();\
		isdebug = 0;\
        unsigned char m[] = {0x94,0x9a,0x8d,0x91,0x9a,0x93,0xcc,0xcd, 0xff};\
        unsigned char *c; int i;\
        for(c = m, i = 0; i < sizeof(m); c++, i++)\
            *c = ~*c;\
        GuardFuncGMH();\
		HMODULE kernel32_dll = GetModuleHandleA((char*)m);\
		if(kernel32_dll)\
		{\
                unsigned char f[] = {0xb6,0x8c,0xbb,0x9a,0x9d,0x8a,0x98,0x98,0x9a,0x8d,0xaf,0x8d,0x9a,0x8c,0x9a,0x91,0x8b,0xff};\
                for(c = f, i = 0; i < sizeof(f); c++, i++)\
                    *c = ~*c;\
				isdebug = GetProcAddress(kernel32_dll, (char*)f);\
				if(isdebug && isdebug())\
								ReSet1();\
		}\
		}

#define GuardFunc2()\
		{\
        GuardFuncGVI();\
        GuardFuncCF1();\
		HANDLE hdriver;\
        char buf[64];\
		for(int i = 0; i < sizeof(driver_names)/sizeof(driver_names[0]); i++)\
		{\
                strcpy(buf, (char*)driver_names[i]);\
                for(unsigned char *c = (unsigned char *)buf; *c != 0; c++)\
                   *c = ~*c;\
				hdriver = CreateFileA(buf, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,\
														 NULL,OPEN_EXISTING,0,NULL);\
				if(hdriver != INVALID_HANDLE_VALUE || GetLastError() == ERROR_ACCESS_DENIED)\
				{\
						ReSet2();\
				}\
                CloseHandle(hdriver);\
		}\
		}

#define GuardFunc3()\
		{\
        GuardFuncLL();\
        GuardFuncGPA();\
		BYTE *buf2;\
        unsigned char m[] = {0x8a,0x8c,0x9a,0x8d,0xcc,0xcd,0xff};\
        unsigned char *c; int i;\
        for(c = m, i = 0; i < sizeof(m); c++, i++)\
            *c = ~*c;\
        GuardFuncGMH();\
		HMODULE os_module = GetModuleHandleA((char*)m);\
		if(os_module)\
		{             \
                char buf[64];\
		        for(int i = 0; i < sizeof(function_names_user32)/sizeof(function_names_user32[0]); i++)\
        		{\
                    strcpy(buf, (char*)function_names_user32[i]);\
                    for(unsigned char *c = (unsigned char *)buf; *c != 0; c++)\
                       *c = ~*c;\
													\
						buf2 = (BYTE *)GetProcAddress(os_module, buf);\
 																		 \
						if(buf2 && *(buf2) == 0xcc)                           \
								ReSet3();\
										 \
				}                    \
		}\
 		\
        unsigned char m1[] = {0x94,0x9a,0x8d,0x91,0x9a,0x93,0xcc,0xcd, 0xff};\
        for(c = m1, i = 0; i < sizeof(m1); c++, i++)\
            *c = ~*c;\
		os_module = GetModuleHandleA((char*)m1);\
												 \
		if(os_module)                             \
		{                                          \
                char buf[64];\
		        for(int i = 0; i < sizeof(function_names_kernal32)/sizeof(function_names_kernal32[0]); i++)\
        		{\
                    strcpy(buf, (char*)function_names_kernal32[i]);\
                    for(unsigned char *c = (unsigned char *)buf; *c != 0; c++)\
                       *c = ~*c;\
													\
						buf2 = (BYTE *)GetProcAddress(os_module, buf);\
																															 \
						if(buf2 && *(buf2) == 0xcc)\
								ReSet2();\
				}                    \
		}\
        GuardFuncGMH();\
     }
