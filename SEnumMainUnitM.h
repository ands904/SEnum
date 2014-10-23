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
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TSEnumMainFormM(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSEnumMainFormM *SEnumMainFormM;
//---------------------------------------------------------------------------
#endif
