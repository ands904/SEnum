//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "EditTiffUnit.h"
#include "SEnumLib.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Fog"
#pragma resource "*.dfm"
TEditTiffForm *EditTiffForm;
//---------------------------------------------------------------------------

// static TTiffImage *ti = NULL;
static TTiffBook *tb = NULL;
static TTiffImageConfig tic;


static String __fastcall CreateTempFiln(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    String filn;
    int filn_n;
    char path[300], buf[50];

    filn_n = 0;
    // ::GetTempPath(300, path);
    aGetTempPath(300, path);
    while(1) {
        wsprintf(buf, "SEnumPrintTmpTiff%04d.tif", filn_n);
        filn = (String)path + "\\" + buf;
        DWORD A = GetFileAttributes(filn.c_str());
        if (A != 0xFFFFFFFF) {
            DeleteFile(filn.c_str());
            A = GetFileAttributes(filn.c_str());
        }
        if (A == 0xFFFFFFFF) break;
        filn_n++;
    }

    return filn;
}


void __fastcall EditTiffFile(String SavedFiln) {
//-------------------------------------------------------------------------------
//              Запрашивает файл у юзера и редактирует его                      |
//-------------------------------------------------------------------------------
    if (SavedFiln.Length() != 0) {
        EditTiffForm->OpenTiffDialog->FileName = SavedFiln;
    }

    if (!EditTiffForm->OpenTiffDialog->Execute()) return;

    String Filn = EditTiffForm->OpenTiffDialog->FileName;
    EditTiffForm->Caption = "Редактирование файла " + Filn;


    StartGdiplus();

    memset(&tic, 0, sizeof(TTiffImageConfig));

    // ti = new TTiffImage();
    // ti->LoadFromFile("c:\\UnitedProjects\\Нумерация страниц смет\\exe\\ttt\\TestTiffUnit.tif");
    // EditTiffForm->SetupPageComboBox(ti->GetNPages(), 0);

    tb = new TTiffBook();
    // tb->Append("c:\\UnitedProjects\\Нумерация страниц смет\\exe\\ttt\\TestTiffUnit.tif");
    String NewFiln = CreateTempFiln();
    CopyFile(Filn.c_str(), NewFiln.c_str(), FALSE);
    //tb->Append(Filn);
    tb->Append(NewFiln);
    tb->Filn = Filn;
    EditTiffForm->SetupPageComboBox(tb->GetNPages(), 0);
    tb->changed = false;

    EditTiffForm->ShowModal();

    delete tb; tb = NULL;

    StopGdiplus();
}










//===============================================================================
//                      Сохранение битмапа в тифовском файле                    |
// Полагает, что система Gdiplus уже запущена                                   |
//===============================================================================
static int SaveBitmap2TiffState = 0;    // 0 - не начали, 1 - первая страница, 2 - следующие страницы
static const GUID aEncoderSaveFlag = {0x292266fc,0xac40,0x47bf,{0x8c, 0xfc, 0xa8, 0x5b, 0x89, 0xa6, 0x55, 0xde}};
static Gdiplus::Bitmap *SaveBitmap2TiffBitmap1 = NULL;  // битмап, который непосредственно выводится (черно-белый)
static Gdiplus::Bitmap *SaveBitmap2TiffBitmap32 = NULL; // битмап, который используется для построений
static WCHAR *SaveBitmap2TiffFiln = NULL;               // имя файла, куда выводится многостраничный битмап
static Gdiplus::Graphics *SaveBitmap2TiffGraphics = NULL;// сюда все рисуем
static int SaveBitmapW, SaveBitmapH;
static Gdiplus::Status sres;

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}


static bool __fastcall IniSaveBitmap2Tiff(char *filn) {
//-------------------------------------------------------------------------------
//            Инициализирует процедуру вывода битмапов в файл *.tif             |
// Если что-нибудь не получилось - выдает сообщение и возвращает false          |
//-------------------------------------------------------------------------------
    HANDLE chan;
    String mes;
    int len;

    if (SaveBitmap2TiffState != 0) {
        Application->MessageBox("Ошибка программиста! Повторная инициализация системы вывода файла *.tif!", "Срочно обратитесь в ОПО!", 0);
        return false;
    }

    DeleteFile(filn);
    chan = CreateFile(filn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (chan != INVALID_HANDLE_VALUE) {
        mes = (String)"Не могу создать файл \"" + filn + "\"";
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return false;
    }

    SaveBitmap2TiffFiln = new WCHAR[strlen(filn) + 1];
    AnsiToUnicode(filn, -1, SaveBitmap2TiffFiln);

    SaveBitmapW = 3008; SaveBitmapH = 4254;
    // iw = 2114; ih = 2990;

    SaveBitmap2TiffBitmap32 = new Gdiplus::Bitmap(SaveBitmapW, SaveBitmapH, PixelFormat32bppARGB);
    SaveBitmap2TiffGraphics = new Gdiplus::Graphics(SaveBitmap2TiffBitmap32);
    SaveBitmap2TiffBitmap1 = new Gdiplus::Bitmap(SaveBitmapW, SaveBitmapH, PixelFormat1bppIndexed);

    SaveBitmap2TiffState = 1;

    return true;
}


static void __fastcall SaveBitmap2Tiff(Gdiplus::Bitmap *bm, int npage) {
//-------------------------------------------------------------------------------
//                  Выводит очередную страницу в файл *.tif                     |
// Если npage == 0, то не выводит номер страницы                                |
// Полагает, что если это многостраничный файл, то страница уже выбрана         |
//-------------------------------------------------------------------------------
    if (SaveBitmap2TiffState == 0) return;


    // ------------------ для начала строим образ в SaveBitmap2TiffBitmap32 (SaveBitmap2TiffGraphics)
    int x, y, w, h;

    h = bm->GetHeight();
    w = bm->GetWidth();

    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    SaveBitmap2TiffGraphics->FillRectangle(&whiteBrush, Gdiplus::Rect(0, 0, SaveBitmapW, SaveBitmapH));

    //Gdiplus::Matrix matr;
    Gdiplus::Rect r;
    //if (w > h) {                            // Если изображение шире, чем выше
    //    matr.Rotate(270.0f);                // поворачиваем его на 270 градусов
    //    SaveBitmap2TiffGraphics->SetTransform(&matr);
    //    r.X = -SaveBitmapH;
    //    r.Y = -30;
    //    r.Width = SaveBitmapH - 30;
    //    r.Height = SaveBitmapW - 30;
    //} else {
    //    r.X = 30;
    //    r.Y = 0;
    //    r.Width = SaveBitmapW - 30;
    //    r.Height = SaveBitmapH - 30;
    //}
    r.X = 0;
    r.Y = 0;
    r.Width = SaveBitmapW;
    r.Height = SaveBitmapH;

    SaveBitmap2TiffGraphics->DrawImage(bm, r, 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    // matr.Reset();
    // SaveBitmap2TiffGraphics->SetTransform(&matr);

    if (npage != 0) {            // Здесь выводим номер страницы
        AnsiString snum;
        wchar_t wnum[20];
        Gdiplus::Pen whitePen40(Gdiplus::Color(255, 255, 255, 255), 45);
        snum.printf("%d", npage);
        snum.WideChar(wnum, 20);
        Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
        Gdiplus::PointF origin(SaveBitmapW - 160, 20.0f);
        Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
        SaveBitmap2TiffGraphics->DrawLine(&whitePen40, SaveBitmapW - 200, 50, SaveBitmapW, 50);
        if (npage > 0) {
            SaveBitmap2TiffGraphics->DrawString(wnum, -1, &myFont, origin, &blackBrush);
        }
    }


    // ------- строим SaveBitmap2TiffBitmap1
    UINT *pix;
    unsigned char byte, bit, *pix1;

    w = SaveBitmapW; h = SaveBitmapH;
    Gdiplus::Rect BitsRect(0, 0, SaveBitmapW, SaveBitmapH);
    Gdiplus::BitmapData *bits = new Gdiplus::BitmapData;
    Gdiplus::BitmapData *bits1 = new Gdiplus::BitmapData;
    sres = SaveBitmap2TiffBitmap32->LockBits(&BitsRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bits);
    sres = SaveBitmap2TiffBitmap1->LockBits(&BitsRect, Gdiplus::ImageLockModeWrite, PixelFormat1bppIndexed, bits1);

    for (y = 0; y < SaveBitmapH; y++) {
        pix = (UINT *)((int)bits->Scan0 + bits->Stride * y);
        pix1 = (unsigned char *)((int)bits1->Scan0 + bits1->Stride * y);
        byte = 0;
        for (x = 0; x < SaveBitmapW; x++, pix++) {
            if ((*pix & 0xFF) > 0xD8) {
                bit = 1;
            }
            else bit = 0;
            byte <<= 1; byte |= bit;
            if ((x & 7) == 7) {
                *pix1++ = byte;
                byte = 0;
            }
        }
    }

    SaveBitmap2TiffBitmap32->UnlockBits(bits); delete bits;
    SaveBitmap2TiffBitmap1->UnlockBits(bits1); delete bits1;


    // ------- и наконец выводим очередную страницу SaveBitmap2TiffBitmap1
    Gdiplus::EncoderParameters encoderParameters;
    ULONG parameterValue;

    // An EncoderParameters object has an array of
    // EncoderParameter objects. In this case, there is only
    // one EncoderParameter object in the array.
    encoderParameters.Count = 1;
    // Initialize the one EncoderParameter object.
    // encoderParameters.Parameter[0].Guid = Gdiplus::EncoderSaveFlag;
    encoderParameters.Parameter[0].Guid = aEncoderSaveFlag;
    encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoderParameters.Parameter[0].NumberOfValues = 1;
    encoderParameters.Parameter[0].Value = &parameterValue;

    // Get the CLSID of the TIFF encoder.
    CLSID encoderClsid;
    GetEncoderClsid(L"image/tiff", &encoderClsid);

    if (SaveBitmap2TiffState == 1) {                    // Требуется вывести первую страницу
        parameterValue = Gdiplus::EncoderValueMultiFrame;
        sres = SaveBitmap2TiffBitmap1->Save(SaveBitmap2TiffFiln, &encoderClsid, &encoderParameters);
    } else {
        parameterValue = Gdiplus::EncoderValueFrameDimensionPage;
        sres = SaveBitmap2TiffBitmap1->SaveAdd(&encoderParameters);
    }

    SaveBitmap2TiffState = 2;
}

static void __fastcall StopSaveBitmap2Tiff(void) {
//-------------------------------------------------------------------------------
//                      Закрывает вывод страниц в *.tif                         |
//-------------------------------------------------------------------------------
    if (SaveBitmap2TiffState == 0) return;

    if (SaveBitmap2TiffState == 2) {
        Gdiplus::EncoderParameters encoderParameters;
        ULONG parameterValue;
        CLSID encoderClsid;
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = aEncoderSaveFlag;
        encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;
        encoderParameters.Parameter[0].Value = &parameterValue;
        GetEncoderClsid(L"image/tiff", &encoderClsid);
        parameterValue = Gdiplus::EncoderValueFlush;
        sres = SaveBitmap2TiffBitmap1->SaveAdd(&encoderParameters);
    }

    delete[] SaveBitmap2TiffFiln; SaveBitmap2TiffFiln = NULL;
    delete SaveBitmap2TiffBitmap32; SaveBitmap2TiffBitmap32 = NULL;
    delete SaveBitmap2TiffGraphics; SaveBitmap2TiffGraphics = NULL;
    delete SaveBitmap2TiffBitmap1; SaveBitmap2TiffBitmap1 = NULL;

    SaveBitmap2TiffState = 0;
}












//===============================================================================
// class TTiffImage                                                             |
//===============================================================================
TTiffImage::TTiffImage(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    fbm = NULL;
    // gra = NULL;
    Page = 0;
    NPages = 0;
    // CanvasHandle = 0;
    DimensionID = GUID();
    Filn = "";
    temp = false;
}

TTiffImage::TTiffImage(Gdiplus::Bitmap *_fbm) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    fbm = _fbm;
    // gra = NULL;
    Page = 1;
    NPages = 1;
    // CanvasHandle = 0;
    DimensionID = GUID();
    Filn = "";
    temp = false;
}

TTiffImage::~TTiffImage(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    //if (gra != NULL)
    //    delete gra;
    if (fbm != NULL)
        delete fbm;
}



Gdiplus::Status __fastcall TTiffImage::GetStatus(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (fbm == NULL)
        return Gdiplus::UnknownImageFormat;
    return fbm->GetLastStatus();
}

Gdiplus::Status __fastcall TTiffImage::LoadFromFile(String filn) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    WCHAR *LoadBitmapFiln;
    String mes;
    Gdiplus::Status s;

    if (fbm != NULL) delete fbm;

    Page = 0;
    LoadBitmapFiln = new WCHAR[filn.Length() + 1];
    AnsiToUnicode(filn.c_str(), -1, LoadBitmapFiln);

    fbm = new Gdiplus::Bitmap(LoadBitmapFiln);
    delete[] LoadBitmapFiln;
    s = fbm->GetLastStatus();
    if (s != Gdiplus::Ok) {
        delete fbm;
        fbm = NULL;
        return s;
    }
    Filn = filn;
    UINT count = 0;
    count = fbm->GetFrameDimensionsCount();
    GUID* pDimensionIDs = (GUID*)malloc(sizeof(GUID)*count);
    fbm->GetFrameDimensionsList(pDimensionIDs, count);
    DimensionID = pDimensionIDs[0];
    NPages = fbm->GetFrameCount(&DimensionID);
    free(pDimensionIDs);

    return Gdiplus::Ok;
}


void __fastcall TTiffImage::SetPage(int n) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    Gdiplus::Status s;
    if (fbm == NULL || n < 0 || n >= NPages) return;
    if (n == Page) return;
    fbm->SelectActiveFrame(&DimensionID, n);
    s = fbm->GetLastStatus();
    if (s == Gdiplus::Ok)
        Page = n;
}


bool __fastcall TTiffImage::IncPageNumber(void) {
//-------------------------------------------------------------------------------
//                          Инкрементирует номер страницы                       |
// Если удалось, то возвращает true                                             |
//-------------------------------------------------------------------------------
    if (fbm == NULL) return false;
    if (Page + 1 >= NPages) return false;
    SetPage(Page + 1);
    return true;
}

bool __fastcall TTiffImage::DecPageNumber(void) {
//-------------------------------------------------------------------------------
//                          Декрементирует номер страницы                       |
// Если удалось, то возвращает true                                             |
//-------------------------------------------------------------------------------
    if (fbm == NULL) return false;
    if (Page == 0) return false;
    SetPage(Page - 1);
    return true;
}





/*
Status DrawImage(          Image *image,
    const Rect &destRect,
    INT srcx,
    INT srcy,
    INT srcwidth,
    INT srcheight,
    Unit srcUnit,
    ImageAttributes *imageAttributes,
    DrawImageAbort callback,
    VOID *callbackData
);
*/

void __fastcall TTiffImage::Draw(TFog *f) {
//-------------------------------------------------------------------------------
//                      Рисует текущую страницу на весь f                       |
//-------------------------------------------------------------------------------
    if (fbm == NULL) return;

    Gdiplus::Graphics g(f->Canvas->Handle);
    Gdiplus::Rect dst(0, 0, f->Width, f->Height);
    Gdiplus::Status s = g.DrawImage(fbm, dst, 0, 0, GetWidth(), GetHeight(), Gdiplus::UnitPixel, 0, 0, 0);
    stop = s;
}



void __fastcall TTiffImage::DrawCenter(TCanvas *c, int xc, int yc, int w1, int h1, int PageNumber) {
//-------------------------------------------------------------------------------
//                  Рисует текущую страницу с центром (xc, yc)                  |
// Масштабирует пропроционально, в итоге будет заполнено либо w, либо h         |
//-------------------------------------------------------------------------------
    if (fbm == NULL) return;

    Gdiplus::Graphics *gra, *g32;
    Gdiplus::Bitmap *b32;
    Gdiplus::Rect r;
    int w = fbm->GetWidth(), h = fbm->GetHeight();

    b32 = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
    g32 = new Gdiplus::Graphics(b32);

    r.X = 0;
    r.Y = 0;
    r.Width = w;
    r.Height = h;

    sres = g32->DrawImage(fbm, r, 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    if (PageNumber != 0) {                      // Надо выводить номер страницы?
        AnsiString snum;
        wchar_t wnum[20];
        Gdiplus::Pen whitePen40(Gdiplus::Color(255, 255, 255, 255), 45);
        snum.printf("%d", PageNumber);
        snum.WideChar(wnum, 20);
        Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
        Gdiplus::PointF origin(w - 160, 20.0f);
        Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
        g32->DrawLine(&whitePen40, w - 200, 50, w, 50);
        if (PageNumber > 0) {
            sres = g32->DrawString(wnum, -1, &myFont, origin, &blackBrush);
        }
    }


    w = w1; h = h1;
    gra = new Gdiplus::Graphics(c->Handle);

    //if (CanvasHandle != c->Handle) {
    //    if (CanvasHandle != NULL) delete gra;
    //    CanvasHandle = c->Handle;
    //    gra = new Gdiplus::Graphics(CanvasHandle);
    //}


    double scalex = (double)w / GetWidth();
    double scaley = (double)h / GetHeight();
    double scale = scalex < scaley ? scalex : scaley;
    w = round(GetWidth() * scale);
    h = round(GetHeight() * scale);
    xc -= w / 2;
    yc -= h / 2;

    Gdiplus::Rect dst(xc, yc, w, h);

    // gra->DrawImage(fbm, dst, 0, 0, GetWidth(), GetHeight(), Gdiplus::UnitPixel, 0, 0, 0);
    gra->DrawImage(b32, dst, 0, 0, GetWidth(), GetHeight(), Gdiplus::UnitPixel, 0, 0, 0);

    delete gra;
    delete g32;
    delete b32;

}












//===============================================================================
//                                  class TTiffBook                             |
//===============================================================================
TTiffBook::TTiffBook(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // Images = new TVds(VdsLen0, sizeof(TTiffImage), 10000);
    Images = new TVds(VdsLen0, 4, 10000);
    Pages = new TVds(VdsLen0, sizeof(TPoint), 10000);
    Page = 0;
    changed = false;
}

TTiffBook::~TTiffBook(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
    TTiffImage *img;
    for (int i = 0; i < Images->Nrecs; i++) {
        Images->RecIndex0 = i;
        img = (TTiffImage *)Images->RecPtr0;
        delete img;
    }
    delete Images;
    delete Pages;
*/
    TTiffImage *img;
    for (int i = 0; i < Images->Nrecs; i++) {
        img = GetImage(i);
        if (img != NULL) delete img;
    }
    delete Images;
    delete Pages;
}


int __fastcall TTiffBook::AppendImage(TTiffImage *img) {
//-------------------------------------------------------------------------------
//                  Добавляет в конец списка Images образ img                   |
// Возвращает номер вставленной записи в Images                                 |
//-------------------------------------------------------------------------------
    Images->Append();
    // Images->RecLen0 = sizeof(TTiffImage);
    // memmove(Images->RecPtr0, img, sizeof(TTiffImage));
    Images->RecLen0 = 4;
    *((int *)Images->RecPtr0) = (int)(img);
    changed = true;
    return Images->Nrecs - 1;
}


bool __fastcall TTiffBook::LoadTiff(String TiffFiln) {
//-------------------------------------------------------------------------------
//                      Загружает указанный тифовский файл                      |
// Собственно, добавляет его в Images - в Pages его еще надо прописать          |
// Если что не так, то выдате сообщение об ошибке и возвращает false            |
//-------------------------------------------------------------------------------
    TTiffImage *img = new TTiffImage();
    if (img->LoadFromFile(TiffFiln) != Gdiplus::Ok) {
        String mes = (String)"Ошибка загрузки файла \"" + TiffFiln + "\"";
        delete img;
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return false;
    }
    AppendImage(img);
    return true;
}


bool __fastcall TTiffBook::SaveBook(String TiffFiln, TPanel *SaveFilePanel) {
//-------------------------------------------------------------------------------
//                      Сохраняет результат манипуляций                         |
// SaveFilePanel - куда будет рисовать процесс выполнения
// Если что не так - выдает сообщение и вываливается                            |
//-------------------------------------------------------------------------------
    TTiffImage *img;
    TTiffPageInfo *pt;
    String TempFiln, BakFiln;
    int i;
    char buf[50];

    // if (TiffFiln == Filn) return false;                     // /* TODO : Временно для отладки ошибки сохранения файла */

    DWORD A = GetFileAttributes(TiffFiln.c_str());
    if (A != 0xFFFFFFFF) {
        if (A & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
            return false;
    }

    if (A == 0xFFFFFFFF) {
        TempFiln = TiffFiln;
    } else {
        TempFiln = TrimExt(TiffFiln) + "_temp.tif";
    }

    // if (!IniSaveBitmap2Tiff(TiffFiln.c_str())) {
    if (!IniSaveBitmap2Tiff(TempFiln.c_str())) {
        return false;
    }

    for (i = 0; i < Pages->Nrecs; i++) {
        if (SaveFilePanel != NULL) {
            wsprintf(buf, "Запись страницы %d из %d", i + 1, Pages->Nrecs);
            SaveFilePanel->Caption = (String)buf;
            Application->ProcessMessages();
        }
        img = GetImagePage(i);
        pt = GetPageInfo(i);
        SaveBitmap2Tiff(img->fbm, pt->PageNumber);
    }

    StopSaveBitmap2Tiff();

    if (A != 0xFFFFFFFF) {
        String BakFiln = TrimExt(TiffFiln) + ".bak";
        DeleteFile(BakFiln);
        if (MoveFile(TiffFiln.c_str(), BakFiln.c_str()) == 0) return false;
        if (MoveFile(TempFiln.c_str(), TiffFiln.c_str()) == 0) return false;
    }

    return true;
}


TTiffImage * __fastcall TTiffBook::GetImage(int index) {
//-------------------------------------------------------------------------------
//           Возвращает истиный адрес TTiffImage по индексу в дырке Images      |
// Полагает, что индекс в рамках приличий                                       |
//-------------------------------------------------------------------------------
    int ptr;
    Images->RecIndex1 = index;
    ptr = *((int *)(Images->RecPtr1));
    return (TTiffImage *)ptr;
}


void __fastcall TTiffBook::InsPages(int pos) {
//-------------------------------------------------------------------------------
//                      Добавляет страницы в указанной позиции                  |
// Полагает, что номер img - последняя запись Images                            |
//-------------------------------------------------------------------------------
    TTiffImage *img;
    TTiffPageInfo *pt;
    int i, k, n;
    k = Images->Nrecs - 1;
    // Images->RecIndex1 = k;
    // img = (TTiffImage *)Images->RecPtr1;
    img = GetImage(k);

    n = img->GetNPages();
    for (i = 0; i < n; i++) {
        if (pos < Pages->Nrecs) {
            Pages->InsRec(pos);
            pos++;
        } else {
            Pages->Append();
        }
        Pages->RecLen0 = sizeof(TTiffPageInfo);
        pt = (TTiffPageInfo *)Pages->RecPtr0;
        memset(pt, 0, sizeof(TTiffPageInfo));
        pt->ImageIndex = k; pt->PageIndex = i;
    }

    changed = true;
}


bool __fastcall TTiffBook::Append(String TiffFiln) {
//-------------------------------------------------------------------------------
//                  Добавляет тифовский файл в конец списка                     |
// Если что не так - выдает сообщение об ошибке и возвращает false              |
//-------------------------------------------------------------------------------
    if (!LoadTiff(TiffFiln)) return false;
    InsPages(10000);
    changed = true;
    return true;
}

void __fastcall TTiffBook::DelPages(int start, int end) {
//-------------------------------------------------------------------------------
//                  Удаляет страницы с start по end включительно                |
//-------------------------------------------------------------------------------
    int i;
    int n = end - start + 1;

    for (i = 0; i < n && i < Pages->Nrecs; i++) {
        Pages->RecIndex0 = start;
        Pages->DelRec();
    }

    if (GetPage() >= GetNPages()) {
        SetPage(GetNPages() - 1);
    }

    changed = true;
}


void __fastcall TTiffBook::FlipCurPage(int n) {
//-------------------------------------------------------------------------------
//               Переворачивает текущую страницу вверх ногами                   |
// n - номер страницы, который надо нанести. <= 0 - только убрать ном стр       |
//-------------------------------------------------------------------------------
    stop++;
    Gdiplus::Graphics *g32;
    TTiffImage *img = GetImagePage(GetPage());
    Gdiplus::Bitmap *fbm = img->fbm;
    Gdiplus::Rect r;
    int w = fbm->GetWidth(), h = fbm->GetHeight();

    // g = new Gdiplus::Graphics(fbm);

    Gdiplus::Bitmap *b32 = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
    g32 = new Gdiplus::Graphics(b32);

    Gdiplus::Matrix matr;
    matr.Rotate(180.0f);                // поворачиваем его на 270 градусов
    g32->SetTransform(&matr);

    //r.X = 0;
    //r.Y = 0;
    //r.Width = w;
    //r.Height = h;

    //r.X = -SaveBitmapH;
    //r.Y = -30;
    //r.Width = SaveBitmapH - 30;
    //r.Height = SaveBitmapW - 30;

    r.X = -w;
    r.Y = -h;
    r.Width = w;
    r.Height = h;

    sres = g32->DrawImage(fbm, r, 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    matr.Reset();
    g32->SetTransform(&matr);

    // Удаляем черные полосы сверху и снизу
    Gdiplus::Pen whitePen2(Gdiplus::Color(255, 255, 255, 255), 2);
    g32->DrawLine(&whitePen2, 0, 0, w, 0);
    g32->DrawLine(&whitePen2, 0, h-1, w, h-1);

    // Удаляем старый номер страницы - он теперь внизу
    // Gdiplus::Pen whitePen40(Gdiplus::Color(255, 0, 0, 0), 45);   // ??? Пока черный - для отладки!
    Gdiplus::Pen whitePen40(Gdiplus::Color(255, 255, 255, 255), 45);
    g32->DrawLine(&whitePen40, 0, h - 50, 200, h - 50);

    if (n > 0) {                        // Здесь выводим номер страницы
        AnsiString snum;
        wchar_t wnum[20];
        snum.printf("%d", n);
        snum.WideChar(wnum, 20);
        Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
        Gdiplus::PointF origin(w - 160, 20.0f);
        Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
        g32->DrawLine(&whitePen40, w - 200, 50, w, 50);
        sres = g32->DrawString(wnum, -1, &myFont, origin, &blackBrush);
    }

    //if (npage > 0) {            // Здесь выводим номер страницы
    //    AnsiString snum;
    //    wchar_t wnum[20];
    //    snum.printf("%d", npage);
    //    snum.WideChar(wnum, 20);
    //    Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
    //    Gdiplus::PointF origin(SaveBitmapW - 160, 20.0f);
    //    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
    //    SaveBitmap2TiffGraphics->DrawString(wnum, -1, &myFont, origin, &blackBrush);
    //}

    // ------- строим SaveBitmap2TiffBitmap1
    UINT *pix;
    unsigned char byte, bit, *pix1;

    Gdiplus::Bitmap *b1 = new Gdiplus::Bitmap(w, h, PixelFormat1bppIndexed);
    Gdiplus::Rect BitsRect(0, 0, w, h);
    Gdiplus::BitmapData *bits = new Gdiplus::BitmapData;
    Gdiplus::BitmapData *bits1 = new Gdiplus::BitmapData;
    sres = b32->LockBits(&BitsRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bits);
    sres = b1->LockBits(&BitsRect, Gdiplus::ImageLockModeWrite, PixelFormat1bppIndexed, bits1);

    for (int y = 0; y < h; y++) {
        pix = (UINT *)((int)bits->Scan0 + bits->Stride * y);
        pix1 = (unsigned char *)((int)bits1->Scan0 + bits1->Stride * y);
        byte = 0;
        for (int x = 0; x < w; x++, pix++) {
            if ((*pix & 0xFF) > 0xD8) {
                bit = 1;
            }
            else bit = 0;
            byte <<= 1; byte |= bit;
            if ((x & 7) == 7) {
                *pix1++ = byte;
                byte = 0;
            }
        }
    }

    b32->UnlockBits(bits); delete bits;
    b1->UnlockBits(bits1); delete bits1;

    delete g32;
    delete b32;

    TTiffPageInfo *pi = GetPageInfo(GetPage());
    if (img->Filn.Length() == 0) {                  // Это уже временный файл!
        delete img->fbm;
        img->fbm = b1;
    } else {                                        // Надо будет заменить страницу на временный файл
        img = new TTiffImage(b1);
        pi->ImageIndex = AppendImage(img);
        pi->PageIndex = 0;
    }

    changed = true;
}


void __fastcall TTiffBook::SetPage(int n) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (n < 0 || n >= GetNPages()) return;
    Application->BringToFront();
    Page = n;
}

bool __fastcall TTiffBook::IncPageNumber(void) {
//-------------------------------------------------------------------------------
//                          Инкрементирует номер страницы                       |
// Если удалось, то возвращает true                                             |
//-------------------------------------------------------------------------------
    if (Page + 1 >= GetNPages()) return false;
    SetPage(Page + 1);
    return true;
}


bool __fastcall TTiffBook::DecPageNumber(void) {
//-------------------------------------------------------------------------------
//                          Декрементирует номер страницы                       |
// Если удалось, то возвращает true                                             |
//-------------------------------------------------------------------------------
    if (GetPage() == 0) return false;
    SetPage(Page - 1);
    return true;
}


void __fastcall TTiffBook::EnumPages(int start, int first) {
//-------------------------------------------------------------------------------
//                              Нумерует страницы                               |
//-------------------------------------------------------------------------------
    TTiffPageInfo *pt;
    int i, k;

    if (start == 0 || first == 0) return;

    for (i = 1, k = first; i <= Pages->Nrecs; i++) {
        pt = GetPageInfo(i-1);
        if (i < start) {
            pt->PageNumber = -1;
        } else {
            pt->PageNumber = k;
            k++;
        }
    }
}


TTiffImage * __fastcall TTiffBook::GetImagePage(int k) {
//-------------------------------------------------------------------------------
//   Возвращает img[Pages[k].ImageIndex] с уст страницей Pages[k].PageIndex     |
// Полагает, что все это можно                                                  |
//-------------------------------------------------------------------------------
    TTiffImage *img;
    TTiffPageInfo *pt;

    Pages->RecIndex1 = k;
    pt = (TTiffPageInfo *)Pages->RecPtr1;

    // Images->RecIndex1 = pt->x;
    // img = (TTiffImage *)Images->RecPtr1;
    img = GetImage(pt->ImageIndex);
    img->SetPage(pt->PageIndex);

    return img;
}



void __fastcall TTiffBook::DrawCenter(TCanvas *c, int xc, int yc, int w, int h) {
//-------------------------------------------------------------------------------
//                  Рисует текущую страницу с центром (xc, yc)                  |
// Масштабирует пропроционально, в итоге будет заполнено либо w, либо h         |
//-------------------------------------------------------------------------------
    TTiffImage *img;
    if (Page >= GetNPages()) return;
    img = GetImagePage(Page);
    TTiffPageInfo *pt = GetPageInfo(Page);
    img->DrawCenter(c, xc, yc, w, h, pt->PageNumber);
}
















//===============================================================================
//===============================================================================
__fastcall TEditTiffForm::TEditTiffForm(TComponent* Owner) : TForm(Owner) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    InsideChange = false;
}


void __fastcall TEditTiffForm::SetupPageComboBox(int count, int start) {
//-------------------------------------------------------------------------------
//                          Заполняет PageComboBox                              |
// Если start == -1, то не меняет ItemIndex                                     |
//-------------------------------------------------------------------------------
    TStrings *items = PageComboBox->Items;
    int i;
    char buf[50];

    items->Clear();
    for (i = 1; i <= count; i++) {
        sprintf(buf, "%d / %d", i, count);
        items->Add(buf);
    }

    InsideChange = true;
    ScrollBar1->Max = count;
    if (start >= 0) {
        PageComboBox->ItemIndex = start;
        ScrollBar1->Position = start;
    } else if (PageComboBox->ItemIndex == -1) {
        PageComboBox->ItemIndex = 0;
        ScrollBar1->Position = 0;
    }

    EnumTiffPages();
    InsideChange = false;
}


// void __fastcall Rectangle(int X1, int Y1, int X2, int Y2);

void __fastcall TEditTiffForm::Fog1Paint(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TCanvas *c = Fog1->Canvas;
    c->Brush->Style = bsSolid;
    c->Brush->Color = clBtnFace;
    c->Rectangle(0, 0, Fog1->Width, Fog1->Height);

    // ti->DrawCenter(Fog1->Canvas, Fog1->Width / 2, Fog1->Height / 2, Fog1->Width - 4, Fog1->Height - 4);
    tb->DrawCenter(Fog1->Canvas, Fog1->Width / 2, Fog1->Height / 2, Fog1->Width - 4, Fog1->Height - 4);
}

void __fastcall TEditTiffForm::RedrawPage(void) {
//-------------------------------------------------------------------------------
//                      Полагает, что номер страницы изменился                  |
//-------------------------------------------------------------------------------
    Fog1->Invalidate();
    
    // if (ti == NULL) return;
    // PageComboBox->ItemIndex = ti->GetPage();

    if (tb == NULL) return;
    PageComboBox->ItemIndex = tb->GetPage();
    ScrollBar1->Position = tb->GetPage();
}


void __fastcall TEditTiffForm::IncBitBtnClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // if (ti == NULL) return;
    // if (ti->IncPageNumber()) RedrawPage();
    if (tb == NULL) return;
    if (tb->IncPageNumber()) RedrawPage();

}

void __fastcall TEditTiffForm::DecBitBtnClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // if (ti == NULL) return;
    // if (ti->DecPageNumber()) RedrawPage();
    if (tb == NULL) return;
    if (tb->DecPageNumber()) RedrawPage();
}

void __fastcall TEditTiffForm::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (Key == VK_NEXT) {
        IncBitBtnClick(Sender);
    } else if (Key == VK_PRIOR) {
        DecBitBtnClick(Sender);
    }
}

void __fastcall TEditTiffForm::Fog1MouseWheelDown(TObject *Sender,
      TShiftState Shift, const TPoint &MousePos, bool &Handled) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    IncBitBtnClick(Sender);
    Handled = true;
}

void __fastcall TEditTiffForm::Fog1MouseWheelUp(TObject *Sender,
      TShiftState Shift, const TPoint &MousePos, bool &Handled) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    DecBitBtnClick(Sender);
    Handled = true;
}



void __fastcall TEditTiffForm::PageComboBoxSelect(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    //if (ti == NULL) return;
    //ti->SetPage(PageComboBox->ItemIndex);
    if (tb == NULL) return;
    tb->SetPage(PageComboBox->ItemIndex);
    RedrawPage();
}


void __fastcall TEditTiffForm::ScrollBar1Change(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    // if (ti == NULL) return;
    if (tb == NULL) return;
    if (InsideChange) return;
    InsideChange = true;
    // ti->SetPage(ScrollBar1->Position);
    tb->SetPage(ScrollBar1->Position);
    InsideChange = false;
    RedrawPage();
}

static int __fastcall _PackPageNumber(String Text) {
//-------------------------------------------------------------------------------
//                      Если что не так, то возвращает -1                       |
//-------------------------------------------------------------------------------
    String Num;
    char *p, *q, keep;

    p = Text.c_str();
    while(*p == ' ') p++;
    if (*p < '0' || *p > '9') return -1;

    q = p;
    while (*q >= '0' && *q <= '9') q++;
    keep = *q; *q = 0;
    Num = p;
    *q = keep;

    return Num.ToInt();
}


void __fastcall TEditTiffForm::PackPageNumber(void) {
//-------------------------------------------------------------------------------
//                  Пакует текущее значение PageComboBox->Text                  |
// Если вразумительно - то соответственно выбирает PageComboBox->ItemIndex и    |
//   соответственно переходит на нужную страницу, если она еще не               |
// Если нет - вертает в PageComboBox->Text значение текущего ItemIndex          |
// ???? Не доделано, и не уверен, что нужно                                     |
//-------------------------------------------------------------------------------
    String Text = PageComboBox->Text;

    int apage = _PackPageNumber(Text);
    if (apage < 0) {
        InsideChange = true;
        PageComboBox->Text = PageComboBox->Items->Strings[PageComboBox->ItemIndex];
        InsideChange = false;
        return;
    }

    // Когда-нибудь может быть вставить текст, если номер страницы нормальный

}



bool __fastcall TEditTiffForm::EditConfig(String acase) {
//-------------------------------------------------------------------------------
//                      Редактирует текущую конфигурацию                        |
// acase - случай.                                                              |
// Если tic.IsZeroEnum(), то выдает сообщение                                   |
// Возвращает - задали ли конфигурацию                                          |
//-------------------------------------------------------------------------------
    // bool forsed = false;    // Вызов диалога обязателен
    bool res;

    tic.Falsify();
    if (acase == "Перевернуть") {
        if (!tic.IsZeroEnum()) return true;        // Если перевернуть, а параметры заданы - диалог не нужен
        tic.Caption = "Перевернуть текущую страницу на 180 градусов";
        tic.NeedEnumPanel = true;
    } else if (acase == "Параметры") {
        tic.Caption = "Параметры редактирования файла *.tif";
        tic.NeedEnumPanel = true;
    } else if (acase == "Заменить") {
        tic.Caption = "Заменить страницы указанным файлом";
        tic.NeedEnumPanel = true;
        tic.NeedReplacePanel = true;
        tic.NeedFilenamePanel = true;
    } else if (acase == "Вставить") {
        tic.Caption = "Вставить файл перед текущей страницей";
        tic.NeedEnumPanel = true;
        tic.NeedFilenamePanel = true;
    } else if (acase == "Добавить") {
        tic.Caption = "Добавить файл в конец";
        tic.NeedEnumPanel = true;
        tic.NeedFilenamePanel = true;
    } else if (acase == "Удалить") {
        tic.Caption = "Удалить указанные страницы";
        tic.NeedEnumPanel = true;
        tic.NeedDeletePanel = true;
    } else {
        Application->MessageBox("Временно не реализовано", "Надо ждать", 0);
        return false;
    }

    res = EditTiffImageConfig(&tic);
    if (!res) return false;

    if (tic.IsZeroEnum()) {
        Application->MessageBox("Для совершения требуемой операции необходимо задать параметры редактирования", "Не могу!", 0);
        return false;
    }

    return true;
}



bool __fastcall TEditTiffForm::PrintFile(void) {
//-------------------------------------------------------------------------------
//             Печатает файл tic.Filename и кладет его в tb->Images             |
// Если что не получилось, то выдает сообщение и возвращает false               |
//-------------------------------------------------------------------------------
    bool res;

    TFileList *ffl = new TFileList;
    String aFiln = ErasePathFromFiln(tic.Filename);
    ffl->Append("", tic.Filename, "", "");

    String filn;
    int filn_n;
    char buf[50], path[300];

    filn = CreateTempFiln();

    Visible = false;
    res = SEnumMainForm->PrintFilesLoaded(filn.c_str(), ffl, 0, 0);
    delete ffl;
    Visible = true;
    if (!res) return false;

    res = tb->LoadTiff(filn);
    DeleteFile(filn);

    return true;
}


void __fastcall TEditTiffForm::RotateSpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!EditConfig("Перевернуть")) return;
    // Application->MessageBox("Якобы переворачивание страницы", "Инфо", 0);
    int p = tb->GetPage() + 1;
    int n;

    if (p >= tic.FirstEnumPage) {
        n = p - tic.FirstEnumPage + tic.FirstPageNum;
    } else {
        n = 0;
    }

    tb->FlipCurPage(n);
    EnumTiffPages();
    Fog1->Invalidate();
}


void __fastcall TEditTiffForm::ConfigSpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    int FirstEnumPage = tic.FirstEnumPage;
    int FirstPageNum = tic.FirstPageNum;
    if (!EditConfig("Параметры")) return;
    if (tic.FirstEnumPage != 0 && tic.FirstPageNum != 0) {
        EnumTiffPages();
        if (tic.FirstEnumPage != FirstEnumPage || tic.FirstPageNum != FirstPageNum) {
            tb->changed = true;
        }
        Fog1->Invalidate();
    }
}

void __fastcall TEditTiffForm::EditSpeedButtonClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TPoint pt;
    pt.x = EditSpeedButton->Left;
    pt.y = EditSpeedButton->Top + EditSpeedButton->Height + 1;
    pt = Panel1->ClientToScreen(pt);
    EditPopupMenu->Popup(pt.x, pt.y);
}


bool __fastcall TEditTiffForm::DeleteTicStartEnd(void) {
//-------------------------------------------------------------------------------
//              Удаляет страницы tic.Start - tic.End                            |
// Если с диапазоном все в порядке, то удаляет.                                 |
// Иначе выдает сообщение об ошибке и возвращает false                          |
//-------------------------------------------------------------------------------
    String mes;

    if (tic.Start == 0 || tic.Start > tb->GetNPages()) {
        Application->MessageBox("Номер первой удаляемой страницы должен лежать в диапазоне от 1 до количества страниц", "Ошибка!", 0);
        return false;
    }
    if (tic.End == 0 || tic.End > tb->GetNPages()) {
        Application->MessageBox("Номер последней удаляемой страницы должен лежать в диапазоне от 1 до количества страниц", "Ошибка!", 0);
        return false;
    }
    if (tic.Start > tic.End) {
        Application->MessageBox("Номер первой удаляемой страницы должен быть меньше номера последней", "Ошибка!", 0);
        return false;
    }
    if (tic.Start == 1 && tic.End == tb->GetNPages()) {
        Application->MessageBox("Нельзя удалить все страницы! Необходимо оставить хотя бы одну!", "Ошибка!", 0);
        return false;
    }

    tb->DelPages(tic.Start - 1, tic.End - 1);

    return true;
}

void __fastcall TEditTiffForm::EnumTiffPages(void) {
//-------------------------------------------------------------------------------
//              Перенумеровывает страницы tb в соответствии с tic               |
//-------------------------------------------------------------------------------
    if (tic.FirstEnumPage != 0 && tic.FirstPageNum != 0) {
        tb->EnumPages(tic.FirstEnumPage, tic.FirstPageNum);
    }
}


void __fastcall TEditTiffForm::DeleteMenuItemClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!EditConfig("Удалить")) return;

    if (!DeleteTicStartEnd()) return;

    tb->SetPage(tic.Start - 1);
    SetupPageComboBox(tb->GetNPages(), tb->GetPage());
    Fog1->Invalidate();
}


void __fastcall TEditTiffForm::ReplaceMenuItemClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!EditConfig("Заменить")) return;
    // Application->MessageBox("Якобы заменяет страницы", "Инфо", 0);

    if (!DeleteTicStartEnd()) return;

    if (!PrintFile()) return;

    int pos = tic.Start - 1;
    tb->InsPages(pos);

    tb->SetPage(pos);
    SetupPageComboBox(tb->GetNPages(), tb->GetPage());
    Fog1->Invalidate();

}

void __fastcall TEditTiffForm::InsertMenuItemClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!EditConfig("Вставить")) return;

    if (!PrintFile()) return;

    int pos = tb->GetPage();
    tb->InsPages(pos);

    tb->SetPage(pos);
    SetupPageComboBox(tb->GetNPages(), tb->GetPage());
    Fog1->Invalidate();
}

void __fastcall TEditTiffForm::AppendMenuItemClick(TObject *Sender) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    if (!EditConfig("Добавить")) return;

    if (!PrintFile()) return;

    int pos = tb->GetNPages();
    tb->InsPages(10000);

    tb->SetPage(pos);
    SetupPageComboBox(tb->GetNPages(), tb->GetPage());
    Fog1->Invalidate();
}

// GetTempPath

void __fastcall TEditTiffForm::FormCloseQuery(TObject *Sender, bool &CanClose) {
//-------------------------------------------------------------------------------
//              Спросим, не надо ли сохранить файл и если надо - сохраним       |
//-------------------------------------------------------------------------------
    if (tb->changed) {
        int res = Application->MessageBox("Сохранить выполненные изменения?", "Что делать???", MB_YESNO);
        if (res == IDYES) {
            int x0 = ClientWidth / 2;
            int y0 = ClientHeight / 2;
            int x = x0 - SaveFilePanel->Width / 2;
            int y = y0 - SaveFilePanel->Height / 2;
            SaveFilePanel->Left = x;
            SaveFilePanel->Top = y;

            String filn = tb->Filn;
            // String filn = "c:\\UnitedProjects\\Нумерация страниц смет\\exe\\ttt\\TestTiffUnit1.tif";
            String mes;
            bool saveok;
            while(1) {
                Enabled = false;
                SaveFilePanel->Visible = true;
                saveok = tb->SaveBook(filn, SaveFilePanel);
                SaveFilePanel->Visible = false;
                Enabled = true;
                if (saveok) break;
                mes = (String)"Ошибка сохранения файла \"" + filn + "\"\015\012" + "Сохранить файл под другим именем?";
                res = Application->MessageBox(mes.c_str(), "Ошибка!", MB_YESNO);
                if (res != IDYES) break;
                if (!SaveAsTiffDialog->Execute()) break;
                filn = SaveAsTiffDialog->FileName;
            }
        }
    }
    CanClose = true;
}

