#include "Main.h"
#include "Include.h"

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

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hwnd = NULL;
HINSTANCE hInstance;

bool fullscreen = TRUE;
bool active = TRUE;
bool keys[256];

GLfloat xrot;
GLfloat yrot;
GLfloat zrot;

GLuint texture[1];

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

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return TRUE;
}

int DrawGLScene(GLvoid) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	glRotatef(zrot, 0.0f, 0.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBegin(GL_QUADS);
		// Front Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 1.0f);
		// Back Face
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		// Top Face
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
		// Bottom Face
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f,- 1.0f, 1.0f);
		// Right Face
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

	xrot += 0.3f;
	yrot += 0.2f;
	zrot += 0.4f;
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
				}
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

	KillGLWindow();
	return (msg.wParam);
}

int LoadGLTextures() {

	int width, height;
	unsigned char* image = SOIL_load_image("frowny.bmp", &width, &height, 0, SOIL_LOAD_RGB);
	if (image == NULL) {
		MessageBox(NULL, TEXT("Error loading texture image"), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	glGenTextures(1, &texture[0]);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return TRUE;
}