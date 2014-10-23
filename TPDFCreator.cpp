//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#pragma package(smart_init)


#include "TPDFCreator.h"
#include "SEnumMainUnit.h"
#include "tlhelp32.h"
#include <Registry.hpp>

#include "SEnumLib1.h"
#include "SEnumLib.h"
#include "WatchPrintSaveAsUnit.h"

//---------------------------------------------------------------------------

/*

    Variant v;
    char *filn = "D:\\0910_08.pdf"; 
    try {
        // PC = GetActiveOleObject("PDFCreator.clsPDFCreator");
        PC = CreateOleObject("PDFCreator.clsPDFCreator");

        PC.OleProcedure("cClose");
        
        PC.OleProcedure("cStart");
        //v = PC.OlePropertyGet("cOption", "StandardSaveformat");
        //stop = v;

        PC.OlePropertySet("cOption", "StandardSaveformat", 7);
        PC.OlePropertySet("cOption", "AutosaveFormat", 5);
        PC.OlePropertySet("cOption", "UseAutosaveDirectory", 1);
        PC.OlePropertySet("cOption", "AutosaveDirectory", "d:\\For\\PDF\\Creator");
        PC.OlePropertySet("cOption", "AutosaveFilename", "Test123.tif");
        PC.OlePropertySet("cOption", "UseAutosave", 1);
        PC.OleProcedure("cClearCache");
        PC.OleProcedure("cSaveOptions");
        PC.OleProcedure("cPrintFile", filn);
        PC.OlePropertySet("cOption", "UseAutosave", 0);
        PC.OleProcedure("cSaveOptions");

        PC.OleProcedure("cClose");
        PC = Unassigned;
    } catch(...) {
        stop++;
    }
    stop++;

*/



static char* TCHARToChar(const wchar_t* src, char* dest) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    int size =  wcslen(src);
    for (int i=0;i<=size;i++)
    {
        dest[i] = src[i];
    }
    //MultiByteToWideChar(1251,0,src,size,dest,size);
    return dest;
}


// static bool __fastcall IsProcessStarted(PROCESS_INFORMATION pi) {
static bool __fastcall IsProcessStarted(DWORD pi) {
//-------------------------------------------------------------------------------
//                  Проверяет запущен ли в данный момент процесс pi             |
//-------------------------------------------------------------------------------
    HANDLE hProcessSnap;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL fOk = Process32First(hProcessSnap,&pe);
    bool res = false;
    for (; fOk; fOk = Process32Next(hProcessSnap,&pe)) {
        if (pe.th32ProcessID == pi) {
            res = true;
            break;
        }
    }
    CloseHandle(hProcessSnap);
    return res;
}


static String __fastcall GetFullPrinterR(char *printer) {
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

    r->CloseKey();
    delete r;
    return (String)printer + (String)"," + res;
}



DWORD __fastcall TPDFCreator::FindProcess(char * ProcessName) {
//-------------------------------------------------------------------------------
//				   Ищет запущенный процесс с именем ProcessName                 |
// Йэсли не находит, возвращает 0												|
//-------------------------------------------------------------------------------
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(pe) };
	BOOL fOk = Process32First(hProcessSnap,&pe);
    DWORD res = 0;
	for (; fOk; fOk = Process32Next(hProcessSnap,&pe)) {
		if (strstr(pe.szExeFile, ProcessName)) {
            res = pe.th32ProcessID;
            break;
		}
	}

    CloseHandle(hProcessSnap);
	return res;
}


DWORD TPDFCreator::FindPDFCreator(void) {
//-------------------------------------------------------------------------------
//							Ищет запущенный PDFCreator                          |
// Йэсли не находит, возвращает 0												|
//-------------------------------------------------------------------------------
/*
	HANDLE hProcessSnap;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(pe) };
	BOOL fOk = Process32First(hProcessSnap,&pe);
	// char* sz = new char[1024];
    DWORD res = 0;
	for (; fOk; fOk = Process32Next(hProcessSnap,&pe)) {
		// TCHARToChar(pe.szExeFile,sz);
		// if (strstr(sz,"acad.exe")) {
		if (strstr(pe.szExeFile,"PDFCreator.exe")) {
			// delete[] sz;
			// return pe.th32ProcessID;
            res = pe.th32ProcessID;
            break;
		}
	}

	// delete[] sz;
    CloseHandle(hProcessSnap);
	return res;
*/
    return FindProcess("PDFCreator.exe");
}


DWORD __fastcall TPDFCreator::FindAcrobat(void) {
//-------------------------------------------------------------------------------
//                  Ищет запущенный Acrobat.exe или AcroRd32.exe                |
// Йэсли не находит, возвращает 0												|
//-------------------------------------------------------------------------------
    DWORD res;
    res = FindProcess("Acrobat.exe");
    if (res == 0) res = FindProcess("AcroRd32.exe");
    return res;
}


void __fastcall TPDFCreator::KillAcrobat(void) {
//-------------------------------------------------------------------------------
//                  Убивает процесс Acrobat.exe и AcroRd32.exe                  |
//-------------------------------------------------------------------------------
    DWORD pid;
    char *pname;

    pname = "Acrobat.exe"; pid = FindProcess(pname);
    if (pid == 0) {
        pname = "AcroRd32.exe"; pid = FindProcess(pname);
    }
    if (pid != 0) KillProcess(pid, pname);
}


bool __fastcall TPDFCreator::KillAcrobatDeath(void) {
//-------------------------------------------------------------------------------
//              Сначала просит пользователя, а потом убивает все процессы       |
// Acrobat.exe и AcroRd32.exe                                                   |
// Если пользователь отказывается - не убивает.                                 |
// Возвращает false если пользователь отказывается                              |
//-------------------------------------------------------------------------------
    if (!FindAcrobat()) return true;

    int res;
    char *mes = "Для печати файлов PDF необходимо закрыть все открытые в настоящее время файлы PDF.\n"
                "Нажмите OK, когда закроете все файлы PDF или ОТМЕНА, чтобы отказаться от печати";
    res = Application->MessageBox(mes, "Есть проблема!", MB_OKCANCEL);
    if (res == IDCANCEL) return false;
    if (FindAcrobat()) {
        mes = "Вы не закрыли все файлы PDF. Хотите, чтобы программа нумерации закрыла их насильственно?\n"
              "Нажмите ДА, если хотите, НЕТ - если нет (в этом случае задание на печать отменяется)";
        res = Application->MessageBox(mes, "Не могу!", MB_YESNO);
        if (res != IDYES) return false;
        while(FindAcrobat()) KillAcrobat();
    }

    return true;
}



void __fastcall TPDFCreator::KillProcess(DWORD ProcessID, String ProcessName) {
//-------------------------------------------------------------------------------
//					Убивает нАсмерть указанный автокадовский процесс			|
// Безусловно полагает, что процесс безусловно автокадовский					|
//-------------------------------------------------------------------------------
	DWORD aProc;
	HANDLE hProc;
    String cmd = (String)"cmd.exe /c taskkill /f /im " + ProcessName;
	// char cmd[] = "cmd.exe /c taskkill /f /im PDFCreator.exe";

	aProc = ProcessID;
	while(IsProcessStarted(aProc)) {
		hProc = OpenProcess(PROCESS_ALL_ACCESS,NULL, aProc);
		TerminateProcess(hProc, 0);
		CloseHandle(hProc);
		for (int i = 0; i < 20; i++) {
			aWait(200);
			hProc = OpenProcess(PROCESS_ALL_ACCESS,NULL, aProc);
			if (hProc == NULL) return;
			CloseHandle(hProc);
		}
		::WinExec(cmd.c_str(), SW_HIDE);
		for (int i = 0; i < 20; i++) {
			aWait(200);
			hProc = OpenProcess(PROCESS_ALL_ACCESS,NULL, aProc);
			if (hProc == NULL) return;
			CloseHandle(hProc);
		}
		aWait(200);
	}

}

void __fastcall TPDFCreator::KillPDFCreatorProcess(void) {
//-------------------------------------------------------------------------------
//                              Убивает процесс PDFCreator                      |
//-------------------------------------------------------------------------------
    DWORD PDFCreatorID;
    while(1) {
        PDFCreatorID = FindPDFCreator();
        if (PDFCreatorID == 0) break;
        KillProcess(PDFCreatorID);
    }
}


bool __fastcall TPDFCreator::SetDefaultPrinter(void) {
//-------------------------------------------------------------------------------
//          Делает принтер PDFCreator текущим принтером по умолчанию            |
//-------------------------------------------------------------------------------
    String NewCurPrinter;
    char nn[100], OldCurPrinter[100];
    GetProfileString("windows", "device", "nodefault", OldCurPrinter, 100);
    NewCurPrinter = GetFullPrinterR("PDFCreator");
    WriteProfileString("windows", "device", NewCurPrinter.c_str());
    GetProfileString("windows", "device", "nodefault", nn, 100);
    if (!IsSubstr(nn, "PDFCreator", false)) {
        WriteProfileString("windows", "device", OldCurPrinter);
        return false;
    }
    OldDefaultPrinter = OldCurPrinter;
    TiffDefaultPrinter = "PDFCreator";
    return true;
}


void __fastcall TPDFCreator::RestoreDefaultPrinter(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!ok) return;
    WriteProfileString("windows", "device", OldDefaultPrinter.c_str());
}



TPDFCreator::TPDFCreator(void) {
//-------------------------------------------------------------------------------
//                                  Страшная сила:                              |
// 1. Завершает уже работающий PDFCreator                                       |
// 2. Запускает PDFCreator (если не удается - дальше все)                       |
// 3. Делает PDFCreator принтером по умолчанию.                                 |
// В случае успеха ok == true                                                   |
//-------------------------------------------------------------------------------
    ok = false;
    TiffFiln = "";
    AcrobatListCount = 0;

    stop++;

    if (!SetDefaultPrinter()) return;
    stop++;

    KillPDFCreatorProcess();

    try {
        PC = CreateOleObject("PDFCreator.clsPDFCreator");
        PC.OleProcedure("cStart");
    } catch(...) {
        KillPDFCreatorProcess();
        return;
    }

    ok = true;
}

TPDFCreator::~TPDFCreator(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!ok) return;

    RestoreDefaultPrinter();
    ClearUseAutosave();
    PC.OleProcedure("cClose");
    PC = Unassigned;
    aWait(100);
    
    KillPDFCreatorProcess();
}


void __fastcall TPDFCreator::CreateTempFiln(String ext) {
//-------------------------------------------------------------------------------
//                   Создает временный файл с указанным расширением             |
// В результате оказываются заполненными поля TempPath, TempFiln                |                
//-------------------------------------------------------------------------------
    String filn;
    int filn_n;
    static char *FilnPattern = "SEnum2PDFCreator%s%04d.%s";
    char path[300], afiln[50];

    filn_n = 0;
    // ::GetTempPath(300, path);
    aGetTempPath(300, path);
    // strcpy(path, "d:\\For\\PDF\\Creator");
    // strcpy(path, "c:\\UnitedProjects\\Нумерация страниц смет\\exe\\PDFCreatorTest\\Tiff");
    while(1) {
        wsprintf(afiln, FilnPattern, ext.c_str(), filn_n, ext.c_str());
        // filn = (String)path + "\\" + afiln;
        filn = (String)path + afiln;
        DWORD A = GetFileAttributes(filn.c_str());
        if (A != 0xFFFFFFFF) {
            DeleteFile(filn.c_str());
            A = GetFileAttributes(filn.c_str());
        }
        if (A == 0xFFFFFFFF) break;
        filn_n++;
    }

    TempPath = path;
    TempFiln = afiln;
}


String __fastcall TPDFCreator::SetupForPrint(String FullFiln) {
//-------------------------------------------------------------------------------
//          Устанавливает опции для печати документа (в т.ч. UseAutosave)       |
// Если create_temp_filn - то заодно и создает имя временного файла             |
//-------------------------------------------------------------------------------
    stop++;
    if (FullFiln.IsEmpty()) {
        CreateTempFiln("tif");
    } else {
        TempPath = EraseFilnFromPath(FullFiln);
        TempFiln = ErasePathFromFiln(FullFiln);
    }
    TiffFiln = FullFiln;
    PC.OlePropertySet("cOption", "StandardSaveformat", 7);
    PC.OlePropertySet("cOption", "Papersize", "A4");
    PC.OlePropertySet("cOption", "DeviceHeightPoints", 842);
    PC.OlePropertySet("cOption", "DeviceWidthPoints", 595);
    PC.OlePropertySet("cOption", "AutosaveFormat", 5);
    PC.OlePropertySet("cOption", "TIFFColorscount", 2);
    PC.OlePropertySet("cOption", "TIFFResolution", 300);
    PC.OlePropertySet("cOption", "UseAutosaveDirectory", 1);
    PC.OlePropertySet("cOption", "AutosaveDirectory", TempPath.c_str());
    PC.OlePropertySet("cOption", "AutosaveFilename", TempFiln.c_str());
    PC.OlePropertySet("cOption", "UseAutosave", 1);
    PC.OleProcedure("cClearCache");
    PC.OleProcedure("cSaveOptions");
    aWait(100);
    if (!FullFiln.IsEmpty()) {
        return FullFiln;
    }
    // return TempPath + "\\" + TempFiln;
    return TempPath + TempFiln;
}

void __fastcall TPDFCreator::WaitForPrinted(String filn) {
//-------------------------------------------------------------------------------
//                  Ждет, пока указанный файл не будет напечатан                |
// Пока ждет бесконечно, а потом - видно будет...                               |
//-------------------------------------------------------------------------------
    DWORD attri;
    HANDLE h;

    stop++;

    while(1) {
        aWait(100);
        attri = (int)GetFileAttributes(filn.c_str());
        if (attri == 0xFFFFFFFF) {
            stop++;
            continue;
        }
        if (attri & FILE_ATTRIBUTE_READONLY) {
            stop++;
            continue;
        }
        h = CreateFile(filn.c_str(), GENERIC_READ + GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        CloseHandle(h);
        if (h != INVALID_HANDLE_VALUE) {
            stop++;
            break;
        }
    }
}


void __fastcall TPDFCreator::ClearUseAutosave(void) {
//-------------------------------------------------------------------------------
//                              Чистит флаг UseAutosave                         |
//-------------------------------------------------------------------------------
    aWait(50);
    PC.OlePropertySet("cOption", "UseAutosave", 0);
    PC.OleProcedure("cSaveOptions");
    aWait(50);

    static String AutoSavePath = "Software\\PDFCreator\\Program";
    TRegistry *r = new TRegistry;
    String ass;

    r->RootKey = HKEY_CURRENT_USER;
    r->Access = KEY_QUERY_VALUE | KEY_WRITE;
    try {
        if (!r->OpenKey(AutoSavePath, false)) {
            delete r;
            return;
        }
        ass = r->ReadString("UseAutosave");
        if (!ass.IsEmpty() && ass != "0") {
            r->WriteString("UseAutosave", "0");
        }
    } catch(...) {
        delete r;
        return;
    }

    r->CloseKey();
    delete r;
}



String __fastcall TPDFCreator::PrintPDF(String filn) {
//-------------------------------------------------------------------------------
//                              Печатает файл PDF                               |
// Возвращает имя файла tiff                                                    |
//-------------------------------------------------------------------------------
    String TiffFiln = SetupForPrint("");

    if (TiffFiln.Length() == 0) return TiffFiln;

    PrintPDF(TiffFiln, filn);
    return TiffFiln;
}

void __fastcall TPDFCreator::PrintPDF(String _TiffFiln, String filn) {
//-------------------------------------------------------------------------------
//                              Печатает файл PDF                               |
// Возвращает имя файла tiff                                                    |
//-------------------------------------------------------------------------------
    SetupForPrint(_TiffFiln);

    IniCatcher(SEnumMainForm->Handle, "TPDFCreator::PrintPDF", this);
    PC.OleProcedure("cPrintFile", filn.c_str());
    WaitForPrinted(TiffFiln);
    ClearUseAutosave();
}


String __fastcall TPDFCreator::PrintDOC(Variant WA) {
//-------------------------------------------------------------------------------
//                              Печатает файл DOC                               |
// WA должен содержать текущим файл, который надо напечатать                    |
// Возвращает имя файла tiff                                                    |
//-------------------------------------------------------------------------------
    String TiffFiln = SetupForPrint("");
    if (TiffFiln.Length() == 0) return TiffFiln;

    PrintDOC(TiffFiln, WA);
    return TiffFiln;
}


void __fastcall TPDFCreator::PrintDOC(String TiffFiln, Variant WA) {
//-------------------------------------------------------------------------------
//                              Печатает файл DOC                               |
// WA должен содержать текущим файл, который надо напечатать                    |
//-------------------------------------------------------------------------------
    SetupForPrint(TiffFiln);

    IniCatcher(SEnumMainForm->Handle, "TPDFCreator::PrintDOC");
    WA.OleProcedure("Activate");
    WA.OleProcedure("PrintOut");
    // RunCatcher();

    WaitForPrinted(TiffFiln);
    ClearUseAutosave();

    RunCatcher();
}


String __fastcall TPDFCreator::PrintXLS(Variant ActiveSheet) {
//-------------------------------------------------------------------------------
//                              Печатает файл DOC                               |
// ActiveSheet должен содержать текущим файл, который надо напечатать           |
// Возвращает имя файла tiff                                                    |
//-------------------------------------------------------------------------------
    String TiffFiln = SetupForPrint("");
    if (TiffFiln.Length() == 0) return TiffFiln;

    PrintXLS(TiffFiln, ActiveSheet);
    return TiffFiln;
}


void __fastcall TPDFCreator::PrintXLS(String TiffFiln, Variant ActiveSheet) {
//-------------------------------------------------------------------------------
//                              Печатает файл DOC                               |
// ActiveSheet должен содержать текущим файл, который надо напечатать           |
//-------------------------------------------------------------------------------
    SetupForPrint(TiffFiln);

    Variant HPageBreaks, VPageBreaks;
    int n;
    HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
    VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
    n = (HPageBreaks.OlePropertyGet("Count") + 1) * (VPageBreaks.OlePropertyGet("Count") + 1);
    ActiveSheet.OleProcedure("PrintOut", 1, n, 1, false, "PDFCreator");
    WaitForPrinted(TiffFiln);
    ClearUseAutosave();
    // return TiffFiln;
}


int __fastcall TPDFCreator::CountPages(String filn) {
//-------------------------------------------------------------------------------
//              Если отрицательное количество страниц, то возвращает -1         |
//-------------------------------------------------------------------------------
    String TiffFiln = PrintPDF(filn);
    int res;
    StartGdiplus();
    if (LoadTiffFile(TiffFiln.c_str()) == NULL) {
        // String mes = (String)"Ошибка обработки файла \"" + filn + "\"";
        // Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        res = -1;
    } else {
        res = LoadTiffFileNpages;
    }
    ReleaseTiffFile();
    StopGdiplus();
    return res;
}







