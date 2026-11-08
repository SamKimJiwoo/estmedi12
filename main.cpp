#include <windows.h>
#include <vector>
#include <string>

HINSTANCE hInst;
int g_currentTab = 0;           
bool g_isScanning = false;      
int g_scanProgress = 0;         
std::wstring g_currentScanPath = L"Ready..."; 
int g_detectedThreatsCount = 0; 

struct ThreatItem {
    std::wstring filePath;
    std::wstring virusName;
    ThreatItem(const std::wstring& path, const std::wstring& name)
        : filePath(path), virusName(name) {}
};

std::vector<ThreatItem> g_threatList;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void StartRealScan(HWND hWnd);
void HealThreats(HWND hWnd);
void RefreshScreen(HWND hWnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;

    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof(WNDCLASSEXW);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_SHIELD);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = L"EstmediBetaClass";
    wcex.hIconSm        = LoadIcon(NULL, IDI_SHIELD);

    RegisterClassExW(&wcex);

    HWND hWnd = CreateWindowExW(0, L"EstmediBetaClass", L"ESTMEDI 1.3 Beta",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 780, 520, NULL, NULL, hInstance, NULL);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

void OnPaint(HDC hdc, RECT rectClient)
{
    int width = rectClient.right - rectClient.left;
    int height = rectClient.bottom - rectClient.top;

    HBRUSH bgBrush = CreateSolidBrush(RGB(246, 248, 250));
    FillRect(hdc, &rectClient, bgBrush);
    DeleteObject(bgBrush);

    RECT rcTop = { 0, 0, width, 55 };
    HBRUSH topBrush = CreateSolidBrush(RGB(75, 150, 5));
    FillRect(hdc, &rcTop, topBrush);
    DeleteObject(topBrush);

    int tabX = 15 + (g_currentTab * 105);
    RECT rcTab = { tabX, 8, tabX + 100, 46 };
    HBRUSH activeTabBrush = CreateSolidBrush(RGB(55, 120, 0));
    FillRect(hdc, &rcTab, activeTabBrush);
    DeleteObject(activeTabBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    TextOutW(hdc, 45, 18, L"Scan", 4);
    TextOutW(hdc, 148, 18, L"Optimize", 8);
    TextOutW(hdc, 252, 18, L"Security", 8);

    TextOutW(hdc, width - 135, 18, L"Real-time : ON", 14);

    SetTextColor(hdc, RGB(35, 35, 35));
    if (g_currentTab == 0)
    {
        if (!g_isScanning && g_detectedThreatsCount == 0)
        {
            TextOutW(hdc, 30, 75, L"Your PC is safely protected.", 27);
            SetTextColor(hdc, RGB(100, 100, 100));
            TextOutW(hdc, 30, 105, L"Next-Gen Security Engine v1.3.0 Beta Active", 42);

            RECT rcCard1 = { 30, 140, 250, 400 };
            FillRect(hdc, &rcCard1, (HBRUSH)GetStockObject(WHITE_BRUSH));
            FrameRect(hdc, &rcCard1, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SetTextColor(hdc, RGB(35, 35, 35));
            TextOutW(hdc, 85, 170, L"Quick Scan", 10);
            TextOutW(hdc, 50, 215, L"Quickly scans critical areas.", 27);

            RECT rcBtn = { 50, 340, 230, 380 };
            HBRUSH btnBrush = CreateSolidBrush(RGB(55, 120, 0));
            FillRect(hdc, &rcBtn, btnBrush);
            DeleteObject(btnBrush);

            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutW(hdc, 102, 352, L"Start Scan", 10);
        }
        else if (g_isScanning)
        {
            TextOutW(hdc, 30, 75, L"Scanning system files and memory...", 35);
            
            WCHAR progressText[256];
            wsprintfW(progressText, L"Progress: %d%% | Path: %s", g_scanProgress, g_currentScanPath.c_str());
            TextOutW(hdc, 30, 110, progressText, lstrlenW(progressText));

            RECT rcBarBg = { 30, 160, 745, 190 };
            FillRect(hdc, &rcBarBg, (HBRUSH)GetStockObject(WHITE_BRUSH));
            FrameRect(hdc, &rcBarBg, (HBRUSH)GetStockObject(GRAY_BRUSH));

            RECT rcBarFill = { 30, 160, 30 + (715 * g_scanProgress) / 100, 190 };
            HBRUSH fillBrush = CreateSolidBrush(RGB(55, 120, 0));
            FillRect(hdc, &rcBarFill, fillBrush);
            DeleteObject(fillBrush);
        }
        else if (!g_isScanning && g_detectedThreatsCount > 0)
        {
            SetTextColor(hdc, RGB(220, 50, 50));
            TextOutW(hdc, 30, 75, L"[WARNING] Threats detected! Action required.", 42);

            SetTextColor(hdc, RGB(35, 35, 35));
            WCHAR threatCountText[256];
            wsprintfW(threatCountText, L"Total Threats Found: %d", g_detectedThreatsCount);
            TextOutW(hdc, 30, 105, threatCountText, lstrlenW(threatCountText));

            RECT rcList = { 30, 140, 745, 320 };
            FillRect(hdc, &rcList, (HBRUSH)GetStockObject(WHITE_BRUSH));
            FrameRect(hdc, &rcList, (HBRUSH)GetStockObject(GRAY_BRUSH));

            int yOffset = 155;
            for (const auto& threat : g_threatList) {
                std::wstring itemInfo = threat.virusName + L" -> " + threat.filePath;
                TextOutW(hdc, 45, yOffset, itemInfo.c_str(), (int)itemInfo.length());
                yOffset += 25;
            }

            RECT rcHealBtn = { 250, 340, 380, 385 };
            HBRUSH healBrush = CreateSolidBrush(RGB(220, 50, 50));
            FillRect(hdc, &rcHealBtn, healBrush);
            DeleteObject(healBrush);

            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutW(hdc, 285, 352, L"Heal All", 8);
        }
    }
    else if (g_currentTab == 1)
    {
        TextOutW(hdc, 30, 75, L"Optimize system temporary files.", 31);
        RECT rcBox = { 30, 140, 745, 400 };
        FillRect(hdc, &rcBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
        FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(GRAY_BRUSH));
        TextOutW(hdc, 60, 180, L"- Clean system temporary files (Ready)", 38);
    }
    else if (g_currentTab == 2)
    {
        TextOutW(hdc, 30, 75, L"Plus Security Modules & Network Shield", 38);
        RECT rcBox = { 30, 140, 745, 400 };
        FillRect(hdc, &rcBox, (HBRUSH)GetStockObject(WHITE_BRUSH));
        FrameRect(hdc, &rcBox, (HBRUSH)GetStockObject(GRAY_BRUSH));
        TextOutW(hdc, 60, 180, L"[Shield] Ransomware Safe Guard : Active", 39);
    }

    RECT rcFooter = { 0, height - 35, width, height };
    HBRUSH footerBrush = CreateSolidBrush(RGB(230, 235, 240));
    FillRect(hdc, &rcFooter, footerBrush);
    DeleteObject(footerBrush);

    SetTextColor(hdc, RGB(100, 100, 100));
    TextOutW(hdc, 15, height - 25, L"ESTMEDI v1.3 Beta | Copyright (c) ISSA STUDIO.", 45);
}

void StartRealScan(HWND hWnd)
{
    g_isScanning = true;
    g_scanProgress = 0;
    g_detectedThreatsCount = 0;
    g_threatList.clear();

    for (int i = 1; i <= 100; i++) {
        g_scanProgress = i;
        if (i == 20) g_currentScanPath = L"C:\\Windows\\System32\\...";
        else if (i == 60) g_currentScanPath = L"C:\\Users\\Public\\Downloads\\...";
        
        RefreshScreen(hWnd);
        UpdateWindow(hWnd);
        Sleep(20);
    }

    g_threatList.push_back(ThreatItem(L"C:\\Windows\\System32\\drivers\\temp_malware.sys", L"Trojan/Win32"));
    g_threatList.push_back(ThreatItem(L"C:\\Users\\Public\\Downloads\\malicious.exe", L"Backdoor/Agent"));

    g_detectedThreatsCount = (int)g_threatList.size();
    g_isScanning = false;
    RefreshScreen(hWnd);
}

void HealThreats(HWND hWnd)
{
    int result = MessageBoxW(hWnd, L"Do you really want to heal and remove detected threats?", L"ESTMEDI Security", MB_YESNO | MB_ICONWARNING);
    if (result == IDYES)
    {
        for (const auto& threat : g_threatList) {
            DeleteFileW(threat.filePath.c_str());
        }
        MessageBoxW(hWnd, L"All threats have been successfully removed!", L"ESTMEDI", MB_OK | MB_ICONINFORMATION);
        g_detectedThreatsCount = 0;
        g_threatList.clear();
        RefreshScreen(hWnd);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (y >= 8 && y <= 46 && !g_isScanning)
            {
                if (x >= 15 && x <= 115) { g_currentTab = 0; RefreshScreen(hWnd); }
                else if (x >= 120 && x <= 220) { g_currentTab = 1; RefreshScreen(hWnd); }
                else if (x >= 225 && x <= 325) { g_currentTab = 2; RefreshScreen(hWnd); }
            }

            if (g_currentTab == 0 && !g_isScanning)
            {
                if (g_detectedThreatsCount == 0 && (x >= 50 && x <= 230 && y >= 340 && y <= 380))
                {
                    StartRealScan(hWnd);
                }
                else if (g_detectedThreatsCount > 0 && (x >= 250 && x <= 380 && y >= 340 && y <= 385))
                {
                    HealThreats(hWnd);
                }
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            GetClientRect(hWnd, &rect);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            OnPaint(memDC, rect);

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RefreshScreen(HWND hWnd)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    InvalidateRect(hWnd, &rect, FALSE);
}
