#include <Windows.h>
#include <shellapi.h>
#if _DEBUG
#include <stdio.h>
#endif // _DEBUG

#define WM_TRAYICON (WM_APP + 1)
#define IDM_EXIT    1001

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

typedef struct {
	BOOL popup;
} Settings;

void ShowError(LPCSTR message);
DWORD GetOSVersion();
HICON CreateCircleIcon(COLORREF color, BOOL ring);
void UpdateTrayIcon();
void PressKey(int keyCode);
void ReleaseKey(int keyCode);
void SynthesizeAltShift();
void ToggleCapsLockState();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


HHOOK hHook;
HWND hWnd;
BOOL enabled = TRUE;
BOOL keystrokeCapsProcessed = FALSE;
BOOL keystrokeShiftProcessed = FALSE;
BOOL winPressed = FALSE;
HICON hIconEnabled;
HICON hIconDisabled;
NOTIFYICONDATA nid;
UINT WM_TASKBARCREATED;

Settings settings = {
	.popup = FALSE
};


int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "nopopup") == 0)
	{
		settings.popup = FALSE;
	}
	else
	{
		settings.popup = GetOSVersion() >= 10;
	}
#if _DEBUG
	printf("Pop-up is %s\n", settings.popup ? "enabled" : "disabled");
#endif

	HANDLE hMutex = CreateMutex(0, 0, "Switchy");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ShowError("Another instance of Switchy is already running!");
		return 1;
	}

	WNDCLASS wc = {0};
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = GetModuleHandle(NULL);
	wc.lpszClassName = "SwitchyWindow";
	RegisterClass(&wc);

	hWnd = CreateWindow("SwitchyWindow", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);

	hIconEnabled  = CreateCircleIcon(RGB(220, 220, 220), FALSE);
	hIconDisabled = CreateCircleIcon(RGB(90, 90, 90), TRUE);

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = hWnd;
	nid.uID              = 1;
	nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon            = hIconEnabled;
	lstrcpy(nid.szTip, "Switchy - Enabled");
	Shell_NotifyIcon(NIM_ADD, &nid);

	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
	if (hHook == NULL)
	{
		Shell_NotifyIcon(NIM_DELETE, &nid);
		ShowError("Error calling \"SetWindowsHookEx(...)\"");
		return 1;
	}

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	UnhookWindowsHookEx(hHook);
	Shell_NotifyIcon(NIM_DELETE, &nid);
	DestroyIcon(hIconEnabled);
	DestroyIcon(hIconDisabled);

	return 0;
}


void ShowError(LPCSTR message)
{
	MessageBox(NULL, message, "Error", MB_OK | MB_ICONERROR);
}


DWORD GetOSVersion()
{
	HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
	RTL_OSVERSIONINFOW osvi = { 0 };

	if (hMod)
	{
		RtlGetVersionPtr p = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");

		if (p)
		{
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			p(&osvi);
		}
	}

	return osvi.dwMajorVersion;
}


HICON CreateCircleIcon(COLORREF color, BOOL ring)
{
	BYTE maskBits[32] = {0};
	DWORD colorBits[16 * 16] = {0};

	float cx = 7.5f, cy = 7.5f;
	float outerR2 = 6.5f * 6.5f;
	float innerR2 = ring ? (4.0f * 4.0f) : 0.0f;
	DWORD pixel = GetBValue(color) | ((DWORD)GetGValue(color) << 8) | ((DWORD)GetRValue(color) << 16);

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			float dx = x - cx, dy = y - cy;
			float d2 = dx * dx + dy * dy;
			if (d2 <= outerR2 && d2 >= innerR2)
				colorBits[y * 16 + x] = pixel;
			else
				maskBits[y * 2 + x / 8] |= (1 << (7 - x % 8));
		}
	}

	HBITMAP hMask = CreateBitmap(16, 16, 1, 1, maskBits);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = 16;
	bmi.bmiHeader.biHeight      = -16;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	DWORD* bits;
	HDC hdc = GetDC(NULL);
	HBITMAP hColor = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	ReleaseDC(NULL, hdc);

	for (int i = 0; i < 16 * 16; i++) bits[i] = colorBits[i];

	ICONINFO ii = {TRUE, 0, 0, hMask, hColor};
	HICON hIcon = CreateIconIndirect(&ii);

	DeleteObject(hMask);
	DeleteObject(hColor);

	return hIcon;
}


void UpdateTrayIcon()
{
	nid.hIcon = enabled ? hIconEnabled : hIconDisabled;
	lstrcpy(nid.szTip, enabled ? "Switchy - Enabled" : "Switchy - Disabled");
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_TASKBARCREATED)
	{
		Shell_NotifyIcon(NIM_ADD, &nid);
		return 0;
	}

	switch (msg)
	{
	case WM_TRAYICON:
		if (lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, IDM_EXIT, "Exit");
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
			DestroyMenu(hMenu);
		}
		return 0;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDM_EXIT)
		{
			PostQuitMessage(0);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void PressKey(int keyCode)
{
	INPUT input = {0};
	input.type   = INPUT_KEYBOARD;
	input.ki.wVk = (WORD)keyCode;
	SendInput(1, &input, sizeof(INPUT));
}


void ReleaseKey(int keyCode)
{
	INPUT input = {0};
	input.type       = INPUT_KEYBOARD;
	input.ki.wVk     = (WORD)keyCode;
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}


void SynthesizeAltShift()
{
	INPUT inputs[4] = {0};
	inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_MENU;
	inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = VK_LSHIFT;
	inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = VK_MENU;   inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
	inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.wVk = VK_LSHIFT; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(4, inputs, sizeof(INPUT));
}


void ToggleCapsLockState()
{
	INPUT inputs[2] = {0};
	inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_CAPITAL;
	inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = VK_CAPITAL; inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, inputs, sizeof(INPUT));
#if _DEBUG
	printf("Caps Lock state has been toggled\n");
#endif // _DEBUG
}


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
	if (nCode == HC_ACTION && !(key->flags & (LLKHF_INJECTED | LLKHF_LOWER_IL_INJECTED)))
	{
#if _DEBUG
		const char* keyStatus = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? "pressed" : "released";
		printf("Key %d has been %s\n", key->vkCode, keyStatus);
#endif // _DEBUG
		if (key->vkCode == VK_CAPITAL)
		{
			if (wParam == WM_SYSKEYDOWN && !keystrokeCapsProcessed)
			{
				keystrokeCapsProcessed = TRUE;
				enabled = !enabled;
				UpdateTrayIcon();
#if _DEBUG
				printf("Switchy has been %s\n", enabled ? "enabled" : "disabled");
#endif // _DEBUG
				return 1;
			}

			if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
			{
				keystrokeCapsProcessed = FALSE;

				if (winPressed)
				{
					winPressed = FALSE;
					ReleaseKey(VK_LWIN);
				}

				if (enabled && !settings.popup)
				{
					if (!keystrokeShiftProcessed)
					{
						SynthesizeAltShift();
					}
				}

				keystrokeShiftProcessed = FALSE;
			}

			if (!enabled)
			{
				return CallNextHookEx(hHook, nCode, wParam, lParam);
			}

			if (wParam == WM_KEYDOWN && !keystrokeCapsProcessed)
			{
				keystrokeCapsProcessed = TRUE;

				if (keystrokeShiftProcessed == TRUE)
				{
					ToggleCapsLockState();
					return 1;
				}
				else
				{
					if (settings.popup)
					{
						PressKey(VK_LWIN);
						PressKey(VK_SPACE);
						ReleaseKey(VK_SPACE);
						winPressed = TRUE;
					}
				}
			}
			return 1;
		}

		else if (key->vkCode == VK_LSHIFT)
		{

			if ((wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && !keystrokeCapsProcessed)
			{
				keystrokeShiftProcessed = FALSE;
			}

			if (!enabled)
			{
				return CallNextHookEx(hHook, nCode, wParam, lParam);
			}

			if (wParam == WM_KEYDOWN && !keystrokeShiftProcessed)
			{
				keystrokeShiftProcessed = TRUE;

				if (keystrokeCapsProcessed == TRUE)
				{
					ToggleCapsLockState();
					if (settings.popup)
					{
						PressKey(VK_LWIN);
						PressKey(VK_SPACE);
						ReleaseKey(VK_SPACE);
						winPressed = TRUE;
					}

					return 1;
				}
			}
			return CallNextHookEx(hHook, nCode, wParam, lParam);
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
