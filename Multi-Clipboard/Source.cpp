#include "../Shared.h"

#include <Windows.h>

#include <iostream>

/**
* TODO: LEARN HOW TO DO HOTKEYS HERE SO I DON'T HAVE TO USE AUTOHOTKEY RegisterHotKey()
* OR FIGURE OUT HOW TO SEND WM_COPY/WM_PASTE TO FOCUSED WINDOW (GetActiveWindow() -- Send message to self inside the DLL and wait for response (likely not work)
* else send "Copy" message to this window before sending current "KeyPressed" message which will send a WM_COPY and wait before doing clipboard shit IDK)
*/

HOOKPROC hkprcSysMsg;
HINSTANCE hinstDLL;
HHOOK hhookSysMsg;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hwnd;

HGLOBAL arrClipboardHandle[NUM_CLIPBOARDS] = {};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		WINDOW_NAME,					// Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, 0);

	hinstDLL = LoadLibrary(TEXT("DLL.dll"));

	hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "KeyboardProc");

	//Set hook to listen for keyboard input
	HHOOK hHook = SetWindowsHookExA(WH_KEYBOARD, hkprcSysMsg, hinstDLL, 0);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}
	
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT UniqueMessage = RegisterWindowMessageA("MultiClipboard");
	if (uMsg == UniqueMessage)
	{
		if (!OpenClipboard(hwnd))
		{
			return 0;
		}
		// If shift was held
		if (lParam)
		{
			// Save Data (Copy)
			HANDLE clipboardHandle = GetClipboardData(CF_TEXT);
			int size = GlobalSize(clipboardHandle);
			if (size > CLIPBOARD_SIZE)
			{
				exit(1);
			}

			if (arrClipboardHandle[wParam])
			{
				GlobalFree(arrClipboardHandle[wParam]);
			}
			arrClipboardHandle[wParam] = GlobalAlloc(GMEM_FIXED, size);
			
			memcpy(arrClipboardHandle[wParam], clipboardHandle, size);
		}
		else
		{
			// Set Data (Paste)
			if (arrClipboardHandle[wParam])
			{
				int size = GlobalSize(arrClipboardHandle[wParam]);
				HANDLE clipboardHandle = GlobalAlloc(GMEM_FIXED, size);

				memcpy(clipboardHandle, arrClipboardHandle[wParam], size);

				EmptyClipboard();
				SetClipboardData(CF_TEXT, clipboardHandle);
			}
		}
		
		CloseClipboard();
		return 0;
	}

	END:
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
