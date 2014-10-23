//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SEnumMainUnitM.h"
#include "TMyTabUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Fog"
#pragma resource "*.dfm"


extern int stop;

TSEnumMainFormM *SEnumMainFormM;

TMyTabControl *tabs;



//---------------------------------------------------------------------------
__fastcall TSEnumMainFormM::TSEnumMainFormM(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}


void __fastcall TSEnumMainFormM::FormPaint(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    static bool first = true;
    if (!first) return;
    first = false;

    tabs = new TMyTabControl(Fog1);
}

void __fastcall TSEnumMainFormM::FormDestroy(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    delete tabs;
}

