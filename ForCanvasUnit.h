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

struct TTextX {     // Позиция текста по X внутри экселевского файла
    double x0;
    double x1;
};

//---------------------------------------------------------------------------
extern PACKAGE TForCanvasForm *ForCanvasForm;
//---------------------------------------------------------------------------

void __fastcall GetTextX(TTextX &res, Variant Range, int pos, String Text);
//-------------------------------------------------------------------------------
//  Вычисляет положение текста Text, начинающегося в позиции pos в тексте Range | 
//-------------------------------------------------------------------------------
#endif
