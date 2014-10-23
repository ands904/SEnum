//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ForCanvasUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForCanvasForm *ForCanvasForm;
//---------------------------------------------------------------------------

__fastcall TForCanvasForm::TForCanvasForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}


/*
xlHAlignCenter == -4108
xlHAlignCenterAcrossSelection == 7
xlHAlignDistributed == -4117 - ��������� ��������� ���, ����� ����� ������� ��� ������������
xlHAlignFill == 5 - �� ����� ������� �� ����
xlHAlignGeneral == 1 - �� ��������, �.�. ��� ������ - ����
xlHAlignJustify == -4130
xlHAlignLeft == -4131
xlHAlignRight == -4152
*/


void __fastcall GetTextX(TTextX &res, Variant Range, int pos, String Text) {
//-------------------------------------------------------------------------------
//  ��������� ��������� ������ Text, ������������� � ������� pos � ������ Range |
//-------------------------------------------------------------------------------
    // TTextX res = {0, 0};
    TCanvas *c = ForCanvasForm->Canvas;
    Variant MergeArea, VRangeText, VFont, v;
    String FontName;
    String RangeText;           // ���� ����� � ������
    // double k = 529.5 / 700;     // ����������� �������� �������� ������� � �� ������
    // double k = 600. / 700;      // ����������� �������� �������� ������� � �� ������
    double k = 529.5 / 700;      // ����������� �������� �������� ������� � �� ������
    double rwidth;              // ������ ����� ������ � ������ - � �� ������
    double bwidth;              // ������ ������ ����� ����� - � �� ������
    double twidth;              // ������ ������ ������ - � �� ������
    double left;                // ������ ������ Range - � �� ������
    double width;               // ������ ���� ������ Range - � �� ������
    double tleft;               // ��� � ������ ���������� ���������������� �����
    int ali;                    // ������������ ������ � ������
    int fs;

    res.x0 = 0; res.x1 = 0;

    MergeArea = Range.OlePropertyGet("MergeArea");

    //VRangeText = Range.OlePropertyGet("Text");
    //RangeText = VRangeText;
    //v = Range.OlePropertyGet("Left");
    //left = v;
    //v = Range.OlePropertyGet("Width");
    //width = v;

    VRangeText = Range.OlePropertyGet("Text");
    RangeText = VRangeText;
    v = MergeArea.OlePropertyGet("Left");
    left = v;
    v = MergeArea.OlePropertyGet("Width");
    width = v;


    VFont = Range.OlePropertyGet("Font");
    v = VFont.OlePropertyGet("Name");
    FontName = v;
    v = VFont.OlePropertyGet("Size");
    fs = v;

    c->Font->Name = FontName;
    c->Font->Size = fs;

    rwidth = c->TextWidth(RangeText);           // ������ ����� ������ � ������
    rwidth *= k;                                // � �������� ������

    if (pos == 0) {                             // ������ ������ ����� ���������
        bwidth = 0;
    } else {
        if (pos < RangeText.Length()) {
            String BeforeText;
            char *p = RangeText.c_str();
            char keep;
            keep = p[pos];
            p[pos] = 0;
            BeforeText = p;
            p[pos] = keep;
            bwidth = c->TextWidth(BeforeText);
            bwidth *= k;
        } else {
            bwidth = rwidth;
        }
    }

    twidth = c->TextWidth(Text);                // ������ ������ ������
    twidth *= k;

    v = Range.OlePropertyGet("HorizontalAlignment");
    ali = v;
    switch(ali) {
        case -4108:   // xlHAlignCenter == -4108
            tleft = left + width / 2 - rwidth / 2;
            break; 
        case -4152:   // xlHAlignRight == -4152
            tleft = left + width - rwidth;
            break;
        // case 7:       // xlHAlignCenterAcrossSelection == 7
        // case -4117:   // xlHAlignDistributed == -4117 - ��������� ��������� ���, ����� ����� ������� ��� ������������
        // case 5:       // xlHAlignFill == 5 - �� ����� ������� �� ����
        // case 1:       // xlHAlignGeneral == 1 - �� ��������, �.�. ��� ������ - ����
        // case -4130:   // xlHAlignJustify == -4130
        // case -4131:   // xlHAlignLeft == -4131
        default:        // left
            tleft = left;
            break;
    }

    res.x0 = tleft + bwidth;
    res.x1 = res.x0 + twidth;

    return; // res;
}





