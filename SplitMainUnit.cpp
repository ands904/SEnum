//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Clipbrd.hpp>
#include <ComObj.hpp>
#include <Printers.hpp>
#include <Registry.hpp>

#include <process.h>
#include <clipbrd.hpp>
#include "SplitMainUnit.h"
#include "LogUnit.h"

#define log(text)   Log(__LINE__, text)
#define llog() Log(__LINE__, LogBuf)

#define A0PrinterNumber 2
#define A4PrinterNumber 3

// #define A0PrinterNumber 9
// #define A4PrinterNumber 9

enum TPageSizeKind {tpkUndef, tpkA4P, tpkA4L, tpkA3P, tpkA3L, tpkA0, tpkA00};

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------

Variant Ocrobat;

int stop = 0;
String DebugString;

struct TWinInfo {
    HWND Handle;
    TRect Rect;             // Абсолютные координаты, надо полагать
    String ClassName;
    String WindowText;
};

String Acrobat = "C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe";
String ShortFiln = "RUSA-LNY-01-TD-61600";                      // Чистое имя файла без расширения
String MainPath = "C:\\UnitedProjects\\Переформатирование PDF\\";   // Путь до файла
String TempDir = "C:\\pdf2tiff\\";                                  // Куда будем складывать тифовские файлы

String A4PList, A4LList;
int A4PListCnt, A4LListCnt;
String A3PList, A3LList;
int A3PListCnt, A3LListCnt;
String A0List;
String A00List;
int A0ListCnt, A00ListCnt;
bool A4LCreated, A4PCreated, A3LCreated, A3PCreated, A0Created, A00Created;


// C:\UnitedProjects\Переформатирование PDF\RUSA-LNY-01-TD-61600_A4.pdf


PROCESS_INFORMATION pi;


// spawnl
// CreateProcess

TWinInfo CurWI, ParentWI;

char *LogBuf;


//===============================================================================
//===============================================================================
String __fastcall GetFullPrinterR(char *printer) {
//-------------------------------------------------------------------------------
//      Возвращает полное имя принтера из реестра - с winspool,Ne08             |
// Если такого нет - возвращает пустую строку                                   |
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
//                      Возвращает имя принтера по индексу                      |
// Если индекс слишком большой, возвращает пустую строку                        |
//-------------------------------------------------------------------------------
    TPrinter *pr = Printer();
    TStrings *ss = pr->Printers;
    if (index < 0 || index >= ss->Count) return (String)"";
    return ss->Strings[index];
}

int __fastcall GetPrinterIndex(String PrinterName) {
//-------------------------------------------------------------------------------
//              Возвращает индекс принтера по имени принтера                    |
// Если не находит - возвращает -1                                              |
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






AnsiString __fastcall EraseFilnFromPath(AnsiString FullFiln) {
//-------------------------------------------------------------------------------
//              Откусывает имя файла, оставляя только путь с '\\'               |
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
//                      Откусывает путь, оставляя только имя файла              |
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
//                  Возвращает имя файла с откушенным расширением               |
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


static char __fastcall Capitalize(char c) {
//---------------------------------------------------------------------------
//                              В т.ч. и русские буквы                      |
//---------------------------------------------------------------------------
    if (c >= 'a' && c <= 'z') {c -= 'a'; c += 'A';}
    else if (c >= 'а' && c <= 'я') {c -= 'а'; c += 'А';}
    return c;
}

static void __fastcall Capitalize(char *buf) {
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
    while(*buf != 0) *buf++ = Capitalize(*buf);
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


static void __fastcall SuperDoubleClick(TPoint *pt) {
//---------------------------------------------------------------------------
//       Выполняет супер DoubleClick - ажно тройной, на всякий случай       |
//---------------------------------------------------------------------------
    ::SetCursorPos(pt->x, pt->y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);
}

int CtrlCDebug = 0;

static String __fastcall CtrlC(TPoint *pt) {
//---------------------------------------------------------------------------
//        В указанной позиции курсора читает в буфер обмена текст окна      |
//---------------------------------------------------------------------------
    String res;
    HWND thwnd;
    TRect trect;
    int i;

    thwnd = ::WindowFromPoint(*pt);
    ::GetWindowRect(thwnd, &trect);

    ::SetCursorPos(trect.left + 3, trect.top + 10); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
    ::SetCursorPos(trect.right - 4, trect.bottom - 10); aWait(20);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

    keybd_event(VK_CONTROL, 0, 0, 0); aWait(20);
    keybd_event(VK_INSERT, 0, 0, 0); aWait(20);
    keybd_event(VK_INSERT, 0, KEYEVENTF_KEYUP, 0); aWait(20);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(80);

    if (CtrlCDebug) {
        stop++;
        CtrlCDebug = 0;
    }

    TClipboard *c;
    char *buf;
    c = Clipboard();
    buf = new char[100000];
    c->GetTextBuf(buf, 100000);
    for (i = 0; buf[i] != 0; i++) {
        if (buf[i] == 015 || buf[i] == 012) buf[i] = ' ';
    }
    res = buf;
    delete[] buf;

    keybd_event(VK_ESCAPE, 0, 0, 0); aWait(20);
    keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0); aWait(100);

    return res;

}





//===============================================================================
//===============================================================================
#pragma pack(push, 1)
struct TTifHead {                       // Заголовок файла
    char TifId[2];                      // MM или II
    char _42[2];                        // Всегда 42
    unsigned Next;                      // Указатель на первый IFD
};
struct TIFD {                           // Структура IFD
    unsigned short Tag;                 // тег
    unsigned short Type;                // Тип данных
    unsigned Count;                     // Количество значений в поле данных
    unsigned Data;                      // Данные или смещение от начала файла к данным
};
struct TIFDArray {                      // Массив элементов IFD
    unsigned short Count;               // Количество элементов IFD
    unsigned Next;                      // Смещение до следующего массива IFD
    TIFD * __fastcall IFD(int index) {  // Элемент IFD
        return (TIFD *)((int)this + index * 12);
    };
};
#pragma pack(pop)


void __fastcall LoadTiffHead(TTifHead *h, HANDLE chan) {
//-------------------------------------------------------------------------------
//                            Загружает заголовок файла                         |
//-------------------------------------------------------------------------------
    DWORD dummy;
    ReadFile(chan, h, sizeof(TTifHead), &dummy, 0);
}

TIFDArray * _fastcall LoadIFDArray(HANDLE chan, unsigned ptr) {
//-------------------------------------------------------------------------------
//                      Загружает массив элементов IFD                          |
// Предварительно делает new нужного количества элементов.                      |
// Потом отдельно надо его делать delete[] (char *)...                          |
//-------------------------------------------------------------------------------
    TIFDArray *res;
    DWORD dummy;
    unsigned short cnt;
    char *p;
    ::SetFilePointer(chan, (LONG)ptr, 0, FILE_BEGIN);
    ReadFile(chan, &cnt, 2, &dummy, 0);
    p = new char[sizeof(TIFDArray) + 12 * cnt]; res = (TIFDArray *)p;
    ReadFile(chan, res->IFD(0), cnt * 12, &dummy, 0);
    ReadFile(chan, &(res->Next), 4, &dummy, 0);
    return res;
}



double __fastcall ReadRational(HANDLE chan, int offs) {
//-------------------------------------------------------------------------------
//          Читает рациональное значение, расположенное по адресу offs          |
//-------------------------------------------------------------------------------
    DWORD dummy;
    double v0, v1;
    int rational[2];
    ::SetFilePointer(chan, (LONG)offs, 0, FILE_BEGIN);
    ReadFile(chan, &rational, 8, &dummy, 0);
    if (rational[0] == 0) return 0;
    v0 = rational[0]; v1 = rational[1];
    return v1 / v0;
}


bool IsA4PSize(double xsize, double ysize) {
//-------------------------------------------------------------------------------
//           Если размеры  в сантиметрах соответствуют A4 Portrait              |
//-------------------------------------------------------------------------------
    return (xsize <= 22 && ysize <= 30.5) || (xsize <= 30.5 && ysize <= 22);
}

bool IsA4LSize(double xsize, double ysize) {
//-------------------------------------------------------------------------------
//           Если размеры в сантиметрах соответствуют A4 Landscape              |
//-------------------------------------------------------------------------------
    return false;
}

bool IsA3PSize(double xsize, double ysize) {
//-------------------------------------------------------------------------------
//           Если размеры  в сантиметрах соответствуют A3 Portrait              |
//-------------------------------------------------------------------------------
    return (xsize <= 30.5 && ysize <= 43) || (xsize <= 43 && ysize <= 30.5);
}

bool IsA3LSize(double xsize, double ysize) {
//-------------------------------------------------------------------------------
//           Если размеры  в сантиметрах соответствуют A3 Landscape             |
//-------------------------------------------------------------------------------
    return false;
}

bool IsA0Size(double xsize, double ysize) {
//-------------------------------------------------------------------------------
//               Если размеры  в сантиметрах соответствуют A0                   |
//-------------------------------------------------------------------------------
    return  (xsize <= 84.1 || ysize <= 84.1);
}




void __fastcall ClassifyTifFile(String Filn) {
//-------------------------------------------------------------------------------
//   Классифицирует файл по размеру - добавляет имя файла в A4List или A0List   |
// Полагает, что файл - безусловно существует                                   |
//-------------------------------------------------------------------------------
    TTifHead tf;
    TIFDArray *array;
    TIFD *ifd;
    HANDLE chan;
    double xsize, ysize, XResolution, YResolution;
    unsigned Next;
    unsigned Values = 0;            // Полученные значения
    int i;
    int ImageWidth, ImageHeight;
    int ResolutionUnit;     // 1 - нет единиц, 2 - дюймы, 3 - сантиметры

    chan = CreateFile(Filn.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) {
        String mes;
        mes = (String)"Не могу открыть файл \"" + Filn + (String)"\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return;
    }
    LoadTiffHead(&tf, chan);
    Next = tf.Next;
    while(Next != 0) {
        array = LoadIFDArray(chan, Next);
        Next = array->Next;
        for (i = 0; i < array->Count; i++) {
            ifd = array->IFD(i);
            switch(ifd->Tag) {
                case 257: ImageHeight = ifd->Data; Values |= 1; break;
                case 256: ImageWidth = ifd->Data; Values |= 2; break;
                case 296: ResolutionUnit = ifd->Data; Values |= 4; break;
                case 282: XResolution = ReadRational(chan, ifd->Data); Values |= 8; break;
                case 283: YResolution = ReadRational(chan, ifd->Data); Values |= 16; break;
            }
            if (Values == 31) break;
        }
        delete[] array;
    }
    CloseHandle(chan);
    xsize = ImageWidth; xsize *= XResolution;
    ysize = ImageHeight; ysize *= YResolution;
    if (ResolutionUnit == 2) {
        xsize *= 2.54;
        ysize *= 2.54;
    }

    Filn += (String)"\"";
//    if (xsize <= 22 && ysize <= 30.5 ||
//        xsize <= 30.5 && ysize <= 22 ||
//        xsize <= 43 && ysize <= 30.5 ||
//        xsize <= 30.5 && ysize <= 43
//    ) {

//    if (xsize <= 22 && ysize <= 30.5 || xsize <= 30.5 && ysize <= 22) {
//        A4List += Filn;
//        A4ListCnt++;
//        wsprintf(LogBuf, "A4List <- %s", Filn.c_str()); llog();
//    } else {
//        A0List += Filn;
//        A0ListCnt++;
//        wsprintf(LogBuf, "A0List <- %s", Filn.c_str()); llog();
//    }

    if (Form1->CreateA4CheckBox->Checked) {
        // if (xsize <= 22 && ysize <= 30.5) {
        if (IsA4PSize(xsize, ysize)) {
            A4PList += Filn;
            A4PListCnt++;
            wsprintf(LogBuf, "A4PList <- %s", Filn.c_str()); llog();
            return;
        // } else if (xsize <= 30.5 && ysize <= 22) {
        } else if (IsA4LSize(xsize, ysize)) {
            A4LList += Filn;
            A4LListCnt++;
            wsprintf(LogBuf, "A4LList <- %s", Filn.c_str()); llog();
            return;
        }
    }

    if (Form1->CreateA3CheckBox->Checked) {
        // if (xsize <= 43 && ysize <= 30.5) {
        if (IsA3LSize(xsize, ysize)) {
            A3LList += Filn;
            A3LListCnt++;
            wsprintf(LogBuf, "A3LList <- %s", Filn.c_str()); llog();
            return;
        // } else if (xsize <= 30.5 && ysize <= 43) {
        } else if (IsA3PSize(xsize, ysize)) {
            A3PList += Filn;
            A3PListCnt++;
            wsprintf(LogBuf, "A3PList <- %s", Filn.c_str()); llog();
            return;
        }
    }

    if (Form1->CreateA0CheckBox->Checked) {
        // if (xsize > 84.1 && ysize > 84.1) {
        if (!IsA0Size(xsize, ysize)) {
            A00List += Filn;
            A00ListCnt++;
            wsprintf(LogBuf, "A00List <- %s", Filn.c_str()); llog();
        } else {
            A0List += Filn;
            A0ListCnt++;
            wsprintf(LogBuf, "A0List <- %s", Filn.c_str()); llog();
        }
    }
}

void __fastcall ClassifyTif(void) {
//-------------------------------------------------------------------------------
//                      Классифицирует файлы по размерам                        |
//-------------------------------------------------------------------------------
    TSearchRec sr;
    String S;
    char *p;
    int res;

    log("====== ClassifyTif ======");

    A4PList = ""; A4PListCnt = 0;
    A4LList = ""; A4LListCnt = 0;
    A3PList = ""; A3PListCnt = 0;
    A3LList = ""; A3LListCnt = 0;
    A0List = ""; A0ListCnt = 0;
    A00List = ""; A00ListCnt = 0;
    S = TempDir + (String)"*.tif";
    res = FindFirst(S, 0, sr);
    while(res == 0) {
        S = TempDir + sr.Name;
        ClassifyTifFile(S);
        res = FindNext(sr);
    }
    FindClose(sr);
//    if (A4ListCnt == 1) {
//        p = A4List.c_str();
//        p[A4List.Length() - 1] = 0;
//        S = p + 1;
//        A4List = S;
//    }
//    if (A0ListCnt == 1) {
//        p = A0List.c_str();
//        p[A0List.Length() - 1] = 0;
//        S = p + 1;
//        A0List = S;
//    }
}


TPageSizeKind __fastcall GetPageSizeKind(Variant pddoc, int page) {
//-------------------------------------------------------------------------------
//          Вычисляет тип размера страницы - в зависимости от ее размера        |
// В зависимосте от Form1->CreateA0CheckBox->Checked                            |
//-------------------------------------------------------------------------------
    Variant pg, sz;
    TPageSizeKind res;
    double xsize, ysize;
    bool a4, a3;

    pg = pddoc.OleFunction("AcquirePage", page);        // GetPageSize
    sz = pg.OleFunction("GetSize");
    xsize = sz.OlePropertyGet("x");
    xsize *= 2.54 / 72;                                 // чтобы получить см
    ysize = sz.OlePropertyGet("y");
    ysize *= 2.54 / 72;

    if (IsA4PSize(xsize, ysize)) res = tpkA4P;
    else if (IsA4LSize(xsize, ysize)) res = tpkA4L;
    else if (IsA3PSize(xsize, ysize)) res = tpkA3P;
    else if (IsA3LSize(xsize, ysize)) res = tpkA3L;
    else if (IsA0Size(xsize, ysize)) res = tpkA0;
    else res = tpkA00;

    if (res == tpkA3P || res == tpkA3L) {
        switch(Form1->A3ComboBox->ItemIndex) {
            case 0: return tpkUndef;            // не создавать
            case 1: return res;                 // создавать отдельно
            case 2:                             // вместе с А4
                res = res == tpkA3P ? tpkA4P : tpkA4L;
                break;
            case 3:
                res = tpkA0;
                break;
        }
    }

    if (res == tpkA0 || res == tpkA00) {    // если страницы A0, то здесь их добиваем
        if (Form1->CreateA0CheckBox->Checked) return res;
        return tpkUndef;
    }

    if (Form1->CreateA4CheckBox->Checked) return res;

    return tpkUndef;
}


void __fastcall ClassifyDocPages(Variant pddoc) {
//-------------------------------------------------------------------------------
//                Классифицирует страницы документа по размерам                 |
//-------------------------------------------------------------------------------
    TPageSizeKind pk;
    int i, n;

    log("====== ClassifyDocPages ======");

    A4PList = ""; A4PListCnt = 0;
    A4LList = ""; A4LListCnt = 0;
    A3PList = ""; A3PListCnt = 0;
    A3LList = ""; A3LListCnt = 0;
    A0List = ""; A0ListCnt = 0;
    A00List = ""; A00ListCnt = 0;

    n = pddoc.OleFunction("GetNumPages");
    for (i = 0; i < n; i++) {
        pk = GetPageSizeKind(pddoc, i);
        switch(pk) {
            case tpkA4P:
                A4PListCnt++;
                wsprintf(LogBuf, "страница %04d -> A4P", i + 1); llog();
                break;
            case tpkA4L:
                A4LListCnt++;
                wsprintf(LogBuf, "страница %04d -> A4L", i + 1); llog();
                break;
            case tpkA3P:
                A3PListCnt++;
                wsprintf(LogBuf, "страница %04d -> A3P", i + 1); llog();
                break;
            case tpkA3L:
                A3LListCnt++;
                wsprintf(LogBuf, "страница %04d -> A3L", i + 1); llog();
                break;
            case tpkA0:
                A0ListCnt++;
                wsprintf(LogBuf, "страница %04d -> A0", i + 1); llog();
                break;
            case tpkA00:
                A00ListCnt++;
                wsprintf(LogBuf, "страница %04d -> A00", i + 1); llog();
                break;
        }
    }
}






//===============================================================================
//===============================================================================

static void _AnsiToUnicode(char *ansi, int ansi_len, wchar_t *unicode) {
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

void __fastcall Text2Focused(char *text) {
//-------------------------------------------------------------------------------
//                      Отправляет текст в окно с фокусом ввода                 |
//-------------------------------------------------------------------------------
    TClipboard *c;
    HANDLE hmem;
    wchar_t *u1;
    int len = strlen(text);
    c = Clipboard();
    c->Open();
    c->Clear();
    u1 = new wchar_t[len + 2];
    _AnsiToUnicode(text, -1, u1);
    c->SetTextBuf((char *)u1);
    hmem=GlobalAlloc(GMEM_FIXED, len * 2 + 2);
    memmove(hmem, u1, len * 2 + 2);
    SetClipboardData(CF_UNICODETEXT,hmem);
    c->Close();
    delete[] u1;
    keybd_event(VK_CONTROL, 0, 0, 0); Sleep(20);                // Ctrl+V
    keybd_event('V', 0, 0, 0); Sleep(20);
    keybd_event('V', 0, KEYEVENTF_KEYUP, 0); Sleep(20);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); Sleep(20);
}




void __fastcall GetParentWinInfo(TWinInfo *wi, HWND h) {
//-------------------------------------------------------------------------------
//         Заполняет структуру wi информацией о самом главном родителе h        |
//-------------------------------------------------------------------------------
    HWND hh;
    char *class_name;
    hh = h;
    while(1) {
        hh = (HWND)GetWindowLong(h, GWL_HWNDPARENT);
        if (hh == NULL) break;
        h = hh;
    }
    wi->Handle = h;
    ::GetWindowRect(wi->Handle, &wi->Rect);
    class_name = new char[10000];
    ::GetClassName(wi->Handle, class_name, 10000);
    wi->ClassName = (String)class_name;
    ::GetWindowText(wi->Handle, class_name, 10000);
    wi->WindowText = (String)class_name;
    delete[] class_name;
}


void __fastcall GetWinInfo(TWinInfo *wi, TPoint *pt) {
//-------------------------------------------------------------------------------
//  Заполняет структуру wi информацией об окне, расположенном в pt (абс коорд)  |
//-------------------------------------------------------------------------------
    char *class_name;
    wi->Handle = ::WindowFromPoint(*pt);
    ::GetWindowRect(wi->Handle, &wi->Rect);
    class_name = new char[10000];
    ::GetClassName(wi->Handle, class_name, 10000);
    wi->ClassName = (String)class_name;
    ::GetWindowText(wi->Handle, class_name, 10000);
    wi->WindowText = (String)class_name;
    delete[] class_name;
}

void __fastcall GetWinInfo(TWinInfo *wi, int x0, int y0) {
//-------------------------------------------------------------------------------
//    Заполняет структуру wi информацией об окне, располож в x0,y0(абс коорд)   |
//-------------------------------------------------------------------------------
    TPoint pt;
    pt.x = x0; pt.y = y0;
    GetWinInfo(wi, &pt);
}

void __fastcall GetWinInfo(TWinInfo *wi) {
//-------------------------------------------------------------------------------
//               Заполняет структуру wi информацией об окне под курсором        |
//-------------------------------------------------------------------------------
    TPoint pt;
    ::GetCursorPos(&pt);
    GetWinInfo(wi, &pt);
}








//===============================================================================
//===============================================================================

int __fastcall StartAcrobat(PROCESS_INFORMATION &pi, String Path, String Filn) {
//-------------------------------------------------------------------------------
//                      Запускает акробат с указанным именем файла              |
// Если Filn.IsEmpty() - то просто запускает акробат                            |
// На выходе:                                                                   |
//    0 - выполнено                                                             |
//    1 - проблемы с запуском стартовал Adobe Acrobat                           |
//-------------------------------------------------------------------------------
//    static char acrobat[] = "C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe";
//    stop = spawnl(P_NOWAIT, acrobat, acrobat, "C:\\UnitedProjects\\Переформатирование PDF\\RUSA-LNY-01-TD-61600.pdf", NULL);
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
    int res;

    log("====== Start Acrobat ======");

    // Если в наших координатах уже есть - минимизируем их всех
    while(1) {
        GetWinInfo(&wi, 10, 5);
        GetParentWinInfo(&wi, wi.Handle);
        if (strcmp(wi.ClassName.c_str(), "AdobeAcrobat") != 0) break;
        ::ShowWindow(wi.Handle, SW_HIDE);
        aWait(20);
    }

    // Попробуем запустить
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwX = 0; si.dwY = 0;
    si.dwXSize = 800; si.dwYSize = 800;
    si.wShowWindow = SW_RESTORE; // SW_SHOWNORMAL; // SW_SHOW;
    si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE | STARTF_USESHOWWINDOW;
    if (Filn.IsEmpty()) {
        command = (String)"\"" + Acrobat + (String)"\"";
    } else {
        command = (String)"\"" + Acrobat + (String)"\" \"" + Path + Filn + (String)"\"";
    }
    log("before CreateProcess");
    res = CreateProcess(
        NULL, // "C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe",
        // "\"C:\\Program Files\\Adobe\\Acrobat 7.0\\Acrobat\\Acrobat.exe\" \"C:\\UnitedProjects\\Переформатирование PDF\\RUSA-LNY-01-TD-61600.pdf\"",
        command.c_str(),
        NULL,                                                   //security
        NULL,                                                   // security
        FALSE,                                                  //inherits handles
        0,                                                      // dwCreationFlags
        0,                                                      // lpEnvironment
        Path.c_str(),                                           // pCurrentDirectory
        // 0,
        &si,
        &pi
    );
    wsprintf(LogBuf, "after CreateProcess res = %d", res); log(LogBuf);
    if (res == 0) return 1;                                     // Не стартовал Adobe Acrobat
    res = WaitForInputIdle(pi.hProcess, 100000);
    wsprintf(LogBuf, "after WaitForInputIdle res = %d", res); log(LogBuf);
    if (res != 0) return 1;

    // Проверим, удалось ли запустить
    // char wt[300];
    aWait(500);
    GetWinInfo(&wi, 10, 5);
    GetParentWinInfo(&wi, wi.Handle);
    wsprintf(LogBuf, "at (10,5) ClassName = %s", wi.ClassName.c_str()); log(LogBuf);
    if (strcmp(wi.ClassName.c_str(), "AdobeAcrobat") != 0) return 1;
    // stop = ::GetWindowText(wi.Handle, wt, 300);
    stop++;

    return 0;                                                   // Все запустилось успешно
}


// SysListView32 AcrobatMDIChildWnd MDIClient AVL_AVView

void __fastcall ExpandChild(void) {
//-------------------------------------------------------------------------------
//     Распахивает окно AcrobatMDIChildWnd во весь акробат - если оно еще не    |
// Полагает, что имя файла уже лежит в ShortFiln                                |
//-------------------------------------------------------------------------------
    String aFiln;
    HANDLE OldHandle;
    TWinInfo wi0, wi1;
    TPoint pt;
    int x, y, x0, x1;

    log("======= ExpandChild ======");
    OldHandle = 0;
    aFiln = ShortFiln + ".pdf"; Capitalize(aFiln.c_str());
    GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
    for (x = wi0.Rect.left + 1; x < wi0.Rect.right; x += 3) {
        for (y = wi0.Rect.top + 1; y < wi0.Rect.bottom; y += 3) {
            GetWinInfo(&wi1, x, y);
            if (wi1.Handle == OldHandle) continue;
            OldHandle = wi1.Handle;
            wsprintf(LogBuf, "at (%d,%d) ClassName = %s", x, y, wi1.ClassName.c_str()); log(LogBuf); 
            if (strcmp(wi1.ClassName.c_str(), "AcrobatMDIChildWnd") == 0) {
                wsprintf(LogBuf, "WindowText = %s", wi1.WindowText.c_str()); log(LogBuf);
                Capitalize(wi1.WindowText.c_str());
                if (strcmp(wi1.WindowText.c_str(), aFiln.c_str()) == 0) {
                    wsprintf(LogBuf, "Expand %s", aFiln.c_str()); log(LogBuf);
                    ::SetCursorPos(x, y);
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
                    keybd_event(VK_ESCAPE, 0, 0, 0); aWait(20);
                    keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0); aWait(50);
                    x0 = wi1.Rect.left; if (x0 < 0) x0 = 0;
                    x1 = wi1.Rect.right; if (x1 > wi0.Rect.right) x1 = wi0.Rect.right;
                    pt.x = (x0 + x1) / 2;
                    pt.y = wi1.Rect.top + 15;
                    SuperDoubleClick(&pt);
                    return;
                }
            }
        }
    }
}

// GetTempPath


void __fastcall StopAcrobat(void) {
//-------------------------------------------------------------------------------
//    Закрывает все акробаты. Если было что запущено до того - тоже закроет     |
//-------------------------------------------------------------------------------
//    TerminateProcess(pi.hProcess, 0);
//    WaitForSingleObject(pi.hProcess, 90000);
    ::SetCursorPos(45, 5); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(45, 5); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    keybd_event(VK_MENU, 0, 0, 0); aWait(50);
    keybd_event(VK_F4, 0, 0, 0); aWait(50);
    keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0); aWait(50);
}


void __fastcall CreateTempDir(void) {
//-------------------------------------------------------------------------------
//      Создает временную папочку для тифов и гарантирует, что она пустая       |
// Имя папки кладет в TempDir                                                   |
//-------------------------------------------------------------------------------
    TSearchRec sr;
    String *Files, Filn;
    int cnt;                        // количество файлов
    int i, res;
    char *buf = new char[10000];

    log("======= CreateTempDir ======");

    GetTempPath(10000, buf);
    TempDir = (String)buf + (String)"pdf2tiff\\";
    delete[] buf;
    MkDir(TempDir);
    wsprintf(LogBuf, "TempDir = \"%s\"", TempDir.c_str()); llog();
    Files = new String[100000]; cnt = 0;

    res = FindFirst(TempDir + (String)"*.tif", faArchive, sr);
    while(res == 0) {
        Files[cnt++] = sr.Name;
        res = FindNext(sr);
    }
    FindClose(sr);

    for (i = 0; i < cnt; i++) {
        Filn = TempDir + Files[i];
        wsprintf(LogBuf, "Delete(\"%s\")", Filn.c_str()); llog();
        DeleteFile(Filn);
    }

    delete[] Files;
}

void __fastcall SaveTiff(void) {
//-------------------------------------------------------------------------------
//                          Сохраняет тифовские файлы                           |
// Полагает, что TempDir уже есть и файлов там нет                              |
//-------------------------------------------------------------------------------
    String TiffFiln;
    TWinInfo wi0, wib0, wib1, wim0, wim1, wie;
    int wibx0, wiby0, wimx0, wimy0, wiex0, wiey0;

    log("====== SaveTiff ======");

    log("Фокус ввода");
    ::SetCursorPos(30, 5); aWait(50);                           // Фокус ввода
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    ::SetCursorPos(30, 5); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
    aWait(300);

    GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
    wibx0 = wi0.Rect.left + 22; wiby0 = wi0.Rect.bottom - 17;
    GetWinInfo(&wib0, wibx0, wiby0);
    wimx0 = (wi0.Rect.left + wi0.Rect.right) / 2;
    wimy0 = (wi0.Rect.top + wi0.Rect.bottom) / 2;
    GetWinInfo(&wim0, wimx0, wimy0);
    
    log("Вызываем SaveDialog");
    keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);                // Вызываем SaveDialog
    keybd_event(VK_SHIFT, 0, 0, 0); aWait(50);
    keybd_event('S', 0, 0, 0); aWait(50);
    keybd_event('S', 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    while(1) {                                                  // Ждем окна SaveDialog
        GetWinInfo(&wim1, wimx0, wimy0);
        if (wim0.Handle != wim1.Handle) break;
        aWait(50);
    }
    log("Дождались окна SaveDialog");

    keybd_event(VK_TAB, 0, 0, 0); aWait(50);                    // Тип файла
    keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event('T', 0, 0, 0); aWait(50);                       // TIFF
    keybd_event('T', 0, KEYEVENTF_KEYUP, 0); aWait(50);

    keybd_event(VK_SHIFT, 0, 0, 0); aWait(50);                  // Обратно на имя файла
    keybd_event(VK_TAB, 0, 0, 0); aWait(50);
    keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0); aWait(50);

    log("запуск процедуры разбиения на tiff");
    TiffFiln = TempDir + (String)"split.tif";
    Text2Focused(TiffFiln.c_str());
    keybd_event(VK_RETURN, 0, 0, 0); aWait(50);                 // Запускаем процедуру разбиения на tiff
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    while(1) {                                                  // Ждем начала операции
        GetWinInfo(&wib1, wibx0, wiby0);
        if (wib1.Handle != wib0.Handle) break;
        aWait(50);
    }
    log("дождались начала операции");
    while(1) {                                                  // Ждем завершения операции
        GetWinInfo(&wib1, wibx0, wiby0);
        if (wib1.Handle == wib0.Handle) break;
        aWait(50);
    }
    log("дождались завершения операции");
    stop++;
}

void __fastcall CreatePDF(char *suffix, char *list) {
//-------------------------------------------------------------------------------
//          Создает файл PDF с суффиксом suffix из файлов tiff из list          |
// В той же папке, что и исходный файл                                          |
//-------------------------------------------------------------------------------
    String PdfFiln;
    HANDLE hh;
    TWinInfo wi0, wib0, wib1, wim0, wim1, wim2;
    int wibx0, wiby0, wimx0, wimy0, x, y;
    char *p;

    wsprintf(LogBuf, "====== CreatePDF ====== %s %s", suffix, list); llog();
    GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
    wibx0 = wi0.Rect.left + 22; wiby0 = wi0.Rect.bottom - 17;
    GetWinInfo(&wib0, wibx0, wiby0);
    wimx0 = (wi0.Rect.left + wi0.Rect.right) / 2;
    wimy0 = (wi0.Rect.top + wi0.Rect.bottom) / 2;
    GetWinInfo(&wim0, wimx0, wimy0);

    ::SetCursorPos(40, 35); aWait(200);                           // Вызываем диалог созд из неск файлов
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(250);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(250);
    keybd_event(VK_DOWN, 0, 0, 0); aWait(150);
    keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0); aWait(150);
    keybd_event(VK_DOWN, 0, 0, 0); aWait(150);
    keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0); aWait(150);
    keybd_event(VK_RIGHT, 0, 0, 0); aWait(150);
    keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0); aWait(150);
    keybd_event(VK_DOWN, 0, 0, 0); aWait(150);
    keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0); aWait(150);
    keybd_event(VK_RETURN, 0, 0, 0); aWait(150);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(150);
    aWait(1000);
    log("ждем запуска диалога создания файла PDF из нескольких файлов");
    while(1) {
        GetWinInfo(&wim1, wimx0, wimy0);
        if (wim1.Handle != wim0.Handle) break;
        aWait(50);
    }
    aWait(300);
    log("дождались диалога запуска диалога создания файла PDF из нескольких файлов");

    while(1) {
        if (*list == 0) break;
        p = list; while(*p != '\"') p++; *p = 0;
        wsprintf(LogBuf, "помещаем в список файл \"%s\"", list); llog();
        ::SetCursorPos(wim1.Rect.left - 150, wim1.Rect.top + 35);   // Вводим имена файлов tiff
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);
        log("ждем запуска диалога OpenFile");
        aWait(300);
        while(1) {
            GetWinInfo(&wim2, wimx0, wimy0);
            if (wim2.Handle != wim1.Handle) break;
            aWait(50);
        }
        aWait(300);
        log("дождались запуска диалога OpenFile");
        Text2Focused(list); aWait(50);
        keybd_event(VK_RETURN, 0, 0, 0); aWait(50);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(350);
        log("ждем возвращения к окну ввода списка файлов");
        while(1) {                                                  // Ждем, когда вернемся к окну ввода списка файлов
            GetWinInfo(&wim2, wimx0, wimy0);
            if (wim2.Handle == wim1.Handle) break;
            aWait(50);
        }
        aWait(300);
        log("дождались возвращения к окну ввода списка файлов");
        *p++ = '\"'; list = p;
    }

    aWait(300);
    x = (wim1.Rect.left + wim1.Rect.right) / 2 + 50;           // Запускаем процесс создания pdf
    y = wim1.Rect.bottom + 20;
    ::SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(350);
    log("ждем завершение создания pdf и запрос на сохранение файла");
    while(1) {                                                  // Ждем запрос на сохранение файла pdf
        GetWinInfo(&wim2, wimx0, wimy0);
        if (strcmp(wim2.ClassName.c_str(), "SysListView32") == 0) break;
        aWait(50);
    }
    aWait(1000);
    log("дождались запроса на сохранение файла pdf");

    PdfFiln = MainPath + ShortFiln + "_" + suffix + ".pdf";     // Запускаем сохранение файла pdf
    DeleteFile(PdfFiln.c_str());
    Text2Focused(PdfFiln.c_str()); aWait(500);
    wsprintf(LogBuf, "ждем запуск сохранения файла PDF %s", PdfFiln); llog();
    keybd_event(VK_RETURN, 0, 0, 0); aWait(50);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    while(1) {                                                  // Ждем запуска сохранения
        GetWinInfo(&wib1, wibx0, wiby0);
        if (wib1.ClassName == wib0.ClassName) {
            if (wib1.Handle != wib0.Handle) {
                wib1.Handle = 0;
                break;
            }
        }
        if (wib1.Handle != wib0.Handle) break;
        aWait(50);
    }
    aWait(300);
    hh = wib1.Handle;
    log("ждем завершение сохранения файла PDF");
    while(1) {                                                  // Ждем завершения сохранения
        GetWinInfo(&wib1, wibx0, wiby0);
        if (wib1.Handle != hh) break;
        aWait(50);
    }
    aWait(300);
    log("дождались - завершение работы CreatePDF");
}


void __fastcall CreatePDF(char *suffix, TPageSizeKind kind) {
//-------------------------------------------------------------------------------
//        Создает файл PDF с суффиксом suffix из страниц заглавного файла       |
// удовлетворяющих условию fcmp
// В той же папке, что и исходный файл                                          |
//-------------------------------------------------------------------------------
    Variant avdoc, pddoc;
    String PdfFiln, FullPdfFiln;
    HANDLE chan;
    DWORD dummy;
    double xsize, ysize;
    int filen, i, n;
    char *file;

    wsprintf(LogBuf, "====== CreatePDF ====== %s", suffix); llog();
    wsprintf(LogBuf, "Рабочая папка: %s", MainPath); llog();

    PdfFiln = MainPath + ShortFiln + (String)".pdf";
    chan = CreateFile(PdfFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    filen = (int)GetFileSize(chan, 0);
    file = new char[filen];
    ReadFile(chan, file, filen, &dummy, 0);
    CloseHandle(chan);
    log("прочитали исходный файл");

    PdfFiln = ShortFiln + "_" + suffix + ".pdf";
    FullPdfFiln = MainPath + PdfFiln;               // Запускаем сохранение файла pdf
    DeleteFile(FullPdfFiln.c_str());
    wsprintf(LogBuf, "удалили старый файл %s", PdfFiln.c_str()); llog();

    chan = CreateFile(FullPdfFiln.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    WriteFile(chan, file, filen, &dummy, 0);
    CloseHandle(chan);
    delete[] file;
    wsprintf(LogBuf, "создали файл %s, пока еще не прореженный", PdfFiln.c_str()); llog();

    StartAcrobat(pi, MainPath, PdfFiln.c_str());
    wsprintf(LogBuf, "загрузили файл %s, пока еще не прореженный", PdfFiln.c_str()); llog();

    avdoc = Ocrobat.OleFunction("GetActiveDoc");
    pddoc = avdoc.OleFunction("GetPDDoc");
    n = pddoc.OleFunction("GetNumPages");
    for (i = n - 1; i >= 0; i--) {
        if (kind != GetPageSizeKind(pddoc, i)) {
            pddoc.OleFunction("DeletePages", i, i);
        } else {
            wsprintf(LogBuf, "страница %04d -> %s", i + 1, PdfFiln.c_str()); llog();
        }
    }
    pddoc.OleFunction("Save", 1, FullPdfFiln.c_str());

    log("дождались - завершение работы CreatePDF");
}


void __fastcall PrintPDF(int nprint, int mode, String copies) {
//-------------------------------------------------------------------------------
//   Печать текущего документа на принтере номер n, количество копий - copies   |
// Режимы печати: 0 - A0, 1 - A3 ландшафт, 2 - А3 портрет, 3,4 - А4 ландш,порт  |
//-------------------------------------------------------------------------------
    TWinInfo wi0, wi1, wi2, wi3;
    int i, x0, y0, h;

    wsprintf(LogBuf, "====== PrintPDF ====== printer=%d, copies=%s", nprint, copies.c_str()); llog();

    log("Фокус ввода");
    ::SetCursorPos(45, 5); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(50);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(50);

    GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
    x0 = (wi0.Rect.left + wi0.Rect.right) / 2;
    y0 = (wi0.Rect.top + wi0.Rect.bottom) / 2;
    h = (wi0.Rect.bottom - wi0.Rect.top);
    y0 = wi0.Rect.top + h / 2 - 50;
    GetWinInfo(&wi0, x0, y0);

    keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
    keybd_event('P', 0, 0, 0); aWait(50);
    keybd_event('P', 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    aWait(300);

    log("Ждем появления диалога печати");
    while(1) {              // Ждем, когда начнется печать
        GetWinInfo(&wi1, x0, y0);
        if (wi1.Handle != wi0.Handle) break;
        aWait(50);
    }
    aWait(300);

    log("первый принтер в списке принтеров");
    keybd_event(VK_PRIOR, 0, 0, 0); aWait(50);
    keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    keybd_event(VK_PRIOR, 0, 0, 0); aWait(50);
    keybd_event(VK_PRIOR, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    aWait(300);

    log("нужный принтер в списке принтеров");
    for (i = 0; i < nprint; i++) {
        keybd_event(VK_DOWN, 0, 0, 0); aWait(50);
        keybd_event(VK_DOWN, 0, KEYEVENTF_KEYUP, 0); aWait(50);
        aWait(300);
    }
    log("количество копий");
    for (i = 0; i < 8; i++) {
        keybd_event(VK_TAB, 0, 0, 0); aWait(50);
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0); aWait(50);
    }
    aWait(300);
    Text2Focused(copies.c_str());
    aWait(300);

    log("запуск печати");
    keybd_event(VK_RETURN, 0, 0, 0); aWait(50);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); aWait(50);

    while(1) {              // Ждем, когда начнется печать
        GetWinInfo(&wi2, x0, y0);
        if (wi2.Handle != wi1.Handle) break;
        aWait(50);
    }
    log("дождались начала печати");
    while(1) {              // Ждем
        GetWinInfo(&wi3, x0, y0);
        if (wi3.Handle != wi2.Handle) break;
        aWait(50);
    }
    log("дождались окончания печати");

/*
    log("имитация процесса печати");
    aWait(5000);
    keybd_event(VK_ESCAPE, 0, 0, 0); aWait(50);
    keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0); aWait(50);
*/
}


void __fastcall PrintPDF(String PrinterName, String copies) {
//-------------------------------------------------------------------------------
//            Печатает текущий активный документ на указанный принтер           |
// Полагает, что Ocrobat есть и что такой принтер есть                          |
//-------------------------------------------------------------------------------
    Variant avdoc, pddoc;
    String NewCurPrinter;
    int i, n, npages;
    char OldCurPrinter[100];

    wsprintf(LogBuf, "========= Печать на принтер %s", PrinterName.c_str()); llog();
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
    log("Печать закончилась");
}











//===============================================================================
//===============================================================================


__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    LogBuf = new char[2000];
}

String __fastcall GetCfgFiln(void) {
//-------------------------------------------------------------------------------
//                      Возвращает полное имя файла cfg                         |
//-------------------------------------------------------------------------------
    String CfgFiln;
    CfgFiln = TrimExt(Application->ExeName) + (String)".cfg";
    return CfgFiln;
}

#pragma pack(push, 1)
struct TCfg {
    char Id[12];                            // SpitPDFv1.0
    char CreateA4, PrintA4, A4Printer[100];
    int A4Copies;
    char CreateA3, PrintA3, A3Printer[100];
    int A3Copies;
    char CreateA0, PrintA0, A0Printer[100];
    int A0Copies;
    char PdfFiln[260];
    int L0, T0, W0, H0;
    int L1, T1, W1, H1;
};
#pragma pack(pop)

char CfgId[] = "SpitPDFv1.0";


void __fastcall TForm1::FormDestroy(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TCfg cfg;
    String CfgFiln = GetCfgFiln();
    String S;
    HANDLE chan;
    DWORD dummy;
    int n;
    
    strcpy(cfg.Id, CfgId);

    cfg.CreateA4 = CreateA4CheckBox->Checked ? 1 : 0;
    cfg.PrintA4 = PrintA4CheckBox->Checked ? 1 : 0;
    S = P4Edit->Text;
    strcpy(cfg.A4Printer, S.c_str());
    S = A4CopiesEdit->Text;
    cfg.A4Copies = S.ToInt();

    cfg.CreateA3 = A3ComboBox->ItemIndex;
    cfg.PrintA3 = PrintA3CheckBox->Checked ? 1 : 0;
    S = P3Edit->Text;
    strcpy(cfg.A3Printer, S.c_str());
    S = A3CopiesEdit->Text;
    cfg.A3Copies = S.ToInt();

    cfg.CreateA0 = CreateA0CheckBox->Checked ? 1 : 0;
    cfg.PrintA0 = PrintA0CheckBox->Checked ? 1 : 0;
    S = P0Edit->Text;
    strcpy(cfg.A0Printer, S.c_str());
    S = A0CopiesEdit->Text;
    cfg.A0Copies = S.ToInt();

    S = FilnEdit->Text;
    strcpy(cfg.PdfFiln, S.c_str());

    cfg.L0 = Left; cfg.T0 = Top; cfg.W0 = Width; cfg.H0 = Height;
    cfg.L1 = LogForm->Left; cfg.T1 = LogForm->Top; cfg.W1 = LogForm->Width; cfg.H1 = LogForm->Height;

    chan = CreateFile(CfgFiln.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    WriteFile(chan, &cfg, sizeof(cfg), &dummy, 0);
    CloseHandle(chan);
}

void __fastcall ReadCfg(void) {
//-------------------------------------------------------------------------------
//                          Собственно чтение конфигурации                      |
//-------------------------------------------------------------------------------
    TCfg cfg;
    String CfgFiln;
    DWORD dummy;
    HANDLE chan;
    int filen;
    char buf[10];

    CfgFiln = GetCfgFiln();
    chan = CreateFile(CfgFiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) return;
    filen = (int)GetFileSize(chan, 0);
    if (filen != sizeof(TCfg)) {
        CloseHandle(chan);
        return;
    }
    ReadFile(chan, &cfg, sizeof(TCfg), &dummy, 0);
    CloseHandle(chan);
    if ((int)dummy != filen) return;

    if (strcmp(cfg.Id, CfgId) != 0) return;

    Form1->CreateA4CheckBox->Checked = cfg.CreateA4 == 1;
    Form1->PrintA4CheckBox->Checked = cfg.PrintA4 == 1;
    Form1->P4Edit->Text = cfg.A4Printer;
    wsprintf(buf, "%d", cfg.A4Copies);
    Form1->A4CopiesEdit->Text = buf;

    Form1->A3ComboBox->ItemIndex = cfg.CreateA3;
    Form1->PrintA3CheckBox->Checked = cfg.PrintA3 == 1;
    Form1->P3Edit->Text = cfg.A3Printer;
    wsprintf(buf, "%d", cfg.A3Copies);
    Form1->A3CopiesEdit->Text = buf;

    Form1->CreateA0CheckBox->Checked = cfg.CreateA0 == 1;
    Form1->PrintA0CheckBox->Checked = cfg.PrintA0 == 1;
    Form1->P0Edit->Text = cfg.A0Printer;
    wsprintf(buf, "%d", cfg.A0Copies);
    Form1->A0CopiesEdit->Text = buf;

    Form1->FilnEdit->Text = cfg.PdfFiln;

    Form1->Left = cfg.L0; Form1->Top = cfg.T0; Form1->Width = cfg.W0; Form1->Height = cfg.H0;
    LogForm->Left = cfg.L1; LogForm->Top = cfg.T1; LogForm->Width = cfg.W1; LogForm->Height = cfg.H1;
}

void __fastcall TForm1::FormPaint(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    static bool first = true;
    if (!first) return;

    TPrinter *pp;
    String List, Full;
    char *p, *q, keep;

    pp = Printer();
    List = pp->Printers->Text;
    p = List.c_str();

    while(*p != 0) {
        q = p;
        while(*q != 015 && *q != 012 && *q != 0) q++;
        keep = *q; *q = 0;
        Full = GetFullPrinterR(p);
        *q = keep; p = q; while(*p == 015 || *p == 012) p++;
        if (Full.IsEmpty()) {
            Application->MessageBox("Программа не может определить способ работы с принтером. Зовите Шилина А.Е. - пусть разбирается!", "Ошибка!", 0);
            Application->Terminate();
            return;
        }
    }

    // Здесь читаем конфигурацию и распихиваем значения по полям
    ReadCfg();

    // Проверяем указанные принтеры
    TPrinter *pr = Printer();
    String Default = GetPrinterName(pr->PrinterIndex);
    String S;
    int index;

    S = P4Edit->Text; if (GetPrinterIndex(S) == -1) P4Edit->Text = Default;
    S = P3Edit->Text; if (GetPrinterIndex(S) == -1) P3Edit->Text = Default;
    S = P0Edit->Text; if (GetPrinterIndex(S) == -1) P0Edit->Text = Default;
}










//===============================================================================
//===============================================================================

void __fastcall TForm1::Edit1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TPoint pt;
    HWND hand;
    char buf[300];

    if (Key == VK_RETURN) {
        GetWinInfo(&CurWI);
        GetParentWinInfo(&ParentWI, CurWI.Handle);
        wsprintf(buf, "Handle: 0x%08X  ClassName=\"%s\" WindowText=\"%s\"",
            CurWI.Handle, CurWI.ClassName, CurWI.WindowText
        );
        Edit1->Text = (String)buf; // + CurWI.ClassName;
        wsprintf(buf, "Handle: 0x%08X  ClassName=\"%s\" WindowText=\"%s\"",
            ParentWI.Handle, ParentWI.ClassName, ParentWI.WindowText
        );
        Edit2->Text = (String)buf; // + CurWI.ClassName;
        // wsprintf(buf, "Handle: 0x%08X  ", ParentWI.Handle);
        // Edit2->Text = (String)buf + ParentWI.ClassName;
        //Edit2->Text = CurWI.WindowText;
    }
}


void __fastcall TForm1::Button1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                                  Запускает акробата                          |
//-------------------------------------------------------------------------------
    StartAcrobat(pi, MainPath, ShortFiln + (String)".pdf");
    // StartAcrobat(pi, MainPath, "");
}


void __fastcall TForm1::Button3Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                              Сохраняет тифовские файлы                       |
//-------------------------------------------------------------------------------
    SaveTiff();
}


void __fastcall TForm1::Button2Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//              Закрывает акробат. И все ранее запущенные сессии тоже           |
//-------------------------------------------------------------------------------
    StopAcrobat();
}


void __fastcall TForm1::Button4Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//    String List = "\"C:\\pdf2tiff\\split_Страница_1.tiff\" \"C:\\pdf2tiff\\split_Страница_4.tiff\"";
//    CreatePDF("А3", List.c_str());
    if (A4LListCnt == 0 && A4PListCnt == 0 && A3LListCnt == 0 && A3PListCnt == 0 && A0ListCnt == 0 && A00ListCnt == 0) {
        Application->MessageBox("Не сформированы списки файлов *.tif!", "Не хватает!", 0);
        return;
    }
    if (!A4PList.IsEmpty()) {
        CreatePDF("A4P", A4PList.c_str());
    }
    if (!A4LList.IsEmpty()) {
        CreatePDF("A4L", A4LList.c_str());
    }
    if (!A3PList.IsEmpty()) {
        CreatePDF("A3P", A3PList.c_str());
    }
    if (!A3LList.IsEmpty()) {
        CreatePDF("A3L", A3LList.c_str());
    }
    if (!A0List.IsEmpty()) {
        CreatePDF("A0", A0List.c_str());
    }
    if (!A00List.IsEmpty()) {
        CreatePDF("A0", A0List.c_str());
    }
}

void __fastcall TForm1::Button5Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    ExpandChild();
}

void __fastcall TForm1::Button6Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    CreateTempDir();
}

void __fastcall TForm1::Button7Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    ClassifyTif();
}

void __fastcall TForm1::Button8Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    PrintPDF(A4PrinterNumber, 3, "1");
}

void __fastcall TForm1::Button9Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    log("Проверка связи с протоколом");
}









//===============================================================================
//===============================================================================

void __fastcall TForm1::CreateA4CheckBoxClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    PrintA4CheckBox->Enabled = CreateA4CheckBox->Checked;
}

void __fastcall TForm1::P4SpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TPrinter *pr = Printer();
    String PrinterName;
    int keep, index;
    bool res;

    keep = pr->PrinterIndex;
    PrinterName = P4Edit->Text;
    index = GetPrinterIndex(PrinterName);
    if (index < 0) index = keep;
    pr->PrinterIndex = index;
    res = PrinterSetupDialog1->Execute();
    index = pr->PrinterIndex; pr->PrinterIndex = keep;
    if (!res) return;
    P4Edit->Text = GetPrinterName(index);
}

void __fastcall TForm1::CreateA3CheckBoxClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    PrintA3CheckBox->Enabled = CreateA3CheckBox->Checked;
}

void __fastcall TForm1::P3SpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TPrinter *pr = Printer();
    String PrinterName;
    int keep, index;
    bool res;

    keep = pr->PrinterIndex;
    PrinterName = P3Edit->Text;
    index = GetPrinterIndex(PrinterName);
    if (index < 0) index = keep;
    pr->PrinterIndex = index;
    res = PrinterSetupDialog1->Execute();
    index = pr->PrinterIndex; pr->PrinterIndex = keep;
    if (!res) return;
    P3Edit->Text = GetPrinterName(index);
}

void __fastcall TForm1::CreateA0CheckBoxClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    PrintA0CheckBox->Enabled = CreateA0CheckBox->Checked;
}

void __fastcall TForm1::P0SpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TPrinter *pr = Printer();
    String PrinterName;
    int keep, index;
    bool res;

    keep = pr->PrinterIndex;
    PrinterName = P0Edit->Text;
    index = GetPrinterIndex(PrinterName);
    if (index < 0) index = keep;
    pr->PrinterIndex = index;
    res = PrinterSetupDialog1->Execute();
    index = pr->PrinterIndex; pr->PrinterIndex = keep;
    if (!res) return;
    P0Edit->Text = GetPrinterName(index);
}

void __fastcall TForm1::SelectFileSpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    String S;
    if (!OpenDialog1->Execute()) return;
    S = OpenDialog1->FileName;
    FilnEdit->Text = S;
}

void __fastcall TForm1::CancelButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Application->Terminate();
}


/*
void __fastcall TForm1::OkButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//        Старая версия - полностью через эмуляцию человеческого фактора        | 
//-------------------------------------------------------------------------------
    TWinInfo wi0, wi1;
    HANDLE chan;
    String Filn, S;
    int xb, yb;

    A4LCreated = A4PCreated = A3LCreated = A3PCreated = A0Created = A00Created = false;

    Filn = FilnEdit->Text;
    if (Filn.IsEmpty()) {
        Application->MessageBox("Сначала надо задать имя печатаемого файла", "Невозможно!", 0);
        return;
    }
    chan = CreateFile(Filn.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) {
        String mes;
        mes = (String)"Не могу открыть файл \"" + Filn + (String)"\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return;
    }

    MainPath = EraseFilnFromPath(Filn);
    ShortFiln = TrimExt(ErasePathFromFiln(Filn));

    String SS;
    int tt, nn;
    SS = RepeatEdit->Text; nn = SS.ToInt();

    for (tt = 0; tt < nn; tt++) {
        if (tt > 0) {
            wsprintf(LogBuf, "Ok N %d", tt);
            LogForm->Caption = LogBuf;
            Application->ProcessMessages();
        }
        log("");
        log("------------------------------------------------------------");
        StartAcrobat(pi, MainPath, ShortFiln + (String)".pdf");
        aWait(300);
        ExpandChild();
        CreateTempDir();
        SaveTiff();
        ClassifyTif();

        if (A4LListCnt == 0 && A4PListCnt == 0 && A3LListCnt == 0 && A3PListCnt == 0 && A0ListCnt == 0 && A00ListCnt == 0) {
            Application->MessageBox("Не сформированы списки файлов *.tif!", "Не хватает!", 0);
            return;
        }

        if (CreateA4CheckBox->Checked) {
            if (A4PListCnt > 0) {                           // A4 портрет
                CreatePDF("A4P", A4PList.c_str());
                A4PCreated = true;
            }
            if (A4LListCnt > 0) {                           // A4 ландшафт
                CreatePDF("A4L", A4LList.c_str());
                A4LCreated = true;
            }
        }
        if (CreateA3CheckBox->Checked) {
            if (A3PListCnt > 0) {                           // A3 портрет
                CreatePDF("A3P", A3PList.c_str());
                A3PCreated = true;
            }
            if (A3LListCnt > 0) {                           // A3 ландшафт
                CreatePDF("A3L", A3LList.c_str());
                A3LCreated = true;
            }
        }
        if (CreateA0CheckBox->Checked) {                    // A0
            if (A0ListCnt > 0) {
                CreatePDF("A0", A0List.c_str());
                A0Created = true;
            }
            if (A00ListCnt > 0) {                           // A00
                CreatePDF("A00", A00List.c_str());
                A00Created = true;
            }
        }

        if (A00Created) {
            // S = A0CopiesEdit->Text;
            // PrintPDF(A0PrinterNumber, 0, S);
            // ::SetCursorPos(45, 5); aWait(20);
            // mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            // mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A00");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A00");
        }
        if (A0Created) {
            if (PrintA0CheckBox->Checked) {
                S = A0CopiesEdit->Text;
                aWait(300);
                PrintPDF(A0PrinterNumber, 0, S);
            } else {
                aWait(1000);
            }
            ::SetCursorPos(45, 5); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A0");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A0");
        }

        if (A3LCreated) {                       // A3 Ландшафт
            if (PrintA3CheckBox->Checked) {
                S = A3CopiesEdit->Text;
                aWait(300);
                PrintPDF(A4PrinterNumber, 1, S);
            } else {
                aWait(1000);
            }
            ::SetCursorPos(45, 5); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A3 Ландшафт");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A3 Ландшафт");
        }

        if (A3PCreated) {                       // A3 Портрет
            if (PrintA3CheckBox->Checked) {
                S = A3CopiesEdit->Text;
                aWait(300);
                PrintPDF(A4PrinterNumber, 2, S);
            } else {
                aWait(1000);
            }
            ::SetCursorPos(45, 5); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A3 Портрет");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A3 Портрет");
        }

        if (A4LCreated) {                       // A3 Ландшафт
            if (PrintA4CheckBox->Checked) {
                S = A4CopiesEdit->Text;
                aWait(300);
                PrintPDF(A4PrinterNumber, 3, S);
            } else {
                aWait(1000);
            }
            ::SetCursorPos(45, 5); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A4 Ландшафт");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A4 Ландшафт");
        }

        if (A4PCreated) {                       // A4 Портрет
            if (PrintA4CheckBox->Checked) {
                S = A4CopiesEdit->Text;
                aWait(300);
                PrintPDF(A4PrinterNumber, 4, S);
            } else {
                aWait(1000);
            }
            ::SetCursorPos(45, 5); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); aWait(20);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); aWait(20);

            GetWinInfo(&wi0, 30, 5); GetParentWinInfo(&wi0, wi0.Handle);
            xb = wi0.Rect.left + 22; yb = wi0.Rect.bottom - 17;
            GetWinInfo(&wi0, xb, yb);

            log("Ok: запускаем закрытие файла A4 Портрет");
            keybd_event(VK_CONTROL, 0, 0, 0); aWait(50);
            keybd_event('W', 0, 0, 0); aWait(50);
            keybd_event('W', 0, KEYEVENTF_KEYUP, 0); aWait(50);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0); aWait(50);

            while(1) {
                GetWinInfo(&wi1, xb, yb);
                if (wi1.Handle != wi0.Handle) break;
                aWait(50);
            }
            log("Ok: дождались закрытия файла A4 Портрет");
        }

        StopAcrobat();

        Application->BringToFront();
        log("================= Все сделано ====================\015\012");
    }

    if (A00Created) {
        String mes;
        mes = (String)"Одна или несколько страниц имеют минимальный размер более 841 мм - \015\012"
                      "такие страницы невозможно напечатать на данном принтере. \015\012"
                      "Эти страницы собраны в файл\015\012 \"" + ShortFiln + (String)"_A00.PDF"+ (String)"\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
    }
}
*/



void __fastcall TForm1::OkButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
// Нов версия - с постепенной заменой эмуляции человека на обращения к ActiveX  |
//-------------------------------------------------------------------------------
    TWinInfo wi0, wi1;
    HANDLE chan;
    HANDLE hProcess;
    String Filn, S, PrinterName;
    int xb, yb, rest;

    //PrinterSetupDialog1->Execute();
    //PrintDialog1->Execute();

    A4LCreated = A4PCreated = A3LCreated = A3PCreated = A0Created = A00Created = false;

    Filn = FilnEdit->Text;
    if (Filn.IsEmpty()) {
        Application->MessageBox("Сначала надо задать имя печатаемого файла", "Невозможно!", 0);
        return;
    }
    chan = CreateFile(Filn.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan == INVALID_HANDLE_VALUE) {
        String mes;
        mes = (String)"Не могу открыть файл \"" + Filn + (String)"\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return;
    }

    MainPath = EraseFilnFromPath(Filn);
    ShortFiln = TrimExt(ErasePathFromFiln(Filn));

    Variant avdoc, pddoc;
    String SS;
    int tt, nn;
    SS = RepeatEdit->Text; nn = SS.ToInt();

    for (tt = 0; tt < nn; tt++) {
        if (tt > 0) {
            wsprintf(LogBuf, "Ok N %d", tt);
            LogForm->Caption = LogBuf;
            Application->ProcessMessages();
        }
        log("");
        log("------------------------------------------------------------");
        // StartAcrobat(pi, MainPath, ShortFiln + (String)".pdf");
        hProcess = pi.hProcess;
        Ocrobat = CreateOleObject("AcroExch.App");

        avdoc = Ocrobat.OleFunction("GetActiveDoc");
        pddoc = avdoc.OleFunction("GetPDDoc");

        ClassifyDocPages(pddoc);

        if (A4LListCnt == 0 && A4PListCnt == 0 && A3LListCnt == 0 && A3PListCnt == 0 && A0ListCnt == 0 && A00ListCnt == 0) {
            Application->MessageBox("Не сформированы списки файлов *.tif!", "Не хватает!", 0);
            return;
        }

        if (A4PListCnt > 0) {                           // A4 портрет
            CreatePDF("A4P", tpkA4P);
            A4PCreated = true;
        }
        if (A4LListCnt > 0) {                           // A4 ландшафт
            CreatePDF("A4L", tpkA4L);
            A4LCreated = true;
        }
        if (A3PListCnt > 0) {                           // A3 портрет
            CreatePDF("A3P", tpkA3P);
            A3PCreated = true;
        }
        if (A3LListCnt > 0) {                           // A3 ландшафт
            CreatePDF("A3L", tpkA3L);
            A3LCreated = true;
        }
        if (A0ListCnt > 0) {
            CreatePDF("A0", tpkA0);
            A0Created = true;
        }
        if (A00ListCnt > 0) {                           // A00
            CreatePDF("A00", tpkA00);
            A00Created = true;
        }

        if (A00Created) {
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A00");
        }
        if (A0Created) {
            if (PrintA0CheckBox->Checked) {
                S = A0CopiesEdit->Text;
                PrinterName = P0Edit->Text;
                // aWait(300);
                PrintPDF(PrinterName, S);
            //} else {
                // aWait(1000);
            }
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A0");
        }

        if (A3LCreated) {                       // A3 Ландшафт
            if (PrintA3CheckBox->Checked) {
                S = A3CopiesEdit->Text;
                PrinterName = P3Edit->Text;
                // aWait(300);
                PrintPDF(PrinterName, S);
            //} else {
                // aWait(1000);
            }
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A3 Ландшафт");
        }

        if (A3PCreated) {                       // A3 Портрет
            if (PrintA3CheckBox->Checked) {
                S = A3CopiesEdit->Text;
                PrinterName = P3Edit->Text;
                // aWait(300);
                PrintPDF(PrinterName, S);
            //} else {
                //aWait(1000);
            }
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A3 Портрет");
        }

        if (A4LCreated) {                       // A4 Ландшафт
            if (PrintA4CheckBox->Checked) {
                S = A4CopiesEdit->Text;
                PrinterName = P4Edit->Text;
                // aWait(300);
                PrintPDF(PrinterName, S);
            //} else {
                //aWait(1000);
            }
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A4 Ландшафт");
        }

        if (A4PCreated) {                       // A4 Портрет
            if (PrintA4CheckBox->Checked) {
                S = A4CopiesEdit->Text;
                PrinterName = P4Edit->Text;
                // aWait(300);
                PrintPDF(PrinterName, S);
            //} else {
                //aWait(1000);
            }
            avdoc = Ocrobat.OleFunction("GetActiveDoc");
            avdoc.OleFunction("Close", 1);
            log("Закрыли файл A4 Портрет");
        }

        avdoc = Ocrobat.OleFunction("GetActiveDoc");    // Исходный файл тоже надо закрыть
        avdoc.OleFunction("Close", 1);
        rest = Ocrobat.OleFunction("GetNumAVDocs");
        if (rest == 0) {
            TerminateProcess(hProcess , 0);
        }

        Application->BringToFront();
        log("================= Все сделано ====================\015\012");
    }

    if (A00Created) {
        String mes;
        mes = (String)"Одна или несколько страниц имеют минимальный размер более 841 мм - \015\012"
                      "такие страницы невозможно напечатать на данном принтере. \015\012"
                      "Эти страницы собраны в файл\015\012 \"" + ShortFiln + (String)"_A00.PDF"+ (String)"\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
    }
}


/*

14:15:55.0609 00875 ====== PrintPDF ====== printer=2, copies=1
14:15:55.0625 00877 Фокус ввода
14:15:56.0328 00894 первый принтер в списке принтеров
14:15:56.0578 00900 нужный принтер в списке принтеров
14:15:56.0843 00905 количество копий
14:15:57.0937 00912 запуск печати
14:15:58.0078 00920 дождались начала печати
14:16:01.0578 00925 дождались окончания печати
14:16:01.0687 01179 Ok: запускаем закрытие файла A0
14:16:01.0968 01189 Ok: дождались закрытия файла A0
14:16:02.0312 00875 ====== PrintPDF ====== printer=3, copies=1
14:16:02.0312 00877 Фокус ввода
14:16:02.0937 00894 первый принтер в списке принтеров
14:16:03.0187 00900 нужный принтер в списке принтеров
14:16:03.0562 00905 количество копий
14:16:04.0640 00912 запуск печати
14:16:04.0796 00920 дождались начала печати
14:17:24.0281 00925 дождались окончания печати
14:17:24.0421 01209 Ok: запускаем закрытие файла A4
14:17:24.0671 01219 Ok: дождались закрытия файла A4
14:17:25.0515 01227 ================= Все сделано ====================

*/


void __fastcall TForm1::TestOLEClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//                  Проверка возможности работать через OLE                     |
//-------------------------------------------------------------------------------
/*    Dim app As Object, avdoc As Object, pddoc As Object
    Dim nn As Long
    Set app = CreateObject("AcroExch.App")
    Set avdoc = app.GetActiveDoc
    Set pddoc = avdoc.GetPDDoc
    nn = pddoc.GetNumPages
*/

/*  // Связывается с текущим акробатом, берет у него активный документ и перебирает все его страницы
    Variant app, avdoc, pddoc, pg, sz;
    int i, n, x, y;
    app = CreateOleObject("AcroExch.App");
    avdoc = app.OleFunction("GetActiveDoc");
    pddoc = avdoc.OleFunction("GetPDDoc");
    n = pddoc.OleFunction("GetNumPages");
    for (i = 0; i < n; i++) {
        pg = pddoc.OleFunction("AcquirePage", i);
        sz = pg.OleFunction("GetSize");
        x = sz.OlePropertyGet("x");
        y = sz.OlePropertyGet("y");
        stop += x + y;
    }
*/

/*
    Variant app, avdoc, pddoc, pg, sz;
    int i, n;

    stop++;

    app = CreateOleObject("AcroExch.App");
    stop = app.OleFunction("GetNumAVDocs");

    // StartAcrobat(pi, "C:\\UnitedProjects\\Переформатирование PDF\\", "C:\\UnitedProjects\\Переформатирование PDF\\RUSA-LNY-01-TD-61600_A0.pdf");
    StartAcrobat(pi, "C:\\UnitedProjects\\Переформатирование PDF\\", "RUSA-LNY-01-TD-61600_A0.pdf");

    // app = CreateOleObject("AcroExch.App");
    stop = app.OleFunction("GetNumAVDocs");
    avdoc = app.OleFunction("GetActiveDoc");
    pddoc = avdoc.OleFunction("GetPDDoc");
    n = pddoc.OleFunction("GetNumPages");
    // stop = pddoc.OleFunction("DeletePages", 2, 3);
    for (i = n - 1; i >= 0; i--) {
        stop = pddoc.OleFunction("DeletePages", i, i);
        stop++;
    }
    stop = pddoc.OleFunction("GetNumPages");
    stop = pddoc.OleFunction("Save", 1, "C:\\UnitedProjects\\Переформатирование PDF\\RUSA-LNY-01-TD-61600_A0.pdf");
    stop = avdoc.OleFunction("Close", 1);
    stop = app.OleFunction("GetNumAVDocs");
    avdoc = Unassigned;
    pddoc = Unassigned;
    if (stop == 0) {
        // stop = app.OleFunction("Exit");
        TerminateProcess(pi.hProcess , 0);
    }
*/

}



// TPrinter

TPrinter *aPrinter;
String PrinterList;
int PrinterIndex;

void __fastcall TForm1::Button10Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    char retstring[200];
/*
DWORD GetProfileString(
    LPCTSTR lpAppName,	// address of section name
    LPCTSTR lpKeyName,	// address of key name
    LPCTSTR lpDefault,	// address of default string
    LPTSTR lpReturnedString,	// address of destination buffer
    DWORD nSize 	// size of destination buffer
   );


The PrinterProperties function displays a printer-properties dialog box for
the specified printer.

BOOL PrinterProperties(
    HWND hWnd,	// handle of parent window
    HANDLE hPrinter 	// handle of printer object
   );

void __fastcall GetPrinter(char * ADevice, char * ADriver, char * APort, int &ADeviceMode);

*/

    static char pi1buf[164];  PRINTER_INFO_1 *pi1 = (PRINTER_INFO_1 *)pi1buf;
    static char pi2buf[2576]; PRINTER_INFO_2 *pi2 = (PRINTER_INFO_2 *)pi2buf;
    static char pi3buf[300];  PRINTER_INFO_3 *pi3 = (PRINTER_INFO_3 *)pi3buf;
    static char pi4buf[60];   PRINTER_INFO_4 *pi4 = (PRINTER_INFO_4 *)pi4buf;
    static char pi5buf[84];   PRINTER_INFO_5 *pi5 = (PRINTER_INFO_5 *)pi5buf;
    DWORD need;
    HANDLE hPrinter;

    unsigned adevmode = 12345;
    char adev[50], adriv[50], aport[50];

    GetFullPrinterR("EPSON Stylus C79 Series");

//    for (stop = 1000; stop > 0; stop--) {
//        stop--;
        GetProfileString("windows", "device", "nodefault", retstring, 200);
//    }

    WriteProfileString("windows", "device", "EPSON Stylus C79 Series,winspool,Ne08:");

    aPrinter = Printer();

    stop = aPrinter->Printers->Count;
    PrinterList = aPrinter->Printers->Text;
    PrinterIndex = aPrinter->PrinterIndex;
    aPrinter->GetPrinter(adev, adriv, aport, adevmode);

    stop = ::OpenPrinter(adev, &hPrinter, NULL); need = 0;
    memset(&pi1, sizeof(PRINTER_INFO_1), 0);
    memset(&pi2, sizeof(PRINTER_INFO_2), 0);
    memset(&pi3, sizeof(PRINTER_INFO_3), 0);
    memset(&pi4, sizeof(PRINTER_INFO_4), 0);
    memset(&pi5, sizeof(PRINTER_INFO_5), 0);

    stop = ::GetPrinter(aPrinter->Handle, 1, (LPBYTE)(pi1), sizeof(pi1buf), &need);
    stop = ::GetPrinter(hPrinter, 1, (LPBYTE)(pi1), sizeof(pi1buf), &need);
    stop = ::GetPrinter(hPrinter, 2, (LPBYTE)(pi2), sizeof(pi2buf), &need);
    stop = ::GetPrinter(hPrinter, 3, (LPBYTE)(pi3), sizeof(pi3buf), &need);
    stop = ::GetPrinter(hPrinter, 4, (LPBYTE)(pi4), sizeof(pi4buf), &need);
    stop = ::GetPrinter(hPrinter, 5, (LPBYTE)(pi5), sizeof(pi5buf), &need);

    PrinterSetupDialog1->Execute();
    stop++;
    PrinterIndex = aPrinter->PrinterIndex;
    aPrinter->PrinterIndex = stop;
    aPrinter->GetPrinter(adev, adriv, aport, adevmode);

    PrinterSetupDialog1->Execute();
    stop++;
    aPrinter->GetPrinter(adev, adriv, aport, adevmode);

}

/*

To get information about the default printer, call the GetProfileString
function with the section name string set to "windows" and the key name
string set to "device". The returned string contains the name of the
default printer, the name of the printer DRV file, and the port to which
the printer is attached.

*/





