#pragma once
#include <windows.h>
#include <string>
#include "taskmanager.h"

class LockScreen
{
public:
    LockScreen(HINSTANCE hInstance, const Task& task);
    void Run();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnPaint(HWND hwnd);
    void OnChar(HWND hwnd, TCHAR ch);
    bool ValidateAnswer();
    void DrawCenteredText(HDC hdc, const std::string& text, RECT rect, COLORREF color, int fontSize, bool bold);

    HINSTANCE m_hInstance;
    HWND m_hwnd;
    Task m_task;
    std::string m_userInput;
    bool m_completed;
    bool m_failed;
    int m_failTimer;

    static LockScreen* s_instance;
};