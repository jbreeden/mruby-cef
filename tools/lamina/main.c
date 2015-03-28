#include "Windows.h"
#include "lamina.h"

HINSTANCE app_handle;

int APIENTRY WinMain(HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR    lpCmdLine,
   int       nCmdShow)
{
   app_handle = hInstance;
   return lamina_start(__argc, __argv);
}