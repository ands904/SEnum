//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("SEnumMainUnit.cpp", SEnumMainForm);
USEFORM("SaveUnit.cpp", SaveForm);
USEFORM("ForCanvasUnit.cpp", ForCanvasForm);
USEFORM("ContentsUnit.cpp", ContentsForm);
USEFORM("FindReplaceUnit.cpp", FindReplaceForm);
USEFORM("EditTiffUnit.cpp", EditTiffForm);
USEFORM("TiffImageConfigUnit.cpp", TiffImageConfigForm);
USEFORM("SEnumMainUnitM.cpp", SEnumMainFormM);
//---------------------------------------------------------------------------
#ifdef GUARD
#include "lib/guard/guard.h"
#endif

WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#ifdef GUARD
    GuardFunc1();
#endif
    try
    {
#ifdef GUARD
        read_lic();
#endif

         Application->Initialize();
         Application->Title = "Нумерация страниц смет";
         Application->CreateForm(__classid(TSEnumMainForm), &SEnumMainForm);
         Application->CreateForm(__classid(TSEnumMainFormM), &SEnumMainFormM);
         Application->CreateForm(__classid(TSaveForm), &SaveForm);
         Application->CreateForm(__classid(TForCanvasForm), &ForCanvasForm);
         Application->CreateForm(__classid(TContentsForm), &ContentsForm);
         Application->CreateForm(__classid(TFindReplaceForm), &FindReplaceForm);
         Application->CreateForm(__classid(TEditTiffForm), &EditTiffForm);
         Application->CreateForm(__classid(TTiffImageConfigForm), &TiffImageConfigForm);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
