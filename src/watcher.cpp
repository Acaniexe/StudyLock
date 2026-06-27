#include <windows.h>
#include <string>

static HWND g_hwnd = NULL;

static std::string GetExeDir()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string s(path);
    size_t pos = s.find_last_of("\\/");
    return (pos != std::string::npos) ? s.substr(0, pos) : ".";
}

static void LaunchGauntlet()
{
    std::string exePath = GetExeDir() + "\\gauntlet.exe";

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    CreateProcessA(
        exePath.c_str(),
        NULL, NULL, NULL,
        FALSE,
        0, NULL, NULL,
        &si, &pi);

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}

static void InstallStartup()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    HKEY hKey;
    RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    RegSetValueExA(hKey, "GauntletWatcher", 0, REG_SZ,
        (BYTE*)path, (DWORD)(strlen(path) + 1));

    RegCloseKey(hKey);

    MessageBoxA(NULL,
        "Gauntlet Watcher add to startup.\nIt will launch automatically on login.",
    "Gauntlet", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_POWERBROADCAST)
    {
        if (wParam == PBT_APMRESUMEAUTOMATIC || wParam == PBT_APMRESUMESUSPEND)
        {
            Sleep(4000);
            LaunchGauntlet();
        }
    }

    if (msg == WM_POWERBROADCAST && wParam == PBT_POWERSETTINGCHANGE)
    {
        POWERBROADCAST_SETTING* ps = (POWERBROADCAST_SETTING*) lParam;

        static const GUID GUID_MONITOR_POWER_ON = 
            { 0x02731015, 0x4510, 0x4526, { 0x99, 0xE6, 0xE5, 0xA1, 0x7E, 0xBD, 0x1A, 0xEA } };

        if (ps && IsEqualGUID(ps->PowerSetting, GUID_MONITOR_POWER_ON))
        {
            DWORD monitorOn = *((DWORD*) ps->Data);
            if (monitorOn == 1)
            {
                Sleep(4000);
                LaunchGauntlet();
            }
        }
    }

    if (msg == WM_DESTROY)
        PostQuitMessage(0);

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int)
{
    if (lpCmdLine && std::string(lpCmdLine).find("--install") != std::string::npos)
    {
        InstallStartup();
        return 0;
    }

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "GauntletWatcher";
    RegisterClassExA(&wc);

    g_hwnd = CreateWindowExA(0, "GauntletWatcher", "GauntletWatcher",
        0, 0, 0, 0, 0,
        HWND_MESSAGE, NULL, hInstance, NULL);

    ShowWindow(g_hwnd, SW_HIDE);

    static const GUID GUID_MONITOR_POWER_ON =
        { 0x02731015, 0x4510, 0x4526, { 0x99, 0xE6, 0xE5, 0xA1, 0x7E, 0xBD, 0x1A, 0xEA } };

    RegisterPowerSettingNotification(g_hwnd, &GUID_MONITOR_POWER_ON, DEVICE_NOTIFY_WINDOW_HANDLE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}