//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TiffImageConfigUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTiffImageConfigForm *TiffImageConfigForm;

static int __fastcall _PackIntEditText(TEdit *e) {
//-------------------------------------------------------------------------------
//                  Пакует целое число, содержащееся в e->Text                  |
// Если неправильно - возвращает -1                                             |
//-------------------------------------------------------------------------------
    String S = e->Text;
    S = S.Trim();
    int res;
    try {
        res = S.ToInt();
    } catch (...) {
        res = -1;
    }
    return res;
}

bool __fastcall EditTiffImageConfig(TTiffImageConfig *cfg) {
//-------------------------------------------------------------------------------
//              Редактирование конфигурации редактирования файла *.tiff         |
// Возвращает false, если от конфигурации отказались                            |
//-------------------------------------------------------------------------------
    TiffImageConfigForm->Ini(cfg);

    int res = TiffImageConfigForm->ShowModal();
    if (res != mrOk) return false;

    TiffImageConfigForm->Pack(cfg);
    return true;
}










//---------------------------------------------------------------------------
__fastcall TTiffImageConfigForm::TTiffImageConfigForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}


static bool FirstFormPaint = true;

void __fastcall TTiffImageConfigForm::FormPaint(TObject *Sender) {
//-------------------------------------------------------------------------------
//              Проверяет, не надо ли запросить имя файла.                      |
// Запускает запрос имени файла через таймер                                    |
//-------------------------------------------------------------------------------
    if (!FirstFormPaint) return;
    FirstFormPaint = false;

    String S = FilenameEdit->Text;

    if (FilenamePanel->Visible && S.IsEmpty()) {
        Timer1->Enabled = true;
        Enabled = false;
    }
}

void __fastcall TTiffImageConfigForm::Timer1Timer(TObject *Sender) {
//-------------------------------------------------------------------------------
//                          Безусловно запрашивает имя файла                    |
//-------------------------------------------------------------------------------
    Timer1->Enabled = false;
    Enabled = true;

    if (!OpenDialog1->Execute()) {
        Application->MessageBox("Для указанной операции необходимо выбрать файл", "Не могу!", 0);
        ModalResult = mrCancel;
    }

    FilenameEdit->Text = OpenDialog1->FileName;
}


void __fastcall TTiffImageConfigForm::Ini(TTiffImageConfig *cfg) {
//-------------------------------------------------------------------------------
//                      Конфигурирует необходимые панели                        |
//-------------------------------------------------------------------------------
    int y = 0;

    Caption = cfg->Caption;

    if (cfg->NeedEnumPanel) {
        EnumPanel->Top = y;
        EnumPanel->BevelOuter = bvNone;
        y += EnumPanel->Height;
        FirstEnumPageEdit->Text = cfg->FirstEnumPage;
        FirstPageNumEdit->Text = cfg->FirstPageNum;
        EnumPanel->Visible = true;
    } else {
        EnumPanel->Visible = false;
    }

    if (cfg->NeedReplacePanel) {
        ReplacePanel->Top = y;
        ReplacePanel->BevelOuter = bvNone;
        y += ReplacePanel->Height;
        ReplaceStartEdit->Text = cfg->Start;
        ReplaceEndEdit->Text = cfg->End;
        ReplacePanel->Visible = true;
    } else {
        ReplacePanel->Visible = false;
    }

    if (cfg->NeedDeletePanel) {
        DeletePanel->Top = y;
        DeletePanel->BevelOuter = bvNone;
        y += DeletePanel->Height;
        DeleteStartEdit->Text = cfg->Start;
        DeleteEndEdit->Text = cfg->End;
        DeletePanel->Visible = true;
    } else {
        DeletePanel->Visible = false;
    }

    if (cfg->NeedFilenamePanel) {
        FilenamePanel->Top = y;
        FilenamePanel->BevelOuter = bvNone;
        y += FilenamePanel->Height;
        FilenameEdit->Text = cfg->Filename;
        FilenamePanel->Visible = true;
    } else {
        FilenamePanel->Visible = false;
    }

    OkCancelPanel->Top = y;
    OkCancelPanel->BevelOuter = bvNone;
    y += OkCancelPanel->Height;
    OkCancelPanel->Visible = true;

    ClientHeight = y;

    FirstFormPaint = true;
}

void __fastcall TTiffImageConfigForm::Pack(TTiffImageConfig *cfg) {
//-------------------------------------------------------------------------------
//                      Пакует параметры из формы обратно в cfg                 |
//-------------------------------------------------------------------------------
    if (cfg->NeedEnumPanel) {
        cfg->FirstEnumPage = _PackIntEditText(FirstEnumPageEdit);
        cfg->FirstPageNum = _PackIntEditText(FirstPageNumEdit);
    }

    if (cfg->NeedReplacePanel) {
        cfg->Start = _PackIntEditText(ReplaceStartEdit);
        cfg->End = _PackIntEditText(ReplaceEndEdit);
    }

    if (cfg->NeedDeletePanel) {
        cfg->Start = _PackIntEditText(DeleteStartEdit);
        cfg->End = _PackIntEditText(DeleteEndEdit);
    }

    if (cfg->NeedFilenamePanel) {
        String S = FilenameEdit->Text;
        cfg->Filename = S;
    }

}


void __fastcall TTiffImageConfigForm::OkButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    int val;
    val = _PackIntEditText(FirstEnumPageEdit);
    if (val < 0) {
        Application->MessageBox("\"С какой страницы нумеровать\" должно быть целым положительным числом!", "Ошибка!", 0);
        return;
    }

    val = _PackIntEditText(FirstPageNumEdit);
    if (val < 0) {
        Application->MessageBox("\"Номер первой нумеруемой страницы\" должен быть целым положительным числом!", "Ошибка!", 0);
        return;
    }

    ModalResult = mrOk;
}

void __fastcall TTiffImageConfigForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (Key == VK_ESCAPE) {
        ModalResult = mrCancel;
    }
}



void __fastcall TTiffImageConfigForm::SelectFileSpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!OpenDialog1->Execute()) return;
    FilenameEdit->Text = OpenDialog1->FileName;
}

