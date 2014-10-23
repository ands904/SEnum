//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SaveUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSaveForm *SaveForm;
//---------------------------------------------------------------------------

static bool mode;       // 0 - содержаний, 1 - файл

extern int stop;


__fastcall TSaveForm::TSaveForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}

void __fastcall TSaveForm::SelectFilnButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    String S;
    if (mode == 0) {
        if (!CCreateDialog->Execute()) return;
        S = CCreateDialog->FileName;
    } else {
        if (!TCreateDialog->Execute()) return;
        S = TCreateDialog->FileName;
    }
    FilnEdit->Text = S;
}


static void __fastcall SetVisiblePanels(TSaveParams *sp) {
//-------------------------------------------------------------------------------
//             Устанавливает видимые панели в соответствии с флагами sp         |
//-------------------------------------------------------------------------------
    int y = 0;
    if (sp->NeedFiln) {
        SaveForm->FilnPanel->Top = y; y += SaveForm->FilnPanel->Height;
        SaveForm->FilnPanel->Visible = true;
    } else {
        SaveForm->FilnPanel->Visible = false;
    }
    if (sp->NeedF1) {
        SaveForm->F1Panel->Top = y; y += SaveForm->F1Panel->Height;
        SaveForm->F1Panel->Visible = true;
    } else {
        SaveForm->F1Panel->Visible = false;
    }
    SaveForm->P0Panel->Top = y; y += SaveForm->P0Panel->Height;
    SaveForm->ButtonsPanel->Top = y; y += SaveForm->ButtonsPanel->Height;
    SaveForm->ClientHeight = y;
}


bool __fastcall AskSaveContents(TSaveParams *sp) {
//-------------------------------------------------------------------------------
//                          Запрашивает вывод содержания                        |
// Если отказались, то возвращает false                                         |
//-------------------------------------------------------------------------------
    AnsiString S, mes;
    int res, f1, p0;
    bool error;

    SaveForm->Caption = "Параметры создания содержания";
    SaveForm->FilnEdit->Text = sp->Filn;
    SaveForm->f1Edit->Text.sprintf("%d", sp->f1);
    SaveForm->p0Edit->Text.sprintf("%d", sp->p0);
    mode = 0;

    SetVisiblePanels(sp);

    if (sp->NeedFiln && sp->Filn.IsEmpty()) {
        SaveForm->Visible = true;
        res = SaveForm->CCreateDialog->Execute();
        SaveForm->Visible = false;
        if (!res) return false;
        S = SaveForm->CCreateDialog->FileName;
        sp->Filn = S;
        SaveForm->FilnEdit->Text = S;
    }
Retry:
    if (SaveForm->ShowModal() != mrOk) return false;
    error = false;
    try {
        S = SaveForm->f1Edit->Text; f1 = S.ToInt();
        S = SaveForm->p0Edit->Text; p0 = S.ToInt();
    } catch (...) {
        error = true;
    }
    if (error) {
        mes = (String)"Синтаксическая ошибка при вводе числа!";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        goto Retry;
    }
    S = SaveForm->FilnEdit->Text; sp->Filn = S;
    sp->f1 = f1 <= 0 ? 0 : f1 - 1;
    sp->p0 = p0;
    return true;
}



bool __fastcall AskSaveTiff(TSaveParams *sp) {
//-------------------------------------------------------------------------------
//                          Запрашивает вывод файла tif                         |
// Если отказались, то возвращает false                                         |
//-------------------------------------------------------------------------------
    AnsiString S, mes;
    int res, f1, p0;
    bool error;
    SaveForm->Caption = "Параметры вывода файла tif";
    SaveForm->FilnEdit->Text = sp->Filn;
    SaveForm->f1Edit->Text.sprintf("%d", sp->f1);
    SaveForm->p0Edit->Text.sprintf("%d", sp->p0);

    SetVisiblePanels(sp);

    mode = 1;
    if (sp->Filn.IsEmpty()) {
        SaveForm->Visible = true;
        res = SaveForm->TCreateDialog->Execute();
        SaveForm->Visible = false;
        if (!res) return false;
        S = SaveForm->TCreateDialog->FileName;
        sp->Filn = S;
        SaveForm->FilnEdit->Text = S;
    }
Retry:
    if (SaveForm->ShowModal() != mrOk) return false;
    error = false;
    try {
        S = SaveForm->f1Edit->Text; f1 = S.ToInt();
        S = SaveForm->p0Edit->Text; p0 = S.ToInt();
    } catch (...) {
        error = true;
    }
    if (error) {
        mes = (String)"Синтаксическая ошибка при вводе числа!";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        goto Retry;
    }
    S = SaveForm->FilnEdit->Text; sp->Filn = S;
    sp->f1 = f1 <= 0 ? 0 : f1 - 1;
    sp->p0 = p0;
    return true;
}


static TRect __fastcall GetRect(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TRect r;
    static int k = 0;
    if (k == 0) {
        r.Left = 1; r.Top = 2; r.Right = 3; r.Bottom = 4;
    } else if (k == 1) {
        r.Left = 10; r.Top = 20; r.Right = 30; r.Bottom = 40;
    } else {
        r.Left = 100; r.Top = 200; r.Right = 300; r.Bottom = 400;
    }
    k++;
    return r;
}


#include "ForCanvasUnit.h"



void __fastcall TSaveForm::BitBtn1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
// Так... плюшечками баловался... проверял соотношение ширины текста в canvas   |
// и в Excel                                                                    |
//-------------------------------------------------------------------------------
/*
    TRect r0, r1, r2;

    r0 = GetRect();
    r1 = GetRect();
    r2 = GetRect();

    stop = r0.Left + r1.Left + r2.Left;
*/


    TCanvas *c = ForCanvasForm->Canvas;
    int i;
    char txt[200];

    setmem(txt, 200, 0);
    setmem(txt, 117, 'a');
    int len1 = c->TextWidth(txt);

    setmem(txt, 200, 0);
    setmem(txt, 88, 'a');
    c->Font->Size = 11;
    int len2 = c->TextWidth(txt);

    setmem(txt, 200, 0);
    // setmem(txt, 78, 'a');
    strcpy(txt, "aksjrm/ lkjer98734LKJLDFJ lkjf;alseoiju43kjm34m3p09vm.,m98ikj3498ulkjmsdfu9p8ujm;vmp90uj;lklkLKJ9J81ff");
    c->Font->Size = 12;
    int len3 = c->TextWidth(txt);

    setmem(txt, 200, 0);
    setmem(txt, 70, 'A');
    c->Font->Size = 10;
    int len4 = c->TextWidth(txt);

    stop = len1 + len2 + len3 + len4;

}

