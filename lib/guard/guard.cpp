#include "guard.h"

#include <windows.h>
#include <time.h>

#include <fstream>
using namespace std;

#include <stdio.h>
#include "idea.h"

#include "rsa.h"

#ifdef _MSC_VER
#pragma warning( disable : 4312 4309 4101 4996 4311)
#endif

//#define GUARD_PROTECT_V1

#ifdef GUARD_PROTECT_V1
	enum {CKEY = 0x5fcde34a};
	#define GP_DDA_OFFS_FNAME {0x9c,0xc5,0xa3,0x8c,0x86,0x8c,0x91,0x8b,0xd1,0x8c,0x86,0x8c,0xff}
	#define GP_FIX_FNAME      {0x9c,0xc5,0xa3,0x91,0x93,0x90,0x98,0xd1,0x8c,0x86,0x8c,0xff}
    enum {IDEA_XOR_MASK = 0xf4c034fc};
#else
	enum {CKEY = 0x75e6a2e3};
	#define GP_DDA_OFFS_FNAME {0x9c,0xc5,0xa3,0x8c,0x86,0x8c,0xce,0xcd,0xd1,0x8c,0x86,0x8c,0xff}
	#define GP_FIX_FNAME      {0xed,0xb4,0xd2,0xba,0xb8,0xb8,0xb7,0xb9,0xb6,0xb9,0xbe,0xa0,0xfd,0xf7,0xfd, 0x8e}
    enum {IDEA_XOR_MASK = 0};
#endif

static char *_lic_buf = 0;
static int _lic_size;

bool _guard_crc, _guard_ida, _guard_hiew;

unsigned char driver_names[][20] = {
{0xa3,0xa3,0xd1,0xa3,0xac,0x96,0xbc,0x9a,0},
{0xa3,0xa3,0xd1,0xa3,0x91,0x8b,0x96,0x9c,0x9a,0},
{0xa3,0xa3,0xd1,0xa3,0xac,0x96,0xa8,0xa9,0},
{0xa3,0xa3,0xd1,0xa3,0xb9,0xb6,0xb3,0x9a,0xb2,0x90,0xb1,0},
{0xa3,0xa3,0xd1,0xa3,0xb9,0x96,0xb3,0xba,0x89,0xa7,0xbb,0},
{0xa3,0xa3,0xd1,0xa3,0xad,0x9a,0xb8,0xb2,0x90,0xb1,0},
{0xa3,0xa3,0xd1,0xa3,0xad,0xba,0x98,0xa9,0xa7,0x9b,0},
/*"\\\\.\\SiCe",
"\\\\.\\ntice",
"\\\\.\\SiWV",
"\\\\.\\FILeMoN",
"\\\\.\\FiLEvXD",
"\\\\.\\ReGMoN",
"\\\\.\\REgVXd",*/
};

unsigned *_dec_lic_buf2 = 0;

unsigned char function_names_user32[][50] = {
{0xb2,0x9a,0x8c,0x8c,0x9e,0x98,0x9a,0xbd,0x90,0x87,0xbe,0},
{0xb8,0x9a,0x8b,0xbb,0x93,0x98,0xb6,0x8b,0x9a,0x92,0xab,0x9a,0x87,0x8b,0xbe,0},
{0xb8,0x9a,0x8b,0xbb,0x93,0x98,0xb6,0x8b,0x9a,0x92,0xb6,0x91,0x8b,0},
{0xb8,0x9a,0x8b,0xa8,0x96,0x91,0x9b,0x90,0x88,0xab,0x9a,0x87,0x8b,0xbe,0},
{0xac,0x9a,0x8b,0xbe,0x9c,0x8b,0x96,0x89,0x9a,0xa8,0x96,0x91,0x9b,0x90,0x88,0},
{0xb8,0x9a,0x8b,0xbe,0x9c,0x8b,0x96,0x89,0x9a,0xa8,0x96,0x91,0x9b,0x90,0x88,0},
/*"MessageBoxA",
"GetDlgItemTextA",
"GetDlgItemInt",
"GetWindowTextA",
"SetActiveWindow",
"GetActiveWindow",*/
};

unsigned *_dec_lic_buf3 = 0;

unsigned char function_names_kernal32[][50] = {
{0xba,0x87,0x96,0x8b,0xaf,0x8d,0x90,0x9c,0x9a,0x8c,0x8c,0},
{0xb6,0x8c,0xbb,0x9a,0x9d,0x8a,0x98,0x98,0x9a,0x8d,0xaf,0x8d,0x9a,0x8c,0x9a,0x91,0x8b,0},
{0xb8,0x9a,0x8b,0xa9,0x90,0x93,0x8a,0x92,0x9a,0xb6,0x91,0x99,0x90,0x8d,0x92,0x9e,0x8b,0x96,0x90,0x91,0xbe,0},
{0xa9,0x96,0x8d,0x8b,0x8a,0x9e,0x93,0xaf,0x8d,0x90,0x8b,0x9a,0x9c,0x8b,0},
{0xa9,0x96,0x8d,0x8b,0x8a,0x9e,0x93,0xaf,0x8d,0x90,0x8b,0x9a,0x9c,0x8b,0xba,0x87,0},
{0xb3,0x90,0x9e,0x9b,0xb3,0x96,0x9d,0x8d,0x9e,0x8d,0x86,0xbe,0},
{0xbc,0x8d,0x9a,0x9e,0x8b,0x9a,0xb9,0x96,0x93,0x9a,0xbe,0},
{0xb8,0x9a,0x8b,0xb2,0x90,0x9b,0x8a,0x93,0x9a,0xb7,0x9e,0x91,0x9b,0x93,0x9a,0xbe,0},
{0xb8,0x9a,0x8b,0xaf,0x8d,0x90,0x9c,0xbe,0x9b,0x9b,0x8d,0x9a,0x8c,0x8c,0},
{0xbc,0x8d,0x9a,0x9e,0x8b,0x9a,0xaf,0x8d,0x90,0x9c,0x9a,0x8c,0x8c,0xbe,0},
/*"ExitProcess",
"IsDebuggerPresent",
"GetVolumeInformationA",
"VirtualProtect",
"VirtualProtectEx",
"LoadLibraryA",
"CreateFileA",
"GetModuleHandleA",
"GetProcAddress",
"CreateProcessA",*/
};


#define GuardFuncEW()\
{                       \
   static BYTE buf[] = {0x33, 0xe4, 0xc3};\
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset GetCurrentProcess + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0xff) == (0xcc ^ 0xff))                               \
       {                                                            \
        	int s = (int)buf;                \
        	_asm {call dword ptr s}                                 \
      }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset OpenProcessToken + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0xff) == (0xcc ^ 0xff))                               \
       {                                                            \
        	int s = (int)buf;                \
        	_asm {call dword ptr s}                                 \
      }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset LookupPrivilegeValue + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0x11) == (0xcc ^ 0x11))                               \
       {                                                            \
        	int s = (int)buf;                \
        	_asm {call dword ptr s}                                 \
      }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset AdjustTokenPrivileges + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0x22) == (0xcc ^ 0x22))                               \
       {                                                            \
        	int s = (int)buf;                \
        	_asm {call dword ptr s}                                 \
       }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset ExitWindowsEx + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc^ 0x31) == (0xcc ^ 0x31))                               \
       {                                                            \
        	int s = (int)buf;                \
        	_asm {call dword ptr s}                                 \
      }                                                            \
   }                                                                \
}

#define GuardFuncVP()\
{                       \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset VirtualProtect + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc + 2) == (0xcc + 2))                               \
       {                                                            \
            GuardBye();\
      }                                                            \
   }                                                                \
}

#define GuardFuncRF2()\
{                       \
   GuardFuncVP();\
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset SetFilePointer + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0x11) == (0xcc ^ 0x11))                               \
       {                                                            \
            DWORD d;                                               \
            VirtualProtect(test_cc, 1, PAGE_EXECUTE_READWRITE, &d);\
            *test_cc = 0x55;\
      }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset ReadFile + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0x22) == (0xcc ^ 0x22))                               \
       {                                                            \
            DWORD d;                                               \
            VirtualProtect(test_cc, 1, PAGE_EXECUTE_READWRITE, &d);\
            *test_cc = 0x55;\
      }                                                            \
   }                                                                \
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset CloseHandle + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 0x33) == (0xcc ^ 0x33))                               \
       {                                                            \
            DWORD d;                                               \
            VirtualProtect(test_cc, 1, PAGE_EXECUTE_READWRITE, &d);\
            *test_cc = 0x55;\
      }                                                            \
   }                                                             \
}

#define GuardFuncCF2()\
{                       \
   GuardFuncVP();\
   {                    \
       DWORD ** offs;\
       _asm{mov offs, offset CreateFileA + 2}\
       unsigned char *test_cc = (unsigned char *)(**offs);                   \
       if((*test_cc ^ 10) == (0xcc ^ 10))                               \
       {                                                            \
            DWORD d;                                               \
            VirtualProtect(test_cc, 1, PAGE_EXECUTE_READWRITE, &d);\
            *test_cc = 0x55;\
      }                                                            \
   }                                                                \
}


unsigned *_dec_lic_buf1 = 0;

static bool fix_guard;

void get_cpu_sn(DWORD &sn1, DWORD &sn2);


void read_lic()
{
#ifdef __BCPLUSPLUS__ 
    GuardFuncRF2();
    GuardFuncCF2();    
#endif
    if(_lic_buf != 0)
        return;

    unsigned char new_f[] = GP_FIX_FNAME;
    int i;
    for(unsigned char *c = new_f, i = 0; i < sizeof(new_f); c++, i++)
        *c = *c ^ 0x8e;
    new_f[sizeof(new_f)-1] = 0;
    ifstream fs((char*)new_f, ios_base::in | ios_base::binary);
    if(!fs.is_open())
    {
        unsigned char f[] = {0x23,0x7a,0x1c,0x74,0x76,0x76,0x79,0x77,0x78,0x77,0x73,0x6e,0x33,0x39,0x33, 0x2};
        for(unsigned char *c = f, i = 0; i < sizeof(f); c++, i++)
            *c = *c ^ 0x40;
        f[sizeof(f)-1] = 0;
        fs.open((char*)f, ios_base::in | ios_base::binary);
        if(!fs.is_open())
             return;

        fix_guard = false;
    }
    else
        fix_guard = true;

    GuardFuncLL();

    fs.seekg(0, ios_base::end);
    _lic_size = fs.tellg();
    fs.seekg(0, ios_base::beg);

    _lic_buf = new char[_lic_size];

    fs.read(_lic_buf, _lic_size);
    _encrypt((_lic_buf+5*4), (_lic_buf+5*4), (_lic_size-5*4), CKEY);
}

void decrypt_lic()
{
    if(_dec_lic_buf1 != 0)
        return;

    if(_lic_buf == 0)
    {
        _dec_lic_buf1 = new unsigned[FIX_DATA_LEN];
        _dec_lic_buf2 = new unsigned[FIX_DATA_LEN];
        _dec_lic_buf3 = new unsigned[FIX_DATA_LEN];
        return;
    }

    _dec_lic_buf2 = new unsigned[FIX_DATA_LEN];

    public_key rsa;

    rsa.m.set_uints((unsigned*)_lic_buf, 4);
    rsa.e.set_uints((unsigned*)(_lic_buf + 4*4), 1);

    _dec_lic_buf1 = new unsigned[FIX_DATA_LEN];

    unsigned *cur_val = (unsigned*)(_lic_buf + 5*4);
    _dec_lic_buf3 = new unsigned[FIX_DATA_LEN];
    vlong text, res;
    int d;
    for(int i = 0; i < FIX_DATA_LEN; i++)
    {
        if(*cur_val == 0)
        {
            _dec_lic_buf1[i] = *cur_val;
            _dec_lic_buf2[i] = *cur_val;
            _dec_lic_buf3[i] = *cur_val;
            cur_val++;
            continue;
        }

        int count;

        if(fix_guard)
        {
            count = *cur_val;
            cur_val++;
        }
        else
            count = 4;

        text.set_uints(cur_val, count);
        res = rsa.encrypt(text);
        int e = res.get_count_uints();
        _dec_lic_buf1[i] = res.get_uints()[0];
        _dec_lic_buf2[i] = res.get_uints()[0];
        _dec_lic_buf3[i] = res.get_uints()[0];
        cur_val += count;
    }

    delete[] _lic_buf;
}

#ifdef __BCPLUSPLUS__ 
void load_dda_offs(int *offs1, int *offs2)
{
    unsigned char f[] = GP_DDA_OFFS_FNAME;
    int i;
	for(unsigned char *c = f, i = 0; i < sizeof(f); c++, i++)
		*c = ~*c;
    GuardFuncCF2();
	GuardFuncRF2();
	FILE *file = fopen((char*)f, "rb");

	fread(offs1, 4, 0xb0, file);

	DWORD key[4];
	get_cpu_sn(key[0], key[1]);
	key[2] = key[0];
	key[3] = key[1];
	for(int i = 0; i < 4; ++i)
		key[i] |= IDEA_XOR_MASK;


    word16 EK[IDEAKEYLEN], DK[IDEAKEYLEN];

    ideaExpandKey((byte*)key, EK);
    ideaInvertKey(EK, DK);

	for(byte *cur_p = (byte*)offs1; cur_p < (byte*)offs1 + 0xb0*4; cur_p += 8)
		ideaCipher((byte*)cur_p, (byte*)cur_p, DK);

    fread(offs2, 4, 0xb0, file);

	for(byte *cur_p = (byte*)offs2; cur_p < (byte*)offs2 + 0xb0*4; cur_p += 8)
		ideaCipher((byte*)cur_p, (byte*)cur_p, DK);

    fclose(file);
}
#endif

BOOL CALLBACK G_EnumWindowsProc1(HWND hwnd, LPARAM lParam)
{
    static char start_name[1024] = "";
    char buf[1024];

    GetWindowTextA(hwnd, buf, 1024);

    if(start_name[0] == 0)
        strcpy(start_name, buf);
    else
    if(strcmp(start_name, buf) == 0)
        return false;

    unsigned char hn[] = {0xb7,0x96,0x9a,0x88,0xc5,0xff};
    int i;
    for(unsigned char *c = hn, i = 0; i < sizeof(hn); c++, i++)
        *c = ~*c;

    if(strncmp(buf, (char*)hn, 5) == 0)
    {
        int i = GetTickCount();
        srand(i);
        if(rand() / (double)RAND_MAX > 0.8)
        {
            ShowWindow(hwnd, SW_HIDE);
        }
        _guard_hiew = true;

        return false;
    }

    return true;
}


BOOL CALLBACK G_EnumWindowsProc2(HWND hwnd, LPARAM lParam)
{
    static char start_name[1024] = "";
    char buf[1024];

    GetWindowTextA(hwnd, buf, 1024);

    if(start_name[0] == 0)
        strcpy(start_name, buf);
    else
    if(strcmp(start_name, buf) == 0)
        return false;

    unsigned char hn[] = {0xb6,0xbb,0xbe,0xdf,0xd2,0xdf};
    int i = 0;
    for(unsigned char *c = hn, i = 0; i < sizeof(hn); c++, i++)
        *c = ~*c;

    if(strncmp(buf, (char*)hn, 6) == 0)
    {
        char *end_c;
        if( ((end_c = strrchr(buf, 'e')) != 0 || (end_c = strrchr(buf, 'E')) != 0) && *(end_c+1) == 0)
        {
            int i = GetTickCount();
            srand(i);
            if(rand() / (double)RAND_MAX > 0.8)
            {
                DWORD pi;
                GetWindowThreadProcessId(hwnd, &pi);
                HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi);
                for(int i = 0; i < 1000; i++)
                    WriteProcessMemory(h, (void*)(rand() * 1000), start_name, sizeof(start_name), 0);
                //SetWindowPos(hwnd, 0, 0,0,0,0, SWP_HIDEWINDOW);
            }
            _guard_ida = true;
            return false;
       }
    }

    return true;
}

void get_cpu_sn(DWORD &sn1, DWORD &sn2)
{
    unsigned char b[] = {0xf, 0xa2, 0xc3};
    int s = (int)b;
    _asm
	{
        //.586
        mov eax, 1
        call s
        bt edx, 18
        jnc __guard_get_cpu_sn_not_psn
        mov eax, 3
        call s
		mov ebx, [sn1]
        mov [ebx], ecx
		mov ebx, [sn2]
        mov [ebx], edx
		jmp __guard_get_cpu_sn_ret

        __guard_get_cpu_sn_not_psn:
     	and eax, not 0f000c000h
		and edx, not 40100400h
		mov ebx, [sn1]
        mov [ebx], eax
		mov ebx, [sn2]
        mov [ebx], edx
__guard_get_cpu_sn_ret:
    }
}

bool cpuid(char *buf, int offs)
{
    unsigned char b[] = {0xf, 0xa2, 0xc3};
	int s = (int)b;

	_asm
	{
    //.586
		mov edi, buf
		add edi, offs
		mov eax, 1
		call s
		and eax, not 0f000c000h
		xor eax, dword ptr [edi]
		jnz cpuid_exit
		add edi, 4
        and ebx, not 01000000h
        and dword ptr [edi], not 01000000h
        xor ebx, dword ptr [edi]
		jnz cpuid_exit
		add edi, 4
		and edx, not 40100400h
		xor edx, dword ptr [edi]
		jnz cpuid_exit
	}
	return true;

cpuid_exit:

	return false;
}
