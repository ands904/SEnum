//---------------------------------------------------------------------------


#pragma hdrstop

#include "timeguard.h"

#include <windows.h>
#include <system.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define CSIDL_PROFILE 0x0028
#define CSIDL_WINDOWS 0x0024

#define decrypt_str(buf, mask)\
{                                \
    int i;                       \
    unsigned char *c;            \
	for(c = buf, i = 0; i < sizeof(buf); c++, i++) \
		*c = *c ^ mask;   \
    *(--c) = 0;     \
}

bool test_valid_time()
{
    char path[MAX_PATH];

    const int TG_MASK = 0xffffffff;

    int start_year = TG_START_YEAR ^ TG_MASK;
    int start_month = TG_START_MONTH ^ TG_MASK;
    int start_day = TG_START_DAY ^ TG_MASK;

    int end_year = TG_END_YEAR ^ TG_MASK;
    int end_month = TG_END_MONTH ^ TG_MASK;
    int end_day = TG_END_DAY ^ TG_MASK;

    char lib_name[] = {0x17,0xc,0x1,0x8,0x8,0x57,0x56,0x4a,0x0,0x8, 0x8, 0xdd};
    decrypt_str(lib_name, 0x64);
    HMODULE shell32_dll = LoadLibraryA(lib_name);

    char get_folder[] = {0x9,0x12,0x1d,0x3f,0x2e,0x9,0x2a,0x3f,0x39,0x33,0x3b,0x36,0x1c,0x35,0x36,0x3e,0x3f,0x28,0xa,0x3b,0x2e,0x32,0x1b, 0xcc};
    decrypt_str(get_folder, 0x5a);

    BOOL  (_stdcall * SHGetSpecialFolderPath_)(HWND, LPSTR, int, BOOL);                                                     \
    SHGetSpecialFolderPath_ = (BOOL  (_stdcall *)(HWND, LPSTR, int, BOOL))GetProcAddress(shell32_dll, get_folder);                 \

    if(SHGetSpecialFolderPath_(0, path, CSIDL_PROFILE, FALSE))
    {
        char ntuser_dat[] = {0x2b,0x19,0x3,0x2,0x4,0x12,0x5,0x59,0x13,0x16,0x3,0x59,0x3b,0x38,0x30, 0x31};
        decrypt_str(ntuser_dat, 0x77);
        strcat(path, ntuser_dat);

        char kernel32_dll_str[] = {0x35,0x3b,0x2c,0x30,0x3b,0x32,0x6d,0x6c,0x70,0x3a,0x32,0x32,0xfe};
        decrypt_str(kernel32_dll_str, 0x5e);
        HMODULE kernel32_dll = GetModuleHandleA(kernel32_dll_str);

        char find_first_file[] = {0x73,0x5c,0x5b,0x51,0x73,0x5c,0x47,0x46,0x41,0x73,0x5c,0x59,0x50,0x74, 0x43};
        decrypt_str(find_first_file, 0x35);
        HANDLE  (_stdcall * FindFirstFile_)(LPCTSTR, LPWIN32_FIND_DATA);
        FindFirstFile_ = (HANDLE  (_stdcall *)(LPCTSTR, LPWIN32_FIND_DATA))GetProcAddress(kernel32_dll, find_first_file);

        WIN32_FIND_DATA fd;
        HANDLE hf = FindFirstFile_(path, &fd);
        if(hf != INVALID_HANDLE_VALUE)
        {
            FILETIME cur_file_time = fd.ftLastWriteTime;

            LARGE_INTEGER time1;
            time1.LowPart = cur_file_time.dwLowDateTime;
            time1.HighPart = cur_file_time.dwHighDateTime;

            if(SHGetSpecialFolderPath_(0, path, CSIDL_WINDOWS, FALSE))
            {
                char bootstat_dat[] = {0x22,0x1c,0x11,0x11,0xa,0xd,0xa,0x1f,0xa,0x50,0x1a,0x1f,0xa,0x29};
                decrypt_str(bootstat_dat, 0x7e);
                strcat(path, bootstat_dat);

                hf = FindFirstFile_(path, &fd);
                if(hf != INVALID_HANDLE_VALUE)
                {
                    LARGE_INTEGER time2;
                    time2.LowPart = fd.ftLastWriteTime.dwLowDateTime;
                    time2.HighPart = fd.ftLastWriteTime.dwHighDateTime;

                    __int64 diff = time1.QuadPart - time2.QuadPart;

                    if(diff < 0)//перевод часов назад
                        cur_file_time =  fd.ftLastWriteTime;
                    /*{
                       WORD buf[] = {0xe431, 0x310f, 0xe0ff};
                       void  (_stdcall * ex)() = (void  (_stdcall * )())&buf[0];
                       ex();
                    }*/
                }

                SHGetSpecialFolderPath_(0, path, CSIDL_WINDOWS, FALSE);
                char EventSystem_log[] = {0x2, 0x1b,0x28,0x3b,0x30,0x2a,0xd,0x27,0x2d,0x2a,0x3b,0x33,0x70,0x32,0x31,0x39,0xa};
                decrypt_str(EventSystem_log, 0x5e);
                strcat(path, EventSystem_log);

                hf = FindFirstFile_(path, &fd);
                if(hf != INVALID_HANDLE_VALUE)
                {
                    LARGE_INTEGER time2;
                    time2.LowPart = fd.ftLastWriteTime.dwLowDateTime;
                    time2.HighPart = fd.ftLastWriteTime.dwHighDateTime;

                    __int64 diff = time1.QuadPart - time2.QuadPart;

                    if(diff < 0)//перевод часов назад
                        cur_file_time =  fd.ftLastWriteTime;
                }

                SHGetSpecialFolderPath_(0, path, CSIDL_WINDOWS, FALSE);
                char win_ini[] = {0x49,0x62,0x7c,0x7b,0x3b,0x7c,0x7b,0x7c,0x39};
                decrypt_str(win_ini, 0x15);
                strcat(path, win_ini);

                hf = FindFirstFile_(path, &fd);
                if(hf != INVALID_HANDLE_VALUE)
                {
                    LARGE_INTEGER time2;
                    time2.LowPart = fd.ftLastWriteTime.dwLowDateTime;
                    time2.HighPart = fd.ftLastWriteTime.dwHighDateTime;

                    __int64 diff = time1.QuadPart - time2.QuadPart;

                    if(diff < 0)//перевод часов назад
                        cur_file_time =  fd.ftLastWriteTime;
                }
            }

            char FileTimeToSystemTime_str[] = {0x21,0xe,0xb,0x2,0x33,0xe,0xa,0x2,0x33,0x8,0x34,0x1e,0x14,0x13,0x2,0xa,0x33,0xe,0xa,0x2,0x19};
            decrypt_str(FileTimeToSystemTime_str, 0x67);
            BOOL  (_stdcall * FileTimeToSystemTime_)(const FILETIME*, LPSYSTEMTIME);
            FileTimeToSystemTime_ = (BOOL  (_stdcall *)(const FILETIME*, LPSYSTEMTIME))GetProcAddress(kernel32_dll, FileTimeToSystemTime_str);

            SYSTEMTIME sys_time;
            FileTimeToSystemTime_(&cur_file_time, &sys_time);
            TDateTime cur_date(sys_time.wYear, sys_time.wMonth, sys_time.wDay);

            TDateTime start_date(start_year ^ TG_MASK, start_month ^ TG_MASK, start_day ^ TG_MASK);
            TDateTime end_date(end_year ^ TG_MASK, end_month ^ TG_MASK, end_day ^ TG_MASK);

            if(cur_date < start_date || cur_date > end_date)
                return false;
        }
    }

    return true;
}

static
string get_formated_date(string &line)
{
    string::iterator start;
    if((start = find(line.begin(), line.end(), '/')) != line.end())
    {
        string::iterator end;
        if((end = find(start, line.end(), ' ')) != line.end())
        {
            *end = 0;
            while(start != line.begin() && *start != ' ')
                --start;

           return &(*start);
        }
    }

    return string();
}

static
bool get_date_form_string(string &date, TDateTime &date_time)
{
    string::iterator it;
    string::iterator start = date.begin();
    if((it = find(date.begin(), date.end(), '/')) != date.end())
    {
        *it = 0;
        char *endptr;
        int month = strtol(&(*start), &endptr, 10);
        start = ++it;

        if((it = find(it, date.end(), '/')) != date.end())
        {
            *it = 0;
            int day = strtol(&(*start), &endptr, 10);
            start = ++it;

            int year = strtol(&(*start), &endptr, 10);

            date_time = TDateTime(year, month, day);

            return true;
        }
    }

    return false;
}

bool test_network_time()
{
    SECURITY_ATTRIBUTES sa;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	ecode;

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = true;

    char temp_filename[MAX_PATH];
    const size_t size_buf = sizeof(temp_filename)/sizeof(temp_filename[0]);
    ::GetTempPath(size_buf, temp_filename);
    ::GetTempFileName(temp_filename, "", 0, temp_filename);

    HANDLE hf = CreateFileA(temp_filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
	if(hf == INVALID_HANDLE_VALUE)
        return true;

	ZeroMemory(&si,  sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hf;
	si.wShowWindow = SW_HIDE;

    char net_view[] = {0x8f,0x84,0x95,0xc1,0x97,0x88,0x84,0x96,0x1c};
    decrypt_str(net_view, 0xe1);

    if(CreateProcessA(0, net_view ,0,0,true,0,0,0,&si,&pi))
	{
		CloseHandle(pi.hThread);
		WaitForSingleObject(pi.hProcess,INFINITE);
		GetExitCodeProcess(pi.hProcess ,&ecode);
		CloseHandle(pi.hProcess);

        vector<string> comps;

        ifstream ifs(temp_filename);

        string line;
    	while(getline(ifs, line))
	    {
            if(line.size() > 2 && line[0] == '\\' && line[1] == '\\')
            {
                for(string::iterator it = line.begin(); it != line.end(); ++it)
                {
                    if(*it == '\t' || *it == ' ')
                        *it = 0;
                }

                comps.push_back(&line[0]);
            }
        }
        ifs.close();
        CloseHandle(hf);

        hf = CreateFileA(temp_filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
    	if(hf == INVALID_HANDLE_VALUE)
            return true;

        int count = 0;
        for(vector<string>::iterator it = comps.begin(); it != comps.end(); ++it)
        {
            char net_time[] = {0x10,0x1b,0xa,0x5e,0xa,0x17,0x13,0x1b,0x5e, 0x44};
            decrypt_str(net_time, 0x7e);

            string str = net_time;
            str += *it;

            if(CreateProcessA(0, (char *)str.c_str() ,0,0,true,0,0,0,&si,&pi))
            {
                CloseHandle(pi.hThread);
                WaitForSingleObject(pi.hProcess,INFINITE);
                GetExitCodeProcess(pi.hProcess ,&ecode);
                CloseHandle(pi.hProcess);
                ++count;

                if(count >= 10)
                    break;
            }
        }
        CloseHandle(hf);

        if(count)
        {
            vector<string> comps_date;

            ifstream ifs(temp_filename);

            string line;
            while(getline(ifs, line))
            {
                string date = get_formated_date(line);
                if(date.size() > 0)
                    comps_date.push_back(date);
            }
            ifs.close();

            hf = CreateFileA(temp_filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
            if(hf == INVALID_HANDLE_VALUE)
                return true;

            char net_time[] = {0x10,0x1b,0xa,0x5e,0xa,0x17,0x13,0x1b,0x5e, 0x44};
            decrypt_str(net_time, 0x7e);

            char localhost[] = {0x60,0x60,0xd,0xe,0xb,0x12,0xc,0x12,0xc,0x12,0xd, 0x33};
            decrypt_str(localhost, 0x3c);

            string str = net_time;
            str += localhost;

            if(CreateProcessA(0, (char *)str.c_str() ,0,0,true,0,0,0,&si,&pi))
            {
                CloseHandle(pi.hThread);
                WaitForSingleObject(pi.hProcess,INFINITE);
                GetExitCodeProcess(pi.hProcess ,&ecode);
                CloseHandle(pi.hProcess);

                ifstream ifs(temp_filename);

                string line;
                string cur_date;
                while(getline(ifs, line))
                {
                    const int TG_MASK = 0x2e57;

                    int start_year = TG_START_YEAR ^ TG_MASK;
                    int start_month = TG_START_MONTH ^ TG_MASK;
                    int start_day = TG_START_DAY ^ TG_MASK;

                    int end_year = TG_END_YEAR ^ TG_MASK;
                    int end_month = TG_END_MONTH ^ TG_MASK;
                    int end_day = TG_END_DAY ^ TG_MASK;

                    TDateTime real_date;

                    TDateTime start_date(start_year ^ TG_MASK, start_month ^ TG_MASK, start_day ^ TG_MASK);
                    TDateTime end_date(end_year ^ TG_MASK, end_month ^ TG_MASK, end_day ^ TG_MASK);

                    cur_date = get_formated_date(line);
                    if(cur_date.size() > 0)
                    {
                        string network_date;
                        string network_date_2;
                        for(vector<string>::iterator it = comps_date.begin(); it != comps_date.end(); ++it)
                        {
                            if(*it != cur_date)
                            {
                                if(*it == network_date)
                                {
                                    if(*it == network_date_2)
                                    {
                                        if(get_date_form_string(network_date, real_date))
                                        {
                                            if(real_date < start_date || real_date > end_date)
                                            {
                                                CloseHandle(hf);
                                                DeleteFile(temp_filename);
                                                return false;
                                            }
                                        }
                                    }
                                    network_date_2 = *it;
                                }
                                network_date = *it;
                            }
                        }

                        if(get_date_form_string(cur_date, real_date))
                        {
                            if(real_date < start_date || real_date > end_date)
                            {
                                CloseHandle(hf);
                                DeleteFile(temp_filename);
                                return false;
                            }
                        }

                        break;
                    }
                }
            }
        }
    }

    CloseHandle(hf);
    DeleteFile(temp_filename);

    return true;
}
