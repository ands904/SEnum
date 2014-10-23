//---------------------------------------------------------------------------

#ifndef SplitMainUnitH
#define SplitMainUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TButton *Button1;
    TButton *Button2;
    TEdit *Edit1;
    TEdit *Edit2;
    TButton *Button3;
    TButton *Button4;
    TButton *Button5;
    TButton *Button6;
    TButton *Button7;
    TButton *Button8;
    TNotebook *Notebook1;
    TCheckBox *CreateA4CheckBox;
    TCheckBox *CreateA3CheckBox;
    TLabel *Label1;
    TEdit *FilnEdit;
    TSpeedButton *SelectFileSpeedButton;
    TCheckBox *PrintA4CheckBox;
    TCheckBox *PrintA3CheckBox;
    TCheckBox *CreateA0CheckBox;
    TCheckBox *PrintA0CheckBox;
    TButton *OkButton;
    TButton *CancelButton;
    TLabel *Label2;
    TEdit *A4CopiesEdit;
    TLabel *Label3;
    TEdit *A0CopiesEdit;
    TOpenDialog *OpenDialog1;
    TButton *Button9;
    TEdit *RepeatEdit;
    TLabel *Label4;
    TEdit *A3CopiesEdit;
    TButton *TestOLE;
    TPrintDialog *PrintDialog1;
    TPrinterSetupDialog *PrinterSetupDialog1;
    TButton *Button10;
    TEdit *P4Edit;
    TSpeedButton *P4SpeedButton;
    TEdit *P3Edit;
    TEdit *P0Edit;
    TSpeedButton *P3SpeedButton;
    TSpeedButton *P0SpeedButton;
    TLabel *Label5;
    TComboBox *A3ComboBox;
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Edit1KeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall Button3Click(TObject *Sender);
    void __fastcall Button4Click(TObject *Sender);
    void __fastcall Button5Click(TObject *Sender);
    void __fastcall Button6Click(TObject *Sender);
    void __fastcall Button7Click(TObject *Sender);
    void __fastcall Button8Click(TObject *Sender);
    void __fastcall CreateA4CheckBoxClick(TObject *Sender);
    void __fastcall CreateA0CheckBoxClick(TObject *Sender);
    void __fastcall SelectFileSpeedButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall Button9Click(TObject *Sender);
    void __fastcall CreateA3CheckBoxClick(TObject *Sender);
    void __fastcall TestOLEClick(TObject *Sender);
    void __fastcall Button10Click(TObject *Sender);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall P4SpeedButtonClick(TObject *Sender);
    void __fastcall P3SpeedButtonClick(TObject *Sender);
    void __fastcall P0SpeedButtonClick(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
