//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TMyTabUnit.h"
#pragma package(smart_init)

extern int stop;

//---------------------------------------------------------------------------
static bool __fastcall _InsideRect(TRect *r, int x, int y) {
//-------------------------------------------------------------------------------
//                          Если x,y внутри прямоугольника r                    |
//-------------------------------------------------------------------------------
    return x >= r->left && x <= r->right && y >= r->top && y <= r->bottom;
}




TMyTabControl::TMyTabControl(TFog *fog) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    status = tcsIdle;
    scrolld = tsdNo;
    timer = new TTimer(NULL);
    timer->Enabled = false;
    timer->OnTimer = TimerTick;

    initial_interval = 700;
    routine_interval = 200;
    timer->Interval = initial_interval;

    f = fog;
    f->OnMouseDown = FogMouseDown;
    f->OnMouseMove = FogMouseMove;
    f->OnMouseUp = FogMouseUp;
    f->OnMouseWheel = FogMouseWheel;
    FOnDragDrop = f->OnDragDrop;
    f->OnDragDrop = FogDragDrop;
    FOnDragOver = f->OnDragOver;
    f->OnDragOver = FogDragOver;
    f->OnPaint = FogPaint;

    Tabs = new TStringList;
    // Tabs->Add("<пусто>");
    Tabs->Add("Вкладка 1");
    Tabs->Add("Вкладка 2");
    Tabs->Add("Вкладка 3");
    Tabs->Add("Вкладка 4");
    Tabs->Add("Вкладка 5");
    Tabs->Add("Вкладка 6");
    Tabs->Add("Вкладка 7");
    Tabs->Add("Вкладка 8");
    Tabs->Add("Вкладка 9");
    Tabs->Add("Вкладка 10");
    Tabs->Add("Вкладка 11");
    Tabs->Add("Вкладка 12");
    Tabs->Add("Вкладка 13");
    Tabs->Add("Вкладка 14");
    Tabs->Add("Вкладка 15");
    Tabs->Add("Вкладка 16");
    Tabs->Add("Вкладка 17");
    Tabs->Add("Вкладка 18");
    Tabs->Add("Вкладка 19");
    Tabs->Add("Вкладка 20");
    Tabs->Add("Вкладка 21");
    Tabs->Add("Вкладка 22");
    Tabs->Add("Вкладка 23");
    Tabs->Add("Вкладка 24");
    Tabs->Add("Вкладка 25");
    Tabs->Add("Вкладка 26");
    Tabs->Add("Вкладка 27");
    Tabs->Add("Вкладка 28");
    Tabs->Add("Вкладка 29");
    FTabIndex = 0;
    FLeftTabIndex = 0;
    dxleft = 7;             // смещение текста вкладки слева
    dxright = 5;            // расстояние между концом текста вкладки и правым ее краем
    dytop = 5;
    dybottom = 8;
    TabRects = NULL;

    FBackColor = clBtnFace;
    // FBackColor = clActiveCaption;
    FShadowLightColor = (TColor)0x808080;
    FShadowDarkColor = (TColor)0x404040;
    FLightBorderColor = (TColor)0xFFFFFF;
    FDarkBorderColor = (TColor)0;

    LeftButtonPressed = false;
    RightButtonPressed = false;

    Refresh();
}


TMyTabControl::~TMyTabControl(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    timer->Enabled = false;
    delete timer;
    delete Tabs;
    if (TabRects != NULL) delete[] TabRects;
    f->OnMouseDown = NULL;
    f->OnMouseMove = NULL;
    f->OnMouseUp = NULL;
    f->OnMouseWheel = NULL;
    f->OnDragDrop = NULL;
    f->OnDragOver = NULL;
    f->OnPaint = NULL;
}









TMyScrollDirection __fastcall TMyTabControl::FindScrollDirection(int X, int Y) {
//-------------------------------------------------------------------------------
//     Находит направление скроллинга в зависимости от координат и scrolld      |
// Если скроллировать уже нЕкуда, то тоже возвращает tsdNo                      |
// Не присваивает scrolld, только возвращает направление!                       |
//-------------------------------------------------------------------------------
    TRect *r;
    int index;

    if (Y < 0 || Y > f->Height) return tsdNo;
    if (X < 0 || X > f->Width) return tsdNo;

    index = GetRectIndex(X, Y, true);
    if (index < 0) return tsdNo;
    r = TabRects + index;
    switch(scrolld) {
        case tsdNo:
            if (index < 0) break;
            if (r->right > LeftButtonRect.left) return tsdRight;
            if (index == FLeftTabIndex && X < r->Width() / 4) return tsdLeft;
            break;
        case tsdLeft:
            if (index < 0) break;
            if (X >= r->Width() / 3) break;
            return tsdLeft;
        case tsdRight:
            if (r->right > LeftButtonRect.left) return tsdRight;
            break;
    }
    return tsdNo;
}


void __fastcall TMyTabControl::ScrollLeft(void) {
//-------------------------------------------------------------------------------
//                Если состояние tcsScrollLeft и LeftButtonActive               |
// Кнопку при этом не нажимает!                                                 |
//-------------------------------------------------------------------------------
    if (status != tcsScrollLeft || !LeftButtonActive) return;
    DecLeftTabIndex();
}

void __fastcall TMyTabControl::ScrollRight(void) {
//-------------------------------------------------------------------------------
//                Если состояние tcsScrollRight и RightButtonActive             |
// Кнопку при этом не нажимает!                                                 |
//-------------------------------------------------------------------------------
    if (status != tcsScrollRight || !RightButtonActive) return;
    IncLeftTabIndex();
}


void __fastcall TMyTabControl::ScrollDragLeft(void) {
//-------------------------------------------------------------------------------
//              Скроллирует влево для Drag-and-Drop - по таймеру                |
//-------------------------------------------------------------------------------
    DecLeftTabIndex();
}


void __fastcall TMyTabControl::ScrollDragRight(void) {
//-------------------------------------------------------------------------------
//              Скроллирует вправо для Drag-and-Drop - по таймеру               |
//-------------------------------------------------------------------------------
    int x = LeftButtonRect.left + 2;
    int y = f->Height / 2;
    int index = GetRectIndex(x, y, true);
    if (index > 0) IncLeftTabIndex();
}


void __fastcall TMyTabControl::DragRight(int mode) {
//-------------------------------------------------------------------------------
//                      Тянет закладку вправо, если можно                       |
// mode - условие тянутия:                                                      |
//    1 - курсор мыши правее на 10 пикселов от текущей закладки (просто тянем)  |
//    2 - курсор мыши в левой четверти текущей (скроллирование)                 |
//-------------------------------------------------------------------------------
    POINT PT;
    TRect *r;
    TPoint pt;
    int X;
    if (FTabIndex >= Tabs->Count - 1) return;      // некуда дальше скроллировать
    ::GetCursorPos(&PT);
    pt.x = PT.x; pt.y = PT.y;
    pt = f->ScreenToClient(pt);
    X = pt.x;
    r = TabRects + FTabIndex;
    if (mode == 1 && X >= r->right + 10 || mode == 2 && X >= LeftButtonRect.left - 10) {
    // if (mode == 1 && X >= r->right + 10) {
        //if (FTabIndex == FLeftTabIndex) FLeftTabIndex--;
        //Tabs->Exchange(FTabIndex, FTabIndex - 1);
        //SetTabIndex(FTabIndex - 1);
        //r = TabRects + FTabIndex;
        //MouseDownedX = (r->left + r->right) / 2;
        Tabs->Exchange(FTabIndex, FTabIndex + 1);
        SetTabIndex(FTabIndex + 1);
        r = TabRects + FTabIndex;
        MouseDownedX = (r->left + r->right) / 2;
    }
}


void __fastcall TMyTabControl::DragLeft(int mode) {
//-------------------------------------------------------------------------------
//                      Тянет закладку влево, если можно                        |
// mode - условие тянутия:                                                      |
//    1 - курсор мыши левее на 10 пикселов от текущей закладки (просто тянем)   |
//    2 - курсор мыши в левой четверти текущей (скроллирование)                 |
//-------------------------------------------------------------------------------
    POINT PT;
    TRect *r;
    TPoint pt;
    int X;
    if (FTabIndex == 0) return;      // некуда дальше скроллировать
    ::GetCursorPos(&PT);
    pt.x = PT.x; pt.y = PT.y;
    pt = f->ScreenToClient(pt);
    X = pt.x;
    r = TabRects + FTabIndex;
    // if (mode == 1 && X <= r->left - 10 || mode == 2 && X < r->Width() / 4 && FLeftTabIndex > 0) {
    if (mode == 1 && X <= r->left - 10 || mode == 2 && X < r->Width() / 4) {
        // if (FTabIndex == FLeftTabIndex) FLeftTabIndex--;
        Tabs->Exchange(FTabIndex, FTabIndex - 1);
        SetTabIndex(FTabIndex - 1);
        r = TabRects + FTabIndex;
        MouseDownedX = (r->left + r->right) / 2;
    }
}


void __fastcall TMyTabControl::StartTimer(void) {
//-------------------------------------------------------------------------------
//                 Запускает таймер с интервалом initial_interval               |
//-------------------------------------------------------------------------------
    timer->Interval = initial_interval;
    timer->Enabled = true;
}

void __fastcall TMyTabControl::StopTimer(void) {
//-------------------------------------------------------------------------------
//                                Останавливает таймер                          |
//-------------------------------------------------------------------------------
    timer->Enabled = false;
}


void __fastcall TMyTabControl::TimerTick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    stop++;
    timer->Interval = routine_interval;
    switch(status) {
        case tcsIdle:
            timer->Enabled = false;
            break;
        case tcsScrollLeft:
            ScrollLeft();
            break;
        case tcsScrollRight:
            ScrollRight();
            break;
        case tcsMouseDraggedLeft:
            DragLeft(2);                // драг со скроллингом
            break;
        case tcsMouseDraggedRight:
            DragRight(2);               // драг со скроллингом
            break;
        case tcsObjectDraggedLeft:      // драгаем объект над табулятором
            ScrollDragLeft();
            break;
        case tcsObjectDraggedRight:     // драгаем объект над табулятором
            ScrollDragRight();
            break;
    }
}



void __fastcall TMyTabControl::FogMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
//-------------------------------------------------------------------------------
// ??? Пока временно обрабатывает только левую кнопку по вкладкам и кнопкам     |
//-------------------------------------------------------------------------------
    if (Button != mbLeft) return;
    int index = GetRectIndex(X, Y);
    if (index >= 0) {
        SetTabIndex(index);
        status = tcsMouseDowned;
        MouseDownedX = X;
        MouseDownedY = Y;
    } else if (index == -1 && LeftButtonActive) {
        LeftButtonPressed = true;
        DecLeftTabIndex();
        status = tcsScrollLeft;
        StartTimer();
    } else if (index == -2 && RightButtonActive) {
        RightButtonPressed = true;
        IncLeftTabIndex();
        status = tcsScrollRight;
        StartTimer();
        // timer->Interval = initial_interval;
        // timer->Enabled = true;
    }
}

// snprintf

void __fastcall TMyTabControl::FogMouseMove(TObject *Sender, TShiftState Shift, int X, int Y) {
//-------------------------------------------------------------------------------
// ??? Пока временно только таскает вкладку                                     |
//-------------------------------------------------------------------------------
    TRect *r;
    if (status == tcsMouseDowned) {
        int dx = MouseDownedX - X;
        if (dx < 0) dx = -dx;
        if (dx > 5) {
            r = TabRects + FTabIndex;
            MouseDownedX = (r->left + r->right) / 2;
            status = tcsMouseDragged;
            SetCursor(crDrag);
        }

    } else if (status == tcsMouseDragged) {
        if (X < f->Left || X > f->Left + f->Width || Y < f->Top || Y > f->Top + f->Height) {
            status = tcsMouseDraggedOut;
            SetCursor(crNoDrop);
            return;
        }

        r = TabRects + FTabIndex;
        if (X < MouseDownedX) {                 // курсор мыши левее середины текущей вкладки
            if (FTabIndex == 0) return;         // некуда дальше скроллировать
            if (FTabIndex == FLeftTabIndex) {   // а не надо ли скроллировать влево?
                // if (X < r->Width() / 4 && FLeftTabIndex > 0) {       // точно надо?
                if (X < r->Width() / 4) {       // точно надо?
                    //FLeftTabIndex--;
                    //Tabs->Exchange(FTabIndex, FTabIndex - 1);
                    //SetTabIndex(FTabIndex - 1);
                    //r = TabRects + FTabIndex;
                    //MouseDownedX = (r->left + r->right) / 2;
                    DragLeft(2);
                    status = tcsMouseDraggedLeft;
                    StartTimer();
                }
            } else {                            // пока не надо, проверим, не перетащить вкладку влево
                if (X <= r->left - 10) {        // перетащить
                    //Tabs->Exchange(FTabIndex, FTabIndex - 1);
                    //SetTabIndex(FTabIndex - 1);
                    //r = TabRects + FTabIndex;
                    //MouseDownedX = (r->left + r->right) / 2;
                    DragLeft(1);
                }
            }
        } else {                                // курсор мыши правее середины текущей вкладки
            if (FTabIndex >= Tabs->Count - 1) return;      // некуда дальше скроллировать
            if (r->right >= LeftButtonRect.left - 10) {    // а не проверить ли скроллируемость вправо?
                DragRight(2);
                status = tcsMouseDraggedRight;
                StartTimer();
            } else {                                       // скроллировать не надо, проверим на перетащить
                if (X >= r->right + 10) {
                    int k = FLeftTabIndex;
                    DragRight(1);
                    if (FLeftTabIndex != k) {
                        status = tcsMouseDraggedRight;
                        StartTimer();
                    }
                }
            }
        }

    } else if (status == tcsMouseDraggedLeft) {
        r = TabRects + FTabIndex;
        if (X >= r->Width() / 3) {
            status = tcsMouseDragged;
        }

    } else if (status == tcsMouseDraggedRight) {
        r = TabRects + FTabIndex;
        if (X <= r->right - r->Width() / 4) {
            status = tcsMouseDragged;
        }

    } else if (status == tcsMouseDraggedOut) {
        if (X >= f->Left && X <= f->Left + f->Width && Y >= f->Top && Y <= f->Top + f->Height) {
            status = tcsMouseDragged;
            SetCursor(crDrag);
        }
    }
}

void __fastcall TMyTabControl::FogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y) {
//-------------------------------------------------------------------------------
// ??? Пока временно обрабатывает только отпускание левой кнопки над кнопками   |
//-------------------------------------------------------------------------------
    bool NeedRefresh = false;
    timer->Enabled = false;
    status = tcsIdle;
    SetCursor(crArrow);
    if (LeftButtonPressed) {
        LeftButtonPressed = false;
        NeedRefresh = true;
    } else if (RightButtonPressed) {
        RightButtonPressed = false;
        NeedRefresh = true;
    }
    if (NeedRefresh) Refresh();
}

void __fastcall TMyTabControl::FogMouseWheel(TObject *Sender,
      TShiftState Shift, int WheelDelta, const TPoint &MousePos,
      bool &Handled)
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
{

}

void __fastcall TMyTabControl::FogDragDrop(TObject *Sender, TObject *Source, int X, int Y) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    StopTimer();
    if (!FOnDragDrop) return;
    DropIndex = GetRectIndex(X, Y, true);
    FOnDragDrop(Sender, Source, X, Y);
}

void __fastcall TMyTabControl::FogDragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TRect *r;
    TMyScrollDirection sd;

    if (FOnDragOver == NULL) {
        Accept = false;
    } else {
        FOnDragOver(Sender, Source, X, Y, State, Accept);
    }
    if (!Accept) {
        StopTimer();
        status = tcsIdle;
        scrolld = tsdNo;
        return;
    }

    if (status == tcsIdle) {
        scrolld = tsdNo;
        sd = FindScrollDirection(X, Y);
        if (sd == tsdLeft) {
            scrolld = sd;
            status = tcsObjectDraggedLeft;
            ScrollDragLeft();
            StartTimer();
        } else if (sd == tsdRight) {
            scrolld = sd;
            status = tcsObjectDraggedRight;
            ScrollDragRight();
            StartTimer();
        }
    } else if (status == tcsObjectDraggedLeft) {        // скроллировали влево - проверим, надо ли дальше
        sd = FindScrollDirection(X, Y);
        if (sd != tsdLeft) {
            StopTimer();
            status = tcsIdle;
            scrolld = tsdNo;
        }
    } else if (status == tcsObjectDraggedRight) {       // скроллировали вправо - проверим, надо ли дальше
        sd = FindScrollDirection(X, Y);
        if (sd != tsdRight) {
            StopTimer();
            status = tcsIdle;
            scrolld = tsdNo;
        }
    }

}


void __fastcall TMyTabControl::SetCursor(TCursor cursor) {
//-------------------------------------------------------------------------------
//                          Устанавливает курсор мыши f                         |
//-------------------------------------------------------------------------------
    f->Cursor = cursor;
    ::SetCursor(Screen->Cursors[cursor]);
    Application->ProcessMessages();
}


void __fastcall TMyTabControl::BuildTabRects(void) {
//-------------------------------------------------------------------------------
//                              Строит прямоугольники вкладок                   |
// Вкладки левее LeftTabIndex будут иметь отрицательные координаты              |
//-------------------------------------------------------------------------------
    TCanvas *c = f->Canvas;
    TRect *r;
    int i, x, dx, offs;

    if (TabRects != NULL) delete[] TabRects;
    TabRects = new TRect[Tabs->Count];

    for (r = TabRects, i = 0, x = 4, offs = 0; i < Tabs->Count; i++, r++) {
        if (i == GetLeftTabIndex()) offs = x;
        r->left = x; r->top = 0;
        dx = c->TextWidth(Tabs->Strings[i]) + dxleft + dxright;
        r->right = x + dx;
        x += dx;
        r->bottom = f->ClientHeight;
    }

    if (offs != 0) {
        offs -= 4;
        for (r = TabRects, i = 0; i < Tabs->Count; i++, r++) {
            r->left -= offs;
            r->right -= offs;
            stop++;
        }
    }

    int h = 16;
    int top = f->ClientHeight / 2 - h / 2;
    LeftButtonRect.left = f->ClientWidth - h * 2 - 5;
    LeftButtonRect.top = top;
    LeftButtonRect.right = LeftButtonRect.left + h;
    LeftButtonRect.bottom = top + h;
    RightButtonRect.left = LeftButtonRect.right + 1;
    RightButtonRect.top = top;
    RightButtonRect.right = RightButtonRect.left + h;
    RightButtonRect.bottom = top + h;

    LeftButtonActive = FLeftTabIndex != 0;
    r = TabRects + Tabs->Count - 1;
    RightButtonActive = r->right >= LeftButtonRect.left - 10;

}


int __fastcall TMyTabControl::GetRectIndex(int x, int y, bool tabsonly) {
//-------------------------------------------------------------------------------
//            Вычисляет индекс вкладки, в которую попали координаты x,y         |
// tabsonly - если искать только среди табуляторов, без кнопок                  |
// Если положительное число, то собственно индекс вкладки                       |
// -1 - стрелка влево                                                           |
// -2 - стрелка вправо                                                          |
// < -10 - никуда не попали                                                     |
//-------------------------------------------------------------------------------
    TRect *r;
    int i;

    if (x < 0 || x > f->ClientWidth || y < 0 || y > f->ClientHeight) return -10;

    BuildTabRects();

    if (!tabsonly) {
        if (_InsideRect(&LeftButtonRect, x, y)) return -1;
        if (_InsideRect(&RightButtonRect, x, y)) return -2;
    }

    for (i = 0, r = TabRects; i < Tabs->Count; i++, r++) {
        if (_InsideRect(r, x, y)) return i;
        // if (x >= r->left && x <= r->right && y >= r->top && y <= r->bottom) return i;
    }

    return -10;
}


void __fastcall TMyTabControl::RoundTab(TRect *r, bool index) {
//-------------------------------------------------------------------------------
//              Окружает табулятор. Если index, то это - текущий                |
//-------------------------------------------------------------------------------
    TCanvas *c = f->Canvas;
    int left, right, top, bottom;

    left = r->left; right = r->right;
    top = r->top + dytop;
    bottom = r->bottom - dybottom + 1;
    if (index) {
        top -= 2;
        left -= 2;
        right += 2;
    }

    c->Pen->Style = psSolid;
    c->Pen->Color = GetLightBorderColor();
    c->MoveTo(left, bottom);
    c->LineTo(left, top + 2);                   // левый край
    c->LineTo(left+2, top);                     // наклон
    c->LineTo(right-2, top);                    // верх
    c->Pen->Color = GetShadowLightColor();
    c->MoveTo(right-2, top+1);
    c->LineTo(right-2, bottom+1);
    c->Pen->Color = GetShadowDarkColor();
    c->MoveTo(right-1, top+2);
    c->LineTo(right-1, bottom+1);

    if (index) {
        int xa, xb;
        bottom++;
        xa = 1;
        xb = left;
        if (xb >= f->ClientWidth) xb = f->ClientWidth - 1;
        if (xa < xb) {
            c->Pen->Color = GetLightBorderColor();
            xb++;
            c->MoveTo(xa, bottom);
            c->LineTo(xb, bottom);
            c->Pen->Color = GetBackColor();
            c->LineTo(xb, top + 2);
        }
        xa = right; xb = f->ClientWidth;
        if (xa < xb) {
            c->Pen->Color = GetLightBorderColor();
            c->MoveTo(xa, bottom);
            c->LineTo(xb, bottom);
        }
    }
}


void __fastcall TMyTabControl::DrawScrollButton(TRect *r) {
//-------------------------------------------------------------------------------
//       Рисует кнопку для скроллирования в зависимости от pressed и Active     |
//-------------------------------------------------------------------------------
    TCanvas *c = f->Canvas;
    TColor c0, c1, c2, c3;
    int x, y, dy, dx;
    bool active, pressed;

    c->Brush->Color = GetBackColor();
    c->Brush->Style = bsSolid;
    c->Pen->Color = GetBackColor();
    c->Pen->Style = psSolid;
    c->Rectangle(*r);

    c->Pen->Style = psSolid;

    if (r == &LeftButtonRect) {
        dx = -1;
        active = LeftButtonActive;
        pressed = LeftButtonPressed;
    } else {
        dx = 1;
        active = RightButtonActive;
        pressed = RightButtonPressed;
    }
    x = (r->left + r->right) / 2 - dx;
    y = (r->top + r->bottom) / 2;

    if (pressed) {
        c0 = GetShadowLightColor();
        c1 = GetLightBorderColor();
        c2 = GetShadowDarkColor();
        c3 = GetBackColor();
        x++;
        y++;
    } else {
        c0 = GetBackColor();
        c1 = GetShadowDarkColor();
        c2 = GetLightBorderColor();
        c3 = GetShadowLightColor();
    }

    c->Pen->Color = c0; // GetBackColor();
    c->MoveTo(r->left, r->bottom-1);
    c->LineTo(r->left, r->top);
    c->LineTo(r->right, r->top);

    c->Pen->Color = c1; // GetShadowDarkColor();
    c->LineTo(r->right, r->bottom);
    c->LineTo(r->left-1, r->bottom);

    c->Pen->Color = c2; // GetLightBorderColor();
    c->MoveTo(r->left+1, r->bottom-2);
    c->LineTo(r->left+1, r->top+1);
    c->LineTo(r->right-1, r->top+1);

    c->Pen->Color = c3; // GetShadowLightColor();
    c->LineTo(r->right-1, r->bottom-1);
    c->LineTo(r->left, r->bottom-1);

    c->Pen->Color = active ? GetDarkBorderColor() : GetShadowLightColor();
    for (dy = 3; dy >= 0; dy--, x += dx) {
        c->MoveTo(x, y - dy);
        c->LineTo(x, y + dy + 1);
    }

}


void __fastcall TMyTabControl::Refresh(void) {
//-------------------------------------------------------------------------------
//                          Дает команду перерисовать себя                      |
//-------------------------------------------------------------------------------
    ::InvalidateRect(f->Handle, NULL, FALSE);
    ::PostMessage(f->Handle, WM_PAINT, 0, 0);
}

void __fastcall TMyTabControl::FogPaint(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TCanvas *c = f->Canvas;
    TRect *r;
    int i, index = GetTabIndex(), y, dy, w, h;

    w = f->ClientWidth; h = f->ClientHeight;

    c->Brush->Color = GetBackColor();
    c->Brush->Style = bsSolid;
    c->Pen->Color = GetBackColor();
    c->Pen->Style = psSolid;
    c->Rectangle(0, 0, w, h);

    BuildTabRects();

    i = c->Font->Height; if (i < 0) i = -i;
    y = h / 2 - i / 2;

    for (i = GetLeftTabIndex(), r = TabRects + i; i < Tabs->Count; i++, r++) {
        if (r->left >= f->ClientWidth) break;
        dy = i == index ? -2 : 0;
        c->TextOut(r->left + dxleft, y + dy, Tabs->Strings[i]);
    }

    for (i = GetLeftTabIndex(), r = TabRects + i; i < Tabs->Count; i++, r++) {
        if (r->left >= f->ClientWidth) break;
        if (i == index) continue;
        RoundTab(r, false);
    }

    r = TabRects + index;
    // if (index >= GetLeftTabIndex() && r->left < f->ClientWidth) {
        RoundTab(r, true);
    // }

    DrawScrollButton(&LeftButtonRect);
    DrawScrollButton(&RightButtonRect);

    c->Pen->Color = GetLightBorderColor();
    c->MoveTo(0, h);
    c->LineTo(0, 0);
    c->LineTo(w, 0);
    c->Pen->Color = GetShadowLightColor();
    c->MoveTo(1, h-2);
    c->LineTo(w-2, h-2);
    c->LineTo(w-2, 0);
    c->Pen->Color = GetShadowDarkColor();
    c->MoveTo(0, h-1);
    c->LineTo(w-1, h-1);
    c->LineTo(w-1, -1);


}


void __fastcall TMyTabControl::SetTabIndex(int index) {
//-------------------------------------------------------------------------------
//                      Устанавливает индекс текущей вкладки                    |
// Если она не видна, то манипулирует с LeftTabIndex                            |
// ??? Пока не учитывает стрелочек сдвига списка табуляторов                    |
//-------------------------------------------------------------------------------
    TRect *r;

    if (index < 0 || index == FTabIndex || index >= Tabs->Count) return;

    while(1) {
        BuildTabRects();
        r = TabRects + index;
        if (FLeftTabIndex >= index || r->right < LeftButtonRect.left - 3) break;
        FLeftTabIndex++;
    }

    if (index < FLeftTabIndex) FLeftTabIndex = index;

    FTabIndex = index;
    Refresh();
}


void __fastcall TMyTabControl::SetLeftTabIndex(int index) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
}

void __fastcall TMyTabControl::DecLeftTabIndex(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (FLeftTabIndex > 0) FLeftTabIndex--;
    Refresh();
}

void __fastcall TMyTabControl::IncLeftTabIndex(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (FLeftTabIndex < Tabs->Count - 1) FLeftTabIndex++;
    Refresh();
}





