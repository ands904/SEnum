//---------------------------------------------------------------------------

#ifndef TCatcherUnitSEnumH
#define TCatcherUnitSEnumH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------

#define SNAPSHOT_MAXX   4000
#define SNAPSHOT_MAXY   2000
#define SNAPSHOT_DX     25
#define SNAPSHOT_DY     15

struct TSnapShot {
    HANDLE *handles;            // массив хэндлов, только автокадовских, у остальных NULL
    COLORREF *pixels;           // массив пикселей, только у которых handle != NULL
    int Count;                  // Сколько вообще подлежит делать снэпшотов

    TSnapShot(void);
    ~TSnapShot(void);
    void __fastcall Scan(void);                       // Делает снимок экрана (тк акадовские окна!)
    void __fastcall Index2XY(TPoint &pt, int index);  // Переводит номер в координату на экране
    int __fastcall Compare(TSnapShot *ss, int start); // Сравнивает с указанным снэпшотом
    void __fastcall Accept(TSnapShot *ss);            // Копирует себе указанный снэпшот
};




class TCatcher : public TThread
{
  typedef struct tagTHREADNAME_INFO
  {
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
  } THREADNAME_INFO;
private:
  void SetName();
protected:
    void __fastcall Execute();
public:
    __fastcall TCatcher(bool CreateSuspended);
};
//---------------------------------------------------------------------------



void __fastcall IniTestAcadWindow(Variant Acad);
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------


void __fastcall StartTestAcadWindow(void);
//-------------------------------------------------------------------------------
//   Делает начальный снимок экрана и прочие приготовления для TestAcadWindow   |
// Сразу после надо открывать автокадовский файл, ибо иначе вся работа будет    |
// крутиться втуне                                                              |
//-------------------------------------------------------------------------------



void __fastcall StopTestAcadWindow(void);
//-------------------------------------------------------------------------------
//        Ну типа все, файл загрузился, можно дальше не проверять               |
//-------------------------------------------------------------------------------


#endif
