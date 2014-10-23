//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SEnumLib.h"
#include "ContentsUnit.h"
#include "SaveUnit.h"
#include "FindReplaceUnit.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TContentsForm *ContentsForm;
//---------------------------------------------------------------------------

static TFileList *fl;
static int f1, p0;

class TContentsV;
static TContentsV *contents = NULL;

static TSaveParams *contents_sp;

static TFindReplaceStru fr;



//===============================================================================
//                                  EditContents                                |
//===============================================================================
int __fastcall EditContents(TFileList *_fl, TSaveParams *sp) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    fl = _fl;
    contents_sp = sp;

    TContentsVRec *cr;
    TFileListRec *r;
    String mes;
    TCanvas *c = ContentsForm->ContentsDrawGrid->Canvas;
    int w = ContentsForm->GetContentsWidth();
    int i;
    char buf[200];
    bool bold;

    contents = new TContentsV(c, w);

    for (i = sp->f1; i < fl->Nrecs; i++) {
        r = fl->Rec1(i);
        if (r->oname.IsEmpty()) {
            mes = TrimExt(r->filn);
        } else {
            mes = r->oname;
        }
        //Selection.OleProcedure("MoveRight", 12);        // wdCell
        //Selection.OleProcedure("TypeText", mes.c_str());
        if (IsSubstr(mes.c_str(), "���������", true) ||
            IsSubstr(mes.c_str(), "�������", true)
        ) {
            bold = true;
        } else {
            bold = false;
        }
        cr = contents->Append(bold, r->pi.pcount, mes.c_str());
        // r = contents->Append(false, i % 2 + 1, Contents[i]);
        cr->Split(c, w);
    }

    contents->Enum(sp->p0);

    ContentsForm->ResizeContentsGridHeights();

    ContentsForm->ShowModal();

    delete contents; contents = NULL;

    return 0;
}


void __fastcall DebugEditContents(TSaveParams *sp) {
//-------------------------------------------------------------------------------
//                      ������� ��� ������� EditContents                        |
//-------------------------------------------------------------------------------
    static unsigned char *Contents[] = {
        "������� ������: ��������� ������. ����������� �� ��� ������� ���� ��� ��������� ��������. �� �� ������ ����� � ����������� � ������� �������, ��� �� ������� �� �����������. � ���� ��� ������ � ����� ������� � �����, �����, ������ ������. �������� ��������� �� ������ ��� ����, ��������� �����, �������, ������, ���� �����, � ������ ��� ������� �� �����",
        "������ �������� ������",
        "������ ������ ������� �������, �� ���� ��������",
        "������ ������ ��� �������, �� ���� �� �� �������, � ���, ������. ��� �����",
        "��������� ������: ���� ����� ����� �����, ����� ������� �����, �� ��� ����� ��� ������, �� ��������, ��� ����. ����� ��������. ������ ���� ��������.",
        "��� �������� ��������� ������ �� ����� ������������������ ������ ������������ ������� ����� �������, �������� ������ ����������, ��� ���������� �� ����� ����� ��������� ������, ����� ����� ��������� �� ���� �� ������.",
        //"",
        //"",
        //"",
        //"",
        //"",
        //"",
        0
    };
    TContentsVRec *r;
    TCanvas *c = ContentsForm->ContentsDrawGrid->Canvas;
    int w = ContentsForm->GetContentsWidth();
    int p = sp->p0;

/*
    {
        stop++;
        TContentsVRec vr = {0, 0, 0, "�������� ������ ��� �������� �������"};
        TCanvas *c = ContentsForm->ContentsDrawGrid->Canvas;
        vr.Split(c, false, 500);
        vr.PaintGray("������");
        vr.PaintGray("�� ����");
        // vr.PaintGray("��������");
        vr.PaintGray("�������");
        vr.DelGrays();
        stop++;
    }
*/
    contents_sp = sp;

    contents = new TContentsV(c, w);
    for (int i = 0; Contents[i] != 0; i++) {
        r = contents->Append(i == 1 || i == 4, i % 2 + 1, Contents[i]);
        // r = contents->Append(false, i % 2 + 1, Contents[i]);
        r->Split(c, w);
        // r->page = p; p += r->npages;
    }
    contents->Enum(p);

    // contents->SetGray("����");

    ContentsForm->ResizeContentsGridHeights();

    ContentsForm->ShowModal();


    delete contents; contents = NULL;
}












//===============================================================================
//                                  TContents                                   |
//===============================================================================
void __fastcall TContentsVRec::UnSplit(void) {
//-------------------------------------------------------------------------------
//                          ������� �� ������ ��������� �������                 |
//-------------------------------------------------------------------------------
    unsigned char *p, *q;
    for (p = q = text; *p != 0; p++) {
        if (*p >= 0x20) {
            *q = *p;
            q++;
        }
    }
    *q = 0;
}


void __fastcall TContentsVRec::Split(TCanvas *c, int width) {
//-------------------------------------------------------------------------------
//              ��������� ����� �� ��������� ������� �� ����� width             |
// 0 - ����� ������, ��� ������                                                 |
// 1 - ������ ������� ������                                                    |
// 2 - ������ �����                                                             |
// 3 - ����� �����                                                              |
// 4 - ������ ���������� �������� � ����� ������                                |
// 5 - ����� ������                                                             |
// <1> <2> <�������> <3> <2> <�� �������> <3> <4> <�������> <5> <2> <�� �> <5>  |
//-------------------------------------------------------------------------------
    TFontStyles fs, fsk;
    int cnt, w, ww;
    unsigned char *buf, *p, *q, keep;

    nlines = 0;

    stop = strlen(text);

    UnSplit();
    buf = new unsigned char[5000]; p = buf; q = text;

    fsk = c->Font->Style; fs = fsk;
    if (bold) {
        // *q++ = 1;                       // ������ ������� ������
        c->Font->Style = fs << fsBold;
    }

    // �������� �� ������� <�������> � <�� �������>
    while(*q != 0) {
        if (*q == ' ') {
            *p++ = 2;                   // ������ �����
            while(*q == ' ') {
                cnt++;
                *p++ = *q++;
            }
            *p++ = 3;                   // ����� �����
        }
        if (*q == 0) {
            *p++ = 0;
            break;
        }
        *p++ = 2;                       // ������ �����
        while(*q != ' ' && *q != 0) {
            cnt++;
            *p++ = *q++;
        }
        *p++ = 3;                       // ����� �����
    }
    *p = 0;
    strcpy(text, buf);

    // � ��� ������ ������������� ����������
    p = text;
    w = 0;
    while(1) {
        while(*p != 2 && *p != 0) p++;
        if (*p == 0) break;
        q = p + 1;
        while(*q != 3) q++;
        *q = 0; ww = c->TextWidth((char*)(p+1)); *q = 3;
        if (w + ww >= width) {                      // ����� �����
            nlines++;
            if (w == 0) {                           // ���� ��� ������������ ����� � ������
                *q = 5;                             //   �������� � ����� ����� ����� ����� ������
                p = q;
            } else {                                // ����� �� ������������
                if (p[1] == ' ') {                  //   ���� ����� - �������
                    *p = 4;                         //      �������� ����� ��������� 4
                    *q = 5;                         //      ������, ��� ��� ������� - ����� ������
                    p = q + 1;
                } else {                            //   ����� - �� �������
                    p[-1] = 5;                      //      �������� � ���������� ����� ����� ������
                }
            }
            w = 0;
            continue;
        } else {
            w += ww;
        }
        p = q + 1;
    }
    if (w > 0) nlines++;

    delete[] buf;
    c->Font->Style = fsk;
}


static unsigned char * __fastcall _IsGrayPattern(unsigned char *p, unsigned char *pat) {
//-------------------------------------------------------------------------------
//                  ���������, �� ���� �� � ��������� ����� �������             |
// ���������� ��� ������������                                                  |
// ���� ��, �� ���������� ����� �����, ����� ���������� NULL                    |
//-------------------------------------------------------------------------------
    if (*p <= 0x20) return NULL;
    while(1) {
        while(*pat != 0 && *pat <= 0x20) pat++;
        if (*pat == 0) return p;
        while(*p != 0 && *p <= 0x20) p++;
        if (*p == 0 || Capitalize(*p) != Capitalize(*pat)) break;
        p++; pat++;
    }
    return NULL;
}


void __fastcall TContentsVRec::PaintGray(String pattern) {
//-------------------------------------------------------------------------------
//                      ����������� � ����� ���� ��������� �������              |
// ����������� ������� ������ ����� <8>, <9> ������ �������                     |
// ������� ����������, ��� ������                                               |
//-------------------------------------------------------------------------------
    int len;
    unsigned char *buf, *p, *q, *g, *pat = pattern.c_str();

    if (bold) return;
    if (pattern.Length() == 0) return;

    buf = new char[5000];
    q = buf; p = text;

    while(*p != 0) {
        g = _IsGrayPattern(p, pat);
        if (g != NULL) {
            *q++ = 8;
            len = int(g) - int(p);
            memmove(q, p, len);
            q += len;
            *q++ = 9;
            p = g;
        } else {
            *q++ = *p++;
        }
    }
    *q = 0;

    strcpy(text, buf);
    delete[] buf;
}


static unsigned char * __fastcall _FindGrayEnd(unsigned char *p) {
//-------------------------------------------------------------------------------
//                  ���� ����� ������ �������, ������������� � p                |
// ���� ����� ������� � � �� ���������� - ���������� NULL.                      |
// ���� ���������� - ���������� ����� �����                                     |
// � ������ ��������� ����������                                                |
//-------------------------------------------------------------------------------
    int depth;

    if (*p != 8) return NULL;

    depth = 1; p++;
    while(1) {
        if (*p == 9) {
            if (--depth == 0) break;
        } else if (*p == 8) {
            depth++;
        } else if (*p == 0) {               // �����-�� ������
            return NULL;                    // ������ ���� �� ����� ��������
        }
        p++;
    }
    return p + 1;
}

void __fastcall TContentsVRec::DelGrays(void) {
//-------------------------------------------------------------------------------
//                          ������� ��� ����� �������                           |
// ������� ��� ������� ������ <8> <9>, � ������ ��������� ����������            |
//-------------------------------------------------------------------------------
    unsigned char *buf, *p, *q, *g;

    buf = new char[5000];
    q = buf; p = text;

    while(*p != 0) {
        g = _FindGrayEnd(p);
        if (g != NULL) {
            p = g;
        } else {
            *q++ = *p++;
        }
    }
    *q = 0;

    strcpy(text, buf);
    delete[] buf;
}



TContentsV::TContentsV(TCanvas *_c, int _width) : TVds(VdsLen0, sizeof(TContentsVRec), 100000) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    c = _c; width = _width;
    gray = "";
}

void __fastcall TContentsV::SetGray(String g) {
//-------------------------------------------------------------------------------
//                              �������� ��������� ������                       |
//-------------------------------------------------------------------------------
    TContentsVRec *r;
    gray = g;
    if (g.Length() == 0) return;
    for (int i = 0; i < Nrecs; i++) {
        Rec1(i)->PaintGray(g);
    }
}













//===============================================================================
//                              TContentsForm                                   |
//===============================================================================
__fastcall TContentsForm::TContentsForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}


int __fastcall TContentsForm::GetContentsWidth(void) {
//-------------------------------------------------------------------------------
//                  ���������� ������ ��������� ������� ����������              |
//-------------------------------------------------------------------------------
    return ContentsDrawGrid->ColWidths[0] - 3;
}

int __fastcall TContentsForm::GetFontHeight(void) {
//-------------------------------------------------------------------------------
//         ���������� �������� ������ ����� ������ � ContentsDrawGrid           |
//-------------------------------------------------------------------------------
/*
    TCanvas *c = ContentsDrawGrid->Canvas;
    double h = c->Font->Height;
    if (h < 0) h = -h;
    h *= 1.3;
    return h; // + 0.5;
*/
    TCanvas *c = ContentsDrawGrid->Canvas;
    TSize ss = c->TextExtent("A");
    return ss.cy;
}

void __fastcall TContentsForm::ResizeContentsGridHeights(void) {
//-------------------------------------------------------------------------------
//        ������������� ���������� ����� � ������ ����� � ContentsDrawGrid      |
// � ����������� �� TContentsV                                                  |
//-------------------------------------------------------------------------------
    TContentsVRec *r;
    int i, k, h;

    k = contents->Nrecs + 1;
    if (ContentsDrawGrid->RowCount != k) ContentsDrawGrid->RowCount = k;

    h = GetFontHeight();
    for (i = 0; i < contents->Nrecs; i++) {
        r = contents->Rec1(i);
        k = r->nlines * h + 5;
        if (ContentsDrawGrid->RowHeights[i + 1] != k) ContentsDrawGrid->RowHeights[i + 1] = k;
        stop++;
    }
}


static void __fastcall DrawInsideCell(TCanvas *c, TRect Rect, String text) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TSize s = c->TextExtent(text);
    int x = (Rect.Width() - s.cx) / 2;
    int y = (Rect.Height() - s.cy) / 2;
    c->TextOut(x+Rect.left, y+Rect.top, text);
}


void __fastcall TContentsForm::ContentsDrawGridDrawCell(TObject *Sender,
      int ACol, int ARow, TRect &Rect, TGridDrawState State) {
//-------------------------------------------------------------------------------
// 0 - ����� ������, ��� ������                                                 |
// 1 - ������ ������� ������                                                    |
// 2 - ������ �����                                                             |
// 3 - ����� �����                                                              |
// 4 - ������ ���������� �������� � ����� ������                                |
// 5 - ����� ������                                                             |
// <1> <2> <�������> <3> <2> <�� �������> <3> <4> <�������> <5> <2> <�� �> <5>  |
// <8> <9> - ������� ������                                                     |
//-------------------------------------------------------------------------------
    TCanvas *c = ContentsDrawGrid->Canvas;
    TContentsVRec *r;
    TFontStyles fs, fsk;
    int xstart = 1, ystart = 2;
    int h, x, y;
    int gray_depth;
    unsigned char *p, *q, keep;

    if (contents->ReSplitAll(GetContentsWidth())) {
        ResizeContentsGridHeights();
    }

    if (ARow < 0 || ARow > contents->Nrecs) return;

    if (ARow == 0) {
        fsk = c->Font->Style; fs = fsk;
        // if (bold) {
            c->Font->Style = fs << fsBold;
        // }
        DrawInsideCell(c, Rect, ACol == 0 ? "������������" : "���");
        return;
    }

    r = contents->Rec1(ARow - 1);

    if (r->bold) {
        fsk = c->Font->Style; fs = fsk;
        // if (bold) {
            c->Font->Style = fs << fsBold;
        // }
    }

    if (ACol == 1) {
        char buf[50];
        wsprintf(buf, "%5d", r->page);
        DrawInsideCell(c, Rect, buf);
        return;
    }

    h = GetFontHeight();
    c->Font->Color = clBlack;
    unsigned char *start = NULL, cmd;
    for (p = r->text, x = xstart, y = ystart, gray_depth = 0; *p != 0; p++) {
        cmd = *p;
        if (cmd == 1) {                                             // ������ ����� - ���� �� �����������
        } else if (cmd == 2) {                                      // ������ �����
            start = p + 1;
        } else if (cmd == 3 || cmd == 5 || cmd == 8 || cmd == 9) {  // ����� �����
            if (start != NULL) {
                keep = *p; *p = 0;
                c->TextOut(x+Rect.Left, y+Rect.Top, (char*)start);
                x += c->TextWidth((char*)start);
                *p = keep;
                start = NULL;
                Application->ProcessMessages();
                stop++;
            }
            if (cmd == 5) {                                         // ����� ������
                x = xstart; y += h;
            } else if (cmd == 8 || cmd == 9) {                      // ������������ ������
                gray_depth += *p == 8 ? 1 : -1;
                if (gray_depth) {
                    // c->Font->Color = clGray;
                    // c->Font->Color = clSilver;
                    // c->Font->Color = clBlue;
                    c->Font->Color = clRed;
                } else {
                    c->Font->Color = clBlack;
                }
                start = p + 1;
            }
        } else if (cmd == 4) {                                      // ������ ���������� �������� � ����� ������
            while(*p != 5) p++;
            p--;
        }
/*
        switch(*p) {
            case 1:                 // ������ ����� ���� ����������
                break;
            case 2:                 // ������ �����
                start = ++p;
                break;
                //p++; q = p; while(*q >= ' ') q++;
                //keep = *q; *q = 0;
                //c->TextOut(x+Rect.Left, y+Rect.Top, (char*)p);
                //x += c->TextWidth((char*)p);
                //*q = keep;
                //p = q - 1;
                //break;
            case 3:                 // ��������, ��� � ����� ����� ������ ������ �� ����
                break;
            case 4:                 // ������ ���������� �������� � ����� ������
                while(*p != 5) p++;
                p--;
                break;
            case 5:                 // ����� ������
                x = xstart; y += h;
                break;
            case 8: case 9:         // ������������ ������� ������
                gray_depth += *p == 8 ? 1 : -1;
                if (gray_depth) {
                    c->Font->Color = clGray;
                } else {
                    c->Font->Color = clBlack;
                }
                break;
        }
*/        
    }

}

// AnsiString

void __fastcall TContentsForm::StopEditCurCell(bool Save) {
//-------------------------------------------------------------------------------
//                ���������� �������������� ������� ������                      |
// Save - ��������� �� ���������                                                |
// ���� �� �� ����������� - ������ ����� ������, ������ �� ���� ������          |
//-------------------------------------------------------------------------------
    TContentsVRec *r;
    String S;

    if (!CurCellMemo->Visible) return;
    if (Save) {
        S = CurCellMemo->Text;
        if (EditPageNum) {
            int val;
            try {
                S = S.Trim();
                val = S.ToInt();
            } catch(...) {
                Application->MessageBox("����� �������� ������ ���� ����� ������", "������!", 0);
                return;
            }
            contents_sp->p0 = val;
            contents->Enum(val);
            ContentsDrawGrid->Invalidate();
        } else {
            r = contents->Rec1(CurCellIndex);
            strcpy(r->text, S.c_str());
            contents->SplitRec(CurCellIndex);
            ResizeContentsGridHeights();
        }
    }

    CurCellMemo->Visible = false;
    ContentsDrawGrid->SetFocus();
}


void __fastcall TContentsForm::ContentsDrawGridKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // if (Key != VK_F2 && Key != VK_RETURN) return;
    if (Key != VK_F2) return;
    int col = ContentsDrawGrid->Col;
    int row = ContentsDrawGrid->Row;

    // if (col != 0 || row < 1 || row > contents->Nrecs) return;
    bool doit = row == 1 || (row > 1 && row <= contents->Nrecs && col == 0);
    if (!doit) return;

    TRect rect = ContentsDrawGrid->CellRect(col, row);
    TContentsVRec *r, rr;

    r = contents->Rec1(row - 1);
    memmove(&rr, r, sizeof(TContentsVRec));
    rr.UnSplit();

/*
    TFontStyles fs, fsk;
    fsk = CurCellMemo->Font->Style; fs = fsk;
    if (r->bold) {
        CurCellMemo->Font->Style = fs << fsBold;
    } else {
        CurCellMemo->Font->Style = fs >> fsBold;
    }
*/

    CurCellIndex = row - 1;
    if (col == 1) {
        EditPageNum = true;
        char buf[30];
        wsprintf(buf, "%d", r->page);
        CurCellMemo->Text = buf;
    } else {
        EditPageNum = false;
        CurCellMemo->Text = (char*)rr.text;
    }
    CurCellMemo->Width = rect.Width();
    CurCellMemo->Height = rect.Height();
    CurCellMemo->Left = rect.Left + 1;
    CurCellMemo->Top = rect.Top + 2;
    CurCellMemo->SelStart = 0;
    CurCellMemo->SelLength = 0;
    CurCellMemo->Visible = true;
    CurCellMemo->SetFocus();
}



static bool JustSelectedSameCell = false;

void __fastcall TContentsForm::ContentsDrawGridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    JustSelectedSameCell = (ACol == ContentsDrawGrid->Col) && (ARow == ContentsDrawGrid->Row); 
}


void __fastcall TContentsForm::ContentsDrawGridClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (JustSelectedSameCell) {
        WORD Key = VK_F2;
        ContentsDrawGridKeyDown(Sender, Key, TShiftState());
    }
}


void __fastcall TContentsForm::CurCellMemoKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (Key == VK_ESCAPE) {
        StopEditCurCell(false);
    } else if (Key == VK_RETURN) {
        StopEditCurCell(true);
    }
}

void __fastcall TContentsForm::CurCellMemoExit(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    StopEditCurCell(true);
}



void __fastcall TContentsForm::FormResize(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    int w1 = ContentsDrawGrid->ColWidths[1];
    int ww = ContentsDrawGrid->ClientWidth;
    int w0 = ww - w1 - 3;
    if (ContentsDrawGrid->ColWidths[0] != w0) {
        ContentsDrawGrid->ColWidths[0] = w0;
    }
}

void __fastcall TContentsForm::N3Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                      ���� ����->�����                                        |
//-------------------------------------------------------------------------------
    ModalResult = mrOk;
}

void __fastcall TContentsForm::doc1Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//                      ���� ����->����� ����������                             |
//-------------------------------------------------------------------------------
    TFileList *fl;
    TFileListRec *flr;
    TContentsVRec *cr, crr;
    int i;
    char buf[20];

    contents_sp->NeedFiln = true;
    contents_sp->NeedF1 = false;
    if (!AskSaveContents(contents_sp)) return;

    contents->Enum(contents_sp->p0);
    ContentsDrawGrid->Invalidate();
    Application->ProcessMessages();

    fl = new TFileList;
    for (i = 0; i < contents->Nrecs; i++) {
        contents->RecIndex1 = i;
        memmove(&crr, contents->Rec1(i), contents->RecLen1);
        crr.UnSplit();
        wsprintf(buf, "c:\\%05d", i);
        fl->Append(buf, buf, buf, (char*)crr.text);
        flr = fl->Rec0(fl->Nrecs - 1);
        flr->pi.pcount = crr.npages;
    }

    // Contents2Word("c:\\UnitedProjects\\��������� ������� ����\\exe\\ttt\\����������1.doc", fl, 0, contents->Rec1(0)->page);
    Contents2Word(contents_sp->Filn.c_str(), fl, 0, contents_sp->p0);

    delete fl;

}

void __fastcall TContentsForm::N4Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    FindReplaceDialog(&fr);
    if (fr.What == 0) return;
    contents->SetGray(fr.From);
    ContentsDrawGrid->Invalidate();
    Application->ProcessMessages();
    if (fr.What == 2) {
        Sleep(500);
        contents->DelGray();
        contents->ReSplitAll(GetContentsWidth());
        ResizeContentsGridHeights();
        ContentsDrawGrid->Invalidate();
    }
}




void __fastcall TContentsForm::N5Click(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    contents_sp->NeedFiln = false;
    contents_sp->NeedF1 = false;
    if (!AskSaveContents(contents_sp)) return;

    contents->Enum(contents_sp->p0);
    ContentsDrawGrid->Invalidate();
}

