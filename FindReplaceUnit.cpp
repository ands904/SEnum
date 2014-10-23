//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FindReplaceUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFindReplaceForm *FindReplaceForm;
//---------------------------------------------------------------------------

void __fastcall FindReplaceDialog(TFindReplaceStru *fr) {
//-------------------------------------------------------------------------------
//                          Вызывает диалог Найти-заменить                      |
//-------------------------------------------------------------------------------
    int res;

    res = FindReplaceForm->ShowModal();
    if (res == mrRetry) {
        fr->What = 1;
    } else if (res == mrOk) {
        fr->What = 2;
    } else {
        fr->What = 0;
    }

    fr->From = FindReplaceForm->FindEdit->Text;
}


__fastcall TFindReplaceForm::TFindReplaceForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}




