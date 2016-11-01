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
typedef uint16_t uint16;			// Unsigned 16 bit quantity
typedef uint32_t uint32;			// Unsigned 32 bit quantity
typedef uint64_t uint64;			// Unsigned 64 bit quantity

// The back buffer struct
struct win32_back_buffer {
	 BITMAPINFO bitmapInfo;
	 void* bitmapMem;
	 int bitmapWidth;
	 int bitmapHeight;
	 int bytesPerPixel;
};

// The dimension struct of the window
struct win32_window_demension {
	int Width;
	int Height;
};

global_variable bool isRunning;
global_variable win32_back_buffer GlobalBackBuffer;

// Get the window size
internal win32_window_demension Win32GetWindowDemension(HWND Window){
	win32_window_demension ret;
	RECT clientRect;
	GetClientRect(Window, &clientRect);
	ret.Width = clientRect.right - clientRect.left;
	ret.Height = clientRect.bottom - clientRect.top;
	return ret;
}

internal void Win32RenderBufferImage(win32_back_buffer Buffer, int xOffset, int yOffset) {
	int Width = Buffer.bitmapWidth;
	int Height = Buffer.bitmapHeight;
	Buffer.bytesPerPixel = 4;

	int Pitch = Width * Buffer.bytesPerPixel;					// The number of byte of each column

	uint8 *Row = (uint8 *)Buffer.bitmapMem;						// uint8 represent 1 byte
	for (int y = 0; y < Buffer.bitmapHeight; y++) {
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

internal void Win32ResizeDIBSection(win32_back_buffer *Buffer, int Width, int Height) {

	if (Buffer->bitmapMem) {
		VirtualFree(Buffer->bitmapMem, 0, MEM_RELEASE);
	}

	Buffer->bitmapWidth = Width;
	Buffer->bitmapHeight = Height;

	Buffer->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);					// Number of bytes required by the bmiheader structure
	Buffer->bitmapInfo.bmiHeader.biWidth = Buffer->bitmapWidth;									// The bitmap's width
	Buffer->bitmapInfo.bmiHeader.biHeight = -Buffer->bitmapHeight;								// The bitmap's height
	Buffer->bitmapInfo.bmiHeader.biPlanes = 1;
	Buffer->bitmapInfo.bmiHeader.biBitCount = 32;									// Number of bits of each pixel
	Buffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;							// The compression mode
	Buffer->bytesPerPixel = 4;

	int bitmapMemSize = Buffer->bytesPerPixel * Width * Height;
	Buffer->bitmapMem = VirtualAlloc(0, bitmapMemSize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32CopyBufferToWindows(win32_back_buffer Buffer, HDC deviceContext, int xSize, int ySize, int windowWidth, int windowHeight, int width, int height) {
	//TODO: Aspect ratio problem
	StretchDIBits(deviceContext,
		0, 0, windowWidth, windowHeight,
		0, 0, Buffer.bitmapWidth, Buffer.bitmapHeight,
		Buffer.bitmapMem,
		&Buffer.bitmapInfo,
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

		win32_window_demension Demension = Win32GetWindowDemension(Window);
		Win32CopyBufferToWindows(GlobalBackBuffer, deviceContext, Demension.Width, Demension.Height, xPaint, yPaint, widthPaint, heightPaint);
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

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

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
		HWND Window = CreateWindowEx(
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
		if (Window) {
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
				Win32RenderBufferImage(GlobalBackBuffer, xOffset, yOffset);

				HDC deviceContext = GetDC(Window);

				win32_window_demension Demension = Win32GetWindowDemension(Window);
				Win32CopyBufferToWindows(GlobalBackBuffer, deviceContext, 0, 0, Demension.Width, Demension.Height, Demension.Width, Demension.Height);

				ReleaseDC(Window, deviceContext);
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