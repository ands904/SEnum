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
    HANDLE *handles;            // ������ �������, ������ �������������, � ��������� NULL
    COLORREF *pixels;           // ������ ��������, ������ � ������� handle != NULL
    int Count;                  // ������� ������ �������� ������ ���������

    TSnapShot(void);
    ~TSnapShot(void);
    void __fastcall Scan(void);                       // ������ ������ ������ (�� ���������� ����!)
    void __fastcall Index2XY(TPoint &pt, int index);  // ��������� ����� � ���������� �� ������
    int __fastcall Compare(TSnapShot *ss, int start); // ���������� � ��������� ���������
    void __fastcall Accept(TSnapShot *ss);            // �������� ���� ��������� �������
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
//   ������ ��������� ������ ������ � ������ ������������� ��� TestAcadWindow   |
// ����� ����� ���� ��������� ������������� ����, ��� ����� ��� ������ �����    |
// ��������� �����                                                              |
//-------------------------------------------------------------------------------



void __fastcall StopTestAcadWindow(void);
//-------------------------------------------------------------------------------
//        �� ���� ���, ���� ����������, ����� ������ �� ���������               |
//-------------------------------------------------------------------------------


#endif
