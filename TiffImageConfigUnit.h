//---------------------------------------------------------------------------

#ifndef TiffImageConfigUnitH
#define TiffImageConfigUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TTiffImageConfig;

class TTiffImageConfigForm : public TForm
{
__published:	// IDE-managed Components
    TPanel *EnumPanel;
    TLabel *Label1;
    TEdit *FirstEnumPageEdit;
    TLabel *Label2;
    TEdit *FirstPageNumEdit;
    TPanel *OkCancelPanel;
    TButton *OkButton;
    TButton *CancelButton;
    TPanel *DeletePanel;
    TLabel *Label3;
    TLabel *Label4;
    TEdit *DeleteStartEdit;
    TEdit *DeleteEndEdit;
    TPanel *ReplacePanel;
    TLabel *Label5;
    TLabel *Label6;
    TEdit *ReplaceStartEdit;
    TEdit *ReplaceEndEdit;
    TPanel *FilenamePanel;
    TLabel *Label7;
    TEdit *FilenameEdit;
    TSpeedButton *SelectFileSpeedButton;
    TOpenDialog *OpenDialog1;
    TTimer *Timer1;
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall SelectFileSpeedButtonClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    void __fastcall Ini(TTiffImageConfig *cfg);
    void __fastcall Pack(TTiffImageConfig *cfg);
    __fastcall TTiffImageConfigForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TTiffImageConfigForm *TiffImageConfigForm;
//---------------------------------------------------------------------------

struct TTiffImageConfig {
    String Caption;

    bool NeedEnumPanel;             // ��������� ������ ��������� �������
    int FirstEnumPage;              // ����� ��������, � ������� �������� ���������
    int FirstPageNum;               // ����� ������ ��������
    bool __fastcall IsZeroEnum(void) {return FirstEnumPage == 0 || FirstPageNum == 0;}

    bool NeedReplacePanel;          // ��������� ������ "��������"
    bool NeedDeletePanel;           // ��������� ������ "�������"
    int Start, End;                 // ������ ������ �������-��������

    bool NeedFilenamePanel;         // ��������� ������ "��� �����"
    String Filename;                // ���������� ��� �����

    void __fastcall Falsify(void) { // ������ ��� ����� false
        NeedEnumPanel = false;
        NeedReplacePanel = false;
        NeedDeletePanel = false;
        NeedFilenamePanel = false;
    }
};


bool __fastcall EditTiffImageConfig(TTiffImageConfig *cfg);
//-------------------------------------------------------------------------------
//              �������������� ������������ �������������� ����� *.tiff         |
// ���������� false, ���� �� ������������ ����������                            |
//-------------------------------------------------------------------------------


#endif
