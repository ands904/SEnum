//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TCatcherUnitSEnum.h"
#pragma package(smart_init)

//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall CatchAutocad::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------


extern int stop;

static void __fastcall TestAcadWindow(void);
static bool NeedTestAcadWindow = false;         // Для отлова несанкционированных окон автокада
static TSnapShot *ss0 = NULL, *ss1 = NULL;


static HWND AcadClassHWND = NULL;
static char *AcadClassName;

bool SpecCatcherMode = false;   // Спецрежим - при обнаружении плохого окна нажимает Enter


__fastcall TCatcher::TCatcher(bool CreateSuspended) : TThread(CreateSuspended) {
}
//---------------------------------------------------------------------------
void TCatcher::SetName() {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = "ACatcher";
    info.dwThreadID = -1;
    info.dwFlags = 0;

    __try
    {
         RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}
//---------------------------------------------------------------------------
void __fastcall TCatcher::Execute() {
    // SetName();
    //---- Place thread code here ----

    static DWORD last_time = 0;

    stop++;

    while(1) {
        Application->ProcessMessages();
        if (!NeedTestAcadWindow) continue;
        if (GetTickCount() - last_time < 300) continue;
        if (!NeedTestAcadWindow) continue;
        TestAcadWindow();
        last_time = GetTickCount();
    }
}
//---------------------------------------------------------------------------









//-------------------------------------------------------------------------------
//===============================================================================
//===============================================================================
static int SameSnapshotCount = 0;       // Количество одинаковых снэпшотов

TSnapShot::TSnapShot(void) {
//-------------------------------------------------------------------------------
//     Полагает, что экран имеет размеры 4000х2000, через 20 по х и 10 по у     |
//-------------------------------------------------------------------------------
    Count = (SNAPSHOT_MAXX / SNAPSHOT_DX) * (SNAPSHOT_MAXY / SNAPSHOT_DY);
    handles = new HANDLE[Count]; memset(handles, 0, sizeof(HWND) * Count);
    pixels = new COLORREF[Count]; memset(pixels, 0, sizeof(COLORREF) * Count);
}

TSnapShot::~TSnapShot(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    delete[] handles;
    delete[] pixels;
}

void __fastcall TSnapShot::Index2XY(TPoint &pt, int index) {
//-------------------------------------------------------------------------------
//                  Переводит номер в координату на экране                      |
//-------------------------------------------------------------------------------
    int w = (SNAPSHOT_MAXX / SNAPSHOT_DX);
    pt.y = index / w;
    pt.x = index - (pt.y * w);
    pt.x *= SNAPSHOT_DX;
    pt.y *= SNAPSHOT_DY;
}


static bool __fastcall _IsAcadClassName(char *class_name) {
//-------------------------------------------------------------------------------
//             Проверяет, является ли данное окно главным окном автокада        |
//-------------------------------------------------------------------------------
	// static char idiosyncrasy[] = "Afx:00400000:8:00010013:00000000:";
	int i, n;
	n = strlen(AcadClassName);
	for (i = 0; i < n; i++) {
		if (class_name[i] != AcadClassName[i]) return false;
	}
	return true;
}


void __fastcall TSnapShot::Scan(void) {
//-------------------------------------------------------------------------------
//                                  Делает снимок экрана                        |
// Запоминает только автокадовские окна!                                        |
//-------------------------------------------------------------------------------
    //::GetWindowRect(wi->Handle, &wi->Rect);
    //class_name = new char[10000];
    //::GetClassName(wi->Handle, class_name, 10000);

    HWND h, hh;
    POINT pt;
    HDC dc;
    int i;
    char *class_name;

    if (this == 0) return;
    if (!NeedTestAcadWindow) return;

    class_name = new char[10000];
    dc = ::GetDC(0);

    DWORD t0 = GetTickCount();

    for (i = 0, pt.x = 1, pt.y = 1; i < Count; i++, pt.x += SNAPSHOT_DX) {
        if (pt.x >= SNAPSHOT_MAXX) {
            pt.x = 1;
            pt.y += SNAPSHOT_DY;
        }
        if (pt.x >= 2560 && pt.y >= 512) {
            stop++;
        }
        h = ::WindowFromPoint(pt); handles[i] = h;
        if (h != 0) {
            while(1) {
                hh = (HWND)GetWindowLong(h, GWL_HWNDPARENT);
                if (hh == NULL) break;
                h = hh;
            }
            ::GetClassName(h, class_name, 10000);
            if (_IsAcadClassName(class_name)) {
                pixels[i] = ::GetPixel(dc, pt.x, pt.y);
            } else {
                pixels[i] = 0;
                handles[i] = 0;
            }
        }
    }

    stop = (int)(GetTickCount() - t0);
    stop++;

    delete[] class_name;
    ::ReleaseDC(0, dc);

}

int __fastcall TSnapShot::Compare(TSnapShot *ss, int start) {
//-------------------------------------------------------------------------------
//                          Сравнивает с указанным снэпшотом                    |
// Начиная от позиции start.                                                    |
// Возвращает первое место с разницей или -1, если разницы нет                  |
//-------------------------------------------------------------------------------
    int n;

    if (this == 0) return -1;
    if (!NeedTestAcadWindow) return -1;

    n = Count; if (ss->Count < n) n = ss->Count;
    for (; start < n; start++) {
        if (handles[start] != ss->handles[start] || pixels[start] != ss->pixels[start]) return start;
    }
    if (Count != ss->Count) {
        return n;
    }
    return -1;
}


void __fastcall TSnapShot::Accept(TSnapShot *ss) {
//-------------------------------------------------------------------------------
//                          Копирует себе указанный снэпшот                     |
//-------------------------------------------------------------------------------
    if (this == 0) return;
    if (!NeedTestAcadWindow) return;

    memmove(handles, ss->handles, sizeof(HWND) * Count);
    memmove(pixels, ss->pixels, sizeof(COLORREF) * Count);
}


static HWND __fastcall GetDialog(HWND h) {
//-------------------------------------------------------------------------------
//          Проверяет, не является ли указанное окно одним из потомков          |
// какого-нибудь диалогового окна автокада                                      |
// Если да - то возвращает хэндл этого окна, иначе NULL                         |
//-------------------------------------------------------------------------------
    HWND hh, hhh;
    RECT wr, cr;
    POINT pt;
    int dd, d;
    char *class_name;
    bool is;

    if (h == NULL) return NULL;

    // hhh <- hh <- h

    hh = (HWND)GetWindowLong(h, GWL_HWNDPARENT);
    if (hh == NULL) return NULL;

    while(1) {
        hhh = (HWND)GetWindowLong(hh, GWL_HWNDPARENT);
        if (hhh == NULL) break;
        h = hh; hh = hhh;
    }

    class_name = new char[10000];
    ::GetClassName(hh, class_name, 10000);
    is = _IsAcadClassName(class_name);
    delete[] class_name;
    if (!is) return NULL;

    if (::GetWindowRect(h, &wr) == 0) return NULL;
    if (::GetClientRect(h, &cr) == 0) return NULL;
    pt.x = cr.left; pt.y = cr.top;
    ::ClientToScreen(h, &pt);
    cr.left = pt.x; cr.top = pt.y;
    pt.x = cr.right; pt.y = cr.bottom;
    ::ClientToScreen(h, &pt);
    cr.right = pt.x; cr.bottom = pt.y;

    if (cr.left < wr.left || cr.right > wr.right || cr.top < wr.top || cr.bottom > wr.bottom) return NULL;

    if (cr.top - wr.top > 12) return h;

    return NULL;
}



static void __fastcall aWait(int milliseconds) {
//---------------------------------------------------------------------------
//         Ожидает указанное количество миллисекунд с ProcessMessages       |
//---------------------------------------------------------------------------
    DWORD t0;
    t0 = GetTickCount();
    while(1) {
        Application->ProcessMessages();
        if ((int)GetTickCount() - (int)t0 > milliseconds) break;
    }
}



static void __fastcall KillDialog(HWND hh) {
//-------------------------------------------------------------------------------
//          Убивает диалоговое окно автокада - с целью его продвижения          |
//-------------------------------------------------------------------------------
//    DestroyWindow(hh);

//    DWORD CurId, DlgId;
//    CurId = ::GetCurrentThreadId();
//    DlgId = ::GetWindowThreadProcessId(hh, NULL);
//    ::AttachThreadInput(CurId, DlgId, TRUE);
//    ::SetFocus(hh);

    RECT wr;
    int x, y;

    ::GetWindowRect(hh, &wr);
    x = (wr.left + wr.right) / 2; y = wr.top + 5;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    aWait(100);
    if (SpecCatcherMode) {
        keybd_event(VK_RETURN, 0, 0, 0); aWait(30);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(30);
    } else {
        keybd_event(VK_ESCAPE, 0, 0, 0); aWait(30);
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0); aWait(30);
    }
}


static void __fastcall TestAcadWindow(void) {
//-------------------------------------------------------------------------------
//			Проверяет наличие несанкционированного окна автокада				|
// Если обнаруживает - убивает его нАфих!										|
//-------------------------------------------------------------------------------
    if (!NeedTestAcadWindow) return;
    if (ss0 == NULL) {
        ss0 = new TSnapShot();
        ss0->Scan();
        ss1 = new TSnapShot();
        return;
    }

    if (!NeedTestAcadWindow) return;
    ss1->Scan();
    if (!NeedTestAcadWindow) return;
    if (ss0->Compare(ss1, 0) != -1) {
        ss0->Accept(ss1);
        SameSnapshotCount = 0;
        return;
    }
    if (++SameSnapshotCount < 5) return;
    // if (++SameSnapshotCount < 15) return;
    // if (++SameSnapshotCount < 3) return;



    // -------------------- Что-то долго застряли на одном и том же автокадовском окне
    HWND dialog;
    int i, j;
    bool processed;

    for (i = 0; i < ss0->Count; i++) {
        if (!NeedTestAcadWindow) break;        
        if (ss0->handles[i] == NULL) continue;
        dialog = GetDialog((HWND)ss0->handles[i]);
        if (dialog == NULL) continue;
        GetDialog((HWND)ss0->handles[i]);
        for (j = 0, processed = false; j < i; j++) {
            // if (ss0->handles[j] == ss0->handles[i]) {
            if (ss0->handles[j] == dialog) {
                processed = true;
                break;
            }
        }
        ss0->handles[i] = dialog;
        if (processed) continue;
        KillDialog(dialog);
    }

    SameSnapshotCount = 0;

}







static TCatcher *Catcher = NULL;


void __fastcall IniTestAcadWindow(Variant Acad) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
	int i, v;
    try {
	    v = Acad.OlePropertyGet("HWND");
    } catch (...) {
        v = 0;
    }
    if (v == 0) {
		v = Acad.OlePropertyGet("HWND32");
	}
    AcadClassHWND = (HWND)v;
	AcadClassName = new char[10000];
	::GetClassNameA(AcadClassHWND, AcadClassName, 10000);
*/
	int i, h1, h2, hh;
	char buf[100];

    try {
	    h1 = Acad.OlePropertyGet("HWND");
    } catch (...) {
        h1 = 0;
    }
    try {
	    h2 = Acad.OlePropertyGet("HWND32");
    } catch (...) {
        h2 = 0;
    }

	AcadClassHWND = (HWND)(h1 == 0 ? h2 : h1);
	AcadClassName = new char[10000];
    memset(AcadClassName, 0, 10000);
	strcpy(AcadClassName, "???");
	::GetClassNameA(AcadClassHWND, AcadClassName, 10000);

	//wsprintf(buf, "h1=%08X h2=%08X %s", h1, h2, AcadClassName);
    // Application->MessageBox(buf, "info", 0);

}



void __fastcall StartTestAcadWindow(void) {
//-------------------------------------------------------------------------------
//   Делает начальный снимок экрана и прочие приготовления для TestAcadWindow   |
// Сразу после надо открывать автокадовский файл, ибо иначе вся работа будет    |
// крутиться втуне                                                              |
//-------------------------------------------------------------------------------
    SameSnapshotCount = 0;
    // if (ss0 != NULL) delete ss0; ss0 = NULL;
    // if (ss1 != NULL) delete ss1; ss1 = NULL;
    // Form1->CatchTimer->Enabled = true;
    if (Catcher == NULL) {
        Catcher = new TCatcher(true);
    }
    SameSnapshotCount = 0;
    NeedTestAcadWindow = true;
    Catcher->Resume();
}

void __fastcall StopTestAcadWindow(void) {
//-------------------------------------------------------------------------------
//        Ну типа все, файл загрузился, можно дальше не проверять               |
//-------------------------------------------------------------------------------
    NeedTestAcadWindow = false;
    SameSnapshotCount = 0;
    // Form1->CatchTimer->Enabled = false;
    if (!Catcher->Suspended) Catcher->Suspend();
    // if (ss0 != NULL) delete ss0; ss0 = NULL;
    // if (ss1 != NULL) delete ss1; ss1 = NULL;
}



// TApplication TThread CreateThread





