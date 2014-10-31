//---------------------------------------------------------------------------

#ifndef SEnumMainUnitMH
#define SEnumMainUnitMH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include "Fog.h"
#include <ExtCtrls.hpp>
#include "TMyTabUnit.h"
//---------------------------------------------------------------------------
class TSEnumMainFormM : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *MainMenu1;
    TMenuItem *N1;
    TMenuItem *N4;
    TMenuItem *N7;
    TMenuItem *N8;
    TMenuItem *N5;
    TMenuItem *N6;
    TMenuItem *N9;
    TMenuItem *PDF1;
    TMenuItem *N10;
    TMenuItem *tiff1;
    TMenuItem *PDFCreator1;
    TMenuItem *N11;
    TMenuItem *N12;
    TMenuItem *N2;
    TMenuItem *N3;
    TFog *Fog1;
    TTabControl *TabControl1;
    TTimer *Timer1;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall Fog1DragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept);
    void __fastcall Fog1DragDrop(TObject *Sender, TObject *Source, int X,
          int Y);
private:	// User declarations
    void __fastcall OnDragDropX(TMyTabControl *Sender, TMyDropData *DropData);
    void __fastcall OnTabIndexChange(TMyTabControl *Sender);
public:		// User declarations
    __fastcall TSEnumMainFormM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSEnumMainFormM *SEnumMainFormM;
//---------------------------------------------------------------------------
#endif
