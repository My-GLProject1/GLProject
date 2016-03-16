#pragma once
#include "Include.h"

class Main
{
public:
	Main();
	~Main();
	GLvoid KillGLWindow(GLvoid);
	int DrawGLScene(GLvoid);
	int InitGL(GLvoid);
	GLvoid ReSizeGLScene(GLsizei, GLsizei);
	BOOL CreateGLWindow(char*, int, int, int, bool);

private:
	HGLRC hRC = NULL;
	HDC hDC = NULL;
	HWND hwnd = NULL;
	HINSTANCE hInstance;

	bool keys[256];
	bool active = TRUE;
	bool fullscreen = TRUE;
};

