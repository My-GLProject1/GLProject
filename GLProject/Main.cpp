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
int LoadGLTextures(char*, int);
GLvoid DrawFloor();
GLvoid glDrawObject();
void ProcessKeyboard();

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hwnd = NULL;
HINSTANCE hInstance;

bool fullscreen = TRUE;
bool active = TRUE;
bool keys[256];

GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat LightPosition[] = { 4.0f, 4.0f, 6.0f, 1.0f };

GLfloat xrot = 0.0f;
GLfloat yrot = 0.0f;
GLfloat xspeed = 0.0f;
GLfloat yspeed = 0.0f;
GLfloat z = -7.0f;
GLfloat height = 2.0f;

GLUquadricObj *quadratic;

GLuint texture[3];

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

	if (!LoadGLTextures("Data/Envwall.bmp", 0)) {
		return FALSE;
	}
	if (!LoadGLTextures("Data/Ball.bmp", 1)) {
		return FALSE;
	}
	if (!LoadGLTextures("Data/Envroll.bmp", 2)) {
		return FALSE;
	}

	glShadeModel(GL_SMOOTH);
	glClearColor(0.2f, 0.5f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	quadratic = gluNewQuadric();
	gluQuadricNormals(quadratic, GLU_SMOOTH);
	gluQuadricTexture(quadratic, GL_TRUE);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	return TRUE;
}

int DrawGLScene(GLvoid) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	double eqr[] = { 0.0f, -1.0f, 0.0f, 0.0f };
	glLoadIdentity();
	glTranslatef(0.0f, -1.0f, z);

	glColorMask(0, 0, 0, 0);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glDisable(GL_DEPTH_TEST);
	DrawFloor();

	glEnable(GL_DEPTH_TEST);
	glColorMask(1, 1, 1, 1);
	glStencilFunc(GL_EQUAL, 1, 1);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_CLIP_PLANE0);

	glClipPlane(GL_CLIP_PLANE0, eqr);
	glPushMatrix();
		glScalef(1.0f, -1.0f, 1.0f);
		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
		glTranslatef(0.0f, height, 0.0f);
		glRotatef(xrot, 1.0f, 0.0f, 0.0f);
		glRotatef(yrot, 0.0f, 1.0f, 0.0f);
		glDrawObject();
	glPopMatrix();
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_STENCIL_TEST);

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawFloor();

	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glTranslatef(0.0f, height, 0.0f);
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	glDrawObject();

	xrot += xspeed;
	yrot += yspeed;
	glFlush();

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
		16, 1, 0,
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
					ProcessKeyboard();
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

int LoadGLTextures(char* text, int num) {

	int width, height;
	unsigned char* image = SOIL_load_image(text, &width, &height, 0, SOIL_LOAD_RGB);
	
	if (image == NULL) {
		MessageBox(NULL, "Error Loading Texture.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return FALSE;
	}
	glGenTextures(1, &texture[num]);

	glBindTexture(GL_TEXTURE_2D, texture[num]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	return TRUE;
}

GLvoid glDrawObject() {

	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	gluSphere(quadratic, 0.35f, 32, 16);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	gluSphere(quadratic, 0.35f, 32, 16);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_BLEND);

}

GLvoid DrawFloor() {

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-2.0f, 0.0f, 2.0f);

			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-2.0f, 0.0f, -2.0f);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(2.0f, 0.0f, -2.0f);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(2.0f, 0.0f, 2.0f);
	glEnd();
}

void ProcessKeyboard() {
	if (keys[VK_RIGHT]) yspeed += 0.008f;
	if (keys[VK_LEFT]) yspeed -= 0.008f;
	if (keys[VK_UP]) xspeed += 0.008f;
	if (keys[VK_DOWN]) xspeed -= 0.008f;

	if (keys['A']) z += 0.005f;
	if (keys['Z']) z -= 0.005f;

	if (keys[VK_PRIOR]) height += 0.003f;
	if (keys[VK_NEXT]) height -= 0.003f;
}