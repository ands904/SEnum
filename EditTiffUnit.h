//---------------------------------------------------------------------------

#ifndef EditTiffUnitH
#define EditTiffUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "SEnumLib1.h"
#include "Fog.h"
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include "vds.h"
#include "TiffImageConfigUnit.h"
#include <Dialogs.hpp>
#include "SEnumMainUnit.h"
//---------------------------------------------------------------------------
class TEditTiffForm : public TForm
{
__published:	// IDE-managed Components
    TFog *Fog1;
    TPanel *Panel1;
    TComboBox *PageComboBox;
    TBitBtn *IncBitBtn;
    TBitBtn *DecBitBtn;
    TScrollBar *ScrollBar1;
    TSpeedButton *RotateSpeedButton;
    TSpeedButton *EditSpeedButton;
    TSpeedButton *ConfigSpeedButton;
    TPopupMenu *EditPopupMenu;
    TMenuItem *ReplaceMenuItem;
    TMenuItem *InsertMenuItem;
    TMenuItem *AppendMenuItem;
    TMenuItem *DeleteMenuItem;
    TPanel *SaveFilePanel;
    TOpenDialog *OpenTiffDialog;
    TSaveDialog *SaveAsTiffDialog;
    void __fastcall Fog1Paint(TObject *Sender);
    void __fastcall IncBitBtnClick(TObject *Sender);
    void __fastcall DecBitBtnClick(TObject *Sender);
    void __fastcall PageComboBoxSelect(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall Fog1MouseWheelDown(TObject *Sender, TShiftState Shift,
          const TPoint &MousePos, bool &Handled);
    void __fastcall Fog1MouseWheelUp(TObject *Sender, TShiftState Shift,
          const TPoint &MousePos, bool &Handled);
    void __fastcall ScrollBar1Change(TObject *Sender);
    void __fastcall ConfigSpeedButtonClick(TObject *Sender);
    void __fastcall EditSpeedButtonClick(TObject *Sender);
    void __fastcall RotateSpeedButtonClick(TObject *Sender);
    void __fastcall ReplaceMenuItemClick(TObject *Sender);
    void __fastcall InsertMenuItemClick(TObject *Sender);
    void __fastcall AppendMenuItemClick(TObject *Sender);
    void __fastcall DeleteMenuItemClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
private:	// User declarations
    bool InsideChange;
    void __fastcall RedrawPage(void);
    void __fastcall PackPageNumber(void);
    bool __fastcall EditConfig(String acase);
    bool __fastcall DeleteTicStartEnd(void);
    void __fastcall EnumTiffPages(void);
public:		// User declarations
    __fastcall TEditTiffForm(TComponent* Owner);
    bool __fastcall PrintFile(void);
    void __fastcall SetupPageComboBox(int count, int start);
};
//---------------------------------------------------------------------------
extern PACKAGE TEditTiffForm *EditTiffForm;
//---------------------------------------------------------------------------


void __fastcall EditTiffFile(String SavedFiln);
//-------------------------------------------------------------------------------
//              Запрашивает файл у юзера и редактирует его                      |
// SavedFiln - если есть, то имя сохраненного тифовского файла                  |
//-------------------------------------------------------------------------------

class TTiffBook;


class TTiffImage {
friend class TTiffBook;
protected:
    Gdiplus::Bitmap *fbm;
    // Gdiplus::Image *fbm;
    // Gdiplus::Graphics *gra;
    // HDC CanvasHandle;
    GUID DimensionID;
    int NPages;
    int Page;
    String Filn;
public:
    bool temp;                              // Признак временного файла

    TTiffImage(void);
    TTiffImage(Gdiplus::Bitmap *_fbm);
    ~TTiffImage(void);
    Gdiplus::Status __fastcall GetStatus(void);
    Gdiplus::Status __fastcall LoadFromFile(String filn);
    int __fastcall GetNPages(void) {return NPages;}
    void __fastcall SetPage(int n);
    int __fastcall GetPage(void) {return Page;}
    bool __fastcall IncPageNumber(void);
    bool __fastcall DecPageNumber(void);
    int __fastcall GetWidth(void) {return fbm == NULL ? 0 : fbm->GetWidth();}
    int __fastcall GetHeight(void) {return fbm == NULL ? 0 : fbm->GetHeight();}

    void __fastcall Draw(TFog *f);
    void __fastcall DrawCenter(TCanvas *c, int xc, int yc, int w, int h, int PageNumber = 0);

};


struct TTiffPageInfo {
    int ImageIndex;         // Номер файла в Images
    int PageIndex;          // Номер страницы в файле Images[PageIndex]
    int PageNumber;         // Номер печатаемой страницы. Если 0, то не надо, -1 - очистить
};

class TTiffBook {
protected:
    TVds *Images;
    TVds *Pages;
    int Page;
    TTiffImage * __fastcall GetImagePage(int k);
    TTiffPageInfo * __fastcall GetPageInfo(int k) {
        Pages->RecIndex1 = k;
        return (TTiffPageInfo *)Pages->RecPtr1;
    }

public:
    String Filn;
    bool changed;

    TTiffBook(void);
    ~TTiffBook(void);
    bool __fastcall LoadTiff(String TiffFiln);
    bool __fastcall SaveBook(String TiffFiln, TPanel *SaveFilePanel = NULL);
    bool __fastcall Append(String TiffFiln);
    int __fastcall AppendImage(TTiffImage *img);
    void __fastcall DelPages(int start, int end);
    void __fastcall FlipCurPage(int n);

    TTiffImage * __fastcall GetImage(int index);

    int __fastcall GetNPages(void) {return Pages->Nrecs;}
    void __fastcall SetPage(int n);
    int __fastcall GetPage(void) {return Page;}
    void __fastcall InsPages(int pos);
    bool __fastcall IncPageNumber(void);
    bool __fastcall DecPageNumber(void);

    void __fastcall EnumPages(int start, int first);

    void __fastcall DrawCenter(TCanvas *c, int xc, int yc, int w, int h);

};


#endif
