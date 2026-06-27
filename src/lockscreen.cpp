#include "lockscreen.h"
#include <algorithm>
#include <cctype>
#include <sstream>

LockScreen* LockScreen::s_instance = nullptr;

static const COLORREF COL_BG        = RGB(10,  10,  18);
static const COLORREF COL_ACCENT    = RGB(99,  179, 237);
static const COLORREF COL_TEXT      = RGB(220, 220, 230);
static const COLORREF COL_DIM       = RGB(100, 100, 120);
static const COLORREF COL_SUCCESS   = RGB(72,  199, 116);
static const COLORREF COL_FAIL      = RGB(220,  60,  60);
static const COLORREF COL_INPUT_BG  = RGB(20,  20,  35);
static const COLORREF COL_INPUT_BOR = RGB(60,  60,  90);

static std::string ToLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

static std::string Trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> WordWrap(HDC hdc, const std::string& text, int maxWidth)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word, line;

    while (stream >> word)
    {
        std::string test = line.empty() ? word : line + " " + word;
        SIZE sz;
        GetTextExtentPoint32A(hdc, test.c_str(), (int)test.size(), &sz);
        if (sz.cx > maxWidth && !line.empty())
        {
            lines.push_back(line);
            line = word;
        }
        else
        {
            line = test;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

LockScreen::LockScreen(HINSTANCE hInstance, const Task& task)
    : m_hInstance(hInstance), m_hwnd(NULL), m_task(task),
      m_completed(false), m_failed(false), m_failTimer(0)
{
    s_instance = this;
}

void LockScreen::Run()
{
    WNDCLASSEXA wc      = {};
    wc.cbSize           = sizeof(wc);
    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = m_hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = CreateSolidBrush(COL_BG);
    wc.lpszClassName    = "GauntletLockScreen";
    RegisterClassExA(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    m_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "GauntletLockScreen", "Gauntlet",
        WS_POPUP,
        0, 0, sw, sh,
        NULL, NULL, m_hInstance, NULL);

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    SetTimer(m_hwnd, 1, 50, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK LockScreen::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LockScreen* ls = s_instance;

    switch (msg)
    {
        case WM_PAINT:
            if (ls) ls->OnPaint(hwnd);
            return 0;

        case WM_CHAR:
            if (ls) ls->OnChar(hwnd, (TCHAR)wParam);
            return 0;

        case WM_TIMER:
            if (ls && ls->m_failed)
            {
                ls->m_failTimer--;
                if (ls->m_failTimer <= 0)
                {
                    ls->m_failed = false;
                    ls->m_failTimer = 0;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

bool LockScreen::ValidateAnswer()
{
    std::string input = ToLower(Trim(m_userInput));

    if (input == "skip") //Emergency skip, change to whatever you want.
        return true;

    if (m_task.answer.empty())
        return input.size() >= 10;

    return input == m_task.answer;
}

void LockScreen::OnChar(HWND hwnd, TCHAR ch)
{
    if (m_completed) return;

    if (ch == VK_RETURN)
    {
        if (ValidateAnswer())
        {
            m_completed = true;
            InvalidateRect(hwnd, NULL, TRUE);
            Sleep(1200);
            DestroyWindow(hwnd);
        }
        else
        {
            m_failed     = true;
            m_failTimer  = 12;
            m_userInput.clear();
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
    else if (ch == VK_BACK)
    {
        if (!m_userInput.empty())
            m_userInput.pop_back();
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else if (ch >= 32)
    {
        m_userInput += (char)ch;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void LockScreen::DrawCenteredText(HDC hdc, const std::string& text, RECT rect,
                                   COLORREF color, int fontSize, bool bold)
{
    HFONT font = CreateFontA(
        fontSize, 0, 0, 0,
        bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Segoe UI");

    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextA(hdc, text.c_str(), -1, &rect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void LockScreen::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    HDC     memDC  = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, sw, sh);
    SelectObject(memDC, memBmp);

    COLORREF bgColor = m_failed ? COL_FAIL :
                       m_completed ? COL_SUCCESS : COL_BG;
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    RECT fullRect  = { 0, 0, sw, sh };
    FillRect(memDC, &fullRect, bgBrush);
    DeleteObject(bgBrush);

    if (m_completed)
    {
        RECT r = { 0, 0, sw, sh };
        DrawCenteredText(memDC, "UNLOCKED", r, RGB(255,255,255), 72, true);
    }
    else
    {
        int cx = sw / 2;
        int padX = sw / 5;

        const char* categoryNames[] = { "TYPING", "TRIVIA", "CODING", "EXERCISE", "MATH" };
        std::string badge = std::string("[ ") + categoryNames[m_task.type] + " ]";
        RECT badgeRect = { 0, sh / 6, sw, sh / 6 + 40 };
        DrawCenteredText(memDC, badge, badgeRect, COL_ACCENT, 18, true);


        RECT promptRect = { 0, sh / 6 + 55, sw, sh / 6 + 100 };
        DrawCenteredText(memDC, m_task.prompt, promptRect, COL_DIM, 16, false);

        if (m_task.type == TYPING && !m_task.prompt.empty())
        {
            // Word-wrap the content
            HFONT wrapFont = CreateFontA(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            HFONT oldF = (HFONT)SelectObject(memDC, wrapFont);
            SetTextColor(memDC, COL_TEXT);
            SetBkMode(memDC, TRANSPARENT);

            int maxW = sw - padX * 2;
            auto lines = WordWrap(memDC, m_task.prompt, maxW);

            int lineH = 30;
            int totalH = (int)lines.size() * lineH;
            int startY = sh / 2 - totalH / 2 - 40;

            for (int i = 0; i < (int)lines.size(); i++)
            {
                RECT lr = { padX, startY + i * lineH, sw - padX, startY + i * lineH + lineH };
                DrawTextA(memDC, lines[i].c_str(), -1, &lr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            SelectObject(memDC, oldF);
            DeleteObject(wrapFont);
        }

        int boxW = sw / 2;
        int boxH = 52;
        int boxX = cx - boxW / 2;
        int boxY = sh * 2 / 3;

        COLORREF borderCol = m_failed ? COL_FAIL : COL_INPUT_BOR;
        HBRUSH inputBg  = CreateSolidBrush(COL_INPUT_BG);
        HPEN   borderPen = CreatePen(PS_SOLID, 2, borderCol);

        HBRUSH oldBr = (HBRUSH)SelectObject(memDC, inputBg);
        HPEN   oldPen = (HPEN)SelectObject(memDC, borderPen);
        Rectangle(memDC, boxX, boxY, boxX + boxW, boxY + boxH);
        SelectObject(memDC, oldBr);
        SelectObject(memDC, oldPen);
        DeleteObject(inputBg);
        DeleteObject(borderPen);

        std::string display = m_userInput + "|";
        HFONT inputFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
        HFONT oldIF = (HFONT)SelectObject(memDC, inputFont);
        SetTextColor(memDC, COL_TEXT);
        SetBkMode(memDC, TRANSPARENT);
        RECT inputTextRect = { boxX + 12, boxY, boxX + boxW - 12, boxY + boxH };
        DrawTextA(memDC, display.c_str(), -1, &inputTextRect,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(memDC, oldIF);
        DeleteObject(inputFont);

        if (m_failed)
        {
            RECT failRect = { 0, boxY + boxH + 12, sw, boxY + boxH + 50 };
            DrawCenteredText(memDC, "Wrong — try again.", failRect, COL_FAIL, 16, false);
        }

        RECT hintRect = { 0, sh - 50, sw, sh - 10 };
        DrawCenteredText(memDC, "Press ENTER to submit  |  BACKSPACE to delete",
                         hintRect, COL_DIM, 14, false);
    }

    BitBlt(hdc, 0, 0, sw, sh, memDC, 0, 0, SRCCOPY);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}