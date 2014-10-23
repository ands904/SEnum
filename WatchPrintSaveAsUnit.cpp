//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <ComObj.hpp>
#include <Printers.hpp>
// #include <Registry.hpp>
#include <Clipbrd.hpp>

#include "SEnumMainUnit.h"

#include "WatchPrintSaveAsUnit.h"
#pragma package(smart_init)

#include "SEnumLib.h"

extern char *TiffCurPrinter;

static HWND MainHandle;             // �������� ���� ���������
static String Filn;                 // ��� �����, ��� ������� ���� ��������� tif
static TWatchPrintSaveAs *Watcher = NULL;
// static TWatchPrintSaveAs *SWatcher = NULL;

static TPDFCreator *pc;             // ��� ������� ������ PDF

static bool volatile NeedSuspendThread = false;
static bool volatile DoneSuspendThread = true;



//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TWatchPrintSaveAs::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TWatchPrintSaveAs::TWatchPrintSaveAs(bool CreateSuspended, int what_catch) : TThread(CreateSuspended) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    WhatCatch = what_catch;
}

void TWatchPrintSaveAs::SetName() {
//-------------------------------------------------------------------------------
//        �������� �������� - ������ �� ������������� thread �����������        |
// ������ � ���� ����� � �������                                                |
//-------------------------------------------------------------------------------
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    switch(WhatCatch) {
        case 0: info.szName = "WatchPrintSaveAs"; break;
        case 1: info.szName = "WatchPrinterSetupDialog"; break;
        case 2: info.szName = "WatchMODISetupDialog"; break;
        case 3: info.szName = "WatchTPDFCreator__PrintDOC"; break;
        case 4: info.szName = "WatchTPDFCreator__PrintPDF"; break;
    }
    info.dwThreadID = -1;
    info.dwFlags = 0;

    __try
    {
         RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
        stop++;
    }
}


static void __fastcall ClickSaveFile(TWinInfo *dlg, char *filn) {
//-------------------------------------------------------------------------------
//      � ��������� ���������� ���� ���������� ���������� ����� ������ ��� �    |
// �������� ������ "���������"                                                  |
//-------------------------------------------------------------------------------
    TWinInfo cb, fn, ob, ob1;
    int i, x, y, checked;
    bool done;

    // --------- ������������ �� ����
//    x = dlg->Rect.Left + 40; if (x <= 0) x = 1;
//    y = dlg->Rect.Top - 10; if (y <= 0) y = 1;
//    ::SetCursorPos(x, y);
//    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
//    ::SetCursorPos(x, y); aWait(50);
//    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
//    aWait(300); �������

    // stop++;              // ����� ����������� ����� �� ������ ���� ����� ���������,
    // aWait(4000);         // ��� ����� ������������� ���������

    aWait(300);

    DWORD CurId, DlgId;
    CurId = ::GetCurrentThreadId();
    DlgId = ::GetWindowThreadProcessId(dlg->Handle, NULL);
    ::AttachThreadInput(CurId, DlgId, TRUE);
    ::SetFocus(dlg->Handle);
    aWait(50);

    // --------- ���������� ��� �����
    x = (dlg->CRect.Left + dlg->CRect.Right) / 2;
    y = dlg->CRect.Bottom - 95;
    GetWinInfo(&fn, x, y);
    Text2Focused(fn.Handle, filn);
    aWait(50);

    // -------- �������� �������� ��������� ����� *.tiff
    y = dlg->CRect.Bottom - 8;
    GetWinInfo(&cb, x, y);

    checked = ::SendMessage(cb.Handle, BM_GETCHECK, 0, 0);
    if (checked == BST_CHECKED) {
        ::SendMessage(cb.Handle, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    // --------- �������� "���������"
    aWait(200);
    x = dlg->CRect.Right - 45;
    y = dlg->CRect.Bottom - 95;
    GetWinInfo(&ob, x, y);
    if (ob.ClassName != "Button") {
        stop++;
    }
    ::SendMessage(ob.Handle, BM_CLICK, 0, 0);

    done = true;
    aWait(100);
    GetWinInfo(&ob1, x, y);
    if (ob.Handle == ob1.Handle) {
        done = false;
        for (i = 0; i < 10; i++) {
            aWait(30);
            GetWinInfo(&ob1, x, y);
            if (ob1.Handle != ob.Handle) {
                done = true;
                break;
            }
        }
    }

    if (!done) {
        ::SendMessage(ob.Handle, BM_CLICK, 0, 0);
        aWait(100);
        GetWinInfo(&ob1, x, y);
        if (ob.Handle == ob1.Handle) {
            for (i = 0; i < 10; i++) {
                aWait(30);
                GetWinInfo(&ob1, x, y);
                if (ob1.Handle != ob.Handle) {
                    done = true;
                    break;
                }
            }
        }
    }

    AttachThreadInput(CurId, DlgId, FALSE);

    if (!done) {
        ::SetCursorPos(x, y); aWait(50);                           // ������ ���� ������!
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
        ::SetCursorPos(x, y); aWait(50);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
        aWait(300);
    }
}


static void __fastcall SCatch(TWinInfo *wi, String ParentHeader);

static void __fastcall ClickCallSetup(TWinInfo *dlg) {
//-------------------------------------------------------------------------------
//              � ���������� ������ �������� �������� ��������                  |
//-------------------------------------------------------------------------------
    TWinInfo ob, msd;
    TPoint pt;
    TClipboard *c;
    char OldDefaultFolder[500];
    int x, y, xy;

    IniCatcher(MainHandle, "MODISetupDialog");

    // --------- �������� "��������..."
    x = dlg->CRect.Left + 370;
    y = dlg->CRect.Top + 40;
    GetWinInfo(&ob, x, y);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    SCatch(&msd, "��������: Microsoft Office Document Image Writer");               // ����� ���������� ���� �������� MODI
    // -------- ������� �������� "�������������"
    x = msd.CRect.left + 115; y = msd.CRect.top + 20;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    // -------- ������� �� ����� �� ���������
    x = msd.CRect.left + 130; y = msd.CRect.top + 240;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    // --------- ���������, ��� ��� ����
    keybd_event(VK_HOME, 0, 0, 0); aWait(20);
    keybd_event(VK_HOME, 0, KEYEVENTF_KEYUP, 0); aWait(20);
    keybd_event(VK_SHIFT, 0, 0, 0); aWait(20);
    keybd_event(VK_END, 0, 0, 0); aWait(20);
    keybd_event(VK_END, 0, KEYEVENTF_KEYUP, 0); aWait(20);
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0); aWait(80);

    keybd_event(VK_CONTROL, 0, 0, 0); aWait(20);
    keybd_event('C', 0, 0, 0); aWait(20);
    keybd_event('C', 0, KEYEVENTF_KEYUP, 0); aWait(20);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(80);

    c = Clipboard();
    c->Open();
    c->GetTextBuf(OldDefaultFolder, 500);
    c->Close();

    // ---------- ������� ���� ��� ���������
    Text2Focused("C:\\2718281828\\314159265");

    // --------- �������� "������"
    x = dlg->CRect.Left + 390;
    y = dlg->CRect.Top + 265;
    GetWinInfo(&ob, x, y);
    ::SendMessage(ob.Handle, BM_CLICK, 0, 0);

    // WM_LBUTTON  F:\InetDownloads

}


static void __fastcall ClickPCatch(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//   �������� ������ "��" ������ ���� "���� ������� [0-9]+ ������� �� �������"  |
//-------------------------------------------------------------------------------
    TWinInfo bu;
    int x, y;
    for (y = wi->Rect.Bottom; y > wi->Rect.Top; y -= 5) {
        for (x = wi->Rect.Left; x < wi->Rect.Right; x += 20) {
            GetWinInfo(&bu, x, y);
            if (bu.ClassName != "Button") continue;
            bool c1 = bu.WindowText == String("��");
            bool c2 = bu.WindowText == String("�&�");
            // if (!(bu.WindowText == "��" || bu.WindowText == "�&a")) continue;
            if (!(c1 || c2)) continue;
            aWait(100);
            x = (bu.Rect.Left + bu.Rect.Right) / 2;
            y = (bu.Rect.Top + bu.Rect.Bottom) / 2;
            ::SetCursorPos(x, y);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
            ::SetCursorPos(x, y);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
            aWait(300);
            return;
        }
    }
}


#define CN_DX 250       /* ��� CatchNet �� X */
#define CN_DY 100       /* ��� CatchNet �� Y */
#define CN_DXCOUNT 20
#define CN_DYCOUNT 40

static HANDLE CatchNet[CN_DXCOUNT][CN_DYCOUNT];

bool __fastcall IniCatcher(HWND _MainHandle, char *filn, TPDFCreator *_pc) {
//-------------------------------------------------------------------------------
//                  ������ ������ ������ - ����� ������ �� �����                |
//-------------------------------------------------------------------------------
    TWinInfo wi;
    TWinInfo pa;
    String NewCurPrinter;
    int i, j, mode;
    char nn[100];

    pc = _pc;

    GetParentWinInfo(&wi, _MainHandle);
    MainHandle = wi.Handle;
    Filn = filn;
    for (i = 0; i < CN_DXCOUNT; i++) {
        for (j = 0; j < CN_DYCOUNT; j++) {
            GetWinInfo(&wi, i * CN_DX + 1, j * CN_DY + 1);
            GetParentWinInfo(&pa, wi.Handle);
            CatchNet[i][j] = wi.Handle;
        }
    }
    mode = 0;
    if (strcmp(filn, "PrinterSetupDialog") == 0) mode = 1;
    if (strcmp(filn, "TPDFCreator::PrintDOC") == 0) mode = 3;
    if (strcmp(filn, "TPDFCreator::PrintPDF") == 0) mode = 4;
    if (strcmp(filn, "MODISetupDialog") != 0) {
        Watcher = new TWatchPrintSaveAs(true, mode);
        Watcher->Resume();
    }

    Application->ProcessMessages();

    NeedSuspendThread = false;
    DoneSuspendThread = true;

    return true;
}

void __fastcall RunCatcher(void) {
//-------------------------------------------------------------------------------
//                  ��������� ����� � �������� ���������� �����                 |
// � ��������������� ����������� ��������� MODI                                 |
//-------------------------------------------------------------------------------
    DoneSuspendThread = false;
    NeedSuspendThread = true;
    for (int i = 0; i < 100; i++) {
        aWait(100);
        if (DoneSuspendThread) break;
    }
    Watcher->Suspend();
    aWait(100);
    Watcher->Terminate();
    delete Watcher;
    Watcher = NULL;
    NeedSuspendThread = false;
    DoneSuspendThread = true;
//    WriteProfileString("windows", "device", OldCurPrinter);
}

static bool __fastcall ScanParentWinInfo(TWinInfo *wi, String ClassName) {
//-------------------------------------------------------------------------------
//          ���� ������� ������ ����� ����, � �������� ����� �� ClassName       |
// ���� �� ����� - ���������� falas                                             |
//-------------------------------------------------------------------------------
    HWND h, hh;
    char *class_name;
    h = wi->Handle;
    class_name = new char[10000];
    while(1) {
        if (wi->ClassName == ClassName) {
            FillWinInfo(wi);
            delete[] class_name;
            return true;
        }
        hh = (HWND)GetWindowLong(h, GWL_HWNDPARENT);
        if (hh == NULL) break;
        h = hh;
        wi->Handle = h;
        ::GetWindowRect(wi->Handle, &wi->Rect);
        ::GetClassName(wi->Handle, class_name, 10000);
        wi->ClassName = (String)class_name;
        ::GetWindowText(wi->Handle, class_name, 10000);
        wi->WindowText = (String)class_name;
    }
    delete[] class_name;
    return false;
}

static void __fastcall Catch(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//       ���� ����, ����� �������� ������� �� CatchNet � �������� ��������      |
// ���� MainHandle                                                              |
// ���������� � wi ���������� ���������� �� ���� ����                           |
//-------------------------------------------------------------------------------
    TWinInfo pa;
    static int cnt;
    int i, j;
    cnt = 0;
    while(1) {
        stop++;
        for (i = 0; i < CN_DXCOUNT; i++) {
            for (j = 0; j < CN_DYCOUNT; j++) {
                if (i == 1 && j == 1) {
                    cnt++;
                    if (cnt > 100) {
                        stop++;
                    }
                }
                GetWinInfo(wi, i * CN_DX + 1, j * CN_DY + 1);
                if (CatchNet[i][j] != wi->Handle) {
                    GetParentWinInfo(&pa, wi->Handle);
                    if (pa.ClassName == "TApplication" ||
                        pa.ClassName == "XLMAIN" ||
                        pa.ClassName == "OpusApp" ||
                        pa.ClassName == "AdobeAcrobat" ||
                        pa.Handle == MainHandle
                    ) {
                        if (ScanParentWinInfo(wi, "#32770")) {
                            stop++;
                            TWinInfo wii;
                            wii.Handle = (HWND)GetWindowLong(wi->Handle, GWL_HWNDPARENT);
                            if (wii.Handle != NULL) {
                                FillWinInfo(&wii);
                                if (wii.ClassName == "#32770") {
                                    wi->Handle = wii.Handle;
                                    FillWinInfo(wi);
                                }
                            }
                            return;
                        }
                    }
                }
            }
        }
        aWait(100);
    }
}

static void __fastcall SCatch(TWinInfo *wi, String ParentHeader) {
//-------------------------------------------------------------------------------
//       ���� ����, ����� �������� ������� �� CatchNet � �������� ��������      |
// ���� MainHandle                                                              |
// ���������� � wi ���������� ���������� �� ���� ����                           |
//-------------------------------------------------------------------------------
    TWinInfo pa;
    static int cnt;
    int i, j;
    cnt = 0;
    while(1) {
        stop++;
        for (i = 0; i < CN_DXCOUNT; i++) {
            for (j = 0; j < CN_DYCOUNT; j++) {
                GetWinInfo(wi, i * CN_DX + 1, j * CN_DY + 1);
                if (CatchNet[i][j] != wi->Handle) {
                    GetParentWinInfo(&pa, wi->Handle);
                    if (pa.Handle == MainHandle) {
                        if (ScanParentWinInfo(wi, "#32770")) {
                            if (ParentHeader.IsEmpty()) return;
                            if (ParentHeader == wi->WindowText) {
                                stop++;
                                return;
                            }
                        }
                    }
                }
            }
        }
        aWait(100);
    }
}



#include <pcreposix.h>
//int regcomp(regex_t *preg, const char *pattern, int cflags);
//int regexec(regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
//size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
//void regfree(regex_t *preg);

static bool __fastcall IsPCatchWindow(TWinInfo *wi, bool regexp_test) {
//-------------------------------------------------------------------------------
//              ���������, �� ���� �� ��� ����, ������� ���� PCatch             |
//-------------------------------------------------------------------------------
    if (!ScanParentWinInfo(wi, "#32770")) return false;
    if (wi->WindowText != "Microsoft Office Word") return false;

    TWinInfo chi;
    int x = (wi->Rect.Left + wi->Rect.Right) / 2;
    int y;
    bool found;

    if (wi->Rect.Right - wi->Rect.Left < 20) return false;
    if (wi->Rect.Bottom - wi->Rect.Top < 20) return false;

    regex_t preg;
    regmatch_t pmatch[50];
    regcomp(&preg, "���� ������� [0-9]+ ������� �� �������", 0);
    for (found = false, y = wi->Rect.Top; y < wi->Rect.Bottom; y += 5) {
        aWait(1);
        GetWinInfo(&chi, x, y);
        if (chi.ClassName != "MSOUNISTAT") continue;
        if (regexec(&preg, chi.WindowText.c_str(), 50, pmatch, 0) == 0) {
            found = true;
            break;
        }
    }
    regfree(&preg);

    return found;
}



static void __fastcall PCatch(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//       ���� ����, ����� �������� ������� �� CatchNet � ��������� ���� -       |
// Microsoft Office Word � ������ ��������  ���� �����                          |
//    ClassName == "MSOUNISTAT"                                                 |
//    WindowText == "���� ������� [0-9]+ ������� �� �������"                    |
// ���������� � wi ���������� ���������� �� ���� ����                           |
// ��� ��� ������ ����������� ����� �� PDFCreator                               |
//-------------------------------------------------------------------------------
    TWinInfo pa;
    static int cnt;
    int i, j;
    cnt = 0;
    while(1) {
        stop++;
        for (i = 0; i < CN_DXCOUNT; i++) {
            aWait(1);
            for (j = 0; j < CN_DYCOUNT; j++) {
                GetWinInfo(wi, i * CN_DX + 1, j * CN_DY + 1);
                if (CatchNet[i][j] != wi->Handle) {
                    stop++;
                    if (IsPCatchWindow(wi, false)) {
                        stop++;
                        if (IsPCatchWindow(wi, true)) {
                            stop++;
                            return;
                        }
                    }
                }
            }
        }
        Application->ProcessMessages();
        aWait(100);
        Application->ProcessMessages();
        if (NeedSuspendThread) {
            DoneSuspendThread = true;
            break;
        }
    }
}


static void __fastcall PDFCatch(void) {
//-------------------------------------------------------------------------------
//      ���� ��������� ����� pc->TiffFiln, ����� ���� ���� ��� ������������     |
// �� pc->WaitForPrinted(), ����� ���� ������� �������� pc->KillAcrobat()       |
//-------------------------------------------------------------------------------
    DWORD attri;
    HANDLE h;

    stop++;
    while(1) {
        aWait(100);
        attri = (int)GetFileAttributes(pc->TiffFiln.c_str());
        if (attri != 0xFFFFFFFF) break;
    }

    stop++;
    pc->WaitForPrinted(pc->TiffFiln);
    pc->KillAcrobat();
}


void __fastcall TWatchPrintSaveAs::Execute() {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TWinInfo me;
//    TPoint pt;

    switch(WhatCatch) {
        case 0:                                 // ������ ������ �����?
            SetName();
            //---- Place thread code here ----
            Application->ProcessMessages();
            aWait(30);
            GetWinInfoDelay = 0; //100;
            Catch(&me);
            // ClickSaveFile(&me, "c:\\UnitedProjects\\��������� ������� ����\\exe\\test.tif");
            //pt.x = 0; pt.y = 0;
            //GetClientRect(me.Handle, &me.Rect);
            //ClientToScreen(me.Handle, &pt);
            //me.Rect.left += pt.x; me.Rect.top += pt.y;
            //me.Rect.right += pt.x; me.Rect.bottom += pt.y;
            GetWinInfoDelay = 0; // 3000;
            ClickSaveFile(&me, Filn.c_str());
            break;
        case 1:                                 // ������ ��������� ������?
            SetName();
            //---- Place thread code here ----
            Application->ProcessMessages();
            aWait(30);
            SCatch(&me, "��������� ������");
            ClickCallSetup(&me);
            break;
        case 2:                                 // ����� �� �����!
            SetName();
            break;
        case 3:                                 // ����� ��������� ����� � ������ ����� �� �������
            SetName();
            Application->ProcessMessages();
            PCatch(&me);                        // ����� ��������
            stop++;
            if (Watcher != NULL && !Watcher->Suspended) {
                ClickPCatch(&me);
            }
            stop++;
            break;
        case 4:                                 // ����� � �������� ����������� �������� ��� ������ PDF
            SetName();
            Application->ProcessMessages();
            PDFCatch();
            break;
    }

    while(1) {
        if (Terminated) break;
        aWait(100);
    }
    stop++;
}


static void __fastcall WaitPrintOut(char *filn) {
//-------------------------------------------------------------------------------
//                      ���� ���������� �������� PrintOut                       |
//-------------------------------------------------------------------------------
    TSearchRec ff;
    DWORD WaitReason;
    HANDLE watcher;
    String dir = EraseFilnFromPath(filn);
    int res, time, size;

    res = FindFirst(filn, faAnyFile, ff);
    FindClose(ff);
    if (res != 0) {
        watcher = FindFirstChangeNotification(dir.c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME);
        while(1) {
            WaitReason = WaitForSingleObject(watcher, 200);
            if (WaitReason == WAIT_OBJECT_0) {
                FindCloseChangeNotification(watcher);
                break;
            }
            FindNextChangeNotification(watcher);
        }
    }

    stop++;
    aWait(200);
    time = -1; size = -1;
    while(1) {
        aWait(100);
        FindFirst(filn, faAnyFile, ff);
        if (time == ff.Time && size == ff.Size) {
            FindClose(ff);
            aWait(300);
            FindFirst(filn, faAnyFile, ff);
            if (time == ff.Time && size == ff.Size) {
                FindClose(ff);
                HANDLE chan = CreateFile(filn, GENERIC_READ + GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                CloseHandle(chan);
                if (chan != INVALID_HANDLE_VALUE) {
                    break;
                } else {
                    continue;
                }
            }
        }
        time = ff.Time; size = ff.Size;
        FindClose(ff);
    }
    aWait(300);
    stop++;
}


bool __fastcall ActiveSheet2Tiff(Variant ActiveSheet, char *TiffFiln) {
//-------------------------------------------------------------------------------
//              ������� �������� �������� � ���� tiff ����� MODI                |
// ��������, ��� �������� ��� ��������� ������ � ������                         |
// � ��� ����� � ���� �� �������� ��� Microsoft Office Document Image Writer    |
// ��������, ��� ������ ����� ��� (�.�. ������� �� ���������� �� �����)         |
// ���� ��� �� ���������� - ������ ��������� � ���������� false                 |
// !!!! �� ���������� �� ����� ���������� ������ �������! ��������� ����!       |
//-------------------------------------------------------------------------------
    Variant HPageBreaks, VPageBreaks;
    int n;
    HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
    VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
    n = (HPageBreaks.OlePropertyGet("Count") + 1) * (VPageBreaks.OlePropertyGet("Count") + 1);
    GetWinInfoDelay = 0;
    if (!IniCatcher(SEnumMainForm->Handle, TiffFiln)) return false;
    // ActiveSheet.OleProcedure("PrintOut", 1, n, 1, false, "Microsoft Office Document Image Writer");
    ActiveSheet.OleProcedure("PrintOut", 1, n, 1, false, TiffCurPrinter);
    RunCatcher();
    stop++;
    ProcessInfoAction("����� ����� .tif");
    WaitPrintOut(TiffFiln);
    return true;
}



bool __fastcall Word2Tiff(Variant WA, char *TiffFiln) {
//-------------------------------------------------------------------------------
//               ������� ���������� ���� � ���� tiff ����� MODI                 |
// ��������, ��� �������� ��� ��������� ������ � ������                         |
// � ��� ����� � ���� �� �������� ��� Microsoft Office Document Image Writer    |
// ��������, ��� ������ ����� ��� (�.�. ������� �� ���������� �� �����)         |
// ���� ��� �� ���������� - ������ ��������� � ���������� false                 |
// !!!! �� ���������� �� ����� ���������� ������ �������! ��������� ����!       |
//-------------------------------------------------------------------------------
    if (!IniCatcher(SEnumMainForm->Handle, TiffFiln)) {
        stop++;
        return false;
    }
    WA.OleProcedure("PrintOut");
    RunCatcher();
    stop++;
    ProcessInfoAction("����� ����� .tif");
    WaitPrintOut(TiffFiln);
    return true;
}


bool __fastcall PDF2Tiff(Variant AA, char *TiffFiln) {
//-------------------------------------------------------------------------------
//               ������� ������������ ���� � ���� tiff ����� MODI               |
// ��������, ��� �������� ��� ��������� ������ � ������                         |
// � ��� ����� � ���� �� �������� ��� Microsoft Office Document Image Writer    |
// ��������, ��� ������ ����� ��� (�.�. ������� �� ���������� �� �����)         |
// ���� ��� �� ���������� - ������ ��������� � ���������� false                 |
// !!!! �� ���������� �� ����� ���������� ������ �������! ��������� ����!       |
//-------------------------------------------------------------------------------
    Variant avdoc, pddoc;
    int npages;
    int i;
    bool yes;

    // SetupMODI(1);
    if (!IniCatcher(SEnumMainForm->Handle, TiffFiln)) return false;

    aWait(300);
    for (i = 0, yes = false; i < 10 && !yes; i++) {
        try {
            avdoc = AA.OleFunction("GetActiveDoc");
            yes = true;
        } catch (...) {
            aWait(100);
        }
    }
    if (!yes) {
        Application->MessageBox("GetActiveDoc", "������ ��������� � Adobe Acrobat!", 0);
    }

    aWait(300);
    for (i = 0, yes = false; i < 10 && !yes; i++) {
        try {
            pddoc = avdoc.OleFunction("GetPDDoc");
            yes = true;
        } catch (...) {
            aWait(100);
        }
    }
    if (!yes) {
        Application->MessageBox("GetPDDoc", "������ ��������� � Adobe Acrobat!", 0);
    }

    for (i = 0, yes = false; i < 10 && !yes; i++) {
        try {
            npages = pddoc.OleFunction("GetNumPages");
            yes = true;
        } catch (...) {
            aWait(100);
        }
    }
    if (!yes) {
        Application->MessageBox("GetNumPages", "������ ��������� � Adobe Acrobat!", 0);
    }

    for (i = 0, yes = false; i < 10 && !yes; i++) {
        try {
            avdoc.OleFunction("PrintPagesSilent", 0, npages - 1, 0, 0, 0);
            yes = true;
        } catch (...) {
            aWait(100);
        }
    }
    if (!yes) {
        Application->MessageBox("PrintPagesSilent", "������ ��������� � Adobe Acrobat!", 0);
    }

    aWait(200);

    RunCatcher();
    stop++;
    ProcessInfoAction("����� ����� .tif");
    WaitPrintOut(TiffFiln);
    return true;
}


void __fastcall IniMODIPrinter(void) {
//-------------------------------------------------------------------------------
//     �������� ��������� ������������� �������� MODI - ����� ������ � ���      |
// �������� ��������� � �������, ���� ��� �������� ��� �������                  |
//-------------------------------------------------------------------------------
    if (!IniCatcher(SEnumMainForm->Handle, "PrinterSetupDialog")) return;
    SEnumMainForm->PrinterSetupDialog1->Execute();
    RunCatcher();
    stop++;
}

