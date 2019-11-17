#include "../Shared.h"

#include <Windows.h>

#include <iostream>

int arrKeycodes[NUM_CLIPBOARDS] = { VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24 };

bool isBeingHeld = false;

extern "C"
{
	__declspec(dllexport) LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{	
		static int MessageType = RegisterWindowMessageA("MultiClipboard");
		static HWND WindowHandle = FindWindow(CLASS_NAME, WINDOW_NAME);

		if(nCode < 0)
		{
			goto END;
		}

		if (wParam == VK_SHIFT)
		{
			isBeingHeld = !(lParam & 1 << 31);
			goto END;
		}
		// If not repeating
		if (lParam & 0xFFFF0000)
		{
			for (int i = 0; i < NUM_CLIPBOARDS; ++i)
			{
				if (wParam == arrKeycodes[i])
				{
					PostMessage(
						WindowHandle,
						MessageType,
						i,
						isBeingHeld);
				}
			}
		}

		END:
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}