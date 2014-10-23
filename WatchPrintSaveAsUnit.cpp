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

static HWND MainHandle;             // Головное окно программы
static String Filn;                 // Имя файла, под которым надо сохранить tif
static TWatchPrintSaveAs *Watcher = NULL;
// static TWatchPrintSaveAs *SWatcher = NULL;

static TPDFCreator *pc;             // Для сторожа печати PDF

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
//        Согласно учебнику - делает из неименованной thread именованную        |
// Только в этом смысл и состоит                                                |
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
//      В указанном диалоговом окне сохранения тифовского файла вводит его и    |
// нажимает кнопку "Сохранить"                                                  |
//-------------------------------------------------------------------------------
    TWinInfo cb, fn, ob, ob1;
    int i, x, y, checked;
    bool done;

    // --------- Фокусируемся на окне
//    x = dlg->Rect.Left + 40; if (x <= 0) x = 1;
//    y = dlg->Rect.Top - 10; if (y <= 0) y = 1;
//    ::SetCursorPos(x, y);
//    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
//    ::SetCursorPos(x, y); aWait(50);
//    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
//    aWait(300); вавафва

    // stop++;              // чтобы переключить фокус на другое окно чтобы убедиться,
    // aWait(4000);         // что фокус переключается корректно

    aWait(300);

    DWORD CurId, DlgId;
    CurId = ::GetCurrentThreadId();
    DlgId = ::GetWindowThreadProcessId(dlg->Handle, NULL);
    ::AttachThreadInput(CurId, DlgId, TRUE);
    ::SetFocus(dlg->Handle);
    aWait(50);

    // --------- Собственно имя файла
    x = (dlg->CRect.Left + dlg->CRect.Right) / 2;
    y = dlg->CRect.Bottom - 95;
    GetWinInfo(&fn, x, y);
    Text2Focused(fn.Handle, filn);
    aWait(50);

    // -------- Отменить открытие просмотра файла *.tiff
    y = dlg->CRect.Bottom - 8;
    GetWinInfo(&cb, x, y);

    checked = ::SendMessage(cb.Handle, BM_GETCHECK, 0, 0);
    if (checked == BST_CHECKED) {
        ::SendMessage(cb.Handle, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    // --------- Кнопочка "Сохранить"
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
        ::SetCursorPos(x, y); aWait(50);                           // Должна быть кнопка!
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
        ::SetCursorPos(x, y); aWait(50);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
        aWait(300);
    }
}


static void __fastcall SCatch(TWinInfo *wi, String ParentHeader);

static void __fastcall ClickCallSetup(TWinInfo *dlg) {
//-------------------------------------------------------------------------------
//              В настройках печати вызывает свойство принтера                  |
//-------------------------------------------------------------------------------
    TWinInfo ob, msd;
    TPoint pt;
    TClipboard *c;
    char OldDefaultFolder[500];
    int x, y, xy;

    IniCatcher(MainHandle, "MODISetupDialog");

    // --------- Кнопочка "Свойства..."
    x = dlg->CRect.Left + 370;
    y = dlg->CRect.Top + 40;
    GetWinInfo(&ob, x, y);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    SCatch(&msd, "Свойства: Microsoft Office Document Image Writer");               // Ловим диалоговое окно настроек MODI
    // -------- Выберем страницу "Дополнительно"
    x = msd.CRect.left + 115; y = msd.CRect.top + 20;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    // -------- Встанем на папку по умолчанию
    x = msd.CRect.left + 130; y = msd.CRect.top + 240;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    // --------- Прочитаем, что там было
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

    // ---------- Запишем свое для опознания
    Text2Focused("C:\\2718281828\\314159265");

    // --------- Кнопочка "Отмена"
    x = dlg->CRect.Left + 390;
    y = dlg->CRect.Top + 265;
    GetWinInfo(&ob, x, y);
    ::SendMessage(ob.Handle, BM_CLICK, 0, 0);

    // WM_LBUTTON  F:\InetDownloads

}


static void __fastcall ClickPCatch(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//   Нажимает кнопку "Да" диалог окна "Поля раздела [0-9]+ выходят за границы"  |
//-------------------------------------------------------------------------------
    TWinInfo bu;
    int x, y;
    for (y = wi->Rect.Bottom; y > wi->Rect.Top; y -= 5) {
        for (x = wi->Rect.Left; x < wi->Rect.Right; x += 20) {
            GetWinInfo(&bu, x, y);
            if (bu.ClassName != "Button") continue;
            bool c1 = bu.WindowText == String("Да");
            bool c2 = bu.WindowText == String("Д&а");
            // if (!(bu.WindowText == "Да" || bu.WindowText == "Д&a")) continue;
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


#define CN_DX 250       /* шаг CatchNet по X */
#define CN_DY 100       /* шаг CatchNet по Y */
#define CN_DXCOUNT 20
#define CN_DYCOUNT 40

static HANDLE CatchNet[CN_DXCOUNT][CN_DYCOUNT];

bool __fastcall IniCatcher(HWND _MainHandle, char *filn, TPDFCreator *_pc) {
//-------------------------------------------------------------------------------
//                  Делает снимок экрана - ловит хэндлы по сетке                |
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
//                  Выполняет ловлю и вызывает сохранение файла                 |
// И восстанавливает сохраненные параметры MODI                                 |
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
//          Ищет начиная отсюда вверх окно, у которого такой же ClassName       |
// Если не нашли - возвращает falas                                             |
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
//       Ищет окно, хэндл которого отличен от CatchNet и родитель которого      |
// есть MainHandle                                                              |
// Собственно в wi возвращает информацию об этом окне                           |
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
//       Ищет окно, хэндл которого отличен от CatchNet и родитель которого      |
// есть MainHandle                                                              |
// Собственно в wi возвращает информацию об этом окне                           |
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
//              Проверяет, не есть ли это окно, которое ищет PCatch             |
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
    regcomp(&preg, "Поля раздела [0-9]+ выходят за границы", 0);
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
//       Ищет окно, хэндл которого отличен от CatchNet и заголовок окна -       |
// Microsoft Office Word и внутри которого  есть текст                          |
//    ClassName == "MSOUNISTAT"                                                 |
//    WindowText == "Поля раздела [0-9]+ выходят за границы"                    |
// Собственно в wi возвращает информацию об этом окне                           |
// Это для печати вордовского файла из PDFCreator                               |
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
//      Ждет появление файла pc->TiffFiln, после чего ждет его стабилизации     |
// по pc->WaitForPrinted(), после чего убивает акробата pc->KillAcrobat()       |
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
        case 0:                                 // Просто печать файла?
            SetName();
            //---- Place thread code here ----
            Application->ProcessMessages();
            aWait(30);
            GetWinInfoDelay = 0; //100;
            Catch(&me);
            // ClickSaveFile(&me, "c:\\UnitedProjects\\Нумерация страниц смет\\exe\\test.tif");
            //pt.x = 0; pt.y = 0;
            //GetClientRect(me.Handle, &me.Rect);
            //ClientToScreen(me.Handle, &pt);
            //me.Rect.left += pt.x; me.Rect.top += pt.y;
            //me.Rect.right += pt.x; me.Rect.bottom += pt.y;
            GetWinInfoDelay = 0; // 3000;
            ClickSaveFile(&me, Filn.c_str());
            break;
        case 1:                                 // Диалог настройки печати?
            SetName();
            //---- Place thread code here ----
            Application->ProcessMessages();
            aWait(30);
            SCatch(&me, "Настройка печати");
            ClickCallSetup(&me);
            break;
        case 2:                                 // Никто не знает!
            SetName();
            break;
        case 3:                                 // Ловля сообщения ворда о выходе полей за границы
            SetName();
            Application->ProcessMessages();
            PCatch(&me);                        // Потом кликнуть
            stop++;
            if (Watcher != NULL && !Watcher->Suspended) {
                ClickPCatch(&me);
            }
            stop++;
            break;
        case 4:                                 // Ловля и убийство запущенного акробата при печати PDF
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
//                      Ждет завершение операции PrintOut                       |
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
//              Выводит активную страницу в файл tiff через MODI                |
// Полагает, что страница уже полностью готова к печати                         |
// В том числе и разб на страницы для Microsoft Office Document Image Writer    |
// Полагает, что такого файла нет (т.е. запроса на перезапись не будет)         |
// Если что не получилось - выдает сообщение и возвращает false                 |
// !!!! Не шевелиться во время выполнения данной функции! Эмулирует мышь!       |
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
    ProcessInfoAction("Вывод файла .tif");
    WaitPrintOut(TiffFiln);
    return true;
}



bool __fastcall Word2Tiff(Variant WA, char *TiffFiln) {
//-------------------------------------------------------------------------------
//               Выводит вордовский файл в файл tiff через MODI                 |
// Полагает, что страница уже полностью готова к печати                         |
// В том числе и разб на страницы для Microsoft Office Document Image Writer    |
// Полагает, что такого файла нет (т.е. запроса на перезапись не будет)         |
// Если что не получилось - выдает сообщение и возвращает false                 |
// !!!! Не шевелиться во время выполнения данной функции! Эмулирует мышь!       |
//-------------------------------------------------------------------------------
    if (!IniCatcher(SEnumMainForm->Handle, TiffFiln)) {
        stop++;
        return false;
    }
    WA.OleProcedure("PrintOut");
    RunCatcher();
    stop++;
    ProcessInfoAction("Вывод файла .tif");
    WaitPrintOut(TiffFiln);
    return true;
}


bool __fastcall PDF2Tiff(Variant AA, char *TiffFiln) {
//-------------------------------------------------------------------------------
//               Выводит пэдэфвовский файл в файл tiff через MODI               |
// Полагает, что страница уже полностью готова к печати                         |
// В том числе и разб на страницы для Microsoft Office Document Image Writer    |
// Полагает, что такого файла нет (т.е. запроса на перезапись не будет)         |
// Если что не получилось - выдает сообщение и возвращает false                 |
// !!!! Не шевелиться во время выполнения данной функции! Эмулирует мышь!       |
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
        Application->MessageBox("GetActiveDoc", "Ошибка обращения к Adobe Acrobat!", 0);
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
        Application->MessageBox("GetPDDoc", "Ошибка обращения к Adobe Acrobat!", 0);
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
        Application->MessageBox("GetNumPages", "Ошибка обращения к Adobe Acrobat!", 0);
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
        Application->MessageBox("PrintPagesSilent", "Ошибка обращения к Adobe Acrobat!", 0);
    }

    aWait(200);

    RunCatcher();
    stop++;
    ProcessInfoAction("Вывод файла .tif");
    WaitPrintOut(TiffFiln);
    return true;
}


void __fastcall IniMODIPrinter(void) {
//-------------------------------------------------------------------------------
//     Проводит начальную инициализацию принтера MODI - чтобы запись о нем      |
// заведомо появилась в реестре, если это знакомый нам принтер                  |
//-------------------------------------------------------------------------------
    if (!IniCatcher(SEnumMainForm->Handle, "PrinterSetupDialog")) return;
    SEnumMainForm->PrinterSetupDialog1->Execute();
    RunCatcher();
    stop++;
}

