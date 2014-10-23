#include <vcl.h>
#pragma hdrstop

#pragma comment(lib, "GdiPlus.lib")


#include "SEnumMainUnit.h"

#include "SEnumLib1.h"
#include "SEnumLib.h"

int ggh, ggw;

static bool GdiplusStarted = false;
static Gdiplus::Bitmap *fbm = NULL;
static GUID DimensionID;
static UINT frameCount;
static int curFrame = 0;
static TImage *aImage;
Gdiplus::Status sres;

static Gdiplus::Bitmap *pbm = NULL;
static Gdiplus::Bitmap *pbm1 = NULL;

/*
#include <winsock2.h>
#include <ScktComp.hpp>
TServerClientThread *clientt = NULL;
TClientWinSocket *clients = NULL;
*/

/*
static int __fastcall ScanRow(Excel_2k::_WorksheetPtr ActiveSheet, int row, int width) {
//-------------------------------------------------------------------------------
//                          Ищет в ряду непустую ячейку                         |
// Возвращает номер самой правой непустой ячейки                                |
//-------------------------------------------------------------------------------
    Variant vval;
    String val;
    int col, maxcol;

    maxcol = 0;
    for (col = width; col >= 1; col--) {
        vval = ActiveSheet->Cells->get_Item(Variant(row), Variant(col));
        val = vval;
        if (!val.IsEmpty()) {
            maxcol = col;
            break;
        }
    }

    return maxcol;
}


TPoint __fastcall FindLastCell2(TPoint Max) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
    TExcelApplication *EA = NULL;
    Excel_2k::_Workbook *Doc = NULL;
    Excel_2k::_WorksheetPtr ActiveSheet;
    Variant vval;
    String val;
    String Filn = "c:\\tt.xls";
    wchar_t *_FileName;
    int len;
    bool Connected;

    stop++;
    try {
        if (EA == NULL) EA = new TExcelApplication(NULL);
        Connected = EA->Workbooks->Count != 0;
        if (!Connected) EA->Connect();
    } catch (...) {
        Application->MessageBox("Не могу установить связь с сервером MS Excel", "Ошибка!", 0);
        if (EA != NULL) {
            if (!Connected) EA->Disconnect();
            delete EA;
            EA = NULL;
        }
        return Max;
    }

    try {
        len = Filn.WideCharBufSize(); _FileName = new wchar_t[len];
        Filn.WideChar(_FileName, Filn.Length()); // AnsiString
        Doc = EA->Workbooks->Open(BSTR(_FileName),  (Variant)false, (Variant)true); // (&FileName, false, true,
        delete[] _FileName;
    } catch(...) {
        delete[] _FileName;
        String Ermes = (String)"Не могу открыть файл \"" + Filn + "\"";
        Application->MessageBox(Ermes.c_str(), "Ошибка!", 0);
        if (Doc != NULL) {
            Doc->Close();
            Doc = NULL;
        }
        if (EA != NULL) {
            if (!Connected) EA->Disconnect();
            delete EA;
            EA = NULL;
        }
        return Max;
    }


    Doc = EA->Workbooks->get_Item(Variant(1));
    ActiveSheet = Doc->ActiveSheet;
*/
/*
    int col, row, maxrow, maxcol;
    maxcol = 0; maxrow = 0;
    for (row = Max.y; row >= 1; row--) {
        for (col = 1; col < Max.x; col++) {
            vval = ActiveSheet->Cells->get_Item(Variant(row), Variant(col));
            val = vval;
            if (!val.IsEmpty()) {
                if (maxrow == 0) maxrow = row;
                if (maxcol < col) maxcol = col;
            }
        }
    }
    Max.x = maxcol; Max.y = maxrow;
    return Max;
*/
/*
    int x, y, ymax, xmax, empties;
    xmax = 0; ymax = 0; empties = 0;
    for (y = 1; y <= Max.y; y++) {
        x = ScanRow(ActiveSheet, y, Max.x);
        if (x > 0) {
            empties = 0;
            ymax = y;
            if (x > xmax) xmax = x;
        } else {
            empties++;
            if (empties > 10) break;
        }
    }
    Max.x = xmax; Max.y = ymax;
    return Max;
}
*/


// ----------------- ни фига не получилось ---------------------
/*
    TExcelApplication *EA = NULL;
    Excel_2k::_Workbook *Doc = NULL;
    // String Filn = "c:\\UnitedProjects\\Нумерация страниц смет\\exe\\Комплект.xls";
    String Filn = "c:\\tt.xls";
    int len;
    wchar_t *_FileName;
    bool Connected;

    stop++;
    try {
        if (EA == NULL) EA = new TExcelApplication(NULL);
        Connected = EA->Workbooks->Count != 0;
        if (!Connected) EA->Connect();
    } catch (...) {
        Application->MessageBox("Не могу установить связь с сервером MS Excel", "Ошибка!", 0);
        if (EA != NULL) {
            if (!Connected) EA->Disconnect();
            delete EA;
            EA = NULL;
        }
        return;
    }


    try {
        len = Filn.WideCharBufSize(); _FileName = new wchar_t[len];
        Filn.WideChar(_FileName, Filn.Length()); // AnsiString
        Doc = EA->Workbooks->Open(BSTR(_FileName),  (Variant)false, (Variant)true); // (&FileName, false, true,
        delete[] _FileName;
    } catch(...) {
        delete[] _FileName;
        String Ermes = (String)"Не могу открыть файл \"" + Filn + "\"";
        Application->MessageBox(Ermes.c_str(), "Ошибка!", 0);
        if (Doc != NULL) {
            Doc->Close();
            Doc = NULL;
        }
        if (EA != NULL) {
            if (!Connected) EA->Disconnect();
            delete EA;
            EA = NULL;
        }
        return;
    }

    Excel_2k::_WorksheetPtr S;
    Excel_2k::SheetsPtr ss;
    Variant SItem;
    Variant vname;
    String sname;
    Excel_2k::RangePtr cl;
    Variant val;

    // EA->Visible = true;
    ss = Doc->Sheets;
    SItem = ss->get_Item(Variant(1));   // Не получается взять отдельный лист
    vname = SItem.OlePropertyGet("Name");
    sname = vname;

    S = Doc->ActiveSheet;

    val = S->Cells->get_Item(Variant(1), Variant(1));
    stop = val;
    S->Cells->set_Item(Variant(1), Variant(1), Variant(2.345));
    val = S->Cells->get_Item(Variant(1), Variant(1));
    stop = val;

    //val = Doc->SaveAs(Variant(Filn.c_str()), TNoParam(), TNoParam(),
    //    TNoParam(), TNoParam(), TNoParam(),
    //    xlNoChange,
    //    TNoParam(), TNoParam(), TNoParam(),
    //    TNoParam(), 0);

    stop = val;
    Doc->Save(xlSaveChanges);

    stop = EA->Workbooks->Count;
    Doc->Close();
    stop = EA->Workbooks->Count;
    // if (!Connected) EA->Disconnect();
    EA->Disconnect();
    stop = EA->Workbooks->Count;
*/



/*
    * image/bmp
    * image/jpeg
    * image/gif
    * image/tiff
    * image/png

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

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


Status MakeCompoundFile()
{
   IStorage* pIStorage = NULL;
   IStream* pIStream1 = NULL;
   IStream* pIStream2 = NULL;
   HRESULT hr;
   Status stat = Ok;

   // Create two Image objects from existing files.
   Image image1(L"Crayons.jpg");
   Image image2(L"Mosaic.png");

   hr = CoInitialize(NULL);
   if(FAILED(hr))
      goto Exit;

   // Create a compound file object, and get
   // a pointer to its IStorage interface.
   hr = StgCreateDocfile(
      L"CompoundFile.cmp",
      STGM_READWRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE,
      0,
      &pIStorage);

   if(FAILED(hr))
      goto Exit;

   // Create a stream in the compound file.
   hr = pIStorage->CreateStream(
      L"StreamImage1",
      STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
      0,
      0,
      &pIStream1);

   if(FAILED(hr))
      goto Exit;

   // Create a second stream in the compound file.
   hr = pIStorage->CreateStream(
      L"StreamImage2",
      STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
      0,
      0,
      &pIStream2);

   if(FAILED(hr))
      goto Exit;

   // Get the class identifier for the JPEG encoder.
   CLSID jpgClsid;
   GetEncoderClsid(L"image/jpeg", &jpgClsid);

   // Get the class identifier for the PNG encoder.
   CLSID pngClsid;
   GetEncoderClsid(L"image/png", &pngClsid);

   // Save image1 as a stream in the compound file.
   stat = image1.Save(pIStream1, &jpgClsid);
   if(stat != Ok)
      goto Exit;

   // Save image2 as a stream in the compound file.
   stat = image2.Save(pIStream2, &pngClsid);

Exit:
   if(pIStream1)
      pIStream1->Release();
   if(pIStream2)
      pIStream2->Release();
   if(pIStorage)
      pIStorage->Release();

   if(stat != Ok || FAILED(hr))
      return GenericError;

   return Ok;
}
*/




//===============================================================================
//                      Вспомогательные функции для Gdiplus                     |
//===============================================================================
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











//===============================================================================
//                      Запуск и остановка системы Gdiplus                      |
//===============================================================================
static int GdiplusStartCount = 0;
static ULONG_PTR gdiplusToken;

int __fastcall StartGdiplus(void) {
//-------------------------------------------------------------------------------
//                      Инициализирует систему Gdiplus                          |
// Возвращает количество незакрытых инициализаций Gdiplus                       |
//-------------------------------------------------------------------------------
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    if (GdiplusStartCount == 0) {
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }
    GdiplusStartCount++;
    return GdiplusStartCount;
}

int __fastcall StopGdiplus(void) {
//-------------------------------------------------------------------------------
//                          Завершает работу Gdiplus                            |
// Возвращает количество незакрытых инициализаций Gdiplus                       |
//-------------------------------------------------------------------------------
    if (GdiplusStartCount == 0) return GdiplusStartCount;
    GdiplusStartCount--;
    if (GdiplusStartCount == 0) {
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }
    return GdiplusStartCount;
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

bool __fastcall IniSaveBitmap2Tiff(char *filn) {
//-------------------------------------------------------------------------------
//            Инициализирует процедуру вывода битмапов в файл *.tiff            |
// Если что-нибудь не получилось - выдает сообщение и возвращает false          |
//-------------------------------------------------------------------------------
    HANDLE chan;
    String mes;
    int len;

    if (SaveBitmap2TiffState != 0) {
        Application->MessageBox("Ошибка программиста! Повторная инициализация системы вывода файла *.tiff!", "Срочно обратитесь в ОПО!", 0);
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


void __fastcall SaveBitmap2Tiff(Gdiplus::Bitmap *bm, int npage) {
//-------------------------------------------------------------------------------
//                  Выводит очередную страницу в файл *.tiff                    |
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

    Gdiplus::Matrix matr;
    Gdiplus::Rect r;
    if (w > h) {                            // Если изображение шире, чем выше
        matr.Rotate(270.0f);                // поворачиваем его на 270 градусов
        SaveBitmap2TiffGraphics->SetTransform(&matr);
        r.X = -SaveBitmapH;
        r.Y = -30;
        r.Width = SaveBitmapH - 30;
        r.Height = SaveBitmapW - 30;
    } else {
        r.X = 30;
        r.Y = 0;
        r.Width = SaveBitmapW - 30;
        r.Height = SaveBitmapH - 30;
    }

    SaveBitmap2TiffGraphics->DrawImage(bm, r, 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    matr.Reset();
    SaveBitmap2TiffGraphics->SetTransform(&matr);

    if (npage > 0) {            // Здесь выводим номер страницы
        AnsiString snum;
        wchar_t wnum[20];
        snum.printf("%d", npage);
        snum.WideChar(wnum, 20);
        Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
        Gdiplus::PointF origin(SaveBitmapW - 160, 20.0f);
        Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
        SaveBitmap2TiffGraphics->DrawString(wnum, -1, &myFont, origin, &blackBrush);
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

void __fastcall StopSaveBitmap2Tiff(void) {
//-------------------------------------------------------------------------------
//                      Закрывает вывод страниц в *.tiff                        |
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
//                  Загрузка, листание, выгрузка тифовского файла               |
// Полагает, что система Gdiplus уже запущена                                   |
//===============================================================================
int LoadTiffFileNpages = 0;         // Количество страниц в тифовском файле
static GUID LoadTiffFileDimensionID;
static Gdiplus::Bitmap *LoadTiffFileBitmap = NULL;

Gdiplus::Bitmap * __fastcall LoadTiffFile(char *filn) {
//-------------------------------------------------------------------------------
//                      Загружает тифовский файл для листания                   |
// Если загрузить не удалось, возвращает NULL                                   |
//-------------------------------------------------------------------------------
    Gdiplus::Bitmap *fbm;
    WCHAR *LoadBitmapFiln;
    String mes;
    Gdiplus::Status s;

    if (LoadTiffFileBitmap != NULL) ReleaseTiffFile();

    LoadBitmapFiln = new WCHAR[strlen(filn) + 1];
    AnsiToUnicode(filn, -1, LoadBitmapFiln);

    fbm = new Gdiplus::Bitmap(LoadBitmapFiln);
    delete[] LoadBitmapFiln;
    s = fbm->GetLastStatus();
    if (s != Gdiplus::Ok) {
        stop++;
        delete fbm;

        fbm = new Gdiplus::Bitmap(LoadBitmapFiln);
        s = fbm->GetLastStatus();

        mes = (String)"Не могу открыть файл \"" + filn + "\" для чтения!";
        Application->BringToFront();
        Application->MessageBox(mes.c_str(), "Ошибка!", 0);
        return NULL;
    }
    UINT count = 0;
    count = fbm->GetFrameDimensionsCount();
    GUID* pDimensionIDs = (GUID*)malloc(sizeof(GUID)*count);
    // Get the list of frame dimensions from the Image object.
    fbm->GetFrameDimensionsList(pDimensionIDs, count);
    // Display the GUID of the first (and only) frame dimension.
    //WCHAR strGuid[39];
    //StringFromGUID2(pDimensionIDs[0], strGuid, 39);
    //wsprintf(buf, "The first (and only) dimension ID is %s.\n", strGuid);
    // Get the number of frames in the first dimension.
    LoadTiffFileDimensionID = pDimensionIDs[0];
    LoadTiffFileNpages = fbm->GetFrameCount(&LoadTiffFileDimensionID);
    free(pDimensionIDs);

    LoadTiffFileBitmap = fbm;
    return fbm;
}

void __fastcall SetTiffFilePage(int npage) {
//-------------------------------------------------------------------------------
//                  Устанавливает номер страницы тифовского файла               |
// Надо ли говорить, что номер страницы не должен быть больше Npages            |
//-------------------------------------------------------------------------------
    if (LoadTiffFileBitmap == NULL) return;
    if (npage < 0 || npage > LoadTiffFileNpages) return;
    LoadTiffFileBitmap->SelectActiveFrame(&LoadTiffFileDimensionID, npage);
}

void __fastcall ReleaseTiffFile(void) {
//-------------------------------------------------------------------------------
//              Освобождает ресурсы, связанные с загруженным файлом             |
// При этом и fbm тоже освобождается!                                           |
//-------------------------------------------------------------------------------
    if (LoadTiffFileBitmap == NULL) return;
    delete LoadTiffFileBitmap; LoadTiffFileBitmap = NULL;
    LoadTiffFileNpages = 0;
}



/*
PixelFormatUndefined
PixelFormatDontCare
PixelFormat1bppIndexed
PixelFormat4bppIndexed
PixelFormat8bppIndexed
PixelFormat16bppGrayScale
PixelFormat16bppARGB1555
*/

static Gdiplus::REAL ascale;
static Gdiplus::Matrix matr;
static Gdiplus::REAL mat_elements[6];
static Gdiplus::Status gres;

Gdiplus::Bitmap * __fastcall BuildPagedBitmap(Gdiplus::Bitmap *fbm, int iw, int ih, int npage) {
//-------------------------------------------------------------------------------
//                  Создает битмап с очередной страницей                        |
// Полагает, что нужная странца fbm уже установлена                             |
// Если npage == 0, то не выводит ее                                            |
// После использования его надо delete вручную                                  |
//-------------------------------------------------------------------------------
    Gdiplus::Graphics *g;
    int w, h;

    h = fbm->GetHeight();
    w = fbm->GetWidth();

    if (pbm == NULL) {
        pbm = new Gdiplus::Bitmap(iw + 30, ih + 30, PixelFormat32bppARGB);
        pbm1 = new Gdiplus::Bitmap(iw + 30, ih + 30, PixelFormat1bppIndexed);
    }
    g = new Gdiplus::Graphics(pbm);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    g->FillRectangle(&whiteBrush, Gdiplus::Rect(0, 0, iw + 30, ih + 30));

    Gdiplus::Matrix matr;
    matr.Rotate(270.0f);
    g->SetTransform(&matr);

    g->DrawImage(fbm, Gdiplus::Rect(-ih - 30, -30, ih, iw), 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    matr.Reset();
    g->SetTransform(&matr);

    AnsiString snum;
    wchar_t wnum[20];
    snum.printf("%d", npage);
    snum.WideChar(wnum, 20);

    Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
    Gdiplus::PointF origin(iw - 50, 20.0f);
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
    g->DrawString(wnum, -1, &myFont, origin, &blackBrush);

    delete g;
    return pbm;
}

static int * __fastcall FindNoWhite(Gdiplus::BitmapData *b, UINT &x, UINT &y) {
//-------------------------------------------------------------------------------
//              Находит ненулевую точку начиная от указанных координат          |
// Полагает, что 32 бита на пиксель                                             |
// Сами указанные координаты пропускаются                                       |
//-------------------------------------------------------------------------------
    int *ptr;
    while(1) {
        x++;
        if (x >= b->Width) {
            x = 0;
            y++;
        }
        if (y >= b->Height) return NULL;
        ptr = (int *)(b->Scan0) + y * b->Stride + x;
        if (*ptr != 0xFFFFFFFF) break;
    }
    return ptr;
}


void __fastcall _2pbm1(void) {
//-------------------------------------------------------------------------------
//                              Переводит pbm в pbm1                            |
// Полагает, что pbm - 32 бита, pbm1 - 1 бит и что они оба имеют одинаковые     |
// размеры, ширина кратна 32 пиксела                                            |
//-------------------------------------------------------------------------------
    UINT *pix;
    int x, y, w, h;
    unsigned char byte, bit, *pix1;

    w = pbm->GetWidth(); h = pbm->GetHeight();
    Gdiplus::Rect BitsRect(0, 0, w, w);
    Gdiplus::BitmapData *bits = new Gdiplus::BitmapData;
    Gdiplus::BitmapData *bits1 = new Gdiplus::BitmapData;
    sres = pbm->LockBits(&BitsRect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bits);
    sres = pbm1->LockBits(&BitsRect, Gdiplus::ImageLockModeWrite, PixelFormat1bppIndexed, bits1);

    for (y = 0; y < h; y++) {
        pix = (UINT *)((int)bits->Scan0 + bits->Stride * y);
        pix1 = (unsigned char *)((int)bits1->Scan0 + bits1->Stride * y);
        byte = 0;
        for (x = 0; x < w; x++, pix++) {
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

    pbm->UnlockBits(bits); delete bits;
    pbm1->UnlockBits(bits1); delete bits1;
}


static void __fastcall DrawCurFrame1(void) {
//-------------------------------------------------------------------------------
//                  Отрисовывает текущий curFrame битмапа fbm                   |
// Полагает, что уже Gdipulus::Startup(...)                                     |
//-------------------------------------------------------------------------------
/*
    Gdiplus::Graphics *g;
    int w, h;
    int iw, ih;

    h = fbm->GetHeight();
    w = fbm->GetWidth();

    ih = 4961; iw = 7016;
    // ih = 1654; iw = 2339;
    // iw = w; ih = h;
    aImage->ClientHeight = iw + 30;
    aImage->ClientWidth = ih + 30;

    Gdiplus::Bitmap *pbm = new Gdiplus::Bitmap(ih + 30, iw + 30, PixelFormat32bppARGB);
    // g = new Gdiplus::Graphics(aImage->Canvas->Handle);
    g = new Gdiplus::Graphics(pbm);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    g->FillRectangle(&whiteBrush, Gdiplus::Rect(0, 0, ih + 30, iw + 30));
    // g->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    // g->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::Matrix matr;
    matr.GetElements(mat_elements);
    matr.Rotate(270.0f);
    matr.GetElements(mat_elements);

    // Gdiplus::Effect RedEye;
    gres = g->SetTransform(&matr);
    if (gres != Gdiplus::Ok) {
        stop++;
    }

    g->GetTransform(&matr);
    matr.GetElements(mat_elements);

    // g->DrawImage(fbm, Gdiplus::RectF(0, 0, w, h), &matr, &RedEye, NULL, Gdiplus::UnitPixel);
    // g->DrawImage(fbm, 0, 0, h, w);
    g->DrawImage(fbm, Gdiplus::Rect(-iw - 30, -30, iw, ih), 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    matr.Reset();
    matr.GetElements(mat_elements);
    g->SetTransform(&matr);

    AnsiString snum;
    wchar_t wnum[20];
    snum.printf("%d", curFrame + 1);
    snum.WideChar(wnum, 20);

    Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
    // myFont.SetStyle(myFont.GetStyle() | Gdiplus::FontStyleBold);
    Gdiplus::PointF origin(ih - 50, 20.0f);
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
    g->DrawString(wnum, -1, &myFont, origin, &blackBrush);

    Gdiplus::Graphics *gs;
    gs = new Gdiplus::Graphics(aImage->Canvas->Handle);
    gs->DrawImage(pbm, 0, 0, ih + 30, iw + 30);

    delete gs;
    delete pbm;
    delete g;
    aImage->Invalidate();
*/    
    Gdiplus::Graphics *g;
    int w, h;
    int iw, ih;

    h = fbm->GetHeight();
    w = fbm->GetWidth();

    iw = 2100; ih = 2970;
    // ih = 1654; iw = 2339;
    // iw = w; ih = h;
    aImage->ClientHeight = ih + 30;
    aImage->ClientWidth = iw + 30;

    Gdiplus::Bitmap *pbm = new Gdiplus::Bitmap(iw + 30, ih + 30, PixelFormat32bppARGB);
    // g = new Gdiplus::Graphics(aImage->Canvas->Handle);
    g = new Gdiplus::Graphics(pbm);
    Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(255, 255, 255, 255));
    g->FillRectangle(&whiteBrush, Gdiplus::Rect(0, 0, iw + 30, ih + 30));
    // g->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    // g->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    Gdiplus::Matrix matr;
    matr.GetElements(mat_elements);
    matr.Rotate(270.0f);
    matr.GetElements(mat_elements);

    // Gdiplus::Effect RedEye;
    gres = g->SetTransform(&matr);
    if (gres != Gdiplus::Ok) {
        stop++;
    }

    g->GetTransform(&matr);
    matr.GetElements(mat_elements);

    // g->DrawImage(fbm, Gdiplus::RectF(0, 0, w, h), &matr, &RedEye, NULL, Gdiplus::UnitPixel);
    // g->DrawImage(fbm, 0, 0, h, w);
    g->DrawImage(fbm, Gdiplus::Rect(-ih - 30, -30, ih, iw), 0, 0, w, h, Gdiplus::UnitPixel, 0, 0, 0);

    matr.Reset();
    matr.GetElements(mat_elements);
    g->SetTransform(&matr);

    AnsiString snum;
    wchar_t wnum[20];
    snum.printf("%d", curFrame + 1);
    snum.WideChar(wnum, 20);

    Gdiplus::Font myFont(L"Arial", 40, Gdiplus::FontStyleBold);
    // myFont.SetStyle(myFont.GetStyle() | Gdiplus::FontStyleBold);
    Gdiplus::PointF origin(iw - 50, 20.0f);
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
    g->DrawString(wnum, -1, &myFont, origin, &blackBrush);

    Gdiplus::Graphics *gs;
    gs = new Gdiplus::Graphics(aImage->Canvas->Handle);
    gs->DrawImage(pbm, 0, 0, iw + 30, ih + 30);

    delete gs;
    delete pbm;
    delete g;
    aImage->Invalidate();
}








//===============================================================================
//                          Последняя версия TestTiff                           |
//===============================================================================
static Gdiplus::Bitmap *TiffBitmap = NULL;

static void __fastcall DrawCurFrame(void) {
//-------------------------------------------------------------------------------
//                  Отрисовывает текущий curFrame битмапа fbm                   |
// Полагает, что уже Gdipulus::Startup(...)                                     |
//-------------------------------------------------------------------------------
/*
    int iw, ih;
    Gdiplus::Bitmap *pbm;

    // iw = 2114; ih = 2990;
    iw = 2978; ih = 4224;
    // iw = 4258; ih = 6064;
    // iw = 4961; ih = 7061
    // ih = 1654; iw = 2339;
    // iw = w; ih = h;
    aImage->ClientHeight = ih + 30;
    aImage->ClientWidth = iw + 30;

    pbm = BuildPagedBitmap(fbm, iw, ih, curFrame + 1);
    _2pbm1();
    // SaveBitmap2Tiff(pbm);
    // SaveBitmap2Tiff(pbm1);

    Gdiplus::Graphics *gs;
    gs = new Gdiplus::Graphics(aImage->Canvas->Handle);
    gs->DrawImage(pbm1, 0, 0, iw + 30, ih + 30);
    // gs->DrawImage(pbm, 0, 0, iw + 30, ih + 30);

    delete gs;
    // delete pbm;
    aImage->Invalidate();
*/
    Gdiplus::Graphics *gs;
    aImage->ClientHeight = SaveBitmapH;
    aImage->ClientWidth = SaveBitmapW;
    gs = new Gdiplus::Graphics(aImage->Canvas->Handle);
    gs->DrawImage(SaveBitmap2TiffBitmap1, 0, 0, SaveBitmapW, SaveBitmapH);

    delete gs;
    aImage->Invalidate();
}



void __fastcall IncImageIndex(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
    if (fbm == NULL) return;
    if (curFrame >= frameCount - 1) return;
    curFrame++;
    //fbm->RotateFlip(Gdiplus::Rotate90FlipNone);
    fbm->SelectActiveFrame(&DimensionID, curFrame);
    // fbm->RotateFlip(Gdiplus::Rotate270FlipNone);
    DrawCurFrame();
*/
    if (TiffBitmap == NULL) return;
    if (curFrame >= LoadTiffFileNpages - 1) return;
    curFrame++;
    SetTiffFilePage(curFrame);
    SaveBitmap2Tiff(TiffBitmap, curFrame + 1);
    DrawCurFrame();
}

void __fastcall DecImageIndex(void) {
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
/*
    if (fbm == NULL) return;
    if (curFrame == 0) return;
    curFrame--;
    //fbm->RotateFlip(Gdiplus::Rotate90FlipNone);
    fbm->SelectActiveFrame(&DimensionID, curFrame);
    // fbm->RotateFlip(Gdiplus::Rotate270FlipNone);
    DrawCurFrame();
*/
    if (TiffBitmap == NULL) return;
    if (curFrame <= 0) return;
    curFrame--;
    SetTiffFilePage(curFrame);
    SaveBitmap2Tiff(TiffBitmap, curFrame + 1);
    DrawCurFrame();
}


void __fastcall TestTiff(TImage *img) {
//-------------------------------------------------------------------------------
//          Проверяет принципиальную возможность работать с tiff                |
//-------------------------------------------------------------------------------
/*
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    // Initialize GDI+.
    if (!GdiplusStarted) {
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        GdiplusStarted = true;
    }

    if (fbm == NULL) {
        fbm = new Gdiplus::Bitmap(L"c:\\01-01-01.tif");
        sres = fbm->GetLastStatus();
        // fbm = new Gdiplus::Bitmap(L"c:\\01-01-01.tif");
        // fbm = new Gdiplus::Bitmap(L"c:\\01-01-02.tif");
        // fbm = new Gdiplus::Bitmap(L"c:\\p0001.tif");
        // fbm = new Gdiplus::Bitmap(L"c:\\p0001.bmp");
        ggh = fbm->GetHeight();
        ggw = fbm->GetWidth();
        UINT count = 0;
        count = fbm->GetFrameDimensionsCount();
        GUID* pDimensionIDs = (GUID*)malloc(sizeof(GUID)*count);
        // Get the list of frame dimensions from the Image object.
        fbm->GetFrameDimensionsList(pDimensionIDs, count);
        // Display the GUID of the first (and only) frame dimension.
        //WCHAR strGuid[39];
        //StringFromGUID2(pDimensionIDs[0], strGuid, 39);
        //wsprintf(buf, "The first (and only) dimension ID is %s.\n", strGuid);
        // Get the number of frames in the first dimension.
        DimensionID = pDimensionIDs[0];
        // frameCount = fbm->GetFrameCount(&pDimensionIDs[0]);
        frameCount = fbm->GetFrameCount(&DimensionID);
        free(pDimensionIDs);
        // fbm->RotateFlip(Gdiplus::Rotate270FlipNone);
    }

    //UINT size = fbm->GetEncoderParameterListSize(&pInfo->Clsid);
    //if(!size) return;
    //EncoderParameters* params=(EncoderParameters*)malloc(size);
    //bitmap.GetEncoderParameterList(&pInfo->Clsid, size, params);

    aImage = img;

    IniSaveBitmap2Tiff("C:\\Tiff1.tif");
    DrawCurFrame();

    // delete(fbm);
    // Gdiplus::GdiplusShutdown(gdiplusToken);

// unlocker
*/
/*
    Variant Doc, Images, Image, Dirty;
    Doc = CreateOleObject("MODI.Document");
    // Images = CreateOleObject("MODI.Images");
    // Dirty = Doc.OlePropertyGet("Dirty");
    Doc.OleFunction("Create", "c:\\tt.tif");
    Images = Doc.OlePropertyGet("Images");
    stop = Images.OlePropertyGet("Count");
    Image = Images.OlePropertyGet("Item", 1);
*/
    StartGdiplus();

    IniSaveBitmap2Tiff("c:\\SavedBitmap.tif");

    aImage = img;

    // if (TiffBitmap == NULL) TiffBitmap = LoadTiffFile("c:\\01-01-02.tif");
    if (TiffBitmap == NULL) TiffBitmap = LoadTiffFile("c:\\tt.tif");
    // IniSaveBitmap2Tiff("c:\\SavedBitmap.tif");

    curFrame = 0;
    SetTiffFilePage(curFrame);
    SaveBitmap2Tiff(TiffBitmap, curFrame + 1);
    DrawCurFrame();
}


void __fastcall CopyFilesToTiff(void) {
//-------------------------------------------------------------------------------
//                          Копирует группу файлов в tiff                       |
// На самом деле надо их создавать и показывать ход процесса. Но пока так       |
//-------------------------------------------------------------------------------
    StartGdiplus();

    IniSaveBitmap2Tiff("c:\\SavedBitmap.tif");

    int i, k;
    TiffBitmap = LoadTiffFile("c:\\01-01-01.tif");
    for (i = 0, k = 1; i < LoadTiffFileNpages; i++, k++) {
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, k);
    }
    ReleaseTiffFile();
    TiffBitmap = LoadTiffFile("c:\\tt.tif");
    for (i = 0; i < LoadTiffFileNpages; i++, k++) {
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, k);
    }
    ReleaseTiffFile();
    TiffBitmap = LoadTiffFile("c:\\01-01-02.tif");
    for (i = 0; i < LoadTiffFileNpages; i++, k++) {
        SetTiffFilePage(i);
        SaveBitmap2Tiff(TiffBitmap, k);
    }
    ReleaseTiffFile();

    StopSaveBitmap2Tiff();
    StopGdiplus();
}

// DEVMODE




/*
http://forum.vingrad.ru/index.php?showtopic=71155

Как изменить права на просмотр ключа реестра ? Есть ключи, которые видит только SYSTEM

Дело в том, что есть один ключик в реестре, в котором IE хранит данные autocomplete и доступ к нему есть только у SYSTEM.

Для проверки (под XP) открываем regedit, ищем ключик:
HKEY_CURRENT_USER\Software\Microsoft\Protected Storage System Provider\S-1-5-21-2052111302-1682526488-1060284298-500

Выбираем его и что видим ? Ничего. Тыкаем правой кнопкой по S-1-5-21-2052111302-1682526488-1060284298-500, выбираем Permissions (ну или как там по русски будет), там видим только SYSTEM.
Тыкаем добавить пользователя и добавляем, например, администратора, ставим ему Full Control для этого ключа. Тыкаем OK, потом F5 для обновления.

Смотрим теперь на него ! Что видим ? Подразделы Data и Data 2. Тоже самое нужно проделать для них, они тоже не такие пустые, как кажется smile

Так вот как сделать всё это и заполучить доступ к этим ключам через Delphi 7 ?

P.S. У меня есть пример на С++ как это сделать, но он длинный и я не сильно-то знаком с C++ smile

Эта процедура как раз удаляет данные автозаполнения форм от IE, которые и хранятся в ключах, что я описал в начале:




switch m_dwClearSettings
  case CLEAR_AUTOCOMPLETE_FORMS
  {
    if (IsWindows2k() || IsWindowsNT())
    {
      CString sBaseKey;
      SECURITY_DESCRIPTOR NewSD;
      BYTE* pOldSD;
      PACL pDacl = NULL;
      PSID pSid = NULL;
      TCHAR szSid[256];
      if (GetUserSid(&pSid))
      {
        //get the hiden key name
        GetSidString(pSid, szSid);
        sKey = _T("Software\\Microsoft\\Protected Storage System Provider\\");
        sKey += szSid;
        //get old SD
        sBaseKey = sKey;
        GetOldSD(HKEY_CURRENT_USER, sBaseKey, &pOldSD);
        //set new SD and then clear
        if (CreateNewSD(pSid, &NewSD, &pDacl))
        {
          RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);
          sKey += _T("\\Data");
          RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);
          sKey += _T("\\e161255a-37c3-11d2-bcaa-00c04fd929db");
          RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, TRUE);
          dwRet = SHDeleteKey(HKEY_CURRENT_USER, sKey);
        }
        if (pDacl != NULL)
          HeapFree(GetProcessHeap(), 0, pDacl);
        //restore old SD
        if (pOldSD)
        {
          RegSetPrivilege(HKEY_CURRENT_USER, sBaseKey,
            (SECURITY_DESCRIPTOR*)pOldSD, FALSE);
          delete pOldSD;
        }
      }
      if (pSid)
        HeapFree(GetProcessHeap(), 0, pSid);
    }
    //win9x
    DWORD dwSize = MAX_PATH;
    TCHAR szUserName[MAX_PATH];
    GetUserName(szUserName, &dwSize);
    sKey = _T("Software\\Microsoft\\Protected Storage System Provider\\");
    sKey += szUserName;
    sKey += _T("\\Data\\e161255a-37c3-11d2-bcaa-00c04fd929db");
    dwRet = SHDeleteKey(HKEY_LOCAL_MACHINE, sKey);
  }
  case CLEAR_AUTOCOMPLETE_PWDS 
  {
    SHDeleteKey(HKEY_CURRENT_USER, 
      _T("Software\\Microsoft\\Internet Explorer\\IntelliForms"));
  }
}
// Helper functions for the above code...
BOOL GetUserSid(PSID* ppSid)
{
  HANDLE hToken;
  BOOL bRes;
  DWORD cbBuffer, cbRequired;
  PTOKEN_USER pUserInfo;
  // The User's SID can be obtained from the process token
  bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
  if (FALSE == bRes)
  {
    return FALSE;
  }
  // Set buffer size to 0 for first call to determine
  // the size of buffer we need.
  cbBuffer = 0;
  bRes = GetTokenInformation(hToken, TokenUser, NULL, cbBuffer, &cbRequired);
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
  {
    return FALSE;
  }
  // Allocate a buffer for our token user data
  cbBuffer = cbRequired;
  pUserInfo = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, cbBuffer);
  if (NULL == pUserInfo)
  {
    return FALSE;
  }
  // Make the "real" call
  bRes = GetTokenInformation(hToken, TokenUser, pUserInfo, cbBuffer, &cbRequired);
  if (FALSE == bRes) 
  {
    return FALSE;
  }
  // Make another copy of the SID for the return value
  cbBuffer = GetLengthSid(pUserInfo->User.Sid);
  *ppSid = (PSID) HeapAlloc(GetProcessHeap(), 0, cbBuffer);
  if (NULL == *ppSid)
  {
    return FALSE;
  }
  bRes = CopySid(cbBuffer, *ppSid, pUserInfo->User.Sid);
  if (FALSE == bRes)
  {
      HeapFree(GetProcessHeap(), 0, *ppSid);
    return FALSE;
  }
  bRes = HeapFree(GetProcessHeap(), 0, pUserInfo);
  return TRUE;
}
void GetSidString(PSID pSid, LPTSTR szBuffer)
{
  //convert SID to string
  SID_IDENTIFIER_AUTHORITY *psia = ::GetSidIdentifierAuthority( pSid );
  DWORD dwTopAuthority = psia->Value[5];
  _stprintf(szBuffer, _T("S-1-%lu"), dwTopAuthority);
  TCHAR szTemp[32];
  int iSubAuthorityCount = *(GetSidSubAuthorityCount(pSid));
  for (int i = 0; i<iSubAuthorityCount; i++) 
  {
    DWORD dwSubAuthority = *(GetSidSubAuthority(pSid, i));
    _stprintf(szTemp, _T("%lu"), dwSubAuthority);
    _tcscat(szBuffer, _T("-"));
    _tcscat(szBuffer, szTemp);
  }
}
BOOL IsWindowsNT()
{
  BOOL bRet = FALSE;
  BOOL bOsVersionInfoEx;
  OSVERSIONINFOEX osvi;
  // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
  // If that fails, try using the OSVERSIONINFO structure.
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
  {
    // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
      return bRet;
  }
  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion <= 4)
  {
    bRet = TRUE;
  }
  return bRet;
}
BOOL IsWindows2k()
{
  BOOL bRet = FALSE;
  BOOL bOsVersionInfoEx;
  OSVERSIONINFOEX osvi;
  // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
  // If that fails, try using the OSVERSIONINFO structure.
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
  {
    // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
      return bRet;
  }
  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
  {
    bRet = TRUE;
  }
  return bRet;
}
BOOL CGlobal::GetOldSD(HKEY hKey, LPCTSTR pszSubKey, BYTE** pSD)
{
  BOOL bRet = FALSE;
  HKEY hNewKey = NULL;
  DWORD dwSize = 0;
    LONG lRetCode;
  *pSD = NULL;
  lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, READ_CONTROL, &hNewKey);
  if(lRetCode != ERROR_SUCCESS)
    goto cleanup;
  lRetCode = RegGetKeySecurity(hNewKey, 
    (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
  if (lRetCode == ERROR_INSUFFICIENT_BUFFER)
  {
    *pSD = new BYTE[dwSize];
    lRetCode = RegGetKeySecurity(hNewKey, 
      (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
    if(lRetCode != ERROR_SUCCESS)
    {
      delete *pSD;
      *pSD = NULL;
      goto cleanup;
    }
  }
  else if (lRetCode != ERROR_SUCCESS)
    goto cleanup;
  bRet = TRUE; // indicate success
cleanup:
  if (hNewKey)
  {
    RegCloseKey(hNewKey);
  }
    return bRet;
}
BOOL CGlobal::CreateNewSD(PSID pSid, SECURITY_DESCRIPTOR* pSD, PACL* ppDacl)
{
  BOOL bRet = FALSE;
    PSID pSystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
  ACCESS_ALLOWED_ACE* pACE = NULL;
    DWORD dwAclSize;
  DWORD dwAceSize;
    // prepare a Sid representing local system account
    if(!AllocateAndInitializeSid(&sia, 1, SECURITY_LOCAL_SYSTEM_RID,
        0, 0, 0, 0, 0, 0, 0, &pSystemSid))
  {
        goto cleanup;
    }
    // compute size of new acl
    dwAclSize = sizeof(ACL) + 2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + 
    GetLengthSid(pSid) + GetLengthSid(pSystemSid);
    // allocate storage for Acl
    *ppDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
    if(*ppDacl == NULL)
    goto cleanup;
    if(!InitializeAcl(*ppDacl, dwAclSize, ACL_REVISION))
        goto cleanup;
//    if(!AddAccessAllowedAce(pDacl, ACL_REVISION, KEY_WRITE, pSid))
//    goto cleanup;
    // add current user
  dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSid); 
  pACE = (ACCESS_ALLOWED_ACE *)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);
  pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
  pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
  pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
  pACE->Header.AceSize = dwAceSize;
  
  memcpy(&pACE->SidStart, pSid, GetLengthSid(pSid));
  if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
    goto cleanup;
    
    // add local system account
  HeapFree(GetProcessHeap(), 0, pACE);
  pACE = NULL;
  dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSystemSid);
  pACE = (ACCESS_ALLOWED_ACE *)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);
  pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
  pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
  pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
  pACE->Header.AceSize = dwAceSize;
  
  memcpy(&pACE->SidStart, pSystemSid, GetLengthSid(pSystemSid));
  if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
    goto cleanup;
  if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        goto cleanup;
    if(!SetSecurityDescriptorDacl(pSD, TRUE, *ppDacl, FALSE))
        goto cleanup;
  
  bRet = TRUE; // indicate success
cleanup:
  if(pACE != NULL)
    HeapFree(GetProcessHeap(), 0, pACE);
    if(pSystemSid != NULL)
        FreeSid(pSystemSid);
    return bRet;
}
BOOL CGlobal::RegSetPrivilege(HKEY hKey, LPCTSTR pszSubKey,
                              SECURITY_DESCRIPTOR* pSD, BOOL bRecursive)
{
  BOOL bRet = FALSE;
  HKEY hSubKey = NULL;
  LONG lRetCode;
  LPTSTR pszKeyName = NULL;;
  DWORD dwSubKeyCnt;
  DWORD dwMaxSubKey;
  DWORD dwValueCnt;
  DWORD dwMaxValueName;
  DWORD dwMaxValueData;
  DWORD i;
  if (!pszSubKey)
    goto cleanup;
  // open the key for WRITE_DAC access
  lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, WRITE_DAC, &hSubKey);
  if(lRetCode != ERROR_SUCCESS)
    goto cleanup;
  // apply the security descriptor to the registry key
  lRetCode = RegSetKeySecurity(hSubKey, 
    (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, pSD);
  if( lRetCode != ERROR_SUCCESS )
    goto cleanup;
  if (bRecursive)
  {
    // reopen the key for KEY_READ access
    RegCloseKey(hSubKey);
    hSubKey = NULL;
    lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, KEY_READ, &hSubKey);
    if(lRetCode != ERROR_SUCCESS)
      goto cleanup;
    // first get an info about this subkey ...
    lRetCode = RegQueryInfoKey(hSubKey, 0, 0, 0, &dwSubKeyCnt, &dwMaxSubKey,
      0, &dwValueCnt, &dwMaxValueName, &dwMaxValueData, 0, 0);
    if( lRetCode != ERROR_SUCCESS )
      goto cleanup;
    // enumerate the subkeys and call RegTreeWalk() recursivly
    pszKeyName = new TCHAR [MAX_PATH + 1];
    for (i=0 ; i<dwSubKeyCnt; i++)
    {
      lRetCode = RegEnumKey(hSubKey, i, pszKeyName, MAX_PATH + 1);
      if(lRetCode == ERROR_SUCCESS)
      {
        RegSetPrivilege(hSubKey, pszKeyName, pSD, TRUE);
      }
      else if(lRetCode == ERROR_NO_MORE_ITEMS)
      {
        break;
      }
    }
    delete [] pszKeyName ;
  }
  bRet = TRUE; // indicate success
cleanup:
  if (hSubKey)
  {
    RegCloseKey(hSubKey);
  }
    return bRet;
}



*/

