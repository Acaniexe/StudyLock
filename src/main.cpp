#include <windows.h>
#include "lockscreen.h"
#include "taskmanager.h"

static HHOOK g_keyboardHook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        if (kb->vkCode == VK_LWIN || kb->vkCode == VK_RWIN)
            return 1;

        if (kb->vkCode == VK_F4 || kb->vkCode == VK_TAB || kb->vkCode == VK_ESCAPE)
        {
            if (GetAsyncKeyState(VK_MENU) & 0x8000)
                return 1;
        }

        if (kb->vkCode == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
            return 1;
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

void DisableTaskManager(bool disable)
{
    HKEY hKey;
    const char* path = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";

    if (RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        if (disable)
        {
            DWORD value = 1;
            RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
        }
        else
        {
            RegDeleteValueA(hKey, "DisableTaskMgr");
        }
        RegCloseKey(hKey);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    DisableTaskManager(true);

    TaskManager taskManager;
    Task currentTask = taskManager.GetRandomTask();

    LockScreen lockScreen(hInstance, currentTask);
    lockScreen.Run();

    DisableTaskManager(false);

    if (g_keyboardHook)
        UnhookWindowsHookEx(g_keyboardHook);

    return 0;
}