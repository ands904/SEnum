//---------------------------------------------------------------------------


#include <vcl.h>
#pragma hdrstop

#include "SEnumMainUnitM.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Fog"
#pragma resource "*.dfm"


extern int stop;

TSEnumMainFormM *SEnumMainFormM;

TMyTabControl *tabs;

// alloca

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
    tabs->SetOnDragDropX(OnDragDropX);
    tabs->SetAcceptFiles(true);
}

void __fastcall TSEnumMainFormM::FormDestroy(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    delete tabs;
}


void __fastcall TSEnumMainFormM::Fog1DragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
//-------------------------------------------------------------------------------
// ??? Чисто для проверки TMyTabUnit->OnDragOver                                |
//-------------------------------------------------------------------------------
    stop++;
    if (Source == Label2) Accept = false;

}

void __fastcall TSEnumMainFormM::Fog1DragDrop(TObject *Sender, TObject *Source, int X, int Y) {
//-------------------------------------------------------------------------------
// ??? Чисто для проверки TMyTabUnit->OnDragDrop                                |
//-------------------------------------------------------------------------------
    char buf[100];
    wsprintf(buf, "tabs->DropIndex = %d", tabs->DropIndex);
    Application->MessageBox(buf, "info", 0);
}


void __fastcall TSEnumMainFormM::OnDragDropX(TMyTabControl *Sender, TMyDropData *DropData) {
//-------------------------------------------------------------------------------
// ??? чиста для проверки - тест драг энд дроп списка файлов                    |
//-------------------------------------------------------------------------------
    AnsiString mes;
    TStringList *sl;

    stop++;

    int index = Sender->GetRectIndex(DropData->DropPt.x, DropData->DropPt.y);

    mes.printf("x=%d, y=%d\015\012", DropData->DropPt.x, DropData->DropPt.y);
    if (index >= 0) {
        DropData->ModifyFileList(MFL_RECURSIVE);
        sl = DropData->GetFileList();
        mes += sl->Text;
        Sender->SetTabIndex(index);
    } else {
        mes += "Out of tabs";
    }

    Application->BringToFront();
    Application->MessageBox(mes.c_str(), "info", 0);

}





