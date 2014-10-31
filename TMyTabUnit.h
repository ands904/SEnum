//---------------------------------------------------------------------------

#ifndef TMyTabUnitH
#define TMyTabUnitH
#include <ExtCtrls.hpp>
#include "Fog.h"
#include "MyDragDrop.h"
//---------------------------------------------------------------------------

class TMyTabControl;
typedef void __fastcall (__closure *TMyTabDropEvent)(TMyTabControl *Sender, TMyDropData *DropData);
typedef void __fastcall (__closure *TMyTabChangeEvent)(TMyTabControl *Sender);

enum TMyScrollDirection {
    tsdNo,                      // �� � ��������� ����������
    tsdLeft,                    // � ��������� ���������� �����
    tsdRight                    // � ��������� ���������� ������
};

enum TMyTabControlStatus {
    tcsScrollLeft,              // ������ ������ ScrollLeft
    tcsScrollRight,             // ������ ������ ScrolRight
    tcsMouseDowned,             // ������ ����� ������ ���� �� ������� ���������� FTabIndex
    tcsMouseDragged,            // �������� ������� � ��������
    tcsMouseDraggedLeft,        // ��������� ��������� �����
    tcsMouseDraggedRight,       // ��������� ��������� ������
    tcsMouseDraggedOut,         // �������� ������� � �������� - �� ������ ���� ������ �� f
    tcsObjectDraggedLeft,       // ������ ������� � ������������ ������������� �����
    tcsObjectDraggedRight,      // ������ ������� � ������������ ������������� ������
    tcsIdle                     // ������ �� ������
};


class TMyTabControl {
    TMyTabControlStatus status;
    TMyScrollDirection scrolld;     // ������� ��������� ����������
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
    TMyScrollDirection __fastcall FindScrollDirection(int X, int Y);
    bool __fastcall MouseInsideFog(void);
    void __fastcall ScrollLeft(void);
    void __fastcall ScrollRight(void);
    void __fastcall ScrollDragLeft(void);
    void __fastcall ScrollDragRight(void);
    void __fastcall DragLeft(int mode);
    void __fastcall DragRight(int mode);
    void __fastcall StartTimer(void);
    void __fastcall StopTimer(void);
    void __fastcall TimerTick(TObject *Sender);
    int MouseDownedX, MouseDownedY;     // ��������� ������� ������� ��������� �� ������� ����

    void __fastcall FogMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FogMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, const TPoint &MousePos, bool &Handled);

    void __fastcall RoundTab(TRect *r, bool index);
    void __fastcall DrawScrollButton(TRect *r);
    void __fastcall SetCursor(TCursor cursor);
    void __fastcall FogPaint(TObject *Sender);

public:
    TMyTabControl(TFog *fog);
    ~TMyTabControl(void);

    void __fastcall Refresh(void);

    TStringList *Tabs;

    // int __fastcall GetRectIndex(int x, int y, bool tabsonly = false);
    int __fastcall GetRectIndex(int x, int y, bool tabsonly = true);
    int __fastcall GetTabIndex(void) {return FTabIndex;}
    void __fastcall SetTabIndex(int index);

    int __fastcall GetLeftTabIndex(void) {return FLeftTabIndex;}
    void __fastcall SetLeftTabIndex(int index);
    void __fastcall DecLeftTabIndex(void);
    void __fastcall IncLeftTabIndex(void);

protected:
    TDragOverEvent FOnDragOver;
    void __fastcall FogDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept);
public:
    __property TDragOverEvent OnDragOver = {read=FOnDragOver, write=FOnDragOver};

protected:
    TDragDropEvent FOnDragDrop;
    void __fastcall FogDragDrop(TObject *Sender, TObject *Source, int X, int Y);
public:
    __property TDragDropEvent OnDragDrop = {read=FOnDragDrop, write=FOnDragDrop};
    int DropIndex;              // ������ ����������, ��� ������� �������


protected:
    TMyDropTarget *droptarget;
    TMyTabDropEvent FOnDragDropX;               // X - external
    bool FAcceptText;
    bool FAcceptHtml;
    bool FAcceptFiles;
    void __fastcall OnDragDropX(TMyDropData *DropData);
    void __fastcall OnDragOverX(int x, int y);
public:
    bool __fastcall GetAcceptText(void) {return FAcceptText;}
    void __fastcall SetAcceptText(bool v) {FAcceptText = v; if (droptarget) droptarget->SetAcceptText(v);}
    bool __fastcall GetAcceptHtml(void) {return FAcceptHtml;}
    void __fastcall SetAcceptHtml(bool v) {FAcceptHtml = v; if (droptarget) droptarget->SetAcceptHtml(v);}
    bool __fastcall GetAcceptFiles(void) {return FAcceptFiles;}
    void __fastcall SetAcceptFiles(bool v) {FAcceptFiles = v; if (droptarget) droptarget->SetAcceptFiles(v);}
    TMyTabDropEvent __fastcall GetOnDragDropX(void); // {return OnDragDropX;}
    void __fastcall SetOnDragDropX(TMyTabDropEvent Event); // {OnDragDropX = Event;}



protected:
    TMyTabChangeEvent FOnChange;
public:
    TMyTabChangeEvent __fastcall GetOnChange(void) {return FOnChange;}
    void __fastcall SetOnChange(TMyTabChangeEvent Event) {FOnChange = Event;}


};


#endif

