#include <windows.h>
#include <stdint.h>

#define local_presist static
#define global_variable static
#define internal static

typedef int8_t int8;				// Signed 8 bit quantity
typedef int16_t int16;				// Signed 16 bit quantity
typedef int32_t int32;				// Signed 32 bit quantity
typedef int64_t int64;				// Signed 64 bit quantity

typedef uint8_t uint8;				// Unsigned 8 bit quantity
typedef uint16_t uint16;				// Unsigned 16 bit quantity
typedef uint32_t uint32;				// Unsigned 32 bit quantity
typedef uint64_t uint64;				// Unsigned 64 bit quantity


global_variable bool isRunning;

global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMem;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;


internal void renderImage(int xOffset, int yOffset) {
	int Width = bitmapWidth;
	int Height = bitmapHeight;
	int Pitch = Width * bytesPerPixel;							// The number of byte of each column

	uint8 *Row = (uint8 *)bitmapMem;							// uint8 represent 1 byte
	for (int y = 0; y < bitmapHeight; y++) {
		uint32 *Pixel = (uint32 *)Row;							// 1 pixel contains 4 bytes -> uint32
		for (int x = 0; x < Width; x++) {
			uint8 Blue = (x + xOffset);
			uint8 Green = (y + yOffset);
			uint8 Red = (x + xOffset);

			*Pixel = ((Red << 16) | (Green << 8));
			Pixel++;
		}
		Row += Pitch;
	} 
}

internal void Win32ResizeDIBSection(int Width, int Height) {

	if (bitmapMem) {
		VirtualFree(bitmapMem, 0, MEM_RELEASE);
	}

	bitmapWidth = Width;
	bitmapHeight = Height;

	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);					// Number of bytes required by the bmiheader structure
	bitmapInfo.bmiHeader.biWidth = Width;									// The bitmap's width
	bitmapInfo.bmiHeader.biHeight = -Height;								// The bitmap's height
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;									// Number of bits of each pixel
	bitmapInfo.bmiHeader.biCompression = BI_RGB;							// The compression mode

	int bitmapMemSize = bytesPerPixel * Width * Height;
	bitmapMem = VirtualAlloc(0, bitmapMemSize, MEM_COMMIT, PAGE_READWRITE);
	//renderImage(0, 0);
}

internal void Win32UpdateWindows(HDC deviceContext, RECT *windowSize, int xSize, int ySize, int Width, int Height) {
	int windowWidth = windowSize->right - windowSize->left;
	int windowHeight = windowSize->bottom - windowSize->top;
	StretchDIBits(deviceContext,
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight,
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

		RECT clientRect;
		GetClientRect(Window, &clientRect);

		Win32UpdateWindows(deviceContext, &clientRect, xPaint, yPaint, widthPaint, heightPaint);
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
			int xOffset = 0;
			int yOffset = 0;
			while (isRunning) {
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
					if (Message.message == WM_QUIT) {
						isRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				renderImage(xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);
				RECT clientRect;
				GetClientRect(windowHandle, &clientRect);
				int windowWidth = clientRect.right - clientRect.left;
				int windowHeight = clientRect.bottom - clientRect.top;
				Win32UpdateWindows(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);

				ReleaseDC(windowHandle, deviceContext);
				xOffset++;
				yOffset++;
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