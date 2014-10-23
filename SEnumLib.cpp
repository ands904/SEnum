//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <ComObj.hpp>
#include <Clipbrd.hpp>
#include <Registry.hpp>

#include "WatchPrintSaveAsUnit.h"
#include "ForCanvasUnit.h"

#include "SEnumMainUnit.h"
#include "SEnumLib.h"
#include "SEnumLib1.h"
#include "ContentsUnit.h"
#include "TPDFCreator.h"


struct TSubscribeParams {           // ��������� ��� ����������
    TTextX Post;                    // ������� ������ ��������� ����������
    int PostCol;                    // ����� �������, � ������� ������� ��������� ���������
    String PostName;                // ������������ ��������� ����������
    TTextX Name;                    // ������� ������ ������� ����������, ���� ����� ��������, �� �����
    int NameCol;                    // ����� �������, � ������� ������� ������� ����������
    String FamFiln;                 // ��� ����� ������� ����������
};



bool NeedBreakProcess = false;

HANDLE hhh;

int stop = 0;
String sstop;

char OldCurPrinter[100] = "";           // ��� ������ �������
char *TiffCurPrinter = NULL;             // ��� ������� ��� ������ � tiff


//===============================================================================
//                                  ����������                                  |
//===============================================================================
unsigned char __fastcall Capitalize(unsigned char c) {
//-------------------------------------------------------------------------------
//                              � �.�. � ������� �����                          |
//-------------------------------------------------------------------------------
    if (c >= 'a' && c <= 'z') {c -= 'a'; c += 'A';}
    else if (c >= (unsigned char)'�') {c -= '�'; c += '�';}
    return c;
}

char * __fastcall SkipSpaces(char *p) {
//-------------------------------------------------------------------------------
//                  ��������� ��������� �� ������ �� ������                     |
//-------------------------------------------------------------------------------
    while(*p == ' ') p++;
    return p;
}

int __fastcall NospaceStrcmp(char *p1, char *p2, bool IgnoreCase) {
//-------------------------------------------------------------------------------
//                  ���������� ��� ������ � �������������� ��������             |
//-------------------------------------------------------------------------------
    int res;
    unsigned char c1, c2;
    while(1) {
        p1 = SkipSpaces(p1); c1 = *p1++;
        p2 = SkipSpaces(p2); c2 = *p2++;
        if (IgnoreCase) {c1 = Capitalize(c1); c2 = Capitalize(c2);}
        if (c1 < c2) {res = -1; break;}
        if (c1 > c2) {res = 1; break;}
        res = 0;
        stop = min(c1, c2);
        if (c1 == 0) break;
    }
    return res;
}


AnsiString __fastcall EraseFilnFromPath(AnsiString FullFiln) {
//-------------------------------------------------------------------------------
//              ���������� ��� �����, �������� ������ ���� � '\\'               |
//-------------------------------------------------------------------------------
    String res;
    int dist;
    char *p, *q;

    q = FullFiln.c_str(); p = q + FullFiln.Length() - 1;
    while(p >= q) {
        if (*p == '\\' || *p == ':') {
            p++; break;
        }
        p--;
    }
    dist = p - q;
    res = FullFiln.SubString(1, dist);
    return res;
}

AnsiString __fastcall ErasePathFromFiln(AnsiString FullFiln) {
//-------------------------------------------------------------------------------
//                      ���������� ����, �������� ������ ��� �����              |
//-------------------------------------------------------------------------------
    char *p, *q;
    q = FullFiln.c_str(); p = q + FullFiln.Length() - 1;
    while(p >= q) {
        if (*p == '\\' || *p == ':') {
            p++; break;
        }
        p--;
    }
    return p;
}


String __fastcall TrimExt(String Filn) {
//-------------------------------------------------------------------------------
//                  ���������� ��� ����� � ���������� �����������               |
//-------------------------------------------------------------------------------
    char *p, *q;
    if (Filn.IsEmpty()) return Filn;
    p = Filn.c_str(); q = p + Filn.Length();
    while(q > p) {
        if (*q == '.') {
            return Filn.SubString(1, q - p);
        }
        q--;
    }
    return Filn;
}


String __fastcall ExtractExt(String Filn) {
//-------------------------------------------------------------------------------
//                      ��������� �� ����� ����� ���� ����������                |
//-------------------------------------------------------------------------------
    char *p, *q;
    if (Filn.IsEmpty()) return "";
    p = Filn.c_str(); q = p + Filn.Length();
    while(q > p) {
        if (*q == '.') {
            return q + 1;
        }
        q--;
    }
    return "";
}


char * __fastcall LoadFile(char *filn, int &filen) {
//-------------------------------------------------------------------------------
//                          ��������� ���� � ����� � ������                     |
// ���� ��� ���������, �� ���������� ��������� �� ����� � ������, ������� ����� |
//   ���� ���������� �� delete[], ��� ���� � filen - ����� �����, ����� �����   |
//   ����������� ����� � ������ �������� 0                                      |
// ���� ������, �� ���������� NULL, ��� ����                                    |
//   filen = 1 - ���� �� ������                                                 |
//           2 - ���� ����, �� ����������� �� ����� (����������?)               |
//           3 - ������ ������                                                  |
//-------------------------------------------------------------------------------
    HANDLE chan;
    DWORD dummy;
    int res;
    char *buf;

    chan = CreateFile(filn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) {
        res = GetFileAttributes(filn);
        if (res == 0xFFFFFFFF) {                // ���� �� ������
            filen = 1;
        } else {                                // ���� ������, �� �� ����� �����������
            filen = 2;
        }
        return NULL;
    }
    filen = GetFileSize(chan, 0);
    buf = new char[filen + 1];
    res = ReadFile(chan, buf, filen, &dummy, 0);
    CloseHandle(chan);
    if (res == 0 || filen != (int)dummy) {
        delete[] buf;
        filen = 3;
        return NULL;
    }
    return buf;
}

int __fastcall SaveFile(bool automessage, char *name, char *file, int filen) {
//-------------------------------------------------------------------------------
//                  ��������� ���� � ��������������� � .bak                     |
// ���� ��� ���������, ���������� 0, ���� ������ - ���������� ��� ������:       |
// ���� automessage, �� � ������ ��������� �� ������                            |
//    1 - �� ���� ������� ��������� ����                                        |
//    2 - ������ ������                                                         |
//    3 - ��� �� ������� �����                                                  |
//    4 - �� ���� ������� .bak                                                  |
//    5 - �� ���� ������������� ������� ��� � bak                               |
//    6 - �� ���� ������������� tmp � name                                      |
// ������� ������� TrimExt                                                      |
//-------------------------------------------------------------------------------
    String s, tmp, bak;
    DWORD dummy;
    HANDLE chan;
    BOOL res;

    s = TrimExt(name);
    tmp = s; tmp += String(".tmp");
    bak = s; bak += String(".bak");

    chan = CreateFileA(tmp.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) {
        if (automessage) {
            s = (String)"�� ���� ������� ��������� ���� \""; s += tmp; s += "\"";
            ::MessageBoxA(NULL, s.c_str(), "������!", 0);
        }
        return 1;
    }

    res = WriteFile(chan, file, filen, &dummy, 0);
    CloseHandle(chan);
    if (res == 0) {
        DeleteFileA(tmp);
        if (automessage) {
            s = (String)"������ ������ ���������� ����� \""; s += tmp; s += "\"";
            ::MessageBoxA(NULL, s.c_str(), "������!", 0);
        }
        return 2;
    }
    if ((int)dummy != filen) {
        DeleteFileA(tmp);
        if (automessage) {
            s = (String)"������ ������ ���������� ����� \""; s += tmp; s += "\" (�������� �� ����� �� ������� �����)";
            ::MessageBoxA(NULL, s.c_str(), "������!", 0);
        }
        return 3;
    }

    res = GetFileAttributesA(bak.c_str());
    if (res != 0xFFFFFFFF) {
        res = DeleteFileA(bak);
        if (res == 0) {
            if (automessage) {
                s = (String)"�� ���� ���������� ����������� ������ ����� \""; s += bak; s += "\"";
                ::MessageBoxA(NULL, s.c_str(), "������!", 0);
           }
           return 4;
        }
    }

    res = GetFileAttributesA(name);
    if (res != 0xFFFFFFFF) {
        res = MoveFileA(name, bak.c_str());
        if (res == 0) {
            if (automessage) {
                s = (String)"�� ���� ������������� ���������� ������ ����� \""; s += name; s += "\" � \""; s += bak; s += "\"";
                ::MessageBoxA(NULL, s.c_str(), "������!", 0);
           }
           return 5;
        }
    }

    res = MoveFileA(tmp.c_str(), name);
    if (res == 0) {
        if (automessage) {
            s = (String)"�� ���� ������������� ��������� ���� \""; s += tmp; s += "\" � \""; s += name; s += "\"";
                s += "\n����������� ���������� ������ ��������� � ����� \""; s += name; s += "\"";
                s += "\n���������� ��� �� �����-������ �������� �������� � ������������ ������ ������� �������";
            ::MessageBoxA(NULL, s.c_str(), "������!", 0);
       }
       return 6;
    }
    return 0;
}


TPoint __fastcall CellFromAddress(String Address) {
//-------------------------------------------------------------------------------
//          ��������� ����� ������ �� ������ ���� $<�����>$<�����>              |
// ���� ����� ���� ��������, �� ����� ������ ������� ���������                  |
// ��������� ���������� �� 1!                                                   |
//-------------------------------------------------------------------------------
    TPoint res;
    String nn;
    char *p = Address.c_str();
    while(1) {
        if (*p == 0) {
            p = Address.c_str();
            break;
        }
        if (*p == ':') {
            p++;
            break;
        }
        p++;
    }
    res.x = p[1] - 'A' + 1;
    nn = p + 3;
    res.y = nn.ToInt();
    return res;
}

String __fastcall AddressFromCell(int row, int col) {
//-------------------------------------------------------------------------------
//                      ��������� ����� ������ �� �� ������                     |
//-------------------------------------------------------------------------------
    char buf[50], acol;
    acol = col - 1 + 'A';
    wsprintf(buf, "$%c$%d", acol, row);
    return (String)buf;
}

void AnsiToUnicode(char *ansi, int ansi_len, wchar_t *unicode) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if(ansi_len < 0)
        ansi_len = strlen(ansi) + 1;

    int i;
    unsigned char c;
    for(i = 0; i < ansi_len; i++) {
        c = ansi[i];
        if(c >= 0xc0)
            unicode[i] = c - 0xc0 + 0x410;
        else
        if(c == 0xa8)
            unicode[i] = 0x401;
        else
        if(c == 0xb8)
            unicode[i] = 0x451;
        else
        if(c == 0xb9)
            unicode[i] = 0x2116;
        else
            unicode[i] = c;

    }
}


void __fastcall UnicodeToAnsi(wchar_t *unicode, char *ansi) {
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
        // wchar_t *unicode = EditString;
        int i, unicode_len = wcslen(unicode) + 1;
        unsigned short c;
        for(i = 0; i < unicode_len; i++)
        {
                c = unicode[i];
                if(c >= 0x410 && c <= 0x44f)
                        ansi[i] = (char)(c - 0x410 + 0xc0);
                else
                if(c == 0x401)
                        ansi[i] = (char)0xa8;
                else
                if(c == 0x451)
                        ansi[i] = (char)0xb8;
                else
                if(c == 0x2116)
                        ansi[i] = (char)0xb9;
                else
                        ansi[i] = (char)c;

        }
        //ansi[i] = 0;
}




void __fastcall FillWinInfo(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//                  ��������� wi ����������� �� ���� wi->Handle                 |
//-------------------------------------------------------------------------------
    TPoint pt;
    char *class_name;
    ::GetWindowRect(wi->Handle, &wi->Rect);
    class_name = new char[10000];
    ::GetClassName(wi->Handle, class_name, 10000);
    wi->ClassName = (String)class_name;
    ::GetWindowText(wi->Handle, class_name, 10000);
    wi->WindowText = (String)class_name;
    delete[] class_name;
    ::GetClientRect(wi->Handle, &wi->CRect);
    pt.x = 0; pt.y = 0;
    ::ClientToScreen(wi->Handle, &pt);
    wi->CRect.left += pt.x; wi->CRect.top += pt.y;
    wi->CRect.right += pt.x; wi->CRect.bottom += pt.y;
}


void __fastcall GetParentWinInfo(TWinInfo *wi, HWND h) {
//-------------------------------------------------------------------------------
//         ��������� ��������� wi ����������� � ����� ������� �������� h        |
//-------------------------------------------------------------------------------
    HWND hh;
    hh = h;
    while(1) {
        hh = (HWND)GetWindowLong(h, GWL_HWNDPARENT);
        if (hh == NULL) break;
        h = hh;
    }
    wi->Handle = h;
    FillWinInfo(wi);
}


int GetWinInfoDelay = 0;

void __fastcall GetWinInfo(TWinInfo *wi, TPoint *pt) {
//-------------------------------------------------------------------------------
//  ��������� ��������� wi ����������� �� ����, ������������� � pt (��� �����)  |
//-------------------------------------------------------------------------------
    // char *class_name;
    wi->Handle = ::WindowFromPoint(*pt);
    FillWinInfo(wi);
    if (GetWinInfoDelay != 0) {
        ::SetCursorPos(pt->x, pt->y);
        aWait(GetWinInfoDelay);
    }
}

void __fastcall GetWinInfo(TWinInfo *wi, int x0, int y0) {
//-------------------------------------------------------------------------------
//    ��������� ��������� wi ����������� �� ����, �������� � x0,y0(��� �����)   |
//-------------------------------------------------------------------------------
    TPoint pt;
    pt.x = x0; pt.y = y0;
    GetWinInfo(wi, &pt);
}

void __fastcall GetWinInfo(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//               ��������� ��������� wi ����������� �� ���� ��� ��������        |
//-------------------------------------------------------------------------------
    TPoint pt;
    ::GetCursorPos(&pt);
    GetWinInfo(wi, &pt);
}

void __fastcall aWait(int milliseconds) {
//---------------------------------------------------------------------------
//         ������� ��������� ���������� ����������� � ProcessMessages       |
//---------------------------------------------------------------------------
    DWORD t0;
    t0 = GetTickCount();
    while(1) {
        Application->ProcessMessages();
        if ((int)GetTickCount() - (int)t0 > milliseconds) break;
    }
}

void __fastcall SuperDoubleClick(TPoint *pt) {
//---------------------------------------------------------------------------
//       ��������� ����� DoubleClick - ���� �������, �� ������ ������       |
//---------------------------------------------------------------------------
    ::SetCursorPos(pt->x, pt->y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);
}

void __fastcall Text2Focused(char *text) {
//-------------------------------------------------------------------------------
//                      ���������� ����� � ���� � ������� �����                 |
//-------------------------------------------------------------------------------
    TClipboard *c;
    HANDLE hmem;
    wchar_t *u1;
    int len = strlen(text);
    c = Clipboard();
    c->Open();
    c->Clear();
    u1 = new wchar_t[len + 2];
    AnsiToUnicode(text, -1, u1);
    c->SetTextBuf((char *)u1);
    hmem=GlobalAlloc(GMEM_FIXED, len * 2 + 2);
    memmove(hmem, u1, len * 2 + 2);
    SetClipboardData(CF_UNICODETEXT,hmem);
    c->Close();
    delete[] u1;
    // for (int i = 0; i < 10; i++) {
        keybd_event(VK_CONTROL, 0, 0, 0); aWait(20);                // Ctrl+V
        keybd_event('V', 0, 0, 0); aWait(20);
        keybd_event('V', 0, KEYEVENTF_KEYUP, 0); aWait(20);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

        // keybd_event(VK_CONTROL, 0, 0, 0); aWait(20);                // Ctrl+Home
    // }
}

void __fastcall Text2Focused(HWND handle, char *text) {
//-------------------------------------------------------------------------------
//                      ���������� ����� � ���� � ������� �����                 |
//-------------------------------------------------------------------------------
    TClipboard *c;
    HANDLE hmem;
    wchar_t *u1;
    int len = strlen(text);
    c = Clipboard();
    c->Open();
    c->Clear();
    u1 = new wchar_t[len + 2];
    AnsiToUnicode(text, -1, u1);
    c->SetTextBuf((char *)u1);
    hmem=GlobalAlloc(GMEM_FIXED, len * 2 + 2);
    memmove(hmem, u1, len * 2 + 2);
    SetClipboardData(CF_UNICODETEXT,hmem);
    c->Close();
    delete[] u1;
    ::SendMessage(handle, WM_PASTE, 0, 0);
/*
    for (int i = 0; i < 5; i++) {
        TWinInfo ww;
        ww.Handle = handle;
        ::SendMessage(handle, WM_PASTE, 0, 0);
        aWait(50);
        GetWinInfo(&ww);
        if (strcmp(ww.WindowText.c_str(), text) == 0) return;
        stop++; 
    }
*/    
}

bool __fastcall IsSubstr(char *buf, char *substr, bool IgnoreCase) {
//-------------------------------------------------------------------------------
//                      ���� ������ substr ���� ��������� buf                   |
//-------------------------------------------------------------------------------
    char b, s;
    while(1) {
        b = *buf++; s = *substr++;
        if (s == 0) return true;
        if (b == 0) return false;
        if (IgnoreCase) {b = Capitalize(b); s = Capitalize(s);}
        if (b != s) break;
    }
    return false;
}


int __fastcall StartAcrobat(PROCESS_INFORMATION &pi, String Filn) {
//-------------------------------------------------------------------------------
//                      ��������� ������� � ��������� ������ �����              |
// ���� Filn.IsEmpty() - �� ������ ��������� �������                            |
// �� ������:                                                                   |
//    0 - ���������                                                             |
//    1 - �������� � �������� ��������� Adobe Acrobat                           |
//-------------------------------------------------------------------------------
//    static char acrobat[] = "C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe";
//    stop = spawnl(P_NOWAIT, acrobat, acrobat, "C:\\UnitedProjects\\������������������ PDF\\RUSA-LNY-01-TD-61600.pdf", NULL);
//    stop++;

/*
BOOL CreateProcess(
    LPCTSTR lpApplicationName,	// pointer to name of executable module
    LPTSTR lpCommandLine,	// pointer to command line string
    LPSECURITY_ATTRIBUTES lpProcessAttributes,	// pointer to process security attributes
    LPSECURITY_ATTRIBUTES lpThreadAttributes,	// pointer to thread security attributes
    BOOL bInheritHandles,	// handle inheritance flag
    DWORD dwCreationFlags,	// creation flags
    LPVOID lpEnvironment,	// pointer to new environment block
    LPCTSTR lpCurrentDirectory,	// pointer to current directory name
    LPSTARTUPINFO lpStartupInfo,	// pointer to STARTUPINFO
    LPPROCESS_INFORMATION lpProcessInformation 	// pointer to PROCESS_INFORMATION
   );
*/

    STARTUPINFO si;
    TWinInfo wi;
    String command;
    static String Acrobat = "C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe";
    String Path = EraseFilnFromPath(Filn);
    int res;

    // ���� � ����� ����������� ��� ���� - ������������ �� ����
    while(1) {
        GetWinInfo(&wi, 10, 5);
        GetParentWinInfo(&wi, wi.Handle);
        if (strcmp(wi.ClassName.c_str(), "AdobeAcrobat") != 0) break;
        ::ShowWindow(wi.Handle, SW_HIDE);
        aWait(20);
    }

    // ��������� ���������
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwX = 0; si.dwY = 0;
    si.dwXSize = 800; si.dwYSize = 800;
    si.wShowWindow = SW_RESTORE; // SW_SHOWNORMAL; // SW_SHOW;
    si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE | STARTF_USESHOWWINDOW;
    if (Filn.IsEmpty()) {
        command = (String)"\"" + Acrobat + (String)"\" \"\"";
    } else {
        command = (String)"\"" + Acrobat + (String)"\" \"" + Filn + (String)"\"";
    }
    res = CreateProcess(
        NULL, // "\"C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe\"",
        // "\"C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe\"",
        command.c_str(),
        NULL,                                                   //security
        NULL,                                                   // security
        FALSE,                                                  //inherits handles
        0,                                                      // dwCreationFlags
        0,                                                      // lpEnvironment
        Filn.IsEmpty() ? NULL : Path.c_str(),                   // pCurrentDirectory
        // 0,
        &si,
        &pi
    );
    if (res == 0) return 1;                                     // �� ��������� Adobe Acrobat
    res = WaitForInputIdle(pi.hProcess, 100000);
    if (res != 0) return 1;

    // ��������, ������� �� ���������
    // char wt[300];
    // aWait(500);
    GetWinInfo(&wi, 10, 5);
    GetParentWinInfo(&wi, wi.Handle);
    if (strcmp(wi.ClassName.c_str(), "AdobeAcrobat") != 0) return 1;
    // stop = ::GetWindowText(wi.Handle, wt, 300);
    stop++;

    return 0;                                                   // ��� ����������� �������
}


bool __fastcall TestAcrobatStandartPresence(void) {
//-------------------------------------------------------------------------------
//            ���� ���������� - ���������� true, ����� ���������� false         |
//-------------------------------------------------------------------------------
    Variant Ocrobat;
    HANDLE hProcess;
    PROCESS_INFORMATION pi;
    int started, rest;
    bool found;

    started = StartAcrobat(pi, "");
    hProcess = pi.hProcess;
    if (started != 0 && hProcess == NULL) return false;
    found = true;
    try {
        Ocrobat = CreateOleObject("AcroExch.App");
    } catch (...) {
        found = false;
    }
    rest = found ? Ocrobat.OleFunction("GetNumAVDocs") : 0;
    if (rest == 0) {
        if (found) {
            Ocrobat.OleFunction("Exit");
            Ocrobat = Unassigned;
        }
        TerminateProcess(hProcess , 0);

        FILETIME lpCreationTime;	// when the process was created
        FILETIME lpExitTime;	    // when the process exited
        FILETIME lpKernelTime;	    // time the process has spent in kernel mode
        FILETIME lpUserTime; 	    // time the process has spent in user mode
        int i;
        for (i = 0; i < 10; i++) {
            aWait(50);
            GetProcessTimes(hProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
            if (lpExitTime.dwLowDateTime != 0) break;
        }
    }

    return found;
}


bool __fastcall TestExcelLoaded(void) {
//-------------------------------------------------------------------------------
//          ���������, ������ �� excel � ���� �� � ��� ����������� ����         |
// ���� �� - ���������� true, ���� ��� - false                                  |
//-------------------------------------------------------------------------------
    Variant Excel;
    int cnt;
    bool found;

    found = true;
    try {
        Excel = GetActiveOleObject("Excel.Application");
    } catch (...) {
        found = false;
    }
    Excel = Unassigned;
    return found;
}

bool __fastcall TestWordLoaded(void) {
//-------------------------------------------------------------------------------
//           ���������, ������ �� word � ���� �� � ��� ����������� ����         |
// ���� �� - ���������� true, ���� ��� - false                                  |
//-------------------------------------------------------------------------------
    Variant Word;
    int cnt;
    bool found;

    found = true;
    try {
        // Word = GetActiveOleObject("Word.Application");
        Word = GetActiveOleObject("Word.Application.11");
    } catch (...) {
        found = false;
    }
    Word = Unassigned;
    return found;
}


bool __fastcall TestFilesLoaded(TFileList *fl, bool TestWord, bool TestExcel) {
//-------------------------------------------------------------------------------
//      ���������, ��� �� ����������� � ���� ��� ������ ������ �� ������ fl     |
// ���� ����, �� ������ ��������� � ����������� ������� ���� � ���������� true  |
//-------------------------------------------------------------------------------
    Variant Word, Docs, Doc, Excel, Workbooks, Workbook;
    String Name, FullName, mes;
    TFileListRec *r;
    int i, j, n;

    stop++;
    if (TestWord && TestWordLoaded()) {
        // Word = GetActiveOleObject("Word.Application");
        Word = GetActiveOleObject("Word.Application.11");
        Docs = Word.OlePropertyGet("Documents");
        n = Docs.OlePropertyGet("Count");
        for (i = 1; i <= n; i++) {
            Doc = Docs.OleFunction("Item", i);
            Name = Doc.OlePropertyGet("Name");
            for (j = 0; j < fl->Nrecs; j++) {
                r = fl->Rec1(j);
                if (NospaceStrcmp(r->filn.c_str(), Name.c_str(), true) == 0) {
                    FullName = Doc.OlePropertyGet("FullName");
                    Doc = Unassigned;
                    Docs = Unassigned;
                    Word = Unassigned;
                    mes = "�������� ���� \"" + FullName + "\"";
                    Application->MessageBox(mes.c_str(), "�� ���� ���������� ������!", 0);
                    return true;
                }
            }
        }
    }

    if (TestExcel && TestExcelLoaded()) {
        Excel = GetActiveOleObject("Excel.Application");
        Workbooks = Excel.OlePropertyGet("Workbooks");
        n = Workbooks.OlePropertyGet("Count");
        for (i = 1; i <= n; i++) {
            Workbook = Workbooks.OlePropertyGet("Item", i);
            Name = Workbook.OlePropertyGet("Name");
            for (j = 0; j < fl->Nrecs; j++) {
                r = fl->Rec1(j);
                if (NospaceStrcmp(r->filn.c_str(), Name.c_str(), true) == 0) {
                    FullName = Workbook.OlePropertyGet("FullName");
                    Workbook = Unassigned;
                    Workbooks = Unassigned;
                    Excel = Unassigned;
                    mes = "�������� ���� \"" + FullName + "\"";
                    Application->MessageBox(mes.c_str(), "�� ���� ���������� ������!", 0);
                    return true;
                }
            }
        }
    }


    return false; 
}

bool __fastcall TestFileLoaded(String FullFiln) {
//-------------------------------------------------------------------------------
//                  ���������, �� �� �������� �� � ���� ��������� ����          |
// ���� ����, �� ������ ��������� � ����������� ������� ���� � ���������� true  |
//-------------------------------------------------------------------------------
    Variant Word, Docs, Doc;
    String Name, FullName, mes;
    int i, n;

    stop++;
    if (TestWordLoaded()) {
        // Word = GetActiveOleObject("Word.Application");
        Word = GetActiveOleObject("Word.Application.11");
        Docs = Word.OlePropertyGet("Documents");
        n = Docs.OlePropertyGet("Count");
        for (i = 1; i <= n; i++) {
            Doc = Docs.OleFunction("Item", i);
            FullName = Doc.OlePropertyGet("FullName");
            if (NospaceStrcmp(FullFiln.c_str(), FullName.c_str(), true) == 0) {
                Doc = Unassigned;
                Docs = Unassigned;
                Word = Unassigned;
                mes = "�������� ���� \"" + FullName + "\"";
                Application->MessageBox(mes.c_str(), "�� ���� ���������� ������!", 0);
                return true;
            }
        }
    }

    return false;
}

int round(double v) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    return v < 0 ? v - 0.5 : v + 0.5;
}

void __fastcall aGetTempPath(DWORD nBufferLength, LPTSTR lpBuffer) {
//-------------------------------------------------------------------------------
//                 ��������, ��� ����� � ������ �������� ����������             |
//-------------------------------------------------------------------------------
    ::GetTempPath(nBufferLength, lpBuffer);
    // strcpy(lpBuffer, "d:\\For\\PDF\\Creator\\");

    // String apath = EraseFilnFromPath(Application->ExeName);
    // strcpy(lpBuffer, apath.c_str());

    //int last = apath.Length() - 1;
    //if (lpBuffer[last] == '\\') lpBuffer[last] = 0;
}












//===============================================================================
//                                  ������������                                |
// char ConfId[] = "SEnumConf 1.0";                                             |
// char 1, TRect pos                                                            |
// char 2, int len, char GridState[]                                            |
// char 3, char path[]                                                          |
// char 0xFF                                                                    |
//===============================================================================
static char ConfId0[] = "SEnumConf 1.0";
static char ConfId[] = "SEnumConf 1.1";


void __fastcall TConfig::Load(void) {
//-------------------------------------------------------------------------------
//                  ��������� ������������. � ����� ���������                   |
//-------------------------------------------------------------------------------
    String cfgname = TrimExt(Application->ExeName) + ".cfg";
    int filen, len;
    char *file, *p, fun;
    char *state = NULL;

    file = LoadFile(cfgname.c_str(), filen);
    if (file == NULL) return;
    if (strcmp(file, ConfId) != 0 && strcmp(file, ConfId0) == 0) {
        delete[] file;
        return;
    }
    p = file + sizeof(ConfId);
    while(1) {
        fun = *p++; if ((unsigned char)fun == 0xFF) break;
        switch(fun) {
            case 1:
                memmove(&Pos, p, sizeof(TRect));
                p += sizeof(TRect);
                break;
            case 2:
                len = *((int *)p); p += sizeof(int);
                state = new char[len];
                memmove(state, p, len); p += len;
                break;
            case 3:
                Path = (String)p;
                p += Path.Length() + 1;
                break;
            case 4:
                SPath = (String)p;
                p += SPath.Length() + 1;
                break;
            default:
                if (state != NULL) delete[] state;
                delete[] file;
                return;
        }
    }
    SEnumMainForm->Left = Pos.Left;
    SEnumMainForm->Top = Pos.Top;
    SEnumMainForm->Width = Pos.Right;
    SEnumMainForm->Height = Pos.Bottom;
    SEnumMainForm->SListMyGrid->RestoreState(state, len);
    SEnumMainForm->OpenDialog1->InitialDir = Path;
    SEnumMainForm->SignatureFolderDialog->Directory = SPath;

    delete[] file;
}


void __fastcall TConfig::Save(void) {
//-------------------------------------------------------------------------------
//                              ��������� ������������                          |
//-------------------------------------------------------------------------------
    String cfgname = TrimExt(Application->ExeName) + ".cfg";
    String pp;
    int len;
    char *file, *p;

    file = new char[100000]; p = file;
    strcpy(p, ConfId); p += sizeof(ConfId);

    *p++ = 1;
    Pos.Left = SEnumMainForm->Left;
    Pos.Top = SEnumMainForm->Top;
    Pos.Right = SEnumMainForm->Width;
    Pos.Bottom = SEnumMainForm->Height;
    memmove(p, &Pos, sizeof(TRect)); p += sizeof(TRect);

    *p++ = 2;
    len = SEnumMainForm->SListMyGrid->SaveState(p + 4, 100000);
    *((int *)p) = len; p += 4 + len;

    *p++ = 3;
    pp = SEnumMainForm->OpenDialog1->InitialDir;
    if (!pp.IsEmpty()) Path = pp;
    strcpy(p, Path.c_str());
    p += Path.Length() + 1;

    pp = SEnumMainForm->SignatureFolderDialog->Directory;
    if (!pp.IsEmpty()) SPath = pp;
    if (!SPath.IsEmpty()) {
        *p++ = 4;
        strcpy(p, SPath.c_str());
        p += SPath.Length() + 1;
    }

    *p++ = 0xFF;

    len = (int)p - (int)file;
    SaveFile(false, cfgname.c_str(), file, len);
    delete[] file;
}







//===============================================================================
//                                  TFileListRec                                |
//===============================================================================
void __fastcall TFileListRec::Clear(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    nsm = "";             // ����� �����
    filn = "";            // ��� ����� �����
    name = "";            // ������������ �����
    oname = "";           // ������������ �������
}


TFileListRec::TFileListRec(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    memset(this, 0, sizeof(TFileListRec));
}

TFileListRec::TFileListRec(String _nsm, String _fullfiln, String _name, String _oname) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    memset(this, 0, sizeof(TFileListRec));
    nsm = _nsm;
    path = EraseFilnFromPath(_fullfiln);
    filn = ErasePathFromFiln(_fullfiln);
    name = _name;
    oname = _oname;
}



//===============================================================================
//                                     TFileList                                |
//===============================================================================
TFileList::TFileList(void) : TVds(VdsLen0, sizeof(TFileListRec), 100000) {};


int __fastcall TFileList::Index(String _fullfiln) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    AnsiString path = EraseFilnFromPath(_fullfiln).LowerCase();
    String filn = ErasePathFromFiln(_fullfiln).LowerCase();
    TFileListRec *r;
    
    for (int i = 0; i < Nrecs; i++) {
        r = Rec3(i);
        if (r->path.LowerCase() == path && r->filn.LowerCase() == filn) {
            return i;
        }
    }

    return -1;
}

void __fastcall TFileList::Append(String _nsm, String _fullfiln, String _name, String _oname) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TFileListRec *r, rr;
    if (Index(_fullfiln) >= 0) return;
    TVds::Append();
    RecLen0 = sizeof(TFileListRec);
    r = new TFileListRec;
    memset(RecPtr0, sizeof(TFileListRec), 0);
    memmove(RecPtr0, r, sizeof(TFileListRec));
    r = Rec0(Nrecs - 1);
    r->nsm = _nsm;
    r->path = EraseFilnFromPath(_fullfiln);
    r->filn = ErasePathFromFiln(_fullfiln);
    r->name = _name;
    r->oname = _oname;
}


void __fastcall TFileList::Ins0(String _nsm, String _fullfiln, String _name, String _oname) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TFileListRec *r, rr;
    if (Index(_fullfiln) >= 0) return;
    r = new TFileListRec(_nsm, _fullfiln, _name, _oname);
    // memset(RecPtr0, sizeof(TFileListRec), 0);
    TVds::InsRec(0);
    RecLen0 = sizeof(TFileListRec);
    memmove(RecPtr0, r, sizeof(TFileListRec));

    // delete r;

    //r = Rec0(0);
    //r->nsm = _nsm;
    //r->path = EraseFilnFromPath(_fullfiln);
    //r->filn = ErasePathFromFiln(_fullfiln);
    //r->name = _name;
    //r->oname = _oname;
}


void __fastcall TFileList::DelRec(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Rec0(RecIndex0)->Clear();
    TVds::DelRec();
}

void __fastcall TFileList::DelRec(int index) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Rec0(index)->Clear();
    TVds::DelRec(index);
}


//struct TVdsRecInfo {
//    int index;
//    int data_offset;
//    int data_len;
//    int offset;
//    int size;
//};


static char *FileListFBuf;

static int FileListCmp(const TVdsRecInfo *r1, const TVdsRecInfo *r2) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TFileListRec *f1, *f2;
    String n1, n2;
    f1 = (TFileListRec *)(FileListFBuf + r1->data_offset);
    f2 = (TFileListRec *)(FileListFBuf + r2->data_offset);
    if (NospaceStrcmp(f1->filn.c_str(), "����������.doc", true) == 0) return -1;
    if (NospaceStrcmp(f2->filn.c_str(), "����������.doc", true) == 0) return 1;
    n1 = TrimExt(f1->filn); n2 = TrimExt(f2->filn);
    return strcmp(n1.c_str(), n2.c_str());
}


void __fastcall TFileList::Sort(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    FileListFBuf = FBuf;
    TVds::Sort(FileListCmp);
}









//===============================================================================
//                              �������                                         |
//===============================================================================
#include <Registry.hpp>

static String __fastcall GetFullPrinterR(char *printer) {
//-------------------------------------------------------------------------------
//      ���������� ������ ��� �������� �� ������� - � winspool,Ne08             |
// ���� ������ ��� - ���������� ������ ������                                   |
//-------------------------------------------------------------------------------
    static String Printers = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Devices\\";
    String res;
    TRegistry *r = new TRegistry;

    r->RootKey = HKEY_CURRENT_USER;
    r->Access = KEY_QUERY_VALUE;
    try {
        if (!r->OpenKey(Printers, false)) {
            return (String)"";
        }
        res = r->ReadString(printer);
        if (res.IsEmpty()) {
            delete r;
            return res;
        }
    } catch(...) {
        delete r;
        return (String)"";
    }

    delete r;
    return (String)printer + (String)"," + res;
}

String __fastcall GetPrinterName(int index) {
//-------------------------------------------------------------------------------
//                      ���������� ��� �������� �� �������                      |
// ���� ������ ������� �������, ���������� ������ ������                        |
//-------------------------------------------------------------------------------
    TPrinter *pr = Printer();
    TStrings *ss = pr->Printers;
    if (index < 0 || index >= ss->Count) return (String)"";
    return ss->Strings[index];
}


int __fastcall GetPrinterIndex(String PrinterName) {
//-------------------------------------------------------------------------------
//              ���������� ������ �������� �� ����� ��������                    |
// ���� �� ������� - ���������� -1                                              |
//-------------------------------------------------------------------------------
    String Name;
    int index;
    for (index = 0;; index++) {
        Name = GetPrinterName(index);
        if (Name.IsEmpty()) break;
        if (Name == PrinterName) return index;
    }
    return -1;
}


static bool __fastcall BadMODIMessage(TRegistry *r) {
//-------------------------------------------------------------------------------
//     ������ ��������� � ������������ ������ MODI; delete r; return false;     |
//-------------------------------------------------------------------------------
    Application->MessageBox("� ������� �� ������ �� ������� \"PDFCreator\", �� ������� ������� \"Microsoft Office Document Image Writer\" \015\012��� ����������� ����������� ������ ����� ��������\015\012��� ���� ������������������� ������� �������", "�� ����!", 0);
    if (r != NULL) {
        r->CloseKey();
        delete r;
    }
    return false;
}


bool __fastcall SwitchToPDFCreator(void) {
//-------------------------------------------------------------------------------
//                   ������ PDFCreator ��������� �� ���������                   |
// ���� �� ������, �� ������ ��������� �� ������ � ��������� false              |
//-------------------------------------------------------------------------------
    String NewCurPrinter;
    char nn[100];
    GetProfileString("windows", "device", "nodefault", OldCurPrinter, 100);
    NewCurPrinter = GetFullPrinterR("PDFCreator");
    WriteProfileString("windows", "device", NewCurPrinter.c_str());
    GetProfileString("windows", "device", "nodefault", nn, 100);
    if (!IsSubstr(nn, "PDFCreator", false)) {
        WriteProfileString("windows", "device", OldCurPrinter);
        *OldCurPrinter = 0;
        // Application->MessageBox("� ������� �� ������ ������� \"Microsoft Office Document Image Writer\" ��� ����������� ����������� ������ ����� ��������", "�� ����!", 0);
        // return false;
        TiffCurPrinter = NULL;
        return false;
    }
    TiffCurPrinter = "PDFCreator";
    return true;
}


bool __fastcall _SwitchToMODI(void) {
//-------------------------------------------------------------------------------
//                      ������ MODI ��������� �� ���������                      |
// ���� �� ������, �� ������ ��������� �� ������ � ��������� false              |
//-------------------------------------------------------------------------------
    String NewCurPrinter;
    char nn[100];
    GetProfileString("windows", "device", "nodefault", OldCurPrinter, 100);
    NewCurPrinter = GetFullPrinterR("Microsoft Office Document Image Writer");
    WriteProfileString("windows", "device", NewCurPrinter.c_str());
    GetProfileString("windows", "device", "nodefault", nn, 100);
    if (!IsSubstr(nn, "Microsoft Office Document Image Writer", false)) {
        WriteProfileString("windows", "device", OldCurPrinter);
        *OldCurPrinter = 0;
        // Application->MessageBox("� ������� �� ������ ������� \"Microsoft Office Document Image Writer\" ��� ����������� ����������� ������ ����� ��������", "�� ����!", 0);
        // return false;
        TiffCurPrinter = NULL;
        return BadMODIMessage(NULL);
    }
    TiffCurPrinter = "Microsoft Office Document Image Writer";
    return true;
}

void __fastcall _RestoreOldPrinter(void) {
//-------------------------------------------------------------------------------
//              ���������� �������, ������� ��� �� SwitchToMODI                 |
//-------------------------------------------------------------------------------
    if (*OldCurPrinter == 0) return;
    WriteProfileString("windows", "device", OldCurPrinter);
}

struct TMODIRegister {          // ������ � ������� MODI
    int OpenInMODI;             // 0(tiff), 1(mod)
    int PrivateFlags;           // 0x10(tiff), 0(mod)
    int Public_Orientation;     // 1(portrait), 2(landscape)
    int Public_PaperSize;       // 9(A4)
    int TIFDPI;                 // 0xc8(200); 0x12c(300)
};

bool __fastcall _SetupMODI(int orient) {
//-------------------------------------------------------------------------------
//   ������������� ��������� �������� Microsoft Office Document Image Writer    |
// orient = 1 - portrait, 2 - landscape, ����� ����������                       |
// ���� �� ������� ����� �������, �� ������ ��������� �� ������ � ����� false   |
//-------------------------------------------------------------------------------
    static String MODI = "Software\\Microsoft\\Office\\11.0\\MODI\\MDI writer";
    // static String MODI1 = "S-1-5-21-4095250297-1415307835-3824348034-1005\\Software\\Microsoft\\Office\\11.0\\MODI\\MDI writer";
    static String Provider = "Software\\Microsoft\\Protected Storage System Provider";
    String PMODI;
    String param;
    TRegistry *r = new TRegistry;
    TStringList *KeyNames;
    bool done = false;

    r->RootKey = HKEY_CURRENT_USER;
    r->Access = KEY_ALL_ACCESS;

    done = true;
    try {
        if (!r->OpenKey(Provider, false)) return BadMODIMessage(r);
        KeyNames = new TStringList;
        r->GetKeyNames(KeyNames);
        PMODI = KeyNames->Strings[0] + "\\" + MODI;
        delete KeyNames;
        r->CloseKey();
        r->RootKey = HKEY_USERS;
        if (!r->OpenKey(PMODI, false)) return BadMODIMessage(r);
        r->WriteInteger("TIFDPI", 300);
        r->WriteInteger("Public_PaperSize", 9);
        if (orient == 1 || orient == 2) {
            r->WriteInteger("Public_Orientation", orient);
        }
        r->WriteInteger("PrivateFlags", 0x10);
        r->WriteInteger("OpenInMODI", 0);
    } catch (...) {
        done = false;
    }
    r->CloseKey();

    if (!done) {
        r->RootKey = HKEY_CURRENT_USER;
        done = true;
        try {
            if (!r->OpenKey(MODI, false)) return BadMODIMessage(r);
            r->WriteInteger("TIFDPI", 300);
            r->WriteInteger("Public_PaperSize", 9);
            if (orient == 1 || orient == 2) {
                r->WriteInteger("Public_Orientation", orient);
            }
            r->WriteInteger("PrivateFlags", 0x10);
            r->WriteInteger("OpenInMODI", 0);
        } catch(...) {
            done = false;
        }
    }

/*
    r->RootKey = HKEY_USERS;
    if (!done) {
        try {
        if (!r->OpenKey(MODI1, false)) return BadMODIMessage(r);
        r->WriteInteger("TIFDPI", 300);
        r->WriteInteger("Public_PaperSize", 9);
        if (orient == 1 || orient == 2) {
            r->WriteInteger("Public_Orientation", orient);
        }
        r->WriteInteger("PrivateFlags", 0x10);
        r->WriteInteger("OpenInMODI", 0);
    } catch (...) {
        done = false;
    }
*/

    if (!done) return BadMODIMessage(r);

    r->CloseKey();
    delete r;
    return true;
}


// HKEY_CURRENT_USER\Software\Microsoft\Protected Storage System Provider

//GetUserSid
//OpenProcessToken



//===============================================================================
//                              ���������� ������                               |
//===============================================================================
double aleft, atop, awidth, aheight;
String cap;


void __fastcall AcceptExcelSheet(Variant EA, int sheet, String ExcelFiln) {
//-------------------------------------------------------------------------------
//         �������� ����������� ���� � ��������� ���� (������ ��� �������)      |
// ��������, ��� ���� ����� ��� ������� � ��� ������������                      |
// ����� ���� ����� ���������!                                                  |
//-------------------------------------------------------------------------------
    Variant Docs;
    Variant MyDoc, MyWorksheets, MyActiveSheet;
    Variant NewDoc, NewWorksheets, NewActiveSheet, NewCells, NewSelection;
    Variant Range;

    Docs = EA.OlePropertyGet("Workbooks");
    MyDoc = Docs.OlePropertyGet("Item", 1);
    NewDoc = Docs.OleFunction("Open", ExcelFiln.c_str(), false, true);

    MyDoc.OleProcedure("Activate");
    MyWorksheets = MyDoc.OlePropertyGet("Worksheets");
    MyActiveSheet = MyWorksheets.OlePropertyGet("Item", sheet);
    MyActiveSheet.OlePropertySet("Visible", -1);                    // xlSheetVisible == -1
    MyActiveSheet.OleProcedure("Select");
    MyActiveSheet.OlePropertySet("Name", ErasePathFromFiln(ExcelFiln).c_str());

    NewDoc.OleProcedure("Activate");
    NewWorksheets = NewDoc.OlePropertyGet("Worksheets");
    NewActiveSheet = NewWorksheets.OlePropertyGet("Item", 1);
    NewCells = NewActiveSheet.OlePropertyGet("Cells");
    NewCells.OleProcedure("Copy");

    MyDoc.OleProcedure("Activate");
    // Range = MyActiveSheet.OlePropertyGet("Range", "$B$1");
    // Range.OleProcedure("Select");
    MyActiveSheet.OleProcedure("Paste");
    Range = MyActiveSheet.OlePropertyGet("Range", "$A$1");
    Range.OleProcedure("Select");

    Clipboard()->Open();
    Clipboard()->Clear();
    Clipboard()->Close();

    NewDoc.OleProcedure("Close");
}


static int __fastcall ScanRow(Variant Cells, int row, int width) {
//-------------------------------------------------------------------------------
//                          ���� � ���� �������� ������                         |
// ���������� ����� ����� ������ �������� ������                                |
//-------------------------------------------------------------------------------
    Variant Cell;
    String val;
    int col, maxcol;

    maxcol = 0;
    for (col = width; col >= 1; col--) {
        Cell = Cells.OlePropertyGet("Item", row, col);
        val = Cell.OlePropertyGet("Value");
        if (!val.IsEmpty()) {
            maxcol = col;
            break;
        }
    }

    return maxcol;
}

static int __fastcall ScanRow1(Variant Cells, int row, int width) {
//-------------------------------------------------------------------------------
//                      ���� � ���� ������ �������� ������                      |
// ���� �������, ���������� �� �����, ���� ��� - ���������� 0                   |
//-------------------------------------------------------------------------------
    Variant Cell;
    String val;
    int col;

    for (col = 1; col <= width; col++) {
        Cell = Cells.OlePropertyGet("Item", row, col);
        val = Cell.OlePropertyGet("Value");
        if (!val.IsEmpty()) return col;
    }

    return 0;
}

static int __fastcall ScanCol1(Variant Cells, int height, int col) {
//-------------------------------------------------------------------------------
//                      ���� � ������� ������ �������� ������                   |
// ���� �������, ���������� �� �����, ���� ��� - ���������� 0                   |
//-------------------------------------------------------------------------------
    Variant Cell;
    String val;
    int row;

    for (row = 1; row <= height; row++) {
        if (row == 54 && col == 12) {
            stop++;
        }
        Cell = Cells.OlePropertyGet("Item", row, col);
        try {
            val = Cell.OlePropertyGet("Value");
        } catch(...) {
            continue;
        }
        if (!val.IsEmpty()) return row;
    }

    return 0;
}


static int __fastcall ScanRow(TFlexCelImport *Fi, int row, int width) {
//-------------------------------------------------------------------------------
//                          ���� � ���� �������� ������                         |
// ���������� ����� ����� ������ �������� ������                                |
//-------------------------------------------------------------------------------
    Variant Cell;
    String val;
    int col, maxcol;

    maxcol = 0;
    for (col = width; col >= 1; col--) {
        val = Fi->CellValue[row][col];
        if (!val.IsEmpty()) {
            maxcol = col;
            break;
        }
    }

    return maxcol;
}


TPoint __fastcall FindLastCell(char *Filn) {
//-------------------------------------------------------------------------------
//         ������� ����� ������ ������ ������� ����� (������� ������)           |
//-------------------------------------------------------------------------------
/*
    Variant EA, ActiveSheet, ActiveCell, SpecialCells, Cells, Cell;
    TPoint res;
    String Address, val;
    int x, y, ymax, xmax, empties;
    bool found;

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    EA = Doc.OlePropertyGet("Application");
    ActiveCell = EA.OlePropertyGet("ActiveCell");
    SpecialCells = ActiveCell.OlePropertyGet("SpecialCells", 11);
    Address = SpecialCells.OlePropertyGet("Address");
    res = CellFromAddress(Address);
    Cells = ActiveSheet.OlePropertyGet("Cells");

    xmax = 0; ymax = 0; empties = 0;
    for (y = 1; y <= res.y; y++) {
        x = ScanRow(Cells, y, res.x);
        if (x > 0) {
            empties = 0;
            ymax = y;
            if (x > xmax) xmax = x;
        } else {
            empties++;
            if (empties > 10) break;
        }
    }

    res.x = xmax; res.y = ymax;
    return res;
*/

    TFlexCelImport *Fi = SEnumMainForm->Fi;
    TPoint res;
    String Address, val;
    int x, y, ymax, xmax, empties;
    bool found;

    Fi->OpenFile(Filn);

    xmax = 0; ymax = 0; empties = 0;
    for (y = 1; y <= res.y; y++) { // ����� res �� ������������������, �.�. ���������� �����
        x = ScanRow(Fi, y, 100);
        if (x > 0) {
            empties = 0;
            ymax = y;
            if (x > xmax) xmax = x;
        } else {
            empties++;
            if (empties > 10) break;
        }
    }

    Fi->CloseFile();

    res.x = xmax; res.y = ymax;
    return res;

}


TPoint __fastcall FindLastCell9(Variant Cells, int height, int width) {
//-------------------------------------------------------------------------------
//                  ���� ��������� ������ ������� ������� ����� 9 �����         |
//-------------------------------------------------------------------------------
    int empties, xmax, ymax, x, y, yy;
    xmax = 0; ymax = 0;
    for (y = 1; y <= height + 10; y += 9) {
        x = ScanRow1(Cells, y, width);
        if (x == 0) {
            for (yy = y - 1; yy >= 1; yy--) {
                x = ScanRow1(Cells, yy, width);
                if (x != 0) break;
            }
            ymax = yy;
            for (yy = y + 1, empties = y - ymax; empties < 10; empties++, yy++) {
                x = ScanRow1(Cells, yy, width);
                if (x != 0) break;
            }
            if (empties >= 10) break;
        }
    }
    for (xmax = width; xmax >= 1; xmax--) {
        y = ScanCol1(Cells, ymax, xmax);
        if (y != 0) break;
    }
    return TPoint(xmax, ymax);
}



static TPoint __fastcall FindLastCellSheet(Variant Sheet) {
//-------------------------------------------------------------------------------
//         ������� ����� ������ ������ ������� ����� (������� ������)           |
//-------------------------------------------------------------------------------
    Variant EA, ActiveSheet, ActiveCell, SpecialCells, Cells, Cell;
    TPoint res, ares;
    String Address, val;
    int x, y, ymax, xmax, empties;
    bool found;

    ActiveSheet = Sheet;                            // ��� ���������� ���������������
    //EA = Doc.OlePropertyGet("Application");
    //ActiveCell = EA.OlePropertyGet("ActiveCell");
    //SpecialCells = ActiveCell.OlePropertyGet("SpecialCells", 11);
    Cells = ActiveSheet.OlePropertyGet("Cells");
    SpecialCells = Cells.OlePropertyGet("SpecialCells", 11);
    Address = SpecialCells.OlePropertyGet("Address");
    res = CellFromAddress(Address);

    return FindLastCell9(Cells, res.y, res.x);
}


TPoint __fastcall FindLastCell(Variant Doc) {
//-------------------------------------------------------------------------------
//         ������� ����� ������ ������ ������� ����� (������� ������)           |
//-------------------------------------------------------------------------------
    Variant EA, ActiveSheet, ActiveCell, SpecialCells, Cells, Cell;
    TPoint res, ares, ares1;
    String Address, val;
    int x, y, ymax, xmax, empties;
    bool found;

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    EA = Doc.OlePropertyGet("Application");
    ActiveCell = EA.OlePropertyGet("ActiveCell");
    SpecialCells = ActiveCell.OlePropertyGet("SpecialCells", 11);
    Address = SpecialCells.OlePropertyGet("Address");
    res = CellFromAddress(Address);
    Cells = ActiveSheet.OlePropertyGet("Cells");

    ares = FindLastCell9(Cells, res.y, res.x);
    ares1 = FindLastCellSheet(ActiveSheet);
    if (ares.x != ares1.x || ares.y != ares1.y) {
        stop++;
    }

    return ares;

    //Variant ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    //return FindLastCellSheet(ActiveSheet);    
}


TPoint __fastcall FindLastCell(Variant EA, int sheet) {
//-------------------------------------------------------------------------------
//         ������� ����� ������ ������ ������� ����� (������� ������)           |
//-------------------------------------------------------------------------------
    Variant Docs, Doc, Worksheets, ActiveSheet; // , ActiveCell, SpecialCells, Cells, Cell;

    Docs = EA.OlePropertyGet("Workbooks");
    Doc = Docs.OlePropertyGet("Item", 1);
    Doc.OleProcedure("Activate");
    Worksheets = Doc.OlePropertyGet("Worksheets");
    ActiveSheet = Worksheets.OlePropertyGet("Item", sheet);
    ActiveSheet.OleProcedure("Select");

    return FindLastCell(Doc);
}

void __fastcall SplitSheetPages(TPageInfo *pi, Variant EA, int sheet) {
//-------------------------------------------------------------------------------
//                        ��������� ���� �� ��������                            |
// ��������� pi �����������                                                     |
//-------------------------------------------------------------------------------
    Variant Docs, Doc, Windows, Window;
    Variant Worksheets, ActiveSheet, PageSetup, Range, Width;
    Variant HPageBreaks;
    Variant Columns, Column;
    Variant Cells, Cell;
    TPoint Bounds;
    String Address;
    double width;

    Docs = EA.OlePropertyGet("Workbooks");
    Doc = Docs.OlePropertyGet("Item", 1);

    Bounds = FindLastCell(EA, sheet);
    pi->rcount = Bounds.y; pi->ccount = Bounds.x;
    Worksheets = Doc.OlePropertyGet("Worksheets");
    ActiveSheet = Worksheets.OlePropertyGet("Item", sheet);
    ActiveSheet.OleProcedure("Select");

    PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
    PageSetup.OlePropertySet("Zoom", false);
    PageSetup.OlePropertySet("FitToPagesTall", false);
    PageSetup.OlePropertySet("FitToPagesWide", 1);
    Address = (String)"$A$10:" + AddressFromCell(10, pi->ccount);
    Range = ActiveSheet.OlePropertyGet("Range", Address.c_str());
    Width = Range.OlePropertyGet("Width");
    width = Width;
    if (width <= 600) pi->ori = 1; else pi->ori = 2;    // 1 - portrait, 2 - landscape
    PageSetup.OlePropertySet("Orientation", pi->ori);

    Columns = ActiveSheet.OlePropertyGet("Columns");
    Column = Columns.OlePropertyGet("Item", pi->ccount + 1);
    Column.OlePropertySet("ColumnWidth", 2.86);         // ��� ���� ���������� - ��������
    Cells = ActiveSheet.OlePropertyGet("Cells");
    Cell = Cells.OlePropertyGet("Item", pi->rcount, pi->ccount + 1);
    Cell.OlePropertySet("Value", " ");                  // ��� ����� �������������� ������� ��� ����� ������

    Windows = Doc.OlePropertyGet("Windows");
    Window = Windows.OlePropertyGet("Item", 1);
    Window.OlePropertySet("View", 2);               // ��� - �������� �������
    Window.OlePropertySet("Zoom", 100);             // ��� - 100%

    HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
    pi->pcount = HPageBreaks.OlePropertyGet("Count") + 1;
}

// ActiveSheet.HPageBreaks(2).Location.Cells.Address

// Gdiplus::Image *img;


void __fastcall SetPageNumber(Variant Cells, int ori, int x, int y, int num) {
//-------------------------------------------------------------------------------
//                  � ��������� ����� ������ ����� ��������                     |
// ������� �� y ���� ���� ������ �������. ����� ���� ���������� ��������(2), �� |
// ���������� 4 ������ ���� � ������ �� ���������� 90 ��������.                 |
//-------------------------------------------------------------------------------
    Variant Cell, Range, Font, Height;
    String Adr, From, To;
    double height;
    char buf[30];
    while(1) {
        Adr = AddressFromCell(y, 1);
        Range = Cells.OlePropertyGet("Range", Adr.c_str());
        Height = Range.OlePropertyGet("Height");
        height = Height;
        if (height > 0) break;
        y++;
    }
    From = AddressFromCell(y, x); To = AddressFromCell(y + 4, x);
    Adr = From + (String)":" + To;
    Range = Cells.OlePropertyGet("Range", Adr.c_str());
    if (num > 0) {
        if (ori == 2) {
            Range.OlePropertySet("HorizontalAlignment", -4108);
            Range.OlePropertySet("VerticalAlignment", -4160);
            Range.OlePropertySet("Orientation", 90);
        }
        Font = Range.OlePropertyGet("Font");
        Font.OlePropertySet("Name", "Arial Cyr");
        Font.OlePropertySet("FontStyle", "����������");
        Font.OlePropertySet("Size", 10);
        Font.OlePropertySet("Strikethrough", false);
        Font.OlePropertySet("Superscript", false);
        Font.OlePropertySet("Subscript", false);
        Font.OlePropertySet("OutlineFont", false);
        Font.OlePropertySet("Shadow", false);
        Font.OlePropertySet("Underline", -4142);
        Font.OlePropertySet("ColorIndex", -4105);
        Range.OlePropertySet("MergeCells", true);
        Range.OleProcedure("Copy");
    } else {
        num = -num;
        Range.OleFunction("PasteSpecial", -4122, -4142, false, false);
    }
    Cell = Cells.OlePropertyGet("Item", y, x);
    wsprintf(buf, "%d", num);
    Cell.OlePropertySet("Value", buf);  // TJPEGImage

    // img = new Gdiplus:Image;
}

/*  // ������������ � ������
    Range("R5:R10").Select
    With Selection
        .HorizontalAlignment = xlCenter = -4108
        .VerticalAlignment = xlTop = -4160
        .WrapText = False
        .Orientation = 90
        .AddIndent = False
        .IndentLevel = 0
        .ShrinkToFit = False
        .ReadingOrder = xlContext
        .MergeCells = True
    End With
-------------------------
    With Selection
        .HorizontalAlignment = xlCenter
        .VerticalAlignment = xlBottom = -4107 
        .WrapText = False
        .Orientation = -90
        .AddIndent = False
        .IndentLevel = 0
        .ShrinkToFit = False
        .ReadingOrder = xlContext
        .MergeCells = True
    End With
-------------------------
    With Selection.Font
        .Name = "Arial Cyr"
        .FontStyle = "����������"
        .Size = 10
        .Strikethrough = False
        .Superscript = False
        .Subscript = False
        .OutlineFont = False
        .Shadow = False
        .Underline = xlUnderlineStyleNone -4142
        .ColorIndex = xlAutomatic -4105
    End With
*/

/*  // ����������� ������� ������
Sub ������1()
    Selection.Copy
    Range("N116:N118").Select
    Selection.PasteSpecial
        Paste:=xlPasteFormats, -4122
        Operation:=xlNone, -4142
        SkipBlanks:=False,
        Transpose:=False
    Application.CutCopyMode = False
End Sub
*/



void __fastcall EnumSheetPages(TPageInfo *pi, Variant EA, int sheet, int start) {
//-------------------------------------------------------------------------------
//                  ��������� �������� �������� ������� �� start                |
//-------------------------------------------------------------------------------
    Variant Docs, Doc, Worksheets, ActiveSheet;
    Variant HPageBreaks, HPageBreak, Location, Cells, Cell, Address, Count;
    String address;
    TPoint adr;
    int i, count;
    char buf[30];

    Docs = EA.OlePropertyGet("Workbooks");
    Doc = Docs.OlePropertyGet("Item", 1);
    Worksheets = Doc.OlePropertyGet("Worksheets");
    ActiveSheet = Worksheets.OlePropertyGet("Item", sheet);
    ActiveSheet.OleProcedure("Select");
    HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");

/*
    for (i = 1; i < pi->pcount; i++) {
        HPageBreak = HPageBreaks.OlePropertyGet("Item", i);
        Location = HPageBreak.OlePropertyGet("Location");
        Cells = Location.OlePropertyGet("Cells");
        Address = Cells.OlePropertyGet("Address");
        address = Address;
        adr = CellFromAddress(address);
        Cells = ActiveSheet.OlePropertyGet("Cells");
        Cell = Cells.OlePropertyGet("Item", adr.y - 1, pi->ccount + 1);
        wsprintf(buf, "%d", start); start++;
        Cell.OlePropertySet("Value", buf);
    }
*/
    Cells = ActiveSheet.OlePropertyGet("Cells");
    SetPageNumber(Cells, pi->ori, pi->ccount + 1, 1, start); start++;
    for (i = 1;; i++) {
        Count = HPageBreaks.OlePropertyGet("Count");
        count = Count;
        if (i > count) break;
        HPageBreak = HPageBreaks.OlePropertyGet("Item", i);
        Location = HPageBreak.OlePropertyGet("Location");
        Cells = Location.OlePropertyGet("Cells");
        Address = Cells.OlePropertyGet("Address");
        address = Address;
        adr = CellFromAddress(address);
        Cells = ActiveSheet.OlePropertyGet("Cells");
        SetPageNumber(Cells, pi->ori, pi->ccount + 1, adr.y, -start); start++;
        Count = HPageBreaks.OlePropertyGet("Count");
        count = Count;
    }
    EA.OlePropertySet("CutCopyMode", false);
}


/*
Sub ������1()
    Range("R5").Select
    Columns("R:R").ColumnWidth = 2.86
End Sub
*/


int izoom, itall, iwide;
TPoint LastCell;
void __fastcall TestPageSetup(Variant EA) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Variant Docs;
    Variant MyDoc, MyWorksheets, MyActiveSheet;
    Variant PageSetup, Zoom, FitToPagesTall, FitToPagesWide;

    Docs = EA.OlePropertyGet("Workbooks");
    MyDoc = Docs.OlePropertyGet("Item", 1);
    MyWorksheets = MyDoc.OlePropertyGet("Worksheets");
    MyActiveSheet = MyWorksheets.OlePropertyGet("Item", 1);
    MyActiveSheet.OleProcedure("Select");

    PageSetup = MyActiveSheet.OlePropertyGet("PageSetup");
    Zoom = PageSetup.OlePropertyGet("Zoom");
    izoom = Zoom;
    FitToPagesTall = PageSetup.OlePropertyGet("FitToPagesTall");
    itall = FitToPagesTall;
    FitToPagesWide = PageSetup.OlePropertyGet("FitToPagesWide");
    iwide = FitToPagesWide;

    PageSetup.OlePropertySet("Zoom", false);
    PageSetup.OlePropertySet("FitToPagesTall", false);
    PageSetup.OlePropertySet("FitToPagesWide", 1);

    Zoom = PageSetup.OlePropertyGet("Zoom");
    izoom = Zoom;
    FitToPagesTall = PageSetup.OlePropertyGet("FitToPagesTall");
    itall = FitToPagesTall;
    FitToPagesWide = PageSetup.OlePropertyGet("FitToPagesWide");
    iwide = FitToPagesWide;

//        .Orientation = xlLandscape
// ActiveCell.SpecialCells(xlLastCell).Select
// Set b = ActiveCell.SpecialCells(xlLastCell)

}






void __fastcall Doit(TFileList *fl) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

// --------- ���������� ������� ��������� �������� � ������������� �� -----------
/*
    TFlexCelImport *Fi = SEnumMainForm->Fi;
    String Filn = "c:\\UnitedProjects\\��������� ������� ����\\exe\\��������.xls";
    String Filn2 = "c:\\UnitedProjects\\��������� ������� ����\\exe\\��������2.xls";
    WideString wname;
    String name;
    int i, n;

    if (fl->Nrecs == 0) {
        Application->MessageBox("���� �����!", "�������!", 0);
        return;
    }

    Fi->OpenFile(Filn);

    for (i = 1; i <= fl->Nrecs; i++) {
        Fi->ActiveSheet = i;
        name = Fi->ActiveSheetName;
        wname = fl->Rec1(i - 1)->filn;
        Fi->ActiveSheetName = wname;
        Fi->ActiveSheetVisible = sv_Visible;
    }

    Fi->Save(Filn2.c_str());
    Fi->CloseFile();
*/


// ActiveSheet.Pictures.Insert("C:\����� ������ ������ ���\�����\�����������������.JPG").Select
// ������� �������

    Variant EA, Docs, Doc, Worksheets, ActiveSheet, Pictures, APicture, Cell, Cells, ActiveWindow;
    Variant Range, MergeArea, Selection, Columns, Column, Rows, Row;
    String Filn = "c:\\UnitedProjects\\��������� ������� ����\\exe\\��������.xls";
    double d;

    // TestTiff();

    // TMiDocView

    stop++;
    try {
        EA = CreateOleObject("Excel.Application");
        Docs = EA.OlePropertyGet("Workbooks");
        stop = Docs.OlePropertyGet("Count");
    } catch (...) {
        Application->MessageBox("�� ���� ���������� ����� � �������� MS Excel", "������!", 0);
        return;
    }

    try {
        Doc = Docs.OleFunction("Open", Filn.c_str(), false, true);
        stop = Docs.OlePropertyGet("Count");
    } catch(...) {
        String Ermes = (String)"�� ���� ������� ���� \"" + Filn + "\"";
        Application->MessageBox(Ermes.c_str(), "������!", 0);
        Docs = Unassigned;
        EA = Unassigned;
        return;
    }

    EA.OlePropertySet("Visible", true);

    // TestPageSetup(EA);

    // AcceptExcelSheet(EA, 2, "c:\\UnitedProjects\\��������� ������� ����\\Komplekt Smet\\�������� ����\\�����\\1-1-1  ��������� �� ����.xls");
    AcceptExcelSheet(EA, 2, "c:\\UnitedProjects\\��������� ������� ����\\���� �������� ����\\���.���.�.������ �������� ���.194 (5.0)\\�����\\01-01-01.xls");
    TPageInfo pi;
    SplitSheetPages(&pi, EA, 2);
    EnumSheetPages(&pi, EA, 2, 5);
    stop++;


    // ActiveWindow = Doc.OlePropertyGet("ActiveWindow");
    Worksheets = Doc.OlePropertyGet("Worksheets");
    stop = Worksheets.OlePropertyGet("Count");              // ���� ���������� ����������
    // ActiveSheet = Worksheets.OleFunction("Item", 1);     // �� ��������!
    // Worksheets.OlePropertySet("ActiveSheet", "a2");      // �� � �������, �� � ������ �������� �� ��������

/*  // ------------------- ������ ������� ������ ���� � ������ ��� �������� -----------
    ActiveSheet = Worksheets.OlePropertyGet("Item", 2);     // ��������!
    cap = ActiveSheet.OlePropertyGet("Name");
    ActiveSheet.OlePropertySet("Visible", -1);
    ActiveSheet.OleProcedure("Select");
*/

/*  // --------------- ���������� ������� ����������� �����, � ������� �������� ������� ------
    Range = ActiveSheet.OlePropertyGet("Range", "$E$6");
    MergeArea = Range.OlePropertyGet("MergeArea");
    cap = MergeArea.OlePropertyGet("Address");              // ���� ������� ����, �� ����� ���� $E$6:$F$9
*/
/*  // ---------- ������ ������� ���������� ��������� ������ ----------
    Range = ActiveSheet.OlePropertyGet("Range", "$G$10:$I$13");
    Range.OlePropertySet("MergeCells", true);
*/

/*  // ---------- ������ � �������� ������ ������� -------------------
    Columns = ActiveSheet.OlePropertyGet("Columns");
    Column = Columns.OlePropertyGet("Item", 5);
    awidth = Column.OlePropertyGet("ColumnWidth");
    Column = Columns.OlePropertyGet("Item", 8);
    Column.OlePropertySet("ColumnWidth", awidth);
*/

/*  // ---------- ������ � �������� ������ ������ ---------------------
    Rows = ActiveSheet.OlePropertyGet("Rows");
    Row = Rows.OlePropertyGet("Item", 6);
    aheight = Row.OlePropertyGet("RowHeight");
    Row = Rows.OlePropertyGet("Item", 8);
    Row.OlePropertySet("RowHeight", aheight);
*/

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    Cells = ActiveSheet.OlePropertyGet("Cells");
    Cell = Cells.OlePropertyGet("Item", 15, 3);
    cap = Cell.OlePropertyGet("Address");                   // ������ ����� ������ � ���� $<�����>$<�����>
    aleft = Cell.OlePropertyGet("Left");                    // ������ ��������� ������
    atop = Cell.OlePropertyGet("Top");

    stop++;

    Pictures = ActiveSheet.OlePropertyGet("Pictures");

    APicture = Pictures.OleFunction("Insert", "C:\\����� ������ ������ ���\\�����\\�����������������.JPG");
    APicture.OlePropertySet("Height", 793.5);
    APicture.OlePropertySet("Width", 578.25);
    APicture.OlePropertySet("Left", aleft);
    APicture.OlePropertySet("Top", atop);

    EA = Unassigned;
}



/*
Sub ������1()
'
' ������1 ������
' ������ ������� 26.11.2010 (AShilin)
'

'
    Sheets("����1").Select
    Sheets.Add After:=Sheets("����1")
    Sheets.Item(2).Name = "kjdf"
End Sub

Sub ������2()
'
' ������2 ������
' ������ ������� 26.11.2010 (AShilin)
'

'
    Sheets("kjdf").Select
    Sheets("kjdf").Move Before:=Sheets(1)
End Sub
*/


// ----------------- �� ���� �� ���������� ---------------------
/*
    Variant EA, Docs, Doc, Worksheets, ActiveSheet, Sheet, Cells, Cell;
    String Filn = "c:\\UnitedProjects\\��������� ������� ����\\exe\\��������.xls";
    double d;

    stop++;
    try {
        EA = CreateOleObject("Excel.Application");
        Docs = EA.OlePropertyGet("Workbooks");
    } catch (...) {
        Application->MessageBox("�� ���� ���������� ����� � �������� MS Excel", "������!", 0);
        return;
    }

    try {
        Doc = Docs.OleFunction("Open", Filn.c_str(), false, true);
    } catch(...) {
        String Ermes = (String)"�� ���� ������� ���� \"" + Filn + "\"";
        Application->MessageBox(Ermes.c_str(), "������!", 0);
        Docs = Unassigned;
        EA = Unassigned;
        return;
    }

    EA.OlePropertySet("Visible", true);
    Worksheets = Doc.OlePropertyGet("Worksheets");
    Sheet = Doc.OlePropertyGet("Sheets");
    ActiveSheet = Sheet.OleFunction("Item", 2);     // �� ���������� ������������� �� ��������� ����
    ActiveSheet.OlePropertySet("Visible", -1);
    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    Sheet.OleProcedure("Add");
    Cells = ActiveSheet.OlePropertyGet("Cells");

    Cell = Cells.OlePropertyGet("Item", 2, 2);
    d = 3.456;
    Cell.OlePropertySet("Value", d);
    Cell = Cells.OlePropertyGet("Item", 2, 3);
    Cell.OlePropertySet("Value", "7,89");


    Filn = EraseFilnFromPath(Application->ExeName) + "������������1.xls";
    // EA.OlePropertySet("Visible", true);
    Doc.OleProcedure("SaveAs", Filn.c_str());
    Doc.OleProcedure("Close");
*/



/*
   Uflxformats::TFlxFormat CFormat;
    int FX;

    stop++;
    Src->OpenFile("c:\\UnitedProjects\\WinAbersRead\\�������������� ����������� ������\\1 ���-������-�������.xls");

    // FX = Src->CellFormat[row][col];

    // FX = Src->CellFormat[1][7];
    FX = Src->CellFormat[13][1];
    if (FX == -1) return;
    Src->GetFormatList(FX, CFormat);
    if (CFormat.Font.Style.Contains(flsBold)) stop++;

    FX = Src->CellFormat[2][2];
    if (FX == -1) return;
    Src->GetFormatList(FX, CFormat);
    if (CFormat.Font.Style.Contains(flsBold)) stop++;

    Src->CloseFile();
*/


static bool __fastcall PresentSmetaTypeCont(String Text) {
//-------------------------------------------------------------------------------
//                      ���� � ������ Text ����� "������� ������"               |
// ���� ������� - ���������� true, ����� - false                                |
//-------------------------------------------------------------------------------
    char *p;
    p = Text.c_str();
    while(*p != 0) {
        if (IsSubstr(p, "������� ������", true)) return true;
        p++;
    }
    return false;
}


static String __fastcall FindSmetaNum(String Text) {
//-------------------------------------------------------------------------------
//                      ���� � ����� ������ ����� �����                         |
// ���� �� ������� - ���������� ������ ������                                   |
//-------------------------------------------------------------------------------
    int cnt;
    char *p;
    if (Text.IsEmpty()) return "";
    p = Text.c_str() + Text.Length() - 1;
    cnt = 0;
    while(*p == ' ') p--;
    while(1) {
        if (p == Text.c_str()) break;
        if (*p != '-' && (*p < '0' || *p > '9')) break;
        cnt++;
        p--;
    }
    if (cnt == 0) return "";
    return *p == ' ' ? (String)p : (String)' ' + (String)p;
}


static String __fastcall FindSmetaNum(Variant Cells, int i, int j) {
//-------------------------------------------------------------------------------
//          ���� � ��������� ������ � ���� ����� �� ����� �����                 |
// ���� �� ������� - ���������� ������ ������                                   |
//-------------------------------------------------------------------------------
    Variant Cell;
    String Text, res;
    int k;
    for (k = 0; k < 10; k++) {
        Cell = Cells.OlePropertyGet("Item", i, j + k);
        Text = Cell.OlePropertyGet("Value");
        res = FindSmetaNum(Text);
        if (!res.IsEmpty()) return res;
    }
    return "";
}


static String __fastcall IsSmetaType(Variant Cells, int i, int j) {
//-------------------------------------------------------------------------------
//       ���� ���������, ��������� ������� ������� ��� ������� ������� ������   |
// �� ���������� ������ � ������� �����.                                        |
// ����� ���������� ������ ������                                               |
//-------------------------------------------------------------------------------
    Variant Cell;
    String Text, num;
    Cell = Cells.OlePropertyGet("Item", i, j);
    Text = Cell.OlePropertyGet("Value");
    if (IsSubstr(Text.c_str(), "���������", true)) {
        if (!PresentSmetaTypeCont(Text)) return "";
        num = FindSmetaNum(Cells, i, j);
        if (num.IsEmpty()) return "";
        num = (String)"��������� ����� � " + num;
        return num;
    } else if (IsSubstr(Text.c_str(), "��������� �����", true)) {
        num = FindSmetaNum(Cells, i, j);
        if (num.IsEmpty()) return "";
        num = (String)"��������� ����� � " + num;
        return num;
    } else if (IsSubstr(Text.c_str(), "���������", true)) {
        if (!PresentSmetaTypeCont(Text)) return "";
        num = FindSmetaNum(Cells, i, j);
        if (num.IsEmpty()) return "";
        num = (String)"��������� ����� � " + num;
        return num;
    } else if (IsSubstr(Text.c_str(), "��������� �����", true)) {
        num = FindSmetaNum(Cells, i, j);
        if (num.IsEmpty()) return "";
        num = (String)"��������� ����� � " + num;
        return num;
    } else if (IsSubstr(Text.c_str(), "�������", true)) {
        if (IsSubstr(Text.c_str(), "������� ������� ������ � �����", true)) return "";
        if (!PresentSmetaTypeCont(Text)) return "";
        return (String)"������� �����";
    } else if (IsSubstr(Text.c_str(), "������� �����", true)) {
        if (IsSubstr(Text.c_str(), "������� ������� ������ � �����", true)) return "";
        return (String)"������� �����";
    }
    return "";
}


String __fastcall FindOName(Variant Doc) {
//-------------------------------------------------------------------------------
//              ���� � ��������� ��������� ������������ �������                 |
// ���� �� ������� - ���������� ������ ������                                   |
//-------------------------------------------------------------------------------
    Variant Worksheets, ActiveSheet, Cells, Cell;
    String Text, SmetaType, first, res;
    int i, j;
    bool found;

    Worksheets = Doc.OlePropertyGet("Worksheets");
    ActiveSheet = Worksheets.OlePropertyGet("Item", 1);
    Cells = ActiveSheet.OlePropertyGet("Cells");
    for (i = 1, found = false; i < 30 && !found; i++) {
        stop++;
        for (j = 1; j < 15 && !found; j++) {
            //Cell = Cells.OlePropertyGet("Item", i, j);
            //Text = Cell.OlePropertyGet("Value");
            //if (IsSubstr(Text.c_str(), "��������� ������� ������", true) ||
            //    IsSubstr(Text.c_str(), "��������� ������� ������", true)) {
            //    found = true;
            //}
            SmetaType = IsSmetaType(Cells, i, j);
            if (!SmetaType.IsEmpty()) found = true;
        }
    }
    if (!found) return "";

    for (; i < 30 && found; i++) {
        for (j = 1; j < 10 && found; j++) {
            Cell = Cells.OlePropertyGet("Item", i, j);
            Text = Cell.OlePropertyGet("Value");
            if (Text.IsEmpty()) continue;
            if (IsSubstr(Text.c_str(), "(��������� �����)", true) ||
                IsSubstr(Text.c_str(), "(��������� �����)", true) ||
                IsSubstr(Text.c_str(), "(������� �����)", true)
            ) {
                // i++;
                break;
            }
            found = false;
        }
    }

    stop++;

    for (i--, j--, res = "", first = ""; j < 10; j++) {
        Cell = Cells.OlePropertyGet("Item", i, j);
        Text = Cell.OlePropertyGet("Value");
        if (!Text.IsEmpty()) {
            if (first.IsEmpty()) {
                first = Text;
            } else {
                res += " ";
                res += Text;
            }
        }
    }

    if ((res.Trim()).IsEmpty()) res = first;

    return SmetaType + " " + res;
}


void __fastcall ProcessSmeta(TFileListRec *r, Variant Doc) {
//-------------------------------------------------------------------------------
//          ���� ������ ���� - �������, �� ��������� ��������� ��������         |
// ����� - ����������� �������, �������� �������� � ������, �������� �����      |
// ���� r != NULL, �� ���������� � r->oname ������������ �����, ���� �������    |
//-------------------------------------------------------------------------------
    if (r == NULL) return;
    r->oname = FindOName(Doc);
}

/*
    PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
    PageSetup.OlePropertySet("Zoom", false);
    PageSetup.OlePropertySet("FitToPagesTall", false);
    PageSetup.OlePropertySet("FitToPagesWide", 1);
    Address = (String)"$A$10:" + AddressFromCell(10, pi->ccount);
    Range = ActiveSheet.OlePropertyGet("Range", Address.c_str());
    Width = Range.OlePropertyGet("Width");
    width = Width;
    if (width <= 600) pi->ori = 1; else pi->ori = 2;    // 1 - portrait, 2 - landscape
    PageSetup.OlePropertySet("Orientation", pi->ori);


    Windows = Doc.OlePropertyGet("Windows");
    Window = Windows.OlePropertyGet("Item", 1);
    Window.OlePropertySet("View", 2);               // ��� - �������� �������
    Window.OlePropertySet("Zoom", 100);             // ��� - 100%

    SetupMODI(2);
*/

int __fastcall Paginate(Variant Doc) {
//-------------------------------------------------------------------------------
//                          ��������� ���� �� ��������                          |
// ���� �������� ������� ��� ���� - ��������� ������� ��������� ������          |
// ����� ������������� ������ ������ - 1 ������� � �������� ����������          |
// ��������, ��� ������� - ��� MODI                                             |
// ���������� ���������� ������� �� ���� ��������                               |
//-------------------------------------------------------------------------------
    Variant Windows, Window, Sheets, ActiveSheet, PageSetup, Range, Width;
    Variant HPageBreaks, VPageBreaks;
    String address;
    TPoint pt;
    int i, view, width, ori, res;

/*
    Windows = Doc.OlePropertyGet("Windows");
    Window = Windows.OlePropertyGet("Item", 1);
    view = Window.OlePropertyGet("View");
    if (view == 2) {                                // ��� ������� �� ��������
        ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
        PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
        ori = PageSetup.OlePropertyGet("Orientation");
        SetupMODI(ori);
        Window.OlePropertySet("View", 1);           // ��� - �������� �������
        Window.OlePropertySet("Zoom", 100);         // ��� - 100%
        Window.OlePropertySet("View", 2);           // ��� - �������� �������
    } else {
        ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
        PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
        PageSetup.OlePropertySet("Zoom", false);
        PageSetup.OlePropertySet("FitToPagesTall", false);
        PageSetup.OlePropertySet("FitToPagesWide", 1);
        pt = FindLastCell(Doc);
        address = (String)"$A$10:" + AddressFromCell(10, pt.x);
        Range = ActiveSheet.OlePropertyGet("Range", address.c_str());
        Width = Range.OlePropertyGet("Width");
        width = Width;
        if (width <= 600) ori = 1; else ori = 2;            // 1 - portrait, 2 - landscape
        PageSetup.OlePropertySet("Orientation", ori);
        SetupMODI(ori);
        Window.OlePropertySet("View", 2);           // ��� - �������� �������
        Window.OlePropertySet("Zoom", 100);         // ��� - 100%
    }
*/

    int OldActiveSheet, SheetsCount, VCount, HCount, SheetVisible;
    bool empty_sheet;

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    OldActiveSheet = ActiveSheet.OlePropertyGet("Index");
    Sheets = Doc.OlePropertyGet("Sheets");
    SheetsCount = Sheets.OlePropertyGet("Count");

    for (i = 1, res = 0; i <= SheetsCount; i++) {
        ActiveSheet = Sheets.OlePropertyGet("Item", i);
        SheetVisible = ActiveSheet.OlePropertyGet("Visible");
        if (SheetVisible == 0) continue;

        ActiveSheet.OleProcedure("Activate");
        Windows = Doc.OlePropertyGet("Windows");
        Window = Windows.OlePropertyGet("Item", 1);
        view = Window.OlePropertyGet("View");
        if (view == 2) {                                // ��� ������� �� ��������
            empty_sheet = false;
            ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
            PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
            ori = PageSetup.OlePropertyGet("Orientation");
            // SetupMODI(ori);
            Window.OlePropertySet("View", 1);           // ��� - �������� �������
            Window.OlePropertySet("Zoom", 100);         // ��� - 100%
            Window.OlePropertySet("View", 2);           // ��� - �������� �������
        } else {
            ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
            PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
            PageSetup.OlePropertySet("Zoom", false);
            PageSetup.OlePropertySet("FitToPagesTall", false);
            PageSetup.OlePropertySet("FitToPagesWide", 1);
            // pt = FindLastCell(Doc);
            pt = FindLastCellSheet(ActiveSheet);
            if (pt.x + pt.y == 0) {
                empty_sheet = true;
            } else {
                empty_sheet = false;
                address = (String)"$A$10:" + AddressFromCell(10, pt.x);
                Range = ActiveSheet.OlePropertyGet("Range", address.c_str());
                Width = Range.OlePropertyGet("Width");
                width = Width;
                if (width <= 600) ori = 1; else ori = 2;            // 1 - portrait, 2 - landscape
                PageSetup.OlePropertySet("Orientation", ori);
                // SetupMODI(ori);
                Window.OlePropertySet("View", 2);           // ��� - �������� �������
                Window.OlePropertySet("Zoom", 100);         // ��� - 100%
            }
        }

        if (!empty_sheet) {
            HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
            VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
            HCount = HPageBreaks.OlePropertyGet("Count");
            if (HCount > 0) {
                try {
                    HPageBreaks.OlePropertyGet("Item", HCount);
                } catch(...) {
                    HCount--;
                }
            }
            VCount = VPageBreaks.OlePropertyGet("Count");
            res += (HCount + 1) * (VCount + 1);
        }
    }

    ActiveSheet = Sheets.OlePropertyGet("Item", OldActiveSheet);
    ActiveSheet.OleProcedure("Activate");

    return res;
}


// ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
// HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
// VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
// r->pi.pcount = (HPageBreaks.OlePropertyGet("Count") + 1) * (VPageBreaks.OlePropertyGet("Count") + 1);



int __fastcall Excel2Tiff(TPDFCreator *pc, Variant EA, String filn, int pn) {
//-------------------------------------------------------------------------------
//              ������� ��������� ����������� ���� � tiff                       |
// EA - Excel.Application                                                       |
// ���������� ���������� ������������ �������                                   |
// ���� ��� �� ��� - ������ ��������������� ���������                           |
//-------------------------------------------------------------------------------
    Variant Docs, Doc, ActiveSheet;
    String mes;

    Docs = EA.OlePropertyGet("Workbooks");
    ProcessInfoAction("�������� �����");
    try {
        Doc = Docs.OleFunction("Open", filn.c_str(), false, true);
    } catch(...) {
        mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
        return 0;
    }

    ProcessInfoAction("��������� �����");
    ProcessSmeta(NULL, Doc);  // ���� ����� - ����������� ������� � ������
    ProcessInfoAction("��������� �� ��������");
    Paginate(Doc);      // ��������� �� ��������

    // � ��� ������ - ��������!
    HANDLE chan;
    String aTiffFiln;       // ��� ���������� �����

    // aTiffFiln = "c:\\UnitedProjects\\��������� ������� ����\\exe\\tmp.tif";
    char *tmp;
    tmp = new char[1000];
    // ::GetTempPath(1000, tmp);
    aGetTempPath(1000, tmp);
    aTiffFiln = (String)tmp + "tmp.tif";
    delete[] tmp;

    ProcessInfoAction("����� �� ��������� ����");
/*
    chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        CloseHandle(chan);
        DeleteFile(aTiffFiln.c_str());
        chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (chan != INVALID_HANDLE_VALUE) {
            CloseHandle(chan);
            mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� ������ �������� TIFF �� ����� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            Doc.OleProcedure("Close", false);
            return 0;
        }
    }
*/

    stop++;
    Gdiplus::Bitmap *TiffBitmap;
    Variant Sheets = Doc.OlePropertyGet("Sheets");
    int Count = Sheets.OlePropertyGet("Count");
    int SheetVisible;
    int j, k, nn, printed_pages = 0;
    char buf[100];

    stop++;
    for (int i = 0; i < Count; i++) {

        // ��������, ���� �� ��� ������ ���� ��������
        ActiveSheet = Sheets.OlePropertyGet("Item", i + 1);
        SheetVisible = ActiveSheet.OlePropertyGet("Visible");
        if (SheetVisible == 0) continue;
        TPoint pt = FindLastCellSheet(ActiveSheet);
        if (pt.x + pt.y == 0) continue;

        // �������� ���������� ����� ��� ������ tiff
        chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (chan != INVALID_HANDLE_VALUE) {
            CloseHandle(chan);
            DeleteFile(aTiffFiln.c_str());
            chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if (chan != INVALID_HANDLE_VALUE) {
                CloseHandle(chan);
                mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� ������ �������� TIFF �� ����� \"" + filn + "\"";
                Application->MessageBox(mes.c_str(), "������!", 0);
                Doc.OleProcedure("Close", false);
                return 0;
            }
        }

        // ���������� ����� tiff
        if (pc == NULL) {
            if (!ActiveSheet2Tiff(ActiveSheet, aTiffFiln.c_str())) {
                mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� ������ �������� TIFF �� ����� \"" + filn + "\"";
                Application->MessageBox(mes.c_str(), "������!", 0);
                Doc.OleProcedure("Close", false);
                return 0;
            }
        } else {
            pc->PrintXLS(aTiffFiln, ActiveSheet);
        }

        // � ������ ������������� ������������ ���� �� ���� ���������
        EA.OlePropertySet("Visible", false);
        TiffBitmap = LoadTiffFile(aTiffFiln.c_str());
        nn = LoadTiffFileNpages; printed_pages += nn;
        for (j = 0, k = pn; j < nn && !NeedBreakProcess; j++, k++) {
            Application->ProcessMessages();
            wsprintf(buf, "����� � �������� ���� �������� %d �� %d", j + 1, nn);
            ProcessInfoAction(buf);
            SetTiffFilePage(j);
            SaveBitmap2Tiff(TiffBitmap, pn == 0 ? 0 : k);
        }
        ReleaseTiffFile();
        DeleteFile(aTiffFiln);
        EA.OlePropertySet("Visible", true);

        aWait(1000);
        if (pn != 0) pn = k;
    }

    // ���, ������ ���� ����������� ���� ��� �� ����� - ��� ���������� ��� � tiff
    Doc.OleProcedure("Close", false);

    // � ������ ������������� ������������ ���� �� ���� ���������
/*
    Gdiplus::Bitmap *TiffBitmap;
    int i, k, nn;
    char buf[100];
    TiffBitmap = LoadTiffFile(aTiffFiln.c_str());
    nn = LoadTiffFileNpages;
    for (i = 0, k = pn; i < nn && !NeedBreakProcess; i++, k++) {
        Application->ProcessMessages();
        wsprintf(buf, "����� � �������� ���� �������� %d �� %d", i + 1, nn);
        ProcessInfoAction(buf);
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, pn == 0 ? 0 : k);
    }
    ReleaseTiffFile();
    DeleteFile(aTiffFiln);
*/
    // return pn == 0 ? 0 : nn;
    return pn == 0 ? 0 : printed_pages;
}


int __fastcall Word2Tiff(TPDFCreator *pc, Variant WA, String filn, int pn) {
//-------------------------------------------------------------------------------
//               ������� ��������� ���������� ���� � tiff                       |
// WA - Word.Application                                                        |
// ���������� ���������� ������������ �������                                   |
// ���� ��� �� ��� - ������ ��������������� ���������                           |
//-------------------------------------------------------------------------------
// ActiveDocument.PageSetup wdOrientLandscape = 1, wdOrientPortrait = 0
// Application.PrintOut

    Variant Docs, Doc;
    String mes;
    int cnt;

    Docs = WA.OlePropertyGet("Documents");
    ProcessInfoAction("�������� �����");
    try {
        Doc = Docs.OleFunction("Open", filn.c_str(), false, true);
    } catch(...) {
        mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
        return 0;
    }

    // ��������� �� �������� - ���� ����� ������������, ��������� �� ��� ��������

    stop = Doc.OleFunction("ComputeStatistics", 2, true);        // ������ ������� ���������� ������� 


    // � ��� ������ - ��������!
    HANDLE chan;
    String aTiffFiln;       // ��� ���������� �����

    // aTiffFiln = "c:\\UnitedProjects\\��������� ������� ����\\exe\\tmp.tif";
    char *tmp;
    tmp = new char[1000];
    // ::GetTempPath(1000, tmp);
    aGetTempPath(1000, tmp);
    aTiffFiln = (String)tmp + "tmp.tif";
    delete[] tmp;
    
    ProcessInfoAction("����� �� ��������� ����");
    chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        CloseHandle(chan);
        DeleteFile(aTiffFiln.c_str());
        chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (chan != INVALID_HANDLE_VALUE) {
            CloseHandle(chan);
            mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� �������� TIFF �� ����� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            Doc.OleProcedure("Close", false);
            return 0;
        }
    }

    if (pc == NULL) {
        if (!Word2Tiff(WA, aTiffFiln.c_str())) {
            mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� �������� TIFF �� ����� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            Doc.OleProcedure("Close", false);
            return 0;
        }
    } else {
        pc->PrintDOC(aTiffFiln, WA);
    }

    // ���, ������ ���� ���������� ���� ��� �� ����� - ��� ���������� ��� � tiff
    Doc.OleProcedure("Close", false);

    // � ������ ������������� ������������ ���� �� ���� ���������
    Gdiplus::Bitmap *TiffBitmap;
    int i, k, nn;
    char buf[100];
    TiffBitmap = LoadTiffFile(aTiffFiln.c_str());
    nn = LoadTiffFileNpages;
    for (i = 0, k = pn; i < nn && !NeedBreakProcess; i++, k++) {
        Application->ProcessMessages();
        wsprintf(buf, "����� � �������� ���� �������� %d �� %d", i + 1, nn);
        ProcessInfoAction(buf);
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, pn == 0 ? 0 : k);
    }
    ReleaseTiffFile();
    DeleteFile(aTiffFiln);

    return pn == 0 ? 0 : nn;
}


int __fastcall PDF2Tiff(TPDFCreator *pc, Variant AA, String filn, int pn) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Variant avdoc;
    String mes;
    PROCESS_INFORMATION pi;
    int npages, res;

    if (pc == NULL) {
        res = StartAcrobat(pi, filn);
        if (res != 0 && pi.hProcess == NULL) {
            mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            return 0;
        }
        avdoc = AA.OleFunction("GetActiveDoc");
    } else {
        avdoc = Unassigned;
    }

    // � ��� ������ - ��������!
    HANDLE chan;
    String aTiffFiln;       // ��� ���������� �����

    // aTiffFiln = "c:\\UnitedProjects\\��������� ������� ����\\exe\\tmp.tif";
    char *tmp;
    tmp = new char[1000];
    // ::GetTempPath(1000, tmp);
    aGetTempPath(1000, tmp);
    aTiffFiln = (String)tmp + "tmp.tif";
    delete[] tmp;

    ProcessInfoAction("����� �� ��������� ����");
    chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        CloseHandle(chan);
        DeleteFile(aTiffFiln.c_str());
        chan = CreateFile(aTiffFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (chan != INVALID_HANDLE_VALUE) {
            if (pc != NULL) {
                avdoc = AA.OleFunction("GetActiveDoc");
                avdoc.OleFunction("Close", 1);
            }
            CloseHandle(chan);
            mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� �������� TIFF �� ����� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            return 0;
        }
    }

    if (pc == NULL) {
        if (!PDF2Tiff(AA, aTiffFiln.c_str())) {
            avdoc = AA.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            mes = (String)"�� ���� ������� ��������� ���� \"" + aTiffFiln + "\" ��� �������� TIFF �� ����� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            return 0;
        }
        avdoc = AA.OleFunction("GetActiveDoc");
        res = AA.OleFunction("GetNumAVDocs");
        if (res != 0) {
            avdoc.OleFunction("Close", 1);
        }
    } else {
        pc->PrintPDF(aTiffFiln, filn);
    }

    // � ������ ������������� ������������ ���� �� ���� ���������
    Gdiplus::Bitmap *TiffBitmap;
    int i, k, nn;
    char buf[100];
    TiffBitmap = LoadTiffFile(aTiffFiln.c_str());
    nn = LoadTiffFileNpages;
    for (i = 0, k = pn; i < nn && !NeedBreakProcess; i++, k++) {
        Application->ProcessMessages();
        wsprintf(buf, "����� � �������� ���� �������� %d �� %d", i + 1, nn);
        ProcessInfoAction(buf);
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, pn == 0 ? 0 : k);
    }
    ReleaseTiffFile();
    DeleteFile(aTiffFiln);

    return pn == 0 ? 0 : nn;
}


int __fastcall Tiff2Tiff(String filn, int pn) {
//-------------------------------------------------------------------------------
//              ������� ��������� ��������� ���� � tiff                         |
// ���������� ���������� ������������ �������                                   |
// ���� ��� �� ��� - ������ ��������������� ���������                           |
//-------------------------------------------------------------------------------
    Gdiplus::Bitmap *TiffBitmap;
    int i, k, nn;
    char buf[100];
    TiffBitmap = LoadTiffFile(filn.c_str());
    if (TiffBitmap == NULL) return 0;
    nn = LoadTiffFileNpages;
    for (i = 0, k = pn; i < nn && !NeedBreakProcess; i++, k++) {
        Application->ProcessMessages();
        wsprintf(buf, "����� � �������� ���� �������� %d �� %d", i + 1, nn);
        ProcessInfoAction(buf);
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, pn == 0 ? 0 : k);
    }
    ReleaseTiffFile();

    return pn == 0 ? 0 : nn;
}


bool __fastcall FileList2Tiff(char *filn, TFileList *fl, int f1, int p0) {
//-------------------------------------------------------------------------------
//         ������� � ��������� ���� filn (������� tif) ������ ������ fl         |
// ������ ������� �������� ������� � ����� ����� f1, ����� ������ �������� �    |
// ���� ����� p0                                                                |
//-------------------------------------------------------------------------------
    Variant EA, WA, AA;
    TFileListRec *r;
    HANDLE chan;
    PROCESS_INFORMATION pi;
    HANDLE hProcess;
    String ext, mes, afiln;
    int i;
    char buf[200];
    bool NeedEA, NeedWA, NeedAA;

    /*
    if (SwitchToPDFCreator()) {
        //if (!SetupPDFCreator()) {
        //    RestoreOldPrinter();
        //    return;
        //}
    } else {
        if (!SwitchToMODI()) return;
        if (!SetupMODI(2)) {
            RestoreOldPrinter();
            return;
        }
    }
    */

/*
    TPDFCreator *pc = new TPDFCreator();
    if (!pc->ok) {
        delete pc;
        pc = NULL;
    }

    if (pc == NULL) {
        if (!SwitchToMODI()) return false;
        if (!SetupMODI(2)) {
            RestoreOldPrinter();
            return false;
        }
    }
*/


    TPDFCreator *pc = new TPDFCreator();
    if (!pc->ok) {
        delete pc;
        Application->MessageBox("��� ������ ��������� ����������, ����� �� ���������� ��� ���������� ������� PDFCreator!", "������!", 0);
        return false;
    }

    DeleteFile(filn);
    chan = CreateFile(filn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        CloseHandle(chan);
        WA.OleProcedure("Quit");
        WA = Unassigned;
        mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
        if (pc != NULL) delete pc;
        return false;
    }

    // ----------- ������� ��������, ��� �� � ��� ���� ��� ������ ---------------
    for (i = 0, NeedEA = false, NeedWA = false, NeedAA = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        ext = ExtractExt(r->filn);
        if (ext == "xls" || ext == "XLS") NeedEA = true;
        else if (ext == "doc" || ext == "DOC") NeedWA = true;
        else if (ext == "pdf" || ext == "PDF") NeedAA = true;
        else if (ext == "tif" || ext == "TIF") ;
        else if (ext == "tiff" || ext == "TIFF") ;
        else if (ext == "jpg" || ext == "JPG") ;
        else if (ext == "jpeg" || ext == "JPEG") ;
        else {
            mes = (String)"����� ������� \"" + ext + "\" �������� �� ��������������";
            Application->MessageBox(mes.c_str(), "���� �����!", 0);
        }
    }
    AA = Unassigned;
    if (NeedAA) {
        if (pc == NULL) {
            if (!TestAcrobatStandartPresence()) {
                NeedAA = false;
                // RestoreOldPrinter();
                StopSaveBitmap2Tiff();
                StopGdiplus();
                Application->MessageBox("��� ��������� ������ PDF ���������, ����� � ������� ��� ���������� Adobe Acrobat Standart", "�� ����!", 0);
                return false;
            } else {
                StartAcrobat(pi, "");
                hProcess = pi.hProcess;
                try {
                    AA = CreateOleObject("AcroExch.App");
                } catch (...) {
                    // RestoreOldPrinter();
                    StopSaveBitmap2Tiff();
                    StopGdiplus();
                    Application->MessageBox("�� ���� ���������� ����� � Adobe Acrobat Standart", "������!", 0);
                    return false;
                }
            }
        } else {                // ����� ������ ������� � ������������ �� Acrobat
            if (!pc->KillAcrobatDeath()) {
                delete pc;
                return false;
            }
        }
    }
    if (NeedEA) {
        try {
            EA = CreateOleObject("Excel.Application");
        } catch (...) {
            // RestoreOldPrinter();
            StopSaveBitmap2Tiff();
            StopGdiplus();
            Application->MessageBox("�� ���� ���������� ����� � Excel", "������!", 0);
            if (pc != NULL) delete pc;
            return false;
        }
        EA.OlePropertySet("Visible", true);
    }
    if (NeedWA) {
        try {
            WA = CreateOleObject("Word.Application");
            // WA = CreateOleObject("Word.Application.11");
        } catch(...) {
            // RestoreOldPrinter();
            StopSaveBitmap2Tiff();
            StopGdiplus();
            Application->MessageBox("�� ���� ���������� ����� � Excel", "������!", 0);
            if (pc != NULL) delete pc;
            return false;
        }
        WA.OlePropertySet("Visible", true);
    }

    // ------------ ����� ������� ��� ���������� -----------------
    StartGdiplus();
    IniSaveBitmap2Tiff(filn);

    stop++;
    NeedBreakProcess = false;
    for (i = 0; i < fl->Nrecs && !NeedBreakProcess; i++) {
        Application->ProcessMessages();
        r = fl->Rec1(i);
        afiln = (String)r->path + r->filn;
        wsprintf(buf, "(%d �� %d) %s", i + 1, fl->Nrecs, r->filn.c_str());
        // ProcessInfoFiln(r->filn.c_str());
        ProcessInfoFiln(buf);
        ProcessInfoAction("");
        ext = ExtractExt(r->filn);
        if (ext == "xls" || ext == "XLS") {
            p0 += Excel2Tiff(pc, EA, afiln, i < f1 ? 0 : p0);
        } else if (ext == "doc" || ext == "DOC") {
            p0 += Word2Tiff(pc, WA, afiln, i < f1 ? 0 : p0);
        } else if (ext == "pdf" || ext == "PDF") {
            if (!NeedAA) continue;
            p0 += PDF2Tiff(pc, AA, afiln, i < f1 ? 0 : p0);
        } else if (ext == "tif" || ext == "TIFF" || ext == "tiff" || ext == "TIFF"
                || ext == "jpg" || ext == "JPG"
                || ext == "jpeg" || ext == "JPEG"
        ) {
            // Application->MessageBox("��������� ��������� ������ �������� �� �����������!", "���� �����!", 0);
            // break;
            p0 += Tiff2Tiff(afiln, i < f1 ? 0 : p0);
        }
    }
    StopSaveBitmap2Tiff();
    StopGdiplus();
    // RestoreOldPrinter();
    if (NeedEA) {
        EA.OleProcedure("Quit");
        EA = Unassigned;
    }
    if (NeedWA) {
        WA.OleProcedure("Quit");
        WA = Unassigned;
    }
    if (NeedAA && pc == NULL) {
        int rest = AA.OleFunction("GetNumAVDocs");
        if (rest == 0) {
            AA.OleFunction("Exit");
            AA = Unassigned;
            TerminateProcess(hProcess , 0);

            FILETIME lpCreationTime;	// when the process was created
            FILETIME lpExitTime;	    // when the process exited
            FILETIME lpKernelTime;	    // time the process has spent in kernel mode
            FILETIME lpUserTime; 	    // time the process has spent in user mode
            int i;
            for (i = 0; i < 10; i++) {
                aWait(50);
                GetProcessTimes(hProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
                if (lpExitTime.dwLowDateTime != 0) break;
            }
        }
    }

    if (pc != NULL) delete pc;

    return true;
}















//===============================================================================
//                          ���������� ����� ����������                         |
//===============================================================================
#include "����������������.cpp"

void __fastcall Contents2Word(char *filn, TFileList *fl, int f1, int p0) {
//-------------------------------------------------------------------------------
//                  ������� ���������� ����� � ���������� ����                  |
// ���� ��������� fl->Rec0(i)->oname � ->pi.pcount                              |
// �������� bold ������� �� ->oname - ���� �� ��� "���������" ��� "�������"     |
//-------------------------------------------------------------------------------
    TFileListRec *r;
    Variant WA, Docs, Doc, Selection, Tables, Table, Cell, Font;
    HANDLE chan;
    String afiln, mes;
    int i, npage;
    char buf[200];
    bool bold;

    try {
        WA = CreateOleObject("Word.Application");
    } catch(...) {
        Application->MessageBox("�� ���� ���������� ����� � Word", "������!", 0);
        return;
    }
    WA.OlePropertySet("Visible", true);

    DeleteFile(filn);
    chan = CreateFile(filn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        CloseHandle(chan);
        mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
        return;
    }

    // afiln = EraseFilnFromPath(filn) + "tmp.doc";
    afiln = filn;    
    if (SaveFile(true, afiln.c_str(), Contents, sizeof(Contents)) != 0) {
        return;
    }
    Docs = WA.OlePropertyGet("Documents");
    // Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
    Doc = Docs.OleFunction("Open", afiln.c_str(), false, false);
    Selection = WA.OlePropertyGet("Selection");
    for (i = 0, npage = p0, bold = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        if (i >= f1) {
            wsprintf(buf, "%d", npage);
            npage += r->pi.pcount;
        } else {
            *buf = 0;
        }
        if (r->oname.IsEmpty()) {
            mes = TrimExt(r->filn);
        } else {
            mes = r->oname;
        }
        Selection.OleProcedure("MoveRight", 12);        // wdCell
        Selection.OleProcedure("TypeText", mes.c_str());
        if (IsSubstr(mes.c_str(), "���������", true) ||
            IsSubstr(mes.c_str(), "�������", true)
        ) {
            if (!bold) {
                Tables = Doc.OlePropertyGet("Tables");
                Table = Tables.OleFunction("Item", 1);
                Cell = Table.OleFunction("Cell", i + 1, 2);
                Cell.OleProcedure("Select");
                Font = Selection.OlePropertyGet("Font");
                Font.OlePropertySet("Bold", true);
            }
            bold = true;
        } else {
            if (bold) {
                Tables = Doc.OlePropertyGet("Tables");
                Table = Tables.OleFunction("Item", 1);
                Cell = Table.OleFunction("Cell", i + 1, 2);
                Cell.OleProcedure("Select");
                Font = Selection.OlePropertyGet("Font");
                Font.OlePropertySet("Bold", false);
            }
            bold = false;
        }
        Selection.OleProcedure("MoveRight", 12);        // wdCell
        if (*buf != 0) Selection.OleProcedure("TypeText", buf);
        if (i < fl->Nrecs - 1) Selection.OleProcedure("MoveRight", 12);        // wdCell
    }

/*
    try {
        Doc.OleProcedure("SaveAs", filn);
        DeleteFile(afiln);
    } catch (...) {
        mes = (String)"�� ���� ��������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
    }
*/
}


void __fastcall CreateContents(char *filn, TFileList *fl, int f1, int p0) {
//-------------------------------------------------------------------------------
//                              ���������� ����������                           |
//-------------------------------------------------------------------------------
    Variant EA, WA, AA, Docs, Doc, ActiveSheet, HPageBreaks, VPageBreaks;
    Variant Selection, Font, Tables, Table, Cell;
    TFileListRec *r;
    String ext, mes, afiln;
    PROCESS_INFORMATION pi;
    HANDLE chan;
    HANDLE hProcess;
    int i, npage;
    char buf[200];
    bool NeedEA, NeedWA, NeedAA;
    bool error, bold;

    // ??? ���������, ����� �� ������� ���� filn

/*
    TPDFCreator *pc = new TPDFCreator();
    if (!pc->ok) {
        delete pc;
        pc = NULL;
    }

    if (pc == NULL) {
        if (!SwitchToMODI()) return;
        if (!SetupMODI(2)) {
            RestoreOldPrinter();
            return;
        }
    }
*/

    TPDFCreator *pc = new TPDFCreator();
    if (!pc->ok) {
        delete pc;
        Application->MessageBox("��� ������ ��������� ����������, ����� �� ���������� ��� ���������� ������� PDFCreator!", "������!", 0);
        return;
    }

    // StartGdiplus();
    // IniSaveBitmap2Tiff(filn);
    stop++;
    for (i = 0, NeedEA = false, NeedWA = false, NeedAA = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        ext = ExtractExt(r->filn);
        if (ext == "xls" || ext == "XLS") NeedEA = true;
        else if (ext == "doc" || ext == "DOC") NeedWA = true;
        else if (ext == "pdf" || ext == "PDF") NeedAA = true;
        else if (ext == "tif" || ext == "TIF") ;
        else if (ext == "tiff" || ext == "TIFF") ;
        else if (ext == "jpg" || ext == "JPG") ;
        else if (ext == "jpeg" || ext == "JPEG") ;
        else {
            mes = (String)"����� ������� \"" + ext + "\" �� ��������������";
            Application->MessageBox(mes.c_str(), "���� ������ ������!", 0);
        }
    }
    if (NeedAA) {
        if (pc == NULL) {                           // PDFCreator �� ����������!
            if (!TestAcrobatStandartPresence()) {
                NeedAA = false;
                Application->MessageBox("��� ��������� ������ PDF ���������, ����� � ������� ��� ���������� Adobe Acrobat Standart", "�� ����!", 0);
                // RestoreOldPrinter();
                if (pc != NULL) delete pc;
                return;
            } else {
                StartAcrobat(pi, "");
                hProcess = pi.hProcess;
                try {
                    AA = CreateOleObject("AcroExch.App");
                } catch (...) {
                    // RestoreOldPrinter();
                    Application->MessageBox("�� ���� ���������� ����� � Adobe Acrobat Standart", "������!", 0);
                    if (pc != NULL) delete pc;
                    return;
                }
            }
        } else {
            if (!pc->KillAcrobatDeath()) {
                delete pc;
                return;
            }
        }
    }
    if (NeedEA) {
        try {
            EA = CreateOleObject("Excel.Application");
        } catch (...) {
            // RestoreOldPrinter();
            //StopSaveBitmap2Tiff();
            //StopGdiplus();
            Application->MessageBox("�� ���� ���������� ����� � Excel", "������!", 0);
            if (pc != NULL) delete pc;
            return;
        }
        EA.OlePropertySet("Visible", true);
    }
    if (NeedWA) {
        try {
            WA = CreateOleObject("Word.Application");
            // WA = CreateOleObject("Word.Application.11");
        } catch(...) {
            // RestoreOldPrinter();
            //StopSaveBitmap2Tiff();
            //StopGdiplus();
            Application->MessageBox("�� ���� ���������� ����� � Word", "������!", 0);
            if (pc != NULL) delete pc;
            return;
        }
        WA.OlePropertySet("Visible", true);
    }

    StartGdiplus();
    for (i = 0, error = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        afiln = (String)r->path + r->filn;
        wsprintf(buf, "(%d �� %d) %s", i + 1, fl->Nrecs, r->filn.c_str());
        // ProcessInfoFiln(r->filn.c_str());
        ProcessInfoFiln(buf);
        ProcessInfoAction("");
        ext = ExtractExt(r->filn);
        if (ext == "xls" || ext == "XLS") {
            Docs = EA.OlePropertyGet("Workbooks");
            ProcessInfoAction("�������� �����");
            try {
                Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
            } catch(...) {
                mes = (String)"�� ���� ������� ���� \"" + afiln + "\"";
                Application->MessageBox(mes.c_str(), "������!", 0);
                error = true;
                break;
            }
            ProcessInfoAction("��������� �����");
            ProcessSmeta(r, Doc);  // ���� ����� - ����������� ������� � ������
            ProcessInfoAction("��������� �� ��������");
            //Paginate(Doc);      // ��������� �� ��������
            //ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
            //HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
            //VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
            //r->pi.pcount = (HPageBreaks.OlePropertyGet("Count") + 1) * (VPageBreaks.OlePropertyGet("Count") + 1);
            r->pi.pcount = Paginate(Doc);
            Doc.OleProcedure("Close", false);
        } else if (ext == "doc" || ext == "DOC") {
            Docs = WA.OlePropertyGet("Documents");
            ProcessInfoAction("�������� �����");
            try {
                Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
            } catch(...) {
                mes = (String)"�� ���� ������� ���� \"" + afiln + "\"";
                Application->MessageBox(mes.c_str(), "������!", 0);
                error = true;
                break;
            }
            r->pi.pcount = Doc.OleFunction("ComputeStatistics", 2, true);        // ������ ������� ���������� �������
            Doc.OleProcedure("Close", false);
        } else if (ext == "pdf" || ext == "PDF") {
            if (pc == NULL) {
                Variant pddoc, avdoc;
                int j, prev, k;
                chan = CreateFile(afiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                if (chan == INVALID_HANDLE_VALUE) {
                    mes = (String)"�� ���� ������� ���� \"" + afiln + "\"";
                    Application->MessageBox(mes.c_str(), "������!", 0);
                    error = true;
                    break;
                }
                CloseHandle(chan);
                prev = AA.OleFunction("GetNumAVDocs");
                StartAcrobat(pi, afiln);
                for (j = 0; j < 10; j++) {
                    aWait(50);
                    k = AA.OleFunction("GetNumAVDocs");
                    if (k != prev) break;
                }
                avdoc = AA.OleFunction("GetActiveDoc");
                pddoc = avdoc.OleFunction("GetPDDoc");
                r->pi.pcount = pddoc.OleFunction("GetNumPages");
                avdoc.OleFunction("Close", 1);
            } else {
                int cnt = pc->CountPages(afiln);
                if (cnt < 0) {
                    cnt = 0;
                    r->rem = "������ �������� �����";
                } else {
                    r->pi.pcount = cnt;
                }
            }
        } else if (ext == "tif" || ext == "TIF" || ext == "tiff" || ext == "TIFF"
                || ext == "jpg" || ext == "JPG"
                || ext == "jpeg" || ext == "JPEG"
        ) {
            ProcessInfoAction("�������� �����");
            if (LoadTiffFile(afiln.c_str()) == NULL) {
                r->rem = "������ �������� �����";
            }
            r->pi.pcount = LoadTiffFileNpages;
            ReleaseTiffFile();
        }

    }
    ProcessInfoAction("");
    StopGdiplus();

    if (NeedAA) {
        int rest = AA.OleFunction("GetNumAVDocs");
        if (rest == 0) {
            AA.OleFunction("Exit");
            AA = Unassigned;
            TerminateProcess(hProcess , 0);

            FILETIME lpCreationTime;	// when the process was created
            FILETIME lpExitTime;	    // when the process exited
            FILETIME lpKernelTime;	    // time the process has spent in kernel mode
            FILETIME lpUserTime; 	    // time the process has spent in user mode
            int i;
            for (i = 0; i < 10; i++) {
                aWait(50);
                GetProcessTimes(hProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
                if (lpExitTime.dwLowDateTime != 0) break;
            }
        }
    }

    // RestoreOldPrinter();
    if (NeedEA) {
        EA.OleProcedure("Quit");
        EA = Unassigned;
    }
    if (NeedWA) {
        WA.OleProcedure("Quit");
        WA = Unassigned;
    }

    //
    //if (!EditContents(fl, f1, p0)) {
    //    return;
    //}

    stop++;

/*
    if (!error) {
        afiln = EraseFilnFromPath(filn) + "tmp.doc";
        if (SaveFile(true, afiln.c_str(), Contents, sizeof(Contents)) != 0) {
            if (NeedWA) {
                WA.OleProcedure("Quit");
                WA = Unassigned;
            }
            return;
        }
        DeleteFile(filn);
        chan = CreateFile(filn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (chan != INVALID_HANDLE_VALUE) {
            CloseHandle(chan);
            WA.OleProcedure("Quit");
            WA = Unassigned;
            mes = (String)"�� ���� ������� ���� \"" + filn + "\"";
            Application->MessageBox(mes.c_str(), "������!", 0);
            return;
        }
        Docs = WA.OlePropertyGet("Documents");
        Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
        Selection = WA.OlePropertyGet("Selection");
        for (i = 0, npage = p0, bold = false; i < fl->Nrecs; i++) {
            r = fl->Rec1(i);
            if (i >= f1) {
                wsprintf(buf, "%d", npage);
                npage += r->pi.pcount;
            } else {
                *buf = 0;
            }
            if (r->oname.IsEmpty()) {
                mes = TrimExt(r->filn);
            } else {
                mes = r->oname;
            }
            Selection.OleProcedure("MoveRight", 12);        // wdCell
            Selection.OleProcedure("TypeText", mes.c_str());
            if (IsSubstr(mes.c_str(), "���������", true) ||
                IsSubstr(mes.c_str(), "�������", true)
            ) {
                if (!bold) {
                    Tables = Doc.OlePropertyGet("Tables");
                    Table = Tables.OleFunction("Item", 1);
                    Cell = Table.OleFunction("Cell", i + 1, 2);
                    Cell.OleProcedure("Select");
                    Font = Selection.OlePropertyGet("Font");
                    Font.OlePropertySet("Bold", true);
                }
                bold = true;
            } else {
                if (bold) {
                    Tables = Doc.OlePropertyGet("Tables");
                    Table = Tables.OleFunction("Item", 1);
                    Cell = Table.OleFunction("Cell", i + 1, 2);
                    Cell.OleProcedure("Select");
                    Font = Selection.OlePropertyGet("Font");
                    Font.OlePropertySet("Bold", false);
                }
                bold = false;
            }
            Selection.OleProcedure("MoveRight", 12);        // wdCell
            if (*buf != 0) Selection.OleProcedure("TypeText", buf);
            if (i < fl->Nrecs - 1) Selection.OleProcedure("MoveRight", 12);        // wdCell
        }
    }

    try {
        Doc.OleProcedure("SaveAs", filn);
        DeleteFile(afiln);
    } catch (...) {
        mes = (String)"�� ���� ��������� ���� \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "������!", 0);
    }
*/
    //if (NeedWA) {
    //    WA.OleProcedure("Quit");
    //    WA = Unassigned;
    //}

    if (!error) {
        Contents2Word(filn, fl, f1, p0);
    }

    if (pc != NULL) delete pc;
}

//    Selection.TypeText text:="kjlkjfa" & vbTab & "999"
//    Selection.TypeParagraph

// GetLocaleInfo TCompressionStream














//===============================================================================
//                              ������������                                    |
//===============================================================================
static bool LegalPictureExt(String ext) {
//-------------------------------------------------------------------------------
//          ���� ���������� - ��������� ��������, �� ���������� true            |
//-------------------------------------------------------------------------------
    static char *legal[] = {
        "emf", "wmf", "jpg", "jpeg", "jfif", "jpe", "png", "bmp", "dib", "bmz", "gif",
        "gfa", "emz", "wmz", "pcz", "tif", "tiff", "cdr", "cgm", "eps", "pct", "pict",
        "wpg",
        0
    };
    int i;
    char *p = ext.c_str();
    for (i = 0; legal[i] != 0; i++) {
        if (NospaceStrcmp(p, legal[i], true) == 0) return true;
    }
    return false;
}

static char * __fastcall SkipRusWord(char *start) {
//-------------------------------------------------------------------------------
//       ���������� ������� ����� - �����, ��������� ������ �� ������� ����     |
// ���������� ��������� �� ����� ����� - ��� �� start, ���� ��� ��� ��� ����    |
//-------------------------------------------------------------------------------
    char c;
    while(1) {
        c = *start;
        if (!(c >= '�' && c <= '�' || c >= '�' && c <= '�')) break;
        start++;
    }
    return start;
}

static bool __fastcall IsCapitalRus(char c) {
//-------------------------------------------------------------------------------
//                          ���� ����� - ��������� �������                      |
//-------------------------------------------------------------------------------
    return c >= '�' && c <= '�';
}

static char Initials[3];

static char * __fastcall SkipRusInitials(char *start) {
//-------------------------------------------------------------------------------
//          ���� ������� ��������, �� ���������� ����� ����� ���                |
// � ����� � Initials ������ ���� ��������                                      |
//-------------------------------------------------------------------------------
    char *p = SkipSpaces(start);
    Initials[0] = 0; Initials[1] = 0; Initials[2] = 0;
    if (!IsCapitalRus(*p)) return start;
    Initials[0] = *p++;
    if (*p == '.') p++;
    p = SkipSpaces(p);
    if (!IsCapitalRus(*p)) return start;
    Initials[1] = *p++;
    if (*p != '.') p++;
    return SkipSpaces(p + 1);
}

static char * __fastcall SkipFamIni(char *start) {
//-------------------------------------------------------------------------------
//              ���� ������� ������� � ��������� �������� - ����������          |
// ���������� ��������� �� ����� ����� - ��� �� start, ���� ��� �� ��           |
//-------------------------------------------------------------------------------
    char *p, *q;
    bool was;
    p = SkipRusInitials(start);
    was = (p != start);
    q = SkipRusWord(p);
    if (p == q) return start;
    if (!was) q = SkipRusInitials(q);
    return SkipSpaces(q);
}


String __fastcall NormalizeFamIni(char *start) {
//-------------------------------------------------------------------------------
//              ���������� ��������������� ������� � ����������                 |
// ��������������� - ��� ��� �������� � �����, �������� ����� �������           |
// ���� ����� ���� �� ������� � ���������� - ���������� ������ ������           |
//-------------------------------------------------------------------------------
    String fam, ini;
    char *p, keep;

    ini = "";
    p = SkipRusInitials(start);
    if (p != start) ini = Initials;
    start = SkipSpaces(p);
    p = SkipRusWord(start);
    if (p == start) return "";
    keep = *p; *p = 0; fam = start; *p = keep;
    if (!ini.IsEmpty()) {
        fam += ini;
        return fam;
    }
    start = SkipRusInitials(p);
    if (p != start) {
        ini = Initials;
        fam += ini;
    }
    return fam;
}

static String __fastcall FindFamily(char *text, String *Fam, int Count) {
//-------------------------------------------------------------------------------
//                              ���� ������� � ������                           |
// ���� ������� - ���������� ��� �����, ����� - ������ ������                   |
//-------------------------------------------------------------------------------
    String tFam, fFam;
    int i;
    char *p, *q, *n2, keep, *ttext;

    ttext = text;
    for (n2 = text; *n2 != 0;) {
        n2 = SkipFamIni(text);
        if (n2 == text) {
            text++;
            continue;
        }
        tFam = NormalizeFamIni(text);
        for (i = 0; i < Count; i++) {
            fFam = NormalizeFamIni(Fam[i].c_str());
            if (strcmp(fFam.c_str(), tFam.c_str()) == 0) return Fam[i];
        }
        text = n2;
    }

    text = ttext;
    for (n2 = text; *n2 != 0;) {
        n2 = SkipRusWord(text);
        if (n2 == text) {
            text++;
            continue;
        }
        for (i = 0; i < Count; i++) {
            p = Fam[i].c_str();
            q = text;
            while(1) {
                if (*p == '.' && q == n2) return Fam[i];
                if (*p != *q) break;
                p++;
                q++;
            }
        }
        text = n2;
    }
    return "";
}

static String __fastcall FindFamilyNext(char *n1, int patlen, Variant Cells, int row, int col, String *Fam, int Count) {
//-------------------------------------------------------------------------------
//      ���� <�������� _____> <������ ������> <�������>, �� �� � ����������     |
// patlen - ����� ������, �������������� �������                                |
//-------------------------------------------------------------------------------
    Variant Cell;
    String text;
    int i;
    char *p;

    p = n1 + patlen;                 // �������� �������� __________
    while(*p == ' ') p++;
    while(*p == '_') p++;
    while(*p == ' ') p++;
    if (*p != 0) return "";

    for (i = 0, col++; i < 20; i++, col++) {
        Cell = Cells.OlePropertyGet("Item", row, col);
        text = Cell.OlePropertyGet("Value");
        if (!text.IsEmpty()) break;
    }

    return FindFamily(text.c_str(), Fam, Count);
}


/*
static TRect __fastcall GetTextRect(Variant Range, int from, String text) {
//-------------------------------------------------------------------------------
//              ���������� ������������� ������ text � �������� ������          |
// Range - ������, � ������� ��������� ������ �����                             |
// from - � ����� ����� ������ ��������� ����� text                             |
//  ��������� �� ����������� 529.5 ������ == 700 ��������                       |
//  � ������ ����� � ������������ ������                                        |
//-------------------------------------------------------------------------------
    Variant MergeArea, Width, HorAl, AText;
    // TCanvas *c =
    TRect r;
    int w, horal;

    MergeArea = Range.OlePropertyGet("MergeArea");
    Width = MergeArea.OlePropertyGet("Width");
    w = Width;
    HorAl = Range.OlePropertyGet("HorizontalAlignment");
    horal = HorAl;

    return r;
}
*/


struct TSubstrParam {                   // ���������, ������������ IsSubstrList
    int pos;                            // ������� � ������, ��� ������ ���������
    int index;                          // ������ ���������� (����� ����� � list)
};


static TSubstrParam __fastcall FindSubstrList(String str, String *list, int Count, bool fam) {
//-------------------------------------------------------------------------------
//                � ������ str ���� �����-������ ��������� list                 |
// Count - ���������� ��������                                                  |
// fam - �� ������� �� ���. �.�. ����� ������ - ����� ���������� � ��� ������-  |
//   ���� ���������� �����. � ���� ����� �������� ����� ��������, ��            |
//   TSubstrParam.pos ���������� �����                                          |
// �� ������ ���� TSubstrParam.pos == -1, �� �� �����                           |
//-------------------------------------------------------------------------------
    TSubstrParam res;
    String afam;
    int i, j, pos;
    char *ps;

    res.pos = -1;
    res.index = -1;

    for (i = 0; i < Count; i++) {
        afam = list[i];
        if (fam) afam = TrimExt(afam);
        pos = str.AnsiPos(afam);
        if (pos > 0) {
            pos--;
            res.pos = pos;
            res.index = i;
            if (fam) {                  // ���� ����� �������� - ������� ����� � �����
                ps = str.c_str();
                for (j = pos - 1; j >= 0; j--) {
                    if (ps[j] == ' ' || ps[j] == '.') continue;
                    if (ps[j] >= '�' && ps[j] <= '�') {
                        res.pos = j;
                        continue;
                    }
                    break;
                }
            }
            return res;
        }
    }

    return res;
}


//struct TSubscribeParams {           // ��������� ��� ����������
//    TTextX Post;                    // ������� ������ ��������� ����������
//    int PostCol;                    // ����� �������, � ������� ������� ��������� ���������
//    String PostName;                // ������������ ��������� ����������
//    TTextX Name;                    // ������� ������ ������� ����������, ���� ����� ��������, �� �����
//    int NameCol;                    // ����� �������, � ������� ������� ������� ����������
//    String FamFiln;                 // ��� ����� ������� ����������
//};

static void __fastcall BuildSubscribeParams(TSubscribeParams &sp, Variant Sheet, TPoint pt, String *Fam, int Count) {
//-------------------------------------------------------------------------------
//                  ���� � ������ pt.y � �������� 1...pt.x ����������           |
// sp - ��������� ���, ��� �������                                              |
// Fam - ������ ���� ������ ������� (� ������������!)                           |
// Count - ���������� �������                                                   |
//-------------------------------------------------------------------------------
    String patterns[] = {                   // ������ ���������� �����������
        "��������:",
        "��������",
        "��������:",
        "��������",
        "������� ������� �������:",
        "������� ������� �������",
        "��������� �������� ������:",
        "��������� �������� ������",
        ""
    };
    String Adr, RangeText;
    Variant Range, VRangeText;
    TSubstrParam sup;
    int i, col, PatCount;

    stop++;
    // �������� PatCount
    for (i = 0;;i++) {
        if (patterns[i].Length() == 0) {
            PatCount = i;
            break;
        }
    }

    sp.PostCol = sp.NameCol = -1;

    for (col = 1; col <= pt.x; col++) {
        Adr = AddressFromCell(pt.y, col);
        Range = Sheet.OlePropertyGet("Range", Adr.c_str());
        VRangeText = Range.OlePropertyGet("Text");
        RangeText = VRangeText;
        sup = FindSubstrList(RangeText, patterns, PatCount, false);     // fam = false
        if (sup.pos != -1) {
            sp.PostCol = col;
            sp.PostName = patterns[sup.index];
            GetTextX(sp.Post, Range, sup.pos, sp.PostName);
        }
        sup = FindSubstrList(RangeText, Fam, Count, true);              // fam = true
        if (sup.pos != -1) {
            sp.NameCol = col;
            sp.FamFiln = Fam[sup.index];
            GetTextX(sp.Name, Range, sup.pos, TrimExt(sp.FamFiln));
        }
    }

    // TTextX textX;
    // GetTextX(textX, Range, 0, thepattern);

}


struct TSheetPictures {                     // �������� ��� SetSubscription
    Variant Pictures;                       // ������ �� ��������
    TRect *Rects;                           // �������������� ��������
    int Count;                              // ����������
};


static void __fastcall SetSubscription(Variant Sheet, TSheetPictures *pic, TSubscribeParams &sp, int y, String SPath) {
//-------------------------------------------------------------------------------
//                                  ������ �������                              |
// ��������, ��� ������ ��� ��� ����� ����������� BuildSubscribeParams          |
// ���� ������� ��� ����� �������� ��� ����, �� �� ������                       |
// pic - ��� ��� �������� ���� ��������, ����� ������� ����� ��� ���� �������   |
// SPath - ��� ����� ����� � ���������                                          |
// y - ����� ������                                                             |
//-------------------------------------------------------------------------------
    Variant Pictures = Sheet.OlePropertyGet("Pictures");
    Variant Picture, Cells, Cell;
    String filn = SPath + "\\" + sp.FamFiln;
    double w, h, k;
    int i, v, ih;

    // �������� �������������, � ������� ���� ������� �������
    int left, top, right, bottom;               // ��� - ��������� ��������������
    Cells = Sheet.OlePropertyGet("Cells");
    Cell = Cells.OlePropertyGet("Item", y, sp.PostCol);
    top = Cell.OlePropertyGet("Top");
    v = Cell.OlePropertyGet("Height");
    bottom = top + v;
    left = sp.Post.x1;
    right = sp.Name.x0;

    // ��������, ��� �� ��� ����-������ �������� �� �������
    for (i = 0; i < pic->Count; i++) {
        if (pic->Rects[i].Width() > 120 || pic->Rects[i].Height() > 40) {   // ��������� �� �������
            continue;
        }
        if (pic->Rects[i].right < left || pic->Rects[i].left > right) {     // �������� ����� ��� ������
            continue;
        }
        if (pic->Rects[i].top > bottom || pic->Rects[i].bottom < top) {     // �������� ���� ��� ����
            continue;
        }
        return;                                                             // �������� ������������ � ���������������!
    }

    // ������ ������ - ����� ������� �������...
    Picture = Pictures.OleFunction("Insert", filn.c_str());
    Picture.OlePropertySet("Name", sp.FamFiln.c_str());
    w = Picture.OlePropertyGet("Width");
    h = Picture.OlePropertyGet("Height");
    k = 95. / w;
    h *= k;
    Picture.OlePropertySet("Width", 95);
    Picture.OlePropertySet("Height", h);

    Picture.OlePropertySet("Left", (left + right) / 2 - 47);
    ih = Picture.OlePropertyGet("Height");
    Picture.OlePropertySet("Top", bottom - ih - 4);

}


//struct TSubscribeParams {           // ��������� ��� ����������
//    TTextX Post;                    // ������� ������ ��������� ����������
//    int PostCol;                    // ����� �������, � ������� ������� ��������� ���������
//    String PostName;                // ������������ ��������� ����������
//    TTextX Name;                    // ������� ������ ������� ����������, ���� ����� ��������, �� �����
//    int NameCol;                    // ����� �������, � ������� ������� ������� ����������
//    String FamFiln;                 // ��� ����� ������� ����������
//};


// TPoint __fastcall FindLastCellSheet(Variant Sheet);


static String __fastcall OrphanSubscriptionMessage(Variant ActiveSheet, int LastSubscription) {
//-------------------------------------------------------------------------------
//          ���� ��������� ������� ���� ���������� ����������� �������          |
// ��� ����� ��������� ������������ ������� � ��������� �������� ���� �����     |
// �� ���������� ���� ��������� "������� ��������"                              |
// ��������, ��� ������� ������� - PDFCreator, ����� ��������� �� ��������      |
//    ����� �� ���������������                                                  |  
//-------------------------------------------------------------------------------
    Variant Doc = ActiveSheet.OlePropertyGet("Application");
    Variant Windows = Doc.OlePropertyGet("Windows");
    Variant Window = Windows.OlePropertyGet("Item", 1);
    Variant PageSetup, Range, Width, HPageBreaks, VPageBreaks;
    TPoint pt, adr;
    String address, text;
    int view = Window.OlePropertyGet("View");
    int ori, width, HCount;
    bool empty_sheet;
    if (view == 2) {                                // ��� ������� �� ��������
        empty_sheet = false;
        ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
        PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
        ori = PageSetup.OlePropertyGet("Orientation");
        // SetupMODI(ori);
        Window.OlePropertySet("View", 1);           // ��� - �������� �������
        Window.OlePropertySet("Zoom", 100);         // ��� - 100%
        Window.OlePropertySet("View", 2);           // ��� - �������� �������
    } else {
        ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
        PageSetup = ActiveSheet.OlePropertyGet("PageSetup");
        PageSetup.OlePropertySet("Zoom", false);
        PageSetup.OlePropertySet("FitToPagesTall", false);
        PageSetup.OlePropertySet("FitToPagesWide", 1);
        // pt = FindLastCell(Doc);
        pt = FindLastCellSheet(ActiveSheet);
        if (pt.x + pt.y == 0) {
            empty_sheet = true;
        } else {
            empty_sheet = false;
            address = (String)"$A$10:" + AddressFromCell(10, pt.x);
            Range = ActiveSheet.OlePropertyGet("Range", address.c_str());
            Width = Range.OlePropertyGet("Width");
            width = Width;
            if (width <= 600) ori = 1; else ori = 2;            // 1 - portrait, 2 - landscape
            PageSetup.OlePropertySet("Orientation", ori);
            // SetupMODI(ori);
            Window.OlePropertySet("View", 2);           // ��� - �������� �������
            Window.OlePropertySet("Zoom", 100);         // ��� - 100%
        }
    }

    if (empty_sheet) {
        return "���������� ������ ��������!";
    }

    if (!empty_sheet) {
        HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
        VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
        HCount = HPageBreaks.OlePropertyGet("Count");
        if (HCount > 0) {
            try {
                HPageBreaks.OlePropertyGet("Item", HCount);
            } catch(...) {
                HCount--;
            }
        }
    }


    int y0, x, y, cnt;

    if (HCount > 0) {
        Variant HPageBreak = HPageBreaks.OlePropertyGet("Item", HCount);
        Variant Location = HPageBreak.OlePropertyGet("Location");
        Variant Cells = Location.OlePropertyGet("Cells");
        Variant Address = Cells.OlePropertyGet("Address");
        address = Address;
        adr = CellFromAddress(address);
        y0 = adr.y;
    } else {
        y0 = 1;
    }

    stop++;
    for (y = y0, cnt = 0; y < LastSubscription; y++) {
        for (x = 1; x < 10; x++) {
            address = AddressFromCell(y, x);
            Range = ActiveSheet.OlePropertyGet("Range", address.c_str());
            text = Range.OlePropertyGet("Text");
            if (text.Length() > 0) {
                cnt++;
                break;
            }
        }
        if (cnt > 5) break;
    }

    return cnt > 5 ? "" : "��������, ������� �������!";
}


static String __fastcall SubscribeExcelSheet(Variant Sheet, String SPath, String *Fam, int Count) {
//-------------------------------------------------------------------------------
//                          ������ ������� � ��������� �����                    |
// SPath - ��� ����� ����� � ���������                                          |
// Fam - ������ ���� ������ ������� (� ������������!)                           |
// Count - ���������� �������                                                   |
// �� ������ - ���������� � ������, ���� ���� ��� ������� ������                |
//   ������: ���� ������ ����, �� ������. ���� ���������, �� ������ ������:     |
//           "������!", ����� ������ ������ ����� "\015\012"                    |
//-------------------------------------------------------------------------------
    TSheetPictures pic;
    TPoint pt = FindLastCellSheet(Sheet);
    String res;                             // ������� ��������� �� ������
    String mes, SheetName;

    int i, j, v;
    char buf[200];

    // �������� �������������� ��� ������������ ��������
    pic.Pictures = Sheet.OlePropertyGet("Pictures");
    pic.Count = pic.Pictures.OlePropertyGet("Count");
    if (pic.Count > 0) {
        pic.Rects = new TRect[Count];
        for (i = 0; i < pic.Count; i++) {
            Variant Picture = pic.Pictures.OlePropertyGet("Item", i + 1);
            pic.Rects[i].left = Picture.OlePropertyGet("Left");
            pic.Rects[i].top = Picture.OlePropertyGet("Top");
            v = Picture.OlePropertyGet("Width");
            pic.Rects[i].right = pic.Rects[i].left + v;
            v = Picture.OlePropertyGet("Height");
            pic.Rects[i].bottom = pic.Rects[i].top + v;
        }
    }

    // ������ ������ ����� ��������
    res = ""; mes = "";
    SheetName = Sheet.OlePropertyGet("Name");
    TSubscribeParams sp;
    int LastSubscription = pt.y;
    for (i = 0; i < 25 && pt.y > 0; i++, pt.y--) {
        BuildSubscribeParams(sp, Sheet, pt, Fam, Count);        // ���� "�������� __________ �����-��"
        sprintf(buf, "���� \"%s\" ������ %d: ", SheetName.c_str(), pt.y);
        if (sp.PostCol != -1 && sp.NameCol == -1) {             // ����� ������ "��������"
            if (res.Length() != 0) res += "\015\012";
            res += buf;
            sprintf(buf, "������ ��������� \"%s\", �� �� ������� ������� �� ������ �������", sp.PostName.c_str());
            res += buf;
            continue;
        } else if (sp.PostCol == -1 && sp.NameCol != -1) {      // ����� ������ "�����-��"
            if (res.Length() != 0) res += "\015\012";
            res += buf;
            sprintf(buf, "������� ������� \"%s\", �� �� ������� ��������� �� ������ ����������", TrimExt(sp.FamFiln).c_str());
            res += buf;
            continue;
        } else if (sp.PostCol == -1 && sp.NameCol == -1) {      // ������ �� ����� - ���� � ������ ������
            continue;
        }
        SetSubscription(Sheet, &pic, sp, pt.y, SPath);          // ����� - �����������
        LastSubscription = pt.y;
    }

    if (pic.Count > 0) delete[] pic.Rects;

    String orphan = OrphanSubscriptionMessage(Sheet, LastSubscription);
    if (orphan.Length() > 0) {
        if (res.Length() != 0) res += "\015\012";
        res += orphan;
    }

    return res;

}


// static String __fastcall Subscribe(Variant Doc, TPoint pt, String SPath, String *Fam, int Count) {
static String __fastcall Subscribe(Variant Doc, String SPath, String *Fam, int Count) {
//-------------------------------------------------------------------------------
//                          �������� ��������� �������                          |
// ��������, ��� ���� - �����                                                   |
// SPath - ��� ����� ����� � ���������                                          |
// �� ������ 0 - ��������� ���������, 1 - ��� ����, 2 - �������� ����� �������  |
//    3 - ������� �� �������                                                    |
// ��, �� ������ - ���������� � ������                                          |
//-------------------------------------------------------------------------------
    Variant Sheets = Doc.OlePropertyGet("Sheets");
    Variant Sheet;
    String arem;
    int SheetCount = Sheets.OlePropertyGet("Count");
    int SheetVisible;

    Sheet = Doc.OlePropertyGet("ActiveSheet");
    int OldActiveSheet = Sheet.OlePropertyGet("Index");

    arem = "";
    for (int i = 0; i < SheetCount; i++) {
        try {
            Sheet = Sheets.OlePropertyGet("Item", i + 1);
        } catch (...) {
            Sheet = Sheets.OleFunction("Item", i + 1);
        }
        SheetVisible = Sheet.OlePropertyGet("Visible");
        if (SheetVisible == 0) continue;
        Sheet.OleProcedure("Activate");
        arem += SubscribeExcelSheet(Sheet, SPath, Fam, Count);
    }

    Sheet = Sheets.OlePropertyGet("Item", OldActiveSheet);
    Sheet.OleProcedure("Activate");

    if (Count >= 0)
        return arem;


    char *patterns[] = {                    // �������� �������
        "��������",
        "������� ������� �������:",
        "��������� �������� ������:",
        0
    };
    char *thepattern;                       // ������� ������� �������

    Variant ActiveSheet, Cells, Cell, Pictures, Picture;
    String text, fam, picnam, filn, res;
    TPoint pt;
    int i, j, k, col, n, aleft, atop, patlen;
    char *n1, *n2;
    bool found;

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    Cells = ActiveSheet.OlePropertyGet("Cells");
    // pt = FindLastCell(Doc);
    // pt.y -= 3;
    for (k = 0, res = "", found = false; k < 25; k++, pt.y--) {
        for (col = 1; col < pt.x; col++) {
            Cell = Cells.OlePropertyGet("Item", pt.y, col);
            text = Cell.OlePropertyGet("Value");
            if (text.IsEmpty()) continue;
            n1 = text.c_str();
            for (j = 0, patlen = -1; patterns[j] != NULL; j++) {    // � ��� �� ��� �������� �������?
                if (IsSubstr(n1, patterns[j], true)) {
                    thepattern = patterns[j];
                    patlen = strlen(thepattern);
                    break;
                }
            }
            if (patlen > 0) {
                fam = FindFamily(n1, Fam, Count);
                if (fam.IsEmpty()) fam = FindFamilyNext(n1, patlen, Cells, pt.y, col, Fam, Count);
            } else {
                continue;
            }
            if (!fam.IsEmpty()) {
                fam = "�������.bmp"; 
                Pictures = ActiveSheet.OlePropertyGet("Pictures");
                n = Pictures.OlePropertyGet("Count");
                for (i = 1; i <= n; i++) {
                    Picture = Pictures.OlePropertyGet("Item", i);
                    picnam = Picture.OlePropertyGet("Name");
                    if (picnam == fam) {                            // ��� ���� ����� �������
                        found = true;
                        if (!res.IsEmpty()) res += "; ";
                        res += TrimExt(fam) + ":��� ����";
                        continue;
                    }
                }
                filn = SPath + "\\" + fam;
                Picture = Pictures.OleFunction("Insert", filn.c_str());
                Picture.OlePropertySet("Name", fam.c_str());
                found = true;
                {                       // ������ ���������� � ���������
                    double w, h, k;
                    w = Picture.OlePropertyGet("Width");
                    h = Picture.OlePropertyGet("Height");
                    k = 95. / w;
                    h *= k;
                    Picture.OlePropertySet("Width", 95);
                    Picture.OlePropertySet("Height", h);
                }
                //if (IsSubstr(n1, "��������", true) ||
                //    IsSubstr(n1, "������� ������� �������", true) ||
                //    IsSubstr(n1, "��������� �������� ������", true)
                //) {
                if (patlen > 0) {
                    //Cell = Cells.OlePropertyGet("Item", pt.y - 2, col + 2);
                    //aleft = Cell.OlePropertyGet("Left");
                    //atop = Cell.OlePropertyGet("Top");
                    //Picture.OlePropertySet("Left", aleft);
                    //Picture.OlePropertySet("Top", atop);

                    Variant Range, MergeCells, MergeArea, Width, HorAl;
                    String Adr;
                    double w, h;
                    int horal;
                    bool bb;

                    Adr = AddressFromCell(pt.y, col);
                    Range = ActiveSheet.OlePropertyGet("Range", Adr.c_str());

                    TTextX textX;
                    GetTextX(textX, Range, 0, thepattern);
                    stop = textX.x0;

                    Cell = Cells.OlePropertyGet("Item", pt.y, col);
                    atop = Cell.OlePropertyGet("Top");
                    Picture.OlePropertySet("Left", textX.x1);
                    // aleft = textX.x1;
                    // Picture.OlePropertySet("Left", aleft);
                    h = Picture.OlePropertyGet("Height");
                    Picture.OlePropertySet("Top", atop - h + 9);

                    /*
                    MergeCells = Range.OlePropertyGet("MergeCells");
                    bb = MergeCells;
                    if (bb) {
                        MergeArea = Range.OlePropertyGet("MergeArea");
                        Width = MergeArea.OlePropertyGet("Width");
                        w = Width;
                        HorAl = Range.OlePropertyGet("HorizontalAlignment");
                        horal = HorAl;
                        if (horal == -4108) {       // ������������ ����� - �����
                            Cell = Cells.OlePropertyGet("Item", pt.y, col);
                            aleft = Cell.OlePropertyGet("Left");
                            atop = Cell.OlePropertyGet("Top");
                            Picture.OlePropertySet("Left", aleft + (w - 95) / 2);
                            h = Picture.OlePropertyGet("Height");
                            Picture.OlePropertySet("Top", atop - h + 9);
                        } else {
                            Cell = Cells.OlePropertyGet("Item", pt.y, col);
                            aleft = Cell.OlePropertyGet("Left");
                            atop = Cell.OlePropertyGet("Top");
                            Picture.OlePropertySet("Left", aleft + 42);
                            h = Picture.OlePropertyGet("Height");
                            Picture.OlePropertySet("Top", atop - h + 9);
                        }
                    } else {
                        Cell = Cells.OlePropertyGet("Item", pt.y, col);
                        aleft = Cell.OlePropertyGet("Left");
                        atop = Cell.OlePropertyGet("Top");
                        Picture.OlePropertySet("Left", aleft + 42);
                        h = Picture.OlePropertyGet("Height");
                        Picture.OlePropertySet("Top", atop - h + 9);
                    }
                    */

                    // return 0;                                       // ���� ��������� ���������
                } else {
                    Cell = Cells.OlePropertyGet("Item", pt.y - 1, col + 2);
                    aleft = Cell.OlePropertyGet("Left");
                    atop = Cell.OlePropertyGet("Top");
                    Picture.OlePropertySet("Left", aleft);
                    Picture.OlePropertySet("Top", atop);
                    if (!res.IsEmpty())
                    if (!res.IsEmpty()) res += "; ";
                    res += TrimExt(fam) + ":�������� ��������� �������";
                    // return 2;                                       // �������� ��������� �������
                }
                break;
            }
        }
    }

    // return 3;                                                       // ������� �� �������
    if (res == "") {
        if (found) res = "Ok";
        else res = "������� �� �������";
    }
    return res;


//    aleft = Cell.OlePropertyGet("Left");                    // ������ ��������� ������
//    atop = Cell.OlePropertyGet("Top");
//    Pictures = ActiveSheet.OlePropertyGet("Pictures");
//    APicture = Pictures.OleFunction("Insert", "C:\\����� ������ ������ ���\\�����\\�����������������.JPG");
//    APicture.OlePropertySet("Height", 793.5);
//    APicture.OlePropertySet("Width", 578.25);
//    APicture.OlePropertySet("Left", aleft);
//    APicture.OlePropertySet("Top", atop);
//  pt = FindLastCell(Doc);

/*
xlHAlignCenter == -4108
xlHAlignCenterAcrossSelection == 7 
xlHAlignDistributed == -4117 - ��������� ��������� ���, ����� ����� ������� ��� ������������ 
xlHAlignFill == 5 - �� ����� ������� �� ����
xlHAlignGeneral == 1 - �� ��������, �.�. ��� ������ - ����
xlHAlignJustify == -4130  
xlHAlignLeft == -4131
xlHAlignRight == -4152
*/


//-------------------------------------------------------------------------------
// set ss = range("a214")                                                       |
// ?ss.MergeCells -> True                                                       |
// ?ss.MergeArea.Width                                                          |
// With Selection : .HorizontalAlignment = xlCenter                             |
//-------------------------------------------------------------------------------
/*
    Variant ActiveSheet, Cells, Cell, Pictures, Picture;
    String text, fam, picnam, filn, res;
    int i, k, col, n, aleft, atop;
    char *n1, *n2;
    bool found;

    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    Cells = ActiveSheet.OlePropertyGet("Cells");
    // pt = FindLastCell(Doc);
    // pt.y -= 3;
    for (k = 0, res = "", found = false; k < 20; k++, pt.y--) {
        for (col = 1; col < pt.x; col++) {
            Cell = Cells.OlePropertyGet("Item", pt.y, col);
            text = Cell.OlePropertyGet("Value");
            if (text.IsEmpty()) continue;
            n1 = text.c_str();
            fam = FindFamily(n1, Fam, Count);
            if (!fam.IsEmpty()) {
                Pictures = ActiveSheet.OlePropertyGet("Pictures");
                n = Pictures.OlePropertyGet("Count");
                for (i = 1; i <= n; i++) {
                    Picture = Pictures.OlePropertyGet("Item", i);
                    picnam = Picture.OlePropertyGet("Name");
                    if (picnam == fam) {                            // ��� ���� ����� �������
                        found = true;
                        if (!res.IsEmpty()) res += "; ";
                        res += TrimExt(fam) + ":��� ����";
                        continue;
                    }
                }
                filn = SPath + "\\" + fam;
                Picture = Pictures.OleFunction("Insert", filn.c_str());
                Picture.OlePropertySet("Name", fam.c_str());
                found = true;
                {                       // ������ ���������� � ���������
                    double w, h, k;
                    w = Picture.OlePropertyGet("Width");
                    h = Picture.OlePropertyGet("Height");
                    k = 95. / w;
                    h *= k;
                    Picture.OlePropertySet("Width", 95);
                    Picture.OlePropertySet("Height", h);
                }
                if (IsSubstr(n1, "��������", true)) {
                    //Cell = Cells.OlePropertyGet("Item", pt.y - 2, col + 2);
                    //aleft = Cell.OlePropertyGet("Left");
                    //atop = Cell.OlePropertyGet("Top");
                    //Picture.OlePropertySet("Left", aleft);
                    //Picture.OlePropertySet("Top", atop);

                    Variant Range, MergeCells, MergeArea, Width, HorAl;
                    String Adr;
                    double w, h;
                    int horal;
                    bool bb;

                    Adr = AddressFromCell(pt.y, col);
                    Range = ActiveSheet.OlePropertyGet("Range", Adr.c_str());
                    MergeCells = Range.OlePropertyGet("MergeCells");
                    bb = MergeCells;
                    if (bb) {
                        MergeArea = Range.OlePropertyGet("MergeArea");
                        Width = MergeArea.OlePropertyGet("Width");
                        w = Width;
                        HorAl = Range.OlePropertyGet("HorizontalAlignment");
                        horal = HorAl;
                        if (horal == -4108) {       // ������������ ����� - �����
                            Cell = Cells.OlePropertyGet("Item", pt.y, col);
                            aleft = Cell.OlePropertyGet("Left");
                            atop = Cell.OlePropertyGet("Top");
                            Picture.OlePropertySet("Left", aleft + (w - 95) / 2);
                            h = Picture.OlePropertyGet("Height");
                            Picture.OlePropertySet("Top", atop - h + 9);
                        } else {
                            Cell = Cells.OlePropertyGet("Item", pt.y, col);
                            aleft = Cell.OlePropertyGet("Left");
                            atop = Cell.OlePropertyGet("Top");
                            Picture.OlePropertySet("Left", aleft + 42);
                            h = Picture.OlePropertyGet("Height");
                            Picture.OlePropertySet("Top", atop - h + 9);
                        }
                    } else {
                        Cell = Cells.OlePropertyGet("Item", pt.y, col);
                        aleft = Cell.OlePropertyGet("Left");
                        atop = Cell.OlePropertyGet("Top");
                        Picture.OlePropertySet("Left", aleft + 42);
                        h = Picture.OlePropertyGet("Height");
                        Picture.OlePropertySet("Top", atop - h + 9);
                    }

                    // return 0;                                       // ���� ��������� ���������
                } else {
                    Cell = Cells.OlePropertyGet("Item", pt.y - 1, col + 2);
                    aleft = Cell.OlePropertyGet("Left");
                    atop = Cell.OlePropertyGet("Top");
                    Picture.OlePropertySet("Left", aleft);
                    Picture.OlePropertySet("Top", atop);
                    if (!res.IsEmpty())
                    if (!res.IsEmpty()) res += "; ";
                    res += TrimExt(fam) + ":�������� ��������� �������";
                    // return 2;                                       // �������� ��������� �������
                }
            }
        }
    }

    // return 3;                                                       // ������� �� �������
    if (res == "") {
        if (found) res = "Ok";
        else res = "������� �� �������";
    }
    return res;
*/

}

void __fastcall Subscribe(TFileList *fl, String sPath) {
//-------------------------------------------------------------------------------
//          ������������ ������ ���� ������ fl ������� �������� �� SPath        |
//-------------------------------------------------------------------------------
    Variant EA, Docs, Doc;
    TSearchRec ff;
    String *Fam, ext, filn, afiln, mes, SPath;
    int i, j, k, col, fres, Count;
    char *n1, *n2;
    bool HaveEA;

    HaveEA = true;
    try {
        EA = CreateOleObject("Excel.Application");
    } catch (...) {
        HaveEA = false;
        Application->MessageBox("�� ������� ���������� ����� � ���������� Excel", "������!", 0);
    }
    if (!HaveEA) return;


    Fam = new String[2000];
    Count = 0;
    SPath = sPath + (String)"\\*.*";

    fres = FindFirst(SPath, faArchive, ff);
    while(fres == 0) {
        ext = ExtractExt(ff.Name);
        if (LegalPictureExt(ext)) {
            filn = TrimExt(ff.Name);
            n1 = filn.c_str();
            // n2 = SkipRusWord(n1);
            n2 = SkipFamIni(n1);
            if (*n2 == 0) {
                Fam[Count] = filn + "." + ext;
                Count++;
            }
        }
        fres = FindNext(ff);
    }
    FindClose(ff);

    if (Count == 0) {
        Application->BringToFront();
        Application->MessageBox("� ��������� ����� �� ������� �� ������ ����� � ��������", "������!", 0);
        EA.OleProcedure("Quit");
        EA = Unassigned;
        return;
    }

    TFileListRec *r;
    String text, afam;
    TPoint pt;
    char keep;
    bool error;

    stop++;

    TPDFCreator *pc = new TPDFCreator();
    if (!pc->ok) {
        delete pc;
        Application->MessageBox("��� ������ ��������� ����������, ����� �� ���������� ��� ���������� ������� PDFCreator!", "������!", 0);
        EA.OleProcedure("Quit");
        EA = Unassigned;
        return;
    }
    
    for (i = 0, error = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        ext = ExtractExt(r->filn);
        if (ext != "xls" && ext != "XLS") continue;
        afiln = (String)r->path + r->filn;
        ProcessInfoFiln(r->filn.c_str());
        try {
            // pt = FindLastCell(afiln.c_str());
            Docs = EA.OlePropertyGet("Workbooks");
            Doc = Docs.OleFunction("Open", afiln.c_str(), false, false);
            EA.OlePropertySet("Visible", true);
            // pt = FindLastCell(Doc);
        } catch (...) {
            r->rem = "�� ���� ������� ����";
            mes = (String)"�� ���� ������� ���� \"" + afiln + "\" ��� ������";
            Application->MessageBox(mes.c_str(), "������!", 0);
            error = true;
        }
        if (error) break;

        //text = FindOName(Doc);
        //if (text.IsEmpty()) {
        //    r->rem = "�� �����";
        //} else {
            // r->rem = Subscribe(Doc, pt, sPath, Fam, Count);
            r->rem = Subscribe(Doc, sPath, Fam, Count);
//            switch(Subscribe(Doc, pt, sPath, Fam, Count)) {
//                case 0: r->rem = "Ok"; break;
//                case 1: r->rem = "������� ��� ����"; break;
//                case 2: r->rem = "�������� ��������� �������"; break;
//                case 3: r->rem = "�� ������ ���� �������"; break;
//                default: r->rem = "������!"; break;
//            }
            Doc.OleProcedure("Save");
        //}
        Doc.OleProcedure("Close", false);
    }

    delete pc;

    EA.OleProcedure("Quit");
    EA = Unassigned;

    delete[] Fam;
}











//===============================================================================
//===============================================================================
void __fastcall PrintPDF(String PrinterName, String copies) {
//-------------------------------------------------------------------------------
//            �������� ������� �������� �������� �� ��������� �������           |
// ��������, ��� Ocrobat ���� � ��� ����� ������� ����                          |
//-------------------------------------------------------------------------------
    Variant avdoc, pddoc;
    Variant Ocrobat;            // ������� ����������, Ocrobat = CreateOleObject("AcroExch.App");
    String NewCurPrinter;
    int i, n, npages;
    char OldCurPrinter[100];

    // wsprintf(LogBuf, "========= ������ �� ������� %s", PrinterName.c_str()); llog();
    GetProfileString("windows", "device", "nodefault", OldCurPrinter, 100);
    NewCurPrinter = GetFullPrinterR(PrinterName.c_str());
    WriteProfileString("windows", "device", NewCurPrinter.c_str());
    n = copies.ToInt();
    avdoc = Ocrobat.OleFunction("GetActiveDoc");
    pddoc = avdoc.OleFunction("GetPDDoc");
    npages = pddoc.OleFunction("GetNumPages");
    for (i = 0; i < n; i++) {
        avdoc.OleFunction("PrintPagesSilent", 0, npages - 1, 0, 0, 0);
    }
    WriteProfileString("windows", "device", OldCurPrinter);
    // log("������ �����������");
}




// DEVMODE

void __fastcall TestPrinters(void) {
//-------------------------------------------------------------------------------
//                          ������ �������� � ���������                         |
//-------------------------------------------------------------------------------
    // GetPrinterIndex("oq-jq-oq");

/*
    TPrinter *pr = Printer();
    char Dev[200];
    char Driv[200];
    char Port[200];
    unsigned int DevMode;
    DEVMODE *PMode;
    char *after;
    pr->GetPrinter(Dev, Driv, Port, DevMode);
    PMode = (DEVMODE *)GlobalLock((HGLOBAL)DevMode);
    after = (char *)PMode + PMode->dmSize;
    if (*after == 0) stop++;
    GlobalUnlock((HGLOBAL)DevMode);
*/

    Variant EA, Docs, Doc, ActiveSheet, Windows, Window;

    EA = CreateOleObject("Excel.Application");
    EA.OlePropertySet("Visible", true);
    Docs = EA.OlePropertyGet("Workbooks");
    stop = Docs.OlePropertyGet("Count");
    Doc = Docs.OleFunction("Open", "c:\\UnitedProjects\\��������� ������� ����\\���� �������� ����\\���.���.�.������ �������� ���.194 (5.0)\\�����\\01-02-02 ��������� ������. ��������� �����.xls", false, true);
    // Doc = Docs.OlePropertyGet("Item", 1);
    ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
    sstop = ActiveSheet.OlePropertyGet("Name");

    String NewCurPrinter;
    char OldCurPrinter[100], nn[100];
    GetProfileString("windows", "device", "nodefault", OldCurPrinter, 100);
    NewCurPrinter = GetFullPrinterR("Microsoft Office Document Image Writer");
    WriteProfileString("windows", "device", NewCurPrinter.c_str());
    GetProfileString("windows", "device", "nodefault", nn, 100);

    Windows = Doc.OlePropertyGet("Windows");
    Window = Windows.OlePropertyGet("Item", 1);
    Window.OlePropertySet("View", 2);               // ��� - �������� �������
    Window.OlePropertySet("Zoom", 100);             // ��� - 100%

    // SetupMODI(2);

    ActiveSheet2Tiff(ActiveSheet, "c:\\UnitedProjects\\��������� ������� ����\\exe\\test.tif");

}








//===============================================================================
//===============================================================================
//===============================================================================
//===============================================================================
void __fastcall proto(char *text) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
    static HANDLE chan = 0;
    DWORD dummy;
    if (chan == 0) {
        String filn = EraseFilnFromPath(Application->ExeName) + (String)"grotocol.txt";
        DeleteFile(filn);
        chan = CreateFile(filn.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    }
    WriteFile(chan, text, strlen(text), &dummy, 0);
    WriteFile(chan, "\015\012", 2, &dummy, 0);
    FlushFileBuffers(chan);
*/    
}







// A$1:$N$505

