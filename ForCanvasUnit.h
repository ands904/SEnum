//---------------------------------------------------------------------------

#ifndef ForCanvasUnitH
#define ForCanvasUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComObj.hpp>
//---------------------------------------------------------------------------
class TForCanvasForm : public TForm
{
__published:	// IDE-managed Components
private:	// User declarations
public:		// User declarations
    __fastcall TForCanvasForm(TComponent* Owner);
};

struct TTextX {     // ������� ������ �� X ������ ������������ �����
    double x0;
    double x1;
};

//---------------------------------------------------------------------------
extern PACKAGE TForCanvasForm *ForCanvasForm;
//---------------------------------------------------------------------------

void __fastcall GetTextX(TTextX &res, Variant Range, int pos, String Text);
//-------------------------------------------------------------------------------
//  ��������� ��������� ������ Text, ������������� � ������� pos � ������ Range | 
//-------------------------------------------------------------------------------
#endif
