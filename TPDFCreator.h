//---------------------------------------------------------------------------
#ifndef TPDFCreatorH
#define TPDFCreatorH
//---------------------------------------------------------------------------

#include <ComObj.hpp>

struct TAcrobatList {
    DWORD th32ProcessID;
    char szExeFile[MAX_PATH];
};


class TPDFCreator {
protected:
    Variant PC;                 // Объект PDFCreator.clsPDFCreator
    String OldDefaultPrinter;
    String TiffDefaultPrinter;
    bool __fastcall SetDefaultPrinter(void);
    void __fastcall RestoreDefaultPrinter(void);
    DWORD FindPDFCreator(void);
    void __fastcall KillProcess(DWORD ProcessID, String ProcessName = (String)"PDFCreator.exe");
    void __fastcall KillPDFCreatorProcess(void);

    void __fastcall CreateTempFiln(String ext);
    String TempPath;
    String TempFiln;    // Имя файла, полученное по CreateTempFiln

    String __fastcall SetupForPrint(String Filn);
    void __fastcall ClearUseAutosave(void);

    TAcrobatList AcrobatList[100];
    int AcrobatListCount;

public:
    bool ok;
    String TiffFiln;


    TPDFCreator(void);
    ~TPDFCreator(void);

    DWORD __fastcall FindProcess(char * ProcessName);
    DWORD __fastcall FindAcrobat(void);
    void __fastcall KillAcrobat(void);
    bool __fastcall KillAcrobatDeath(void);
    void __fastcall WaitForPrinted(String filn);

    String __fastcall PrintPDF(String filn);
    void __fastcall PrintPDF(String TiffFiln, String filn);
    String __fastcall PrintDOC(Variant WA);
    void __fastcall PrintDOC(String TiffFiln, Variant WA);
    String __fastcall PrintXLS(Variant ActiveSheet);
    void __fastcall PrintXLS(String TiffFiln, Variant ActiveSheet);

    int __fastcall CountPages(String filn);
};


#endif
