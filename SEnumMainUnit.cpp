//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SEnumMainUnit.h"
#include "WatchPrintSaveAsUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "MyGrid"
// #pragma link "Excel_2K_SRVR"
#pragma link "UExcelAdapter"
#pragma link "XLSAdapter"
#pragma link "UFlexCelImport"
#pragma link "FolderDialog"
#pragma link "FolderDialog"
#pragma resource "*.dfm"

#include "SEnumLib.h"
#include "SEnumLib1.h"
#include "SaveUnit.h"
#include "ContentsUnit.h"
#include "EditTiffUnit.h"
#include "TPDFCreator.h"

#ifdef GUARD
#include "lib/guard/guard.h"
#endif


/*
    Предложения по улучшению:
1. Выводить в цветном виде - через tiff или через библиотеку pdf
2. Улучшить редактирование файла tiff
3. Сделать инструмент для вырезания подписей

*/



extern String sstop;

TSEnumMainForm *SEnumMainForm;

TFileList *fl;
TConfig *cfg;

static TSaveParams sp = TSaveParams("", 1, 1, true, true);
static TSaveParams contents_sp = TSaveParams("", 1, 1, true, true);


//---------------------------------------------------------------------------

__fastcall TSEnumMainForm::TSEnumMainForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#ifdef GUARD
    proto("TSEnumMainForm::GuardFunc2();...");
    GuardFunc2();
    decrypt_lic();
    GuardFuncCRC();
#endif
    fl = new TFileList;



}

void __fastcall TSEnumMainForm::FormPaint(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    static bool first = true;
    if (!first) return;
    first = false;

    cfg = new TConfig;
    cfg->Load();
}



void __fastcall TSEnumMainForm::FormDestroy(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    StopSaveBitmap2Tiff();
    ReleaseTiffFile();
    StopGdiplus();
    delete fl;
    // RestoreOldPrinter();
    if (cfg != NULL) cfg->Save();
}

void __fastcall TSEnumMainForm::SListMyGridGetPoleAdr(TObject *Sender,
      TMyColumn *c, char *&Adr, bool &f) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    static char dummy[2];
#ifdef GUARD
    proto("SListMyGridGetPoleAdr::GuardFunc3();");
    GuardFunc3();
#endif
    Adr = dummy;
}

void __fastcall TSEnumMainForm::SListMyGridUnPack(TObject *Sender,
      TMyColumn *c, char *Adr, AnsiString &s, bool &f) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TFileListRec *r;
    int index = ((TMyGrid *)Sender)->RecNum;
    if (index < 0 || index >= fl->Nrecs) {
        s = "";
        return;
    }
    r = fl->Rec1(index);
    switch(c->UniqueIndex) {
        // case 1: s = r->nsm; if (s.IsEmpty()) s = "nsm " + (String)index; break;
        case 1: s = (String)(index + 1); break;
        case 2: s = r->path; if (s.IsEmpty()) s = "path " + (String)index; break;
        case 3: s = r->filn; if (s.IsEmpty()) s = "filn " + (String)index; break;
        case 4: s = r->name; if (s.IsEmpty()) s = "name " + (String)index; break;
        case 5: s = r->oname; if (s.IsEmpty()) s = "oname " + (String)index; break;
        case 6: s = r->rem; break;
    }
}

void __fastcall TSEnumMainForm::SListMyGridKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// SListMyGridBtnUser3(TObject *Sender, int Row, TMyColumn *c) {
#ifdef GUARD
    proto("SListMyGridKeyDown::GuardFunc2();");
    GuardFunc2();
#endif
    if (Key == VK_DELETE && !Shift.Contains(ssCtrl) && !Shift.Contains(ssShift)) {
        SListMyGridBtnUser2(0, 0, 0);
        return;
    }
    if (!Shift.Contains(ssCtrl)) return;
    if (Key == VK_UP) {
        SListMyGridBtnUser3(0, 0, 0);
    } else if (Key == VK_DOWN) {
        SListMyGridBtnUser4(0, 0, 0);
    }
    Key = 0;
}


void __fastcall ProcessInfoFiln(char *filn) {
//-------------------------------------------------------------------------------
//              Выводит имя файла в ProcessInfoPanel->FilnLabel                 |
//-------------------------------------------------------------------------------
#ifdef GUARD
    proto("ProcessInfoFiln::GuardFuncPriv3();");
    GuardFuncPriv3();
#endif
    SEnumMainForm->FilnLabel->Caption = filn;
    Application->BringToFront();
    Application->ProcessMessages();
}

void __fastcall ProcessInfoAction(char *filn) {
//-------------------------------------------------------------------------------
//              Выводит имя файла в ProcessInfoPanel->ActionLabel               |
//-------------------------------------------------------------------------------
    SEnumMainForm->ActionLabel->Caption = filn;
    Application->BringToFront();
    Application->ProcessMessages();
}


bool __fastcall TSEnumMainForm::PrintFilesLoaded(char *filn, TFileList *fl, int f1, int p0) {
//-------------------------------------------------------------------------------
//       Делает видимой панели ProcessInfoPanel, NoMousePanel и печатает fl     |
// После этого прячет эти панели взад                                           |
//-------------------------------------------------------------------------------
    FilnLabel->Caption = "Создание файла *.tif";
    ActionLabel->Caption = "";
    ProcessInfoPanel->Visible = true;
    NoMousePanel->Visible = true;
    NeedBreakProcess = false;
    Application->ProcessMessages();
    bool res = FileList2Tiff(filn, fl, f1, p0);
    ProcessInfoPanel->Visible = false;
    NoMousePanel->Visible = false;
    return res;
}

void __fastcall TSEnumMainForm::PDF1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                              Печатать собранный комплект в PDF               |
//-------------------------------------------------------------------------------
    // TestTiff(Image1);
    // Doit(fl);
    // FindLastCell2(TPoint(17, 3200));
    // TestPrinters();

    if (TestFilesLoaded(fl, false, true)) return;
    if (!AskSaveTiff(&sp)) return;

    NeedBreakButton->Visible = true;

    //FilnLabel->Caption = "Создание файла *.tif";
    //ActionLabel->Caption = "";
    //ProcessInfoPanel->Visible = true;
    //NoMousePanel->Visible = true;
    //NeedBreakProcess = false;
    //FileList2Tiff(sp.Filn.c_str(), fl, sp.f1, sp.p0);
    //ProcessInfoPanel->Visible = false;
    //NoMousePanel->Visible = false;
    bool res = PrintFilesLoaded(sp.Filn.c_str(), fl, sp.f1, sp.p0);

    NeedBreakButton->Visible = false;

    if (NeedBreakProcess || !res) {
        Application->MessageBox("Печать собранного комплекта отменена", 0);
    } else {
        Application->MessageBox("Печать собранного комплекта завершена", 0);
    }
}

void __fastcall TSEnumMainForm::NeedBreakButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    NeedBreakProcess = true;
}



void __fastcall TSEnumMainForm::N5Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                              Указать папку с подписями                       |
//-------------------------------------------------------------------------------
#ifdef GUARD
    proto("N5Click::GuardFuncPriv4();");
    GuardFuncPriv4();
#endif
    String SPath;
    if (TestFilesLoaded(fl, false, true)) return;
    if (!SignatureFolderDialog->Execute()) return;
    SPath = SignatureFolderDialog->Directory;
    if (!SPath.IsEmpty()) cfg->SPath = SPath;
    FilnLabel->Caption = "Простановка подписей в файлах сметы";
    ActionLabel->Caption = "";
    ProcessInfoPanel->Visible = true;
    NoMousePanel->Visible = true;
    FormStyle = fsStayOnTop;
    Subscribe(fl, SPath);
    ProcessInfoPanel->Visible = false;
    NoMousePanel->Visible = false;
    FormStyle = fsNormal;
    SListMyGrid->SetAllReRead();
    SListMyGrid->InvalidateRecordField(SListMyGrid->RecNum, 0);
    SListMyGrid->InvalidateAll(true);
}


bool __fastcall TSEnumMainForm::GatherContents(TFileList *fl, int f1, int p0) {
//-------------------------------------------------------------------------------
//                          Сотворяет содержимое документа                      |
// Если что не так - выдает сообщение и возвращает false                        |
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

    // ??? Проверить, можно ли удалить файл filn

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
        Application->MessageBox("Для работы программы необходимо, чтобы на компьютере был установлен принтер PDFCreator!", "Ошибка!", 0);
        return false;
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
            mes = (String)"Файлы формата \"" + ext + "\" не обрабатываются";
            Application->MessageBox(mes.c_str(), "Надо делать заявку!", 0);
        }
    }
    if (NeedAA) {
        if (pc == NULL) {                           // PDFCreator не установлен!
            if (!TestAcrobatStandartPresence()) {
                NeedAA = false;
                Application->MessageBox("Для обработки файлов PDF требуется, чтобы в системы был установлен Adobe Acrobat Standart", "Не могу!", 0);
                // RestoreOldPrinter();
                if (pc != NULL) delete pc;
                return false;
            } else {
                StartAcrobat(pi, "");
                hProcess = pi.hProcess;
                try {
                    AA = CreateOleObject("AcroExch.App");
                } catch (...) {
                    // RestoreOldPrinter();
                    Application->MessageBox("Не могу установить связь с Adobe Acrobat Standart", "Ошибка!", 0);
                    if (pc != NULL) delete pc;
                    return false;
                }
            }
        } else {                                    // PDFCreator установлен
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
            //StopSaveBitmap2Tiff();
            //StopGdiplus();
            Application->MessageBox("Не могу установить связь с Excel", "Ошибка!", 0);
            if (pc != NULL) delete pc;
            return false;
        }
        EA.OlePropertySet("Visible", true);
    }
#ifdef GUARD
    proto("GatherContents::GuardCRCCheck(ReSet13());");
    GuardCRCCheck(ReSet13());
#endif
    if (NeedWA) {
        try {
            WA = CreateOleObject("Word.Application");
            // WA = CreateOleObject("Word.Application.11");
        } catch(...) {
            // RestoreOldPrinter();
            //StopSaveBitmap2Tiff();
            //StopGdiplus();
            Application->MessageBox("Не могу установить связь с Word", "Ошибка!", 0);
            if (pc != NULL) delete pc;
            return false;
        }
        WA.OlePropertySet("Visible", true);
    }

    StartGdiplus();
    for (i = 0, error = false; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        afiln = (String)r->path + r->filn;
        wsprintf(buf, "(%d из %d) %s", i + 1, fl->Nrecs, r->filn.c_str());
        // ProcessInfoFiln(r->filn.c_str());
        ProcessInfoFiln(buf);
        ProcessInfoAction("");
        ext = ExtractExt(r->filn);
        if (ext == "xls" || ext == "XLS") {
            Docs = EA.OlePropertyGet("Workbooks");
            ProcessInfoAction("Открытие файла");
            try {
                Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
            } catch(...) {
                mes = (String)"Не могу открыть файл \"" + afiln + "\"";
                Application->MessageBox(mes.c_str(), "Ошибка!", 0);
                error = true;
                break;
            }
            ProcessInfoAction("Обработка сметы");
            ProcessSmeta(r, Doc);  // Если смета - расставляет подписи и прочее
            ProcessInfoAction("Разбиение на страницы");
            //Paginate(Doc);      // Разбивает на страницы
            //ActiveSheet = Doc.OlePropertyGet("ActiveSheet");
            //HPageBreaks = ActiveSheet.OlePropertyGet("HPageBreaks");
            //VPageBreaks = ActiveSheet.OlePropertyGet("VPageBreaks");
            //r->pi.pcount = (HPageBreaks.OlePropertyGet("Count") + 1) * (VPageBreaks.OlePropertyGet("Count") + 1);
            r->pi.pcount = Paginate(Doc);
            Doc.OleProcedure("Close", false);
        } else if (ext == "doc" || ext == "DOC") {
            Docs = WA.OlePropertyGet("Documents");
            ProcessInfoAction("Открытие файла");
            try {
                Doc = Docs.OleFunction("Open", afiln.c_str(), false, true);
            } catch(...) {
                mes = (String)"Не могу открыть файл \"" + afiln + "\"";
                Application->MessageBox(mes.c_str(), "Ошибка!", 0);
                error = true;
                break;
            }
            r->pi.pcount = Doc.OleFunction("ComputeStatistics", 2, true);        // Должно вернуть количество страниц
            Doc.OleProcedure("Close", false);
        } else if (ext == "pdf" || ext == "PDF") {
            if (pc == NULL) {
                Variant pddoc, avdoc;
                int j, prev, k;
                chan = CreateFile(afiln.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                if (chan == INVALID_HANDLE_VALUE) {
                    mes = (String)"Не могу открыть файл \"" + afiln + "\"";
                    Application->MessageBox(mes.c_str(), "Ошибка!", 0);
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
                    r->rem = "Ошибка открытия файла";
                } else {
                    r->pi.pcount = cnt;
                }
            }
        } else if (ext == "tif" || ext == "TIF" || ext == "tiff" || ext == "TIFF"
                || ext == "jpg" || ext == "JPG"
                || ext == "jpeg" || ext == "JPEG"
        ) {
            ProcessInfoAction("Открытие файла");
            if (LoadTiffFile(afiln.c_str()) == NULL) {
                r->rem = "Ошибка открытия файла";
            }
            r->pi.pcount = LoadTiffFileNpages;
            ReleaseTiffFile();
        }

    }
    ProcessInfoAction("");
    StopGdiplus();

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

    // RestoreOldPrinter();
    if (NeedEA) {
        EA.OleProcedure("Quit");
        EA = Unassigned;
    }
    if (NeedWA) {
        WA.OleProcedure("Quit");
        WA = Unassigned;
    }

    if (pc != NULL) delete pc;
    return true;
}


void __fastcall TSEnumMainForm::N6Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                Создать файл содержания документа (старый стиль)              |
//-------------------------------------------------------------------------------
    contents_sp.NeedFiln = true; contents_sp.NeedF1 = true;
    if (fl->Nrecs == 0) {
        Application->MessageBox("Сначала необходимо выбрать хотя бы один файл", "Невозможно!", 0);
        return;
    }
    if (TestFilesLoaded(fl, false, true)) return;
    if (!AskSaveContents(&contents_sp)) return;
    if (TestFileLoaded(contents_sp.Filn)) return;
    FilnLabel->Caption = "Создание файла содержания документа *.doc";
    ActionLabel->Caption = "";
    FormStyle = fsStayOnTop;
    ProcessInfoPanel->Visible = true;
    NoMousePanel->Visible = true;
#ifdef GUARD
    proto("N6Click::GuardCRCCheck(ReSet13());");
    GuardCRCCheck(ReSet13());
#endif
    // CreateContents(contents_sp.Filn.c_str(), fl, contents_sp.f1, contents_sp.p0);
    if (GatherContents(fl, contents_sp.f1, contents_sp.p0)) {
        Contents2Word(contents_sp.Filn.c_str(), fl, 0, contents_sp.p0);
    }
    ProcessInfoPanel->Visible = false;
    NoMousePanel->Visible = false;
    FormStyle = fsNormal;
}


void __fastcall DebugEditContents(TSaveParams *sp);

void __fastcall TSEnumMainForm::N9Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                Создать файл содержания документа (новый стиль)               |
//-------------------------------------------------------------------------------
    // DebugEditContents();

    contents_sp.NeedFiln = false; contents_sp.NeedF1 = true;
    if (fl->Nrecs == 0) {
        Application->MessageBox("Сначала необходимо выбрать хотя бы один файл", "Невозможно!", 0);
        return;
    }
    if (TestFilesLoaded(fl, false, true)) return;
    if (!AskSaveContents(&contents_sp)) return;
    // if (TestFileLoaded(contents_sp.Filn)) return;
    FilnLabel->Caption = "Создание файла содержания документа *.doc";
    ActionLabel->Caption = "";
    FormStyle = fsStayOnTop;
    ProcessInfoPanel->Visible = true;
    NoMousePanel->Visible = true;
    // CreateContents(contents_sp.Filn.c_str(), fl, contents_sp.f1, contents_sp.p0);
    bool good = GatherContents(fl, contents_sp.f1, contents_sp.p0);
    // DebugEditContents(&contents_sp);
    ProcessInfoPanel->Visible = false;
    NoMousePanel->Visible = false;
    FormStyle = fsNormal;
#ifdef GUARD
    proto("N9Click::GuardCRCCheck(ReSet13());");
    GuardCRCCheck(ReSet13());
#endif

    if (good) EditContents(fl, &contents_sp);
}



void __fastcall TSEnumMainForm::BitBtn2Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    DecImageIndex();
}

void __fastcall TSEnumMainForm::BitBtn3Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    IncImageIndex();
}

void __fastcall TestLocked(void);

void __fastcall TSEnumMainForm::N3Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                                      О программе                             |
//-------------------------------------------------------------------------------
    Application->MessageBox(
        "            Программа \"Нумерация страниц смет\" версия 1.13\015\012\015\012"
        "Изменения в версии 1.13 от 30.07.2014:\015\012"
        "  1. Исправил ошибку, из-за которой зависала печать некоторых файлов\015\012"
        "  2. Заменил шаблон содержания\015\012\015\012"
        "Изменения в версии 1.12 от 29.07.2014:\015\012"
        "  Исправил ошибку, из-за которой простановка подписей иногда приводила к зависанию\015\012\015\012"
        "Изменения в версии 1.11 от 28.07.2014:\015\012"
        "  1. Добавил функцию проверки висячих подписей\015\012"
        "  2. Добавил привязку программы к компьютеру (требуется fixation)\015\012\015\012"
        "Изменения в версии 1.10 от 18.07.2014:\015\012"
        "  Убрал из функции построения содержания обращения к Acrobat Standard\015\012\015\012"
        "Изменения в версии 1.09 от 16.07.2014:\015\012"
        "  Добавил возможность создание .tif файла через PDFCreator версии 1.7.3\015\012\015\012"
        "Изменения в версии 1.08 от 24.06.2014:\015\012"
        "  Добавил возможность редактирования файла .tif (вставка, переворот страницы и т.д.)\015\012\015\012"
        "Изменения в версии 1.07 от 26.05.2014:\015\012"
        "  Доработал возможность автоматической простановки подписей в любых файлах .xls\015\012\015\012"
        "Изменения в версии 1.06 от 07.05.2014:\015\012"
        "  1. Добавил новый пункт меню: \"Создать содержание документа  (новый стиль) ...\"\015\012"
        "  2. Исправил ошибку, проявляющуюся при наличии в файле .xls пустых листов\015\012\015\012"
        "Изменения в версии 1.05 от 16.04.2014:\015\012"
        "  1. Добавил функцию Файл->Добавить все файлы папки в список\015\012"
        "  2. Добавил функцию 'Поднять файл на первую строку' - вызывается правой кнопкой мыши на файле\015\012\015\012"
        "Изменения в версии 1.04 от 14.04.2014:\015\012"
        "  Исправил ошибку, из-за которой на некоторых файлах сбивалась нумерация страниц\015\012\015\012"
        "Изменения в версии 1.03 от 08.04.2014:\015\012"
        "  Исправил зависание, возникающиее при подписывании файла \"Сводка затрат.xls\"\015\012\015\012"
        "Изменения в версии 1.02 от 07.04.2014:\015\012"
        "  1. Подписи теперь ставятся не только после \"Составил\", но и после \"Проверил\", \015\012"
        "     \"Главный инженер проекта\" и \"Начальник сметного отдела\"\015\012"
        "  2. Добавлена обработка смет от Газпрома, с нестандартным заголовком сметы\015\012"
        "     и несколькими вкладками в файле .xls\015\012"
        "  3. Временно удалил обращение к PDFCreator"

        ,"Инфомация",
        0
    );
}

void __fastcall TSEnumMainForm::N4Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                              Добавить файлы в список                         |
//-------------------------------------------------------------------------------
    SListMyGridBtnUser1(0, 0, 0);
}

void __fastcall TSEnumMainForm::SListMyGridBtnUser1(TObject *Sender, int Row, TMyColumn *c) {
//-------------------------------------------------------------------------------
//                              Добавить файлы в список                         |
//-------------------------------------------------------------------------------
    TFileList *ffl;
    TFileListRec *r;
    TStrings *Files;
    String aFiln;
    int i, n;

    if (!cfg->Path.IsEmpty()) {
        OpenDialog1->InitialDir = cfg->Path;
    }
    if (!OpenDialog1->Execute()) return;
    Files = OpenDialog1->Files;
    if (Files->Count == 0) return;
    cfg->Path = EraseFilnFromPath(OpenDialog1->FileName);
    OpenDialog1->InitialDir = cfg->Path;
    ffl = new TFileList;
    for (i = 0; i < Files->Count; i++) {
        aFiln = ErasePathFromFiln(Files->Strings[i]);
        if (NospaceStrcmp(aFiln.c_str(), "tmp.doc", true) == 0) continue;
        ffl->Append("", Files->Strings[i], "", "");
    }
    ffl->Sort();
#ifdef GUARD
    proto("SListMyGridBtnUser1::GuardCRCCheck(ReSet13());");
    GuardFuncPriv3();
#endif
    n = ffl->Nrecs;
    while(ffl->Nrecs > 0) {
        r = ffl->Rec0(0);
        if (NospaceStrcmp(r->filn.c_str(), "Содержание.doc", true) == 0) {
            fl->Ins0(r->nsm, r->path + r->filn, r->name, r->oname);
        } else {
            fl->Append(r->nsm, r->path + r->filn, r->name, r->oname);
        }
        ffl->DelRec(0);
    }
    // SListMyGrid->CountRecords += n;
    SListMyGrid->CountRecords = fl->Nrecs;
    SListMyGrid->InvalidateAll(true);
    SListMyGrid->SetAllReRead();

    delete ffl;
}


static void __fastcall AppendFolders(TFileList *ffl, String Folder) {
//-------------------------------------------------------------------------------
//              Добавляет все файлы папки Folder и ее подпапки в ffl            |
// Все обрабатываемые файлы, разумеется                                         |
// Файл tmp.doc эгнорирует!                                                     |
//-------------------------------------------------------------------------------
    TSearchRec sr;
    String Path = Folder + "\\*.*";
    String Filn;
    String ext;
    int res;

    res = FindFirst(Path, faDirectory + faArchive, sr);
    while(res == 0) {
        if (sr.Attr & faDirectory) {
            if (sr.Name != "." && sr.Name != "..") {
                AppendFolders(ffl, Folder + "\\" + sr.Name);
            }
        } else {
            ext = ExtractExt(sr.Name).LowerCase();
            if (ext == "doc" || ext == "xls" || ext == "pdf" || ext == "tif" || ext == "tiff" ||
                ext == "jpg" || ext == "jpeg"
            ) {
                Filn = Folder + "\\" + sr.Name;
                ffl->Append("", Filn, "", "");                
            }
        }
        res = FindNext(sr);
    }

    FindClose(sr);

}


// Все обрабатываемые файлы|*.doc;*.xls;*.pdf;*.tif;*.tiff;*.jpg;*.jpeg|Файлы Excel *.xls|*.xls|Файлы Word *.doc|*.doc|Файлы Word *.pdf|*.pdf|Растровые рисунки *.tif|*.tif;*.tiff|Растровые рисунки *.jpg|*.jpg;*.jpeg

void __fastcall TSEnumMainForm::N7Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                      Добавить все файлы папки в список...                    |
//-------------------------------------------------------------------------------
    TFileList *ffl;
    TFileListRec *r;
    TStrings *Files;
    String S, aFiln;
    int i, n;

    if (!cfg->Path.IsEmpty()) {
        AddFolderDialog->Directory = cfg->Path;
    }

    if (!AddFolderDialog->Execute()) return;
    S = AddFolderDialog->Directory;
    cfg->Path = S;

    ffl = new TFileList;
    AppendFolders(ffl, S);

    //for (i = 0; i < Files->Count; i++) {
    //    aFiln = ErasePathFromFiln(Files->Strings[i]);
    //    if (NospaceStrcmp(aFiln.c_str(), "tmp.doc", true) == 0) continue;
    //    ffl->Append("", Files->Strings[i], "", "");
    //}
#ifdef GUARD
    proto("N7Click::GuardCRCCheck(ReSet13());");
    GuardCRCCheck(ReSet13());
#endif
    
    ffl->Sort();
    n = ffl->Nrecs;
    while(ffl->Nrecs > 0) {
        r = ffl->Rec0(0);
        if (NospaceStrcmp(r->filn.c_str(), "Содержание.doc", true) == 0) {
            fl->Ins0(r->nsm, r->path + r->filn, r->name, r->oname);
        } else {
            fl->Append(r->nsm, r->path + r->filn, r->name, r->oname);
        }
        ffl->DelRec(0);
    }
    // SListMyGrid->CountRecords += n;
    SListMyGrid->CountRecords = fl->Nrecs;
    SListMyGrid->InvalidateAll(true);

    delete ffl;
}




void __fastcall TSEnumMainForm::SListMyGridBtnUser2(TObject *Sender, int Row, TMyColumn *c) {
//-------------------------------------------------------------------------------
//                                  DelRec                                      |
//-------------------------------------------------------------------------------
    int i, n, index;

    if (SListMyGrid->SelectRecFirst < 0 || SListMyGrid->SelectRecLast < 0) {
        Application->MessageBox("Чтобы удалить что-нибудь, надо сначало выбрать что-нибудь", "Не могу", 0);
        return;
    }
    n = SListMyGrid->SelectRecLast - SListMyGrid->SelectRecFirst;
    index = SListMyGrid->RecNum;
    for (i = 0; i <= n; i++) {
        fl->DelRec(index);
        SListMyGrid->DeleteRecord();
    }
    SListMyGrid->CountRecords = fl->Nrecs;
    SListMyGrid->InvalidateAll(true);
    SListMyGrid->InvalidateActiveCell();
}

void __fastcall TSEnumMainForm::SListMyGridBtnUser3(TObject *Sender, int Row, TMyColumn *c) {
//-------------------------------------------------------------------------------
//                                      Ascend                                  |
//-------------------------------------------------------------------------------
    int i, n, index, k;
    int mu, md;

    if (SListMyGrid->CountRecords == 0) {
        Application->MessageBox("Чтобы поднять что-нибудь, надо сначала выбрать что-нибудь", "Не могу", 0);
        return;
    }

    index = SListMyGrid->RecNum;
    if (index <= 0) {
        // Application->MessageBox("Выше первой строки поднять невозможно!", "Не могу", 0);
        return;
    }
    mu = SListMyGrid->MouseSelectCoord.MURecNo;
    md = SListMyGrid->MouseSelectCoord.MDRecNo;
    if (SListMyGrid->SelectRecFirst < 0 || SListMyGrid->SelectRecLast < 0) {
        n = 0;
    } else {
        n = SListMyGrid->SelectRecLast - SListMyGrid->SelectRecFirst;
    }
    for (i = 0, k = index - 1; i <= n; i++, k++) {
        // SListMyGrid->RecNum = k;
        // SListMyGrid->MoveDownRecord();
        fl->RecIndex0 = k;
        fl->Descend();
    }
    // SListMyGrid->SetAllReRead();
    SListMyGrid->InvalidateAll(true);
    SListMyGrid->InvalidateActiveCell();
    if (n > 0) {
        SListMyGrid->MouseSelectCoord.MURecNo = mu - 1;
        SListMyGrid->MouseSelectCoord.MDRecNo = md - 1;
    }
    SListMyGrid->RecNum = index - 1;
//    SListMyGrid->SelectRecFirst--;
//    SListMyGrid->SelectRecLast--;
}


void __fastcall TSEnumMainForm::SListMyGridBtnUser5(TObject *Sender, int Row, TMyColumn *c) {
//-------------------------------------------------------------------------------
//                                   Поднять файл наверх                        |
//-------------------------------------------------------------------------------
    int index;

    index = SListMyGrid->RecNum;
    if (index <= 0) return;

    for (; index >= 0; index--) {
        fl->RecIndex0 = index;
        fl->Ascend();
    }

    SListMyGrid->RecNum = 0;

    SListMyGrid->InvalidateAll(true);
    SListMyGrid->InvalidateActiveCell();

}


void __fastcall TSEnumMainForm::SListMyGridBtnUser4(TObject *Sender, int Row, TMyColumn *c) {
//-------------------------------------------------------------------------------
//                                      Descend                                 |
//-------------------------------------------------------------------------------
    int i, n, index, k;

    if (SListMyGrid->CountRecords == 0) {
        Application->MessageBox("Чтобы опустить что-нибудь, надо сначала выбрать что-нибудь", "Не могу", 0);
        return;
    }

    index = SListMyGrid->RecNum;
    if (index >= fl->Nrecs - 1) {
        Application->MessageBox("Ниже последней строки опустить невозможно!", "Не могу", 0);
        return;
    }
    if (SListMyGrid->SelectRecFirst < 0 || SListMyGrid->SelectRecLast < 0) {
        n = 1;
    } else {
        n = SListMyGrid->SelectRecLast - SListMyGrid->SelectRecFirst;
    }
    for (i = 0, k = index + n + 1; i <= n; i++, k--) {
        fl->RecIndex0 = k;
        fl->Ascend();
    }
    // SListMyGrid->SetAllReRead();
    SListMyGrid->InvalidateAll(true);
    SListMyGrid->InvalidateActiveCell();
    SListMyGrid->MouseSelectCoord.MURecNo++;
    SListMyGrid->MouseSelectCoord.MDRecNo++;
    SListMyGrid->RecNum = index + 1;
}

// SelectRecFirst  - 	Номер первой выделенной записи. -1 -  ничего не  выделено. 					Только на чтение
// SelectRecLast   - 	Номер последней выделенной записи. Только на чтение





void __fastcall TSEnumMainForm::SListMyGridDblClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TFileListRec *r;
    String Filn;
    int k;

    k = SListMyGrid->RecNum; if (k == -1) return;
    r = fl->Rec1(k);
    Filn = (String)"\"" + (String)r->path + (String)r->filn + (String)"\"";

    // ShellExecute(Handle, "open", Filn.c_str(), NULL, r->path.c_str(), SW_SHOWNORMAL);
    // ShellExecute(NULL, "open", Filn.c_str(), NULL, r->path.c_str(), SW_SHOWNORMAL);
    ShellExecute(NULL, "open", Filn.c_str(), NULL, r->path.c_str(), 0);
}

// GetWindowRect, GetClientRect
// ClientToScreen

// TForm
void __fastcall TSEnumMainForm::Button1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    IniMODIPrinter();
}


void __fastcall TSEnumMainForm::N12Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Application->Terminate();
}



void __fastcall TSEnumMainForm::tiff1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#ifdef GUARD
    proto("tiff1Click::GuardFuncPriv4();");
    GuardFuncPriv4();
#endif
    EditTiffFile(sp.Filn);           /* TODO : Не забыть это раскомментировать, а следующую строку наоборот */
    // EditTiffFile("c:\\UnitedProjects\\Нумерация страниц смет\\exe\\ttt\\TestTiffUnit.tif");
}



void __fastcall TSEnumMainForm::SListMyGridMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#ifdef GUARD
    proto("SListMyGridMouseMove::.....");
    int v = GetTickCount();
    if (v % 3 == 0) {
        GuardFuncPriv3();
    } else if (v % 5 == 0) {
        GuardFuncPriv4();
    } else if (v % 7 == 0) {
        GuardCRCCheck(ReSet13());
    }
#endif

}





/*
    a = PDFCreator1.cOption("StandardSaveformat")
    
    PDFCreator1.cOption("StandardSaveformat") = 7
    
    a = PDFCreator1.cOption("StandardSaveformat")

    filn = "C:\\0910_08.pdf"

    PDFCreator1.cPrintFile (filn)

*/




Variant PC;

void __fastcall TSEnumMainForm::PDFCreator1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    stop++;

/*
    regex_t preg;
    regmatch_t pmatch[50];
    stop = regcomp(&preg, "Поля раздела [0-9]+ выходят за границы", 0);
    stop = regexec(&preg, "проба пера", 50, pmatch, 0);
    stop = regexec(&preg, "Поля раздела 1 выходят за границы области печати. Продолжить?", 50, pmatch, 0);
    stop = regexec(&preg, "проба пера Поля раздела 1 выходят за границы области печати. Продолжить?", 50, pmatch, 0);

    regfree(&preg);
    stop++;
*/

/*
    // ------------ Печать файла PDF ----------------
    TPDFCreator *pc = new TPDFCreator();
    if (pc->ok) {
        sstop = pc->PrintPDF("d:\\For\\PDF\\Проба пера.pdf");
        // sstop = pc->PrintPDF("d:\\For\\PDF\\Проба пера.doc");
        stop = (int)GetFileAttributes(sstop.c_str());
        HANDLE h = CreateFile(sstop.c_str(), GENERIC_READ + GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        stop++;
        CloseHandle(h);
    }
    delete pc;
*/


/*
    // ------------ Печать файла DOC --------------
    Variant WA = CreateOleObject("Word.Application");
    Variant Docs = WA.OlePropertyGet("Documents");
    String filn = "c:\\UnitedProjects\\Нумерация страниц смет\\exe\\PDFCreatorTest\\Проба пера.doc";
    try {
        Variant Doc = Docs.OleFunction("Open", filn.c_str(), false, true);
    } catch(...) {
        String mes = (String)"Не могу открыть файл \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return;
    }

    WA.OlePropertySet("Visible", true);
    TPDFCreator *pc = new TPDFCreator();
    if (pc->ok) {
        sstop = pc->PrintDOC(WA);
        stop = (int)GetFileAttributes(sstop.c_str());
        HANDLE h = CreateFile(sstop.c_str(), GENERIC_READ + GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        stop++;
        CloseHandle(h);
    }
    delete pc;
*/

/*
    // ------------ Печать файла XLS --------------
    Variant EA = CreateOleObject("Excel.Application");
    Variant Docs = EA.OlePropertyGet("Workbooks");
    Variant ActiveSheet;
    String filn = "d:\\For\\PDF\\Проба пера.xls";
    try {
        Variant Doc = Docs.OleFunction("Open", filn.c_str(), false, true);
        Variant Sheets = Doc.OlePropertyGet("Sheets");
        ActiveSheet = Sheets.OlePropertyGet("Item", 1);
    } catch(...) {
        String mes = (String)"Не могу открыть файл \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return;
    }

    TPDFCreator *pc = new TPDFCreator();
    if (pc->ok) {
        sstop = pc->PrintXLS(ActiveSheet);
        stop = (int)GetFileAttributes(sstop.c_str());
        HANDLE h = CreateFile(sstop.c_str(), GENERIC_READ + GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        stop++;
        CloseHandle(h);
    }
    delete pc;
*/

/*
    // ------------ Тест CreateAcrobatList, KillAcrobates --------------
    TPDFCreator *pc = new TPDFCreator();

    // 1. Здесь необходимо запустить несколько акробатов
    stop++;
    pc->CreateAcrobatList();        // Эти акробаты не должны быть убиты

    // 2. Теперь необходимо запустить еще один или несколько акробатов
    stop++;
    pc->KillAcrobates();            // Запущенные в п2 акробаты должны быть убиты

    stop++;
*/

}


