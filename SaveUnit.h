//---------------------------------------------------------------------------

#ifndef SaveUnitH
#define SaveUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSaveForm : public TForm
{
__published:	// IDE-managed Components
    TSaveDialog *CCreateDialog;
    TSaveDialog *TCreateDialog;
    TPanel *FilnPanel;
    TLabel *Label1;
    TEdit *FilnEdit;
    TButton *SelectFilnButton;
    TPanel *F1Panel;
    TLabel *Label2;
    TEdit *f1Edit;
    TPanel *P0Panel;
    TEdit *p0Edit;
    TLabel *Label3;
    TPanel *ButtonsPanel;
    TButton *Button1;
    TButton *Button2;
    TBitBtn *BitBtn1;
    void __fastcall SelectFilnButtonClick(TObject *Sender);
    void __fastcall BitBtn1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TSaveForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSaveForm *SaveForm;
//---------------------------------------------------------------------------

struct TSaveParams {
    String Filn;            // ��� �����
    int f1;                 // ����� ������� �����, � �������� �������� ���������
    int p0;                 // ����� ������ ��������
    bool NeedFiln;          // ���� �� ��� �����?
    bool NeedF1;            // ���� �� ����� ������� �����
    inline TSaveParams(char *_filn, int _f1, int _p0, bool nfn, bool nf1) {
        Filn = _filn; f1 = _f1; p0 = _p0;
        NeedFiln = nfn; NeedF1 = nf1;
    }
};

bool __fastcall AskSaveContents(TSaveParams *sp);
//-------------------------------------------------------------------------------
//                          ����������� ����� ����������                        |
// ���� ����������, �� ���������� false                                         |
//-------------------------------------------------------------------------------

bool __fastcall AskSaveTiff(TSaveParams *sp);
//-------------------------------------------------------------------------------
//                          ����������� ����� ����� tif                         |
// ���� ����������, �� ���������� false                                         |
//-------------------------------------------------------------------------------

#endif
