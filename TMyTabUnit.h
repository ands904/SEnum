//---------------------------------------------------------------------------

#ifndef TMyTabUnitH
#define TMyTabUnitH
#include <ExtCtrls.hpp>
#include "Fog.h"
//---------------------------------------------------------------------------

enum TMyTabControlStatus {
    tcsScrollLeft,              // ������ ������ ScrollLeft
    tcsScrollRight,             // ������ ������ ScrolRight
    tcsMouseDowned,             // ������ ����� ������ ���� �� ������� ���������� FTabIndex
    tcsMouseDragged,            // �������� ������� � ��������
    tcsMouseDraggedOut,         // �������� ������� � �������� - �� ������ ���� ������ �� f
    tcsIdle                     // ������ �� ������
};


class TMyTabControl {
    TMyTabControlStatus status;
    TTimer *timer;

    TFog *f;

    int FTabIndex;          // ������ �������� ����������
    int FLeftTabIndex;      // ������ ������ ������ ����������

    int dxleft;             // �������� ������ ������� �����
    int dxright;            // ���������� ����� ������ ������ ������� � ������ �� �����
    int dytop;              // �������� ����� ����������� �������
    int dybottom;           // �������� ������ ������� �������

    TRect *TabRects;
    TRect LeftButtonRect, RightButtonRect;
    bool LeftButtonActive, RightButtonActive;
    bool LeftButtonPressed, RightButtonPressed;
    void __fastcall BuildTabRects(void);
    int __fastcall GetRectIndex(int x, int y);


    TColor FBackColor;
    TColor FShadowLightColor;
    TColor FShadowDarkColor;
    TColor FLightBorderColor;
    TColor FDarkBorderColor;
    TColor __fastcall GetBackColor(void) {return FBackColor;}
    TColor __fastcall GetShadowLightColor(void) {return FShadowLightColor;}
    TColor __fastcall GetShadowDarkColor(void) {return FShadowDarkColor;}
    TColor __fastcall GetLightBorderColor(void) {return FLightBorderColor;}
    TColor __fastcall GetDarkBorderColor(void) {return FDarkBorderColor;}


    int initial_interval;       // ��������� �������� ����� ������������ �������
    int routine_interval;       // ������� �������� ����������� �������
    void __fastcall ScrollLeft(void);
    void __fastcall ScrollRight(void);
    void __fastcall TimerTick(TObject *Sender);
    int MouseDownedX, MouseDownedY;     // ��������� ������� ������� ��������� �� ������� ����


    void __fastcall FogMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, const TPoint &MousePos, bool &Handled);
    void __fastcall FogDragDrop(TObject *Sender, TObject *Source, int X, int Y);
    void __fastcall FogDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);

    void __fastcall RoundTab(TRect *r, bool index);
    void __fastcall DrawScrollButton(TRect *r);
    void __fastcall SetCursor(TCursor cursor);
    void __fastcall FogPaint(TObject *Sender);

public:
    TMyTabControl(TFog *fog);
    ~TMyTabControl(void);

    void __fastcall Refresh(void);

    TStringList *Tabs;

    int __fastcall GetTabIndex(void) {return FTabIndex;}
    void __fastcall SetTabIndex(int index);

    int __fastcall GetLeftTabIndex(void) {return FLeftTabIndex;}
    void __fastcall SetLeftTabIndex(int index);
    void __fastcall DecLeftTabIndex(void);
    void __fastcall IncLeftTabIndex(void);
};


#endif
