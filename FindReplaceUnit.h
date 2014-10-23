//---------------------------------------------------------------------------

#ifndef FindReplaceUnitH
#define FindReplaceUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
struct TFindReplaceStru {
    int What;           // 0 - cancel, 1 - show, 2 - delete
    String From;
    String To;
    TFindReplaceStru(void) {What = 0; From = ""; To = "";}
};


class TFindReplaceForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *Label1;
    TEdit *FindEdit;
    TButton *ShowButton;
    TButton *DeleteButton;
    TButton *CancelButton;
private:	// User declarations
public:		// User declarations
    __fastcall TFindReplaceForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFindReplaceForm *FindReplaceForm;
//---------------------------------------------------------------------------

void __fastcall FindReplaceDialog(TFindReplaceStru *fr);
//-------------------------------------------------------------------------------
//                          Вызывает диалог Найти-заменить                      |
//-------------------------------------------------------------------------------

#endif
