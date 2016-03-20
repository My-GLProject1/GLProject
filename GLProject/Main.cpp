#include "Main.h"
#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include "SOIL.h"

Main::Main() {}

Main::~Main() {}

GLvoid KillGLWindow(GLvoid);
int DrawGLScene(GLvoid);
int InitGL(GLvoid);
static GLvoid ReSizeGLScene(GLsizei, GLsizei);
BOOL CreateGLWindow(char*, int, int, int, bool);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
int LoadGLTextures();
GLvoid BuildLists();
GLvoid glPrint(const char*, ...);
GLvoid KillFont(GLvoid);
GLvoid BuildFont(GLvoid);

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hwnd = NULL;
HINSTANCE hInstance;

GLuint base;
GLfloat rot;

bool fullscreen = TRUE;
bool active = TRUE;
bool keys[256];

GLuint texture[1];
GLuint box, top, xloop, yloop;

GLfloat xrot, yrot;

static GLfloat boxcol[5][3] = {
	{ 1.0f,0.0f,0.0f },{ 1.0f,0.5f,0.0f },{ 1.0f,1.0f,0.0f },{ 0.0f,1.0f,0.0f },{ 0.0f,1.0f,1.0f }
};

static GLfloat topcol[5][3] = {
	{ .5f,0.0f,0.0f },{ 0.5f,0.25f,0.0f },{ 0.5f,0.5f,0.0f },{ 0.0f,0.5f,0.0f },{ 0.0f,0.5f,0.5f }
};

GLvoid ReSizeGLScene(GLsizei width, GLsizei height) {
	if (height == 0) {
		height = 1;
	}

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid) {

	if (!LoadGLTextures()) {
		return FALSE;
	}
	BuildLists();
	BuildFont();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	return TRUE;
}

int DrawGLScene(GLvoid) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (yloop = 1; yloop < 6; yloop++) {
		for (xloop = 0; xloop < yloop; xloop++) {
			glLoadIdentity();
			glTranslatef(1.4f + (float(xloop) * 2.8f) - (float(yloop) * 1.4f), ((6.0f - float(yloop)) * 2.4f) - 7.0f, -20.0f);

			glRotatef(45.0f - (2.0f*yloop) + xrot, 1.0f, 0.0f, 0.0f);
			glRotatef(45.0f + yrot, 0.0f, 1.0f, 0.0f);

			glColor3fv(boxcol[yloop - 1]);
			glCallList(box);

			glColor3fv(topcol[yloop - 1]);
			glCallList(top);
		}
	}
	glLoadIdentity();
	glTranslatef(1.1f*float(cos(rot / 16.0f)), 0.8f*float(sin(rot / 20.0f)), -3.0f);
	glRotatef(rot, 1.0f, 0.0f, 0.0f);
	glRotatef(rot*1.2f, 0.0f, 1.0f, 0.0f);
	glRotatef(rot*1.4f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.35f, -0.35f, 0.1f);
	glPrint("N");
	rot += 0.01f;
	return TRUE;
}

GLvoid KillGLWindow(GLvoid) {
	if (fullscreen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}
	if (hRC) {
		if (!wglMakeCurrent(NULL, NULL)) {
			MessageBox(NULL, "Release of DC and RC FAILED.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(hRC)) {
			MessageBox(NULL, "Release Rendering context FAILED.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;
	}
	if (hDC && !ReleaseDC(hwnd, hDC)) {
		MessageBox(NULL, "Release Device context FAILED", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	if (hwnd && !DestroyWindow(hwnd)) {
		MessageBox(NULL, "Could Not release HWND.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hwnd = NULL;
	}
	if (!UnregisterClass("OpenGL", hInstance)) {
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}

	KillFont();
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenFlag) {

	GLuint PixelFormat;

	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;

	RECT WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;

	fullscreen = fullscreenFlag;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, "FAILED to Register the window class.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if (fullscreen) {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			if (MessageBox(NULL, "The Requested FullScreen mode is Not supported by \nyour video card. Use Windowed mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				fullscreen = FALSE;
			}
			else {
				MessageBox(NULL, "Program will now close.", "ERROR", MB_OK | MB_ICONINFORMATION);
				return FALSE;
			}
		}
	}

	if (fullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
	}
	else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if (!(hwnd = CreateWindowEx(dwExStyle,
		"OpenGL", title,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, 0, 0,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top,
		NULL, NULL, hInstance, NULL))) {
		KillGLWindow();
		MessageBox(NULL, "Window Creation Error.", "Error", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1, PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		16, 0, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	if (!(hDC = GetDC(hwnd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		KillGLWindow();
		MessageBox(NULL, "Can't Find a Suitable PixelFormat", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		KillGLWindow();
		MessageBox(NULL, "Can't Set the PixelFormat", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if (!(hRC = wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL, "Can't create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC, hRC)) {
		KillGLWindow();
		MessageBox(NULL, "Can't activiate the GL rendering Context.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	ReSizeGLScene(width, height);

	if (!InitGL()) {
		KillGLWindow();
		MessageBox(NULL, "Initialization FAILED", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_ACTIVATE:
		if (!HIWORD(wParam)) {
			active = TRUE;
		}
		else {
			active = FALSE;
		}

		return 0;
	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		keys[wParam] = TRUE;
		return 0;
	case WM_KEYUP:
		keys[wParam] = FALSE;
		return 0;
	case WM_SIZE:
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	MSG msg;
	BOOL done = FALSE;

	if (MessageBox(NULL, "Would You Like to Run in fullscreen mode?", "Start Fullscreen?", MB_YESNO | MB_ICONQUESTION) == IDNO) {
		fullscreen = FALSE;
	}

	if (!CreateGLWindow("NeHe's OpenGL Framework", 640, 480, 16, fullscreen)) {
		return 0;
	}

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				done = TRUE;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (active) {
				if (keys[VK_ESCAPE]) {
					done = TRUE;
				}
				else {
					DrawGLScene();
					SwapBuffers(hDC);
					if (keys[VK_LEFT]) {
						yrot -= 0.02f;
					}
					if (keys[VK_RIGHT]) {
						yrot += 0.02f;
					}
					if (keys[VK_UP]) {
						xrot -= 0.02f;
					}
					if (keys[VK_DOWN]) {
						xrot += 0.02f;
					}
					if (keys[VK_F1]) {
						keys[VK_F1] = FALSE;
						KillGLWindow();
						fullscreen = !fullscreen;
						if (!CreateGLWindow("NeHe's OpenGL Framework", 640, 480, 16, fullscreen)) {
							return 0;
						}
					}
				}
			}
		}
	}

	KillGLWindow();
	return (msg.wParam);
}

int LoadGLTextures() {

	int width, height;
	unsigned char* image = SOIL_load_image("Data/Lights.bmp", &width, &height, 0, SOIL_LOAD_RGB);
	
	if (image == NULL) {
		MessageBox(NULL, "Error Loading Texture.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	glGenTextures(1, &texture[0]);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	return TRUE;
}

GLvoid BuildLists() {
	box = glGenLists(2);
	glNewList(box, GL_COMPILE);

	glBegin(GL_QUADS);
		// Bottom Face
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		// Front Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f); 
		 // Back Face
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		// Right face
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		// Left Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
	glEndList();
	top = box + 1;

	glNewList(top, GL_COMPILE);
	glBegin(GL_QUADS);
		// Top Face
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();
	glEndList();
}

GLvoid BuildFont(GLvoid) {

	GLYPHMETRICSFLOAT gmf[256];
	HFONT font;
	base = glGenLists(96);

	font = CreateFont(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SYMBOL_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "WingDings");
	SelectObject(hDC, font);
	wglUseFontOutlines(hDC, 0, 255, base, 0.1f, 0.2f, WGL_FONT_POLYGONS, gmf);
}

GLvoid KillFont(GLvoid) {
	glDeleteLists(base, 96);
}

GLvoid glPrint(const char* text, ...) {

	if (text == NULL) {
		return;
	}

	glPushAttrib(GL_LIST_BIT);
	glListBase(base);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}