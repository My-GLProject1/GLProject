#pragma once
#include "Include.h"

class Main
{
public:
	Main();
	~Main();
private:
	HGLRC hRC = NULL;
	HDC hDC = NULL;
	HWND hwnd = NULL;
	HINSTANCE hInstance;

	bool keys[256];
	bool active = TRUE;
	bool fullscreen = TRUE;
};

