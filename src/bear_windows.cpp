// This is the windows specific CPP file. Compile this and 
// you'll compile the game for windows. It's as easy as that.
#include <windows.h>
#include "bear_main.cpp"

#ifdef asddas //__DEBUG 
int main(int varc, char *varv[])
#else
int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow
)
#endif
{
	DEBUG_LOG("Windows launch!");
	return(bear_main());
}

