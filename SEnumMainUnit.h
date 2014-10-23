//---------------------------------------------------------------------------

#ifndef SEnumMainUnitH
#define SEnumMainUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include "MyGrid.h"
#include <Dialogs.hpp>
#include "Excel_2K_SRVR.h"
#include <OleServer.hpp>
#include "UExcelAdapter.hpp"
#include "XLSAdapter.hpp"
#include "UFlexCelImport.hpp"
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include "folderdialog.hpp"
#include "FolderDialog.hpp"

#include "SEnumLib.h"
//---------------------------------------------------------------------------
class TSEnumMainForm : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu1;
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *N3;
    TMenuItem *N4;
    TMenuItem *N5;
    TMenuItem *PDF1;
    TMyGrid *SListMyGrid;
    TOpenDialog *OpenDialog1;
    TExcelApplication *ExcelApplication1;
    TXLSAdapter *XLSAdapter1;
    TFlexCelImport *Fi;
    TNotebook *Notebook1;
    TImage *Image1;
    TBitBtn *BitBtn1;
    TScrollBox *ScrollBox1;
    TBitBtn *BitBtn2;
    TBitBtn *BitBtn3;
    TPanel *ProcessInfoPanel;
    TLabel *FilnLabel;
    TLabel *ActionLabel;
    TMenuItem *N6;
    TFolderDialog *SignatureFolderDialog;
    TPrinterSetupDialog *PrinterSetupDialog1;
    TButton *Button1;
    TPanel *NoMousePanel;
    TPopupMenu *PopupMenuEmpty;
    TPopupMenu *PopupMenuOne;
    TPopupMenu *PopupMenuGroup;
    TMenuItem *AddFiles;
    TMenuItem *MoveFileUp;
    TMenuItem *MoveFileDown;
    TMenuItem *DelFile;
    TButton *NeedBreakButton;
    TMenuItem *N7;
    TMenuItem *N8;
    TFolderDialog *AddFolderDialog;
    TMenuItem *N9;
    TMenuItem *N10;
    TMenuItem *tiff1;
    TMenuItem *N11;
    TMenuItem *N12;
    TMenuItem *PDFCreator1;
    void __fastcall N5Click(TObject *Sender);
    void __fastcall PDF1Click(TObject *Sender);
    void __fastcall N3Click(TObject *Sender);
    void __fastcall N4Click(TObject *Sender);
    void __fastcall SListMyGridBtnUser1(TObject *Sender, int Row,
          TMyColumn *c);
    void __fastcall SListMyGridBtnUser2(TObject *Sender, int Row,
          TMyColumn *c);
    void __fastcall SListMyGridBtnUser3(TObject *Sender, int Row,
          TMyColumn *c);
    void __fastcall SListMyGridBtnUser4(TObject *Sender, int Row,
          TMyColumn *c);
    void __fastcall SListMyGridGetPoleAdr(TObject *Sender, TMyColumn *c,
          char *&Adr, bool &f);
    void __fastcall SListMyGridUnPack(TObject *Sender, TMyColumn *c,
          char *Adr, AnsiString &s, bool &f);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall BitBtn2Click(TObject *Sender);
    void __fastcall BitBtn3Click(TObject *Sender);
    void __fastcall N6Click(TObject *Sender);
    void __fastcall SListMyGridKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall SListMyGridDblClick(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall NeedBreakButtonClick(TObject *Sender);
    void __fastcall N7Click(TObject *Sender);
    void __fastcall SListMyGridBtnUser5(TObject *Sender, int Row,
          TMyColumn *c);
    void __fastcall N9Click(TObject *Sender);
    void __fastcall N12Click(TObject *Sender);
    void __fastcall tiff1Click(TObject *Sender);
    void __fastcall PDFCreator1Click(TObject *Sender);
    void __fastcall SListMyGridMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
private:	// User declarations
    bool __fastcall GatherContents(TFileList *fl, int f1, int p0);
public:		// User declarations
    __fastcall TSEnumMainForm(TComponent* Owner);
    bool __fastcall PrintFilesLoaded(char *filn, TFileList *fl, int f1, int p0);
};
//---------------------------------------------------------------------------
extern PACKAGE TSEnumMainForm *SEnumMainForm;
//---------------------------------------------------------------------------

void __fastcall ProcessInfoFiln(char *filn);
//-------------------------------------------------------------------------------
//              ֲûגמהטע טלÿ פאיכא ג ProcessInfoPanel->FilnLabel                 |
//-------------------------------------------------------------------------------

void __fastcall ProcessInfoAction(char *filn);
//-------------------------------------------------------------------------------
//              ֲûגמהטע טלÿ פאיכא ג ProcessInfoPanel->ActionLabel               |
//-------------------------------------------------------------------------------

#endif


