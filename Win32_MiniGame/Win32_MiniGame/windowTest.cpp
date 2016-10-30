#include <windows.h>

#define local_presist static
#define global_variable static
#define internal static

global_variable bool isRunning;

global_variable HDC bitmapDeviceContext;
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMem;
global_variable HBITMAP bitmapHandle;


internal void Win32ResizeDIBSection(int Width, int Height) {
	if (bitmapHandle) {
		DeleteObject(bitmapHandle);
	}
	if (!bitmapDeviceContext) {
		bitmapDeviceContext = CreateCompatibleDC(0);
	}

	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);					// Number of bytes required by the bmiheader structure
	bitmapInfo.bmiHeader.biWidth = Width;									// The bitmap's width
	bitmapInfo.bmiHeader.biHeight = Height;									// The bitmap's height
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;									// Number of bits of each pixel
	bitmapInfo.bmiHeader.biCompression = BI_RGB;							// The compression mode

	CreateDIBSection(bitmapDeviceContext, &bitmapInfo, DIB_RGB_COLORS, &bitmapMem, 0, 0);
}

internal void Win32UpdateWindows(HDC deviceContext, int xSize,int ySize,int Width,int Height) {
	StretchDIBits(deviceContext,
		xSize, ySize, Width, Height,
		xSize, ySize, Width, Height,
		bitmapMem,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}
// Function handles all the message created by the window
LRESULT CALLBACK WindowProcOfMiniGame(
	HWND   Window,
	UINT   Msg,
	WPARAM wParam,
	LPARAM lParam) 
{
	LRESULT callbackResult = 0;

	// Define the message handler
	switch (Msg)
	{
	case WM_QUIT:
	{
		isRunning = false;
	}
		break;

	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(Window, &clientRect);
		int Width = clientRect.right - clientRect.left;
		int Height = clientRect.bottom - clientRect.top;
		Win32ResizeDIBSection(Width, Height);
	}
		break;

	case WM_CREATE:
		OutputDebugStringA("WM_CREATE message received\n");
		break;

	case WM_DESTROY:
	{
		isRunning = false;
	}
	break;

	case WM_ACTIVATEAPP:
		OutputDebugStringA("WM_ACTIVATEAPP message received\n");
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(Window, &paint);
		int xPaint = paint.rcPaint.left;
		int yPaint = paint.rcPaint.top;
		int widthPaint = paint.rcPaint.right - paint.rcPaint.left;
		int heightPaint = paint.rcPaint.bottom - paint.rcPaint.top;
		PatBlt(deviceContext, xPaint, yPaint, widthPaint, heightPaint, BLACKNESS);
		Win32UpdateWindows(deviceContext, xPaint, yPaint, widthPaint, heightPaint);
		EndPaint(Window, &paint);
	}
		break;

	default:
		callbackResult = DefWindowProc(Window, Msg, wParam, lParam);
		break;
	}

	return callbackResult;

}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow) 
{

	// Avoid un-using warning
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInstance);

	WNDCLASS windowsClass = {0};

	// Setting the window's features
    windowsClass.style = CS_HREDRAW | CS_VREDRAW;						// Set the window's type
	windowsClass.lpfnWndProc = WindowProcOfMiniGame;					// Pointer to the window procedure
	windowsClass.hInstance = hInstance;									// Set the instance of the window
//	HICON     hIcon;
//	HBRUSH    hbrBackground;
//	LPCTSTR   lpszMenuName;
	windowsClass.lpszClassName = "MiniGameWindow";						// The name of the window class

	// Register the window class
	if (RegisterClass(&windowsClass)) {
		// Create the window
		HWND windowHandle = CreateWindowEx(
			0,
			windowsClass.lpszClassName,
			"MiniGame",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);

		// The message loop
		if (windowHandle) {
			isRunning = true;
			while (isRunning) {
				MSG Message;
				BOOL messageResult = GetMessage(&Message, 0, 0, 0);
				if (messageResult > 0) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else {
					break;
				}
			}
		}
		else {
			OutputDebugStringA("Creating window failure!\n");
		}
	}
	else {
		OutputDebugStringA("Registering window failure!\n");
	}
	return 0;
}