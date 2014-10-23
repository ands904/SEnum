//---------------------------------------------------------------------------

#ifndef ContentsUnitH
#define ContentsUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include "SaveUnit.h"

//---------------------------------------------------------------------------
class TContentsForm : public TForm
{
__published:	// IDE-managed Components
    TDrawGrid *ContentsDrawGrid;
    TMemo *CurCellMemo;
    TMainMenu *MainMenu1;
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *doc1;
    TMenuItem *N3;
    TMenuItem *N4;
    TMenuItem *N5;
    void __fastcall ContentsDrawGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall ContentsDrawGridKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall CurCellMemoKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall CurCellMemoExit(TObject *Sender);
    void __fastcall ContentsDrawGridClick(TObject *Sender);
    void __fastcall ContentsDrawGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall N3Click(TObject *Sender);
    void __fastcall doc1Click(TObject *Sender);
    void __fastcall N4Click(TObject *Sender);
    void __fastcall N5Click(TObject *Sender);
private:	// User declarations
    int CurCellIndex;
    bool EditPageNum;
    void __fastcall StopEditCurCell(bool Save);
public:		// User declarations
    int __fastcall GetContentsWidth(void);
    void __fastcall ResizeContentsGridHeights(void);
    int __fastcall GetFontHeight(void);
    __fastcall TContentsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TContentsForm *ContentsForm;
//---------------------------------------------------------------------------


//===============================================================================
//===============================================================================
struct TContentsVRec {                              // Одна запись класса TContentsV
    int nlines;                                     // Количество строк
    int npages;                                     // Количество страниц
    int page;                                       // Номер страницы
    bool bold;
    unsigned char text[5000];                       // Самтекст
    void __fastcall Split(TCanvas *c, int width);
    void __fastcall PaintGray(String pattern);
    void __fastcall DelGrays(void);
    void __fastcall UnSplit(void);
    void __fastcall Draw(TCanvas *c, int x0, int y0);
};


class TContentsV : public TVds {
protected:
    TCanvas *c;
    int width;
    String gray;                            // серость
public:
    TContentsV(TCanvas *_c, int _width);

    void __fastcall SetWidth(int _width);
    int __fastcall GetWidth(void) {return width;}
    void __fastcall SplitRec(int recnum) {
        Rec1(recnum)->Split(c, width);
        Rec1(recnum)->PaintGray(gray);
    }
    bool __fastcall ReSplitAll(int w) {
        if (w == width) return false;
        width = w;
        for(int i = 0; i < Nrecs; i++) {
            SplitRec(i);
        }
        SetGray(gray);
        return true;
    }
    void __fastcall SetGray(String g);

    void __fastcall DelGray(void) {
        for (int i = 0; i < Nrecs; i++) {
            Rec1(i)->DelGrays();
        }
    }

    void __fastcall Enum(int start_page) {
        for(int i = 0; i < Nrecs; i++) {
            Rec1(i)->page = start_page;
            start_page += Rec1(i)->npages;
        }
    }


    TContentsVRec *Rec0(int index) {RecIndex0 = index; return (TContentsVRec *)RecPtr0;};
    TContentsVRec *Rec1(int index) {RecIndex1 = index; return (TContentsVRec *)RecPtr1;};
    TContentsVRec *Rec2(int index) {RecIndex2 = index; return (TContentsVRec *)RecPtr2;};
    TContentsVRec *Rec3(int index) {RecIndex3 = index; return (TContentsVRec *)RecPtr3;};

    TContentsVRec * __fastcall Append(bool bold, int _npages, unsigned char *_text) {
        TVds::Append();
        RecLen0 = sizeof(TContentsVRec);
        TContentsVRec *res = (TContentsVRec *)RecPtr0;
        res->bold = bold;
        res->nlines = 0;
        res->npages = _npages;
        strcpy(res->text, _text);
        return res;
    }
};




int __fastcall EditContents(TFileList *fl, TSaveParams *sp);
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#endif
