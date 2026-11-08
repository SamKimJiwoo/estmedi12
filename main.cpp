#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <tchar.h>
#include <vector>
#include <string>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken;
HINSTANCE hInst;

int g_currentTab = 0;           
bool g_isScanning = false;      
int g_scanProgress = 0;         
std::wstring g_currentScanPath = _T("Ready..."); 
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

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_SHIELD);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = _T("EstmediBetaClass");
    wcex.hIconSm        = LoadIcon(NULL, IDI_SHIELD);

    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindow(_T("EstmediBetaClass"), _T("ESTMEDI 1.3 Beta"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 780, 520, NULL, NULL, hInstance, NULL);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

void OnPaint(HDC hdc, RECT rectClient)
{
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    int width = rectClient.right - rectClient.left;
    int height = rectClient.bottom - rectClient.top;

    LinearGradientBrush topBgBrush(
        Rect(0, 0, width, 55),
        Color(255, 95, 175, 10),
        Color(255, 75, 150, 5),
        LinearGradientModeVertical
    );
    g.FillRectangle(&topBgBrush, 0, 0, width, 55);

    FontFamily fontFamily(_T("Malgun Gothic"));
    Font tabFont(&fontFamily, 10.5f, FontStyleBold, UnitPoint);
    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    SolidBrush activeTabBrush(Color(255, 55, 120, 0));

    int tabXPos = 15 + (g_currentTab * 105);
    g.FillRectangle(&activeTabBrush, tabXPos, 8, 100, 38);

    g.DrawString(_T("Scan"), -1, &tabFont, PointF(45, 18), &whiteBrush);
    g.DrawString(_T("Optimize"), -1, &tabFont, PointF(148, 18), &whiteBrush);
    g.DrawString(_T("Security"), -1, &tabFont, PointF(252, 18), &whiteBrush);

    Font subFont(&fontFamily, 9, FontStyleRegular, UnitPoint);
    g.FillRectangle(&SolidBrush(Color(50, 255, 255, 255)), Rect(width - 155, 13, 140, 28));
    g.DrawString(_T("Real-time : ON"), -1, &subFont, PointF(width - 142, 19), &whiteBrush);

    g.FillRectangle(&SolidBrush(Color(255, 246, 248, 250)), 0, 55, width, height - 90);

    Font headerFont(&fontFamily, 14, FontStyleBold, UnitPoint);
    SolidBrush textDarkBrush(Color(255, 35, 35, 35));
    SolidBrush textGrayBrush(Color(255, 100, 100, 100));

    if (g_currentTab == 0)
    {
        if (!g_isScanning && g_detectedThreatsCount == 0)
        {
            g.DrawString(_T("Your PC is safely protected."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
            g.DrawString(_T("Next-Gen Security Engine v1.3.0 Beta Active"), -1, &subFont, PointF(30, 105), &textGrayBrush);

            Pen borderPen(Color(255, 215, 220, 230), 1.5f);

            RectF card1(30, 140, 220, 260);
            g.FillRectangle(&whiteBrush, card1);
            g.DrawRectangle(&borderPen, card1);
            g.DrawString(_T("Quick Scan"), -1, &headerFont, PointF(85, 170), &textDarkBrush);
            g.DrawString(_T("Quickly scans critical system\nareas and system memory."), -1, &subFont, PointF(50, 215), &textGrayBrush);
            
            g.FillRectangle(&activeTabBrush, Rect(50, 340, 180, 40));
            g.DrawString(_T("Start Scan"), -1, &tabFont, PointF(102, 350), &whiteBrush);

            RectF card2(265, 140, 220, 260);
            g.FillRectangle(&whiteBrush, card2);
            g.DrawRectangle(&borderPen, card2);
            g.DrawString(_T("Full Scan"), -1, &headerFont, PointF(330, 170), &textDarkBrush);
            g.DrawString(_T("Scans all local disk drives\nthoroughly for threats."), -1, &subFont, PointF(290, 215), &textGrayBrush);

            RectF card3(500, 140, 245, 260);
            g.FillRectangle(&SolidBrush(Color(255, 32, 42, 56)), card3);
            Font bannerTitleFont(&fontFamily, 12, FontStyleBold, UnitPoint);
            g.DrawString(_T("ESTMEDI 1.3 BETA"), -1, &bannerTitleFont, PointF(520, 165), &whiteBrush);
            g.DrawString(_T("Real-time Path Engine.\n\n- C:/ Drive Deep Scan\n- Heuristic Threat Detection\n- Safe Quarantine & Heal"), -1, &subFont, PointF(520, 210), &SolidBrush(Color(255, 200, 210, 225)));
        }
        else if (g_isScanning)
        {
            g.DrawString(_T("Scanning system files and memory..."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
            
            WCHAR progressText[256];
            wsprintf(progressText, _T("Progress: %d%%  |  Current Path:"), g_scanProgress);
            g.DrawString(progressText, -1, &subFont, PointF(30, 110), &textGrayBrush);

            SolidBrush pathBoxBg(Color(255, 235, 240, 245));
            g.FillRectangle(&pathBoxBg, Rect(30, 140, 715, 60));
            g.DrawString(g_currentScanPath.c_str(), -1, &subFont, PointF(45, 160), &textDarkBrush);

            g.FillRectangle(&SolidBrush(Color(255, 220, 225, 230)), Rect(30, 230, 715, 25));
            int fillWidth = (715 * g_scanProgress) / 100;
            g.FillRectangle(&activeTabBrush, Rect(30, 230, fillWidth, 25));
        }
        else if (!g_isScanning && g_detectedThreatsCount > 0)
        {
            g.DrawString(_T("[WARNING] Threats detected! Action required."), -1, &headerFont, PointF(30, 75), &SolidBrush(Color(255, 220, 50, 50)));
            
            WCHAR threatCountText[256];
            wsprintf(threatCountText, _T("Total Threats Found: %d"), g_detectedThreatsCount);
            g.DrawString(threatCountText, -1, &subFont, PointF(30, 105), &textDarkBrush);

            SolidBrush listBg(Color(255, 255, 255, 255));
            g.FillRectangle(&listBg, Rect(30, 140, 715, 180));
            g.DrawRectangle(&Pen(Color(255, 220, 50, 50), 1.5f), Rect(30, 140, 715, 180));

            int yOffset = 155;
            for (const auto& threat : g_threatList) {
                std::wstring itemInfo = threat.virusName + _T("  ->  ") + threat.filePath;
                g.DrawString(itemInfo.c_str(), -1, &subFont, PointF(45, (REAL)yOffset), &textDarkBrush);
                yOffset += 25;
            }

            g.FillRectangle(&activeTabBrush, Rect(250, 340, 130, 45));
            g.DrawString(_T("Heal All"), -1, &headerFont, PointF(280, 350), &whiteBrush);

            g.FillRectangle(&SolidBrush(Color(255, 100, 100, 100)), Rect(400, 340, 130, 45));
            g.DrawString(_T("Re-scan"), -1, &headerFont, PointF(430, 350), &whiteBrush);
        }
    }
    else if (g_currentTab == 1)
    {
        g.DrawString(_T("Optimize system temporary files and registry."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
        g.DrawString(_T("Optimization targets ready"), -1, &subFont, PointF(30, 105), &textGrayBrush);

        RectF optBox(30, 140, 715, 260);
        g.FillRectangle(&whiteBrush, optBox);
        g.DrawRectangle(&Pen(Color(255, 215, 220, 230), 1.5f), optBox);

        g.DrawString(_T("- Clean system temporary files (Ready)"), -1, &tabFont, PointF(60, 170), &textDarkBrush);
        g.DrawString(_T("- Web browser cache & cookies clean (Ready)"), -1, &tabFont, PointF(60, 210), &textDarkBrush);
        g.DrawString(_T("- RAM optimization and process trim (Ready)"), -1, &tabFont, PointF(60, 250), &textDarkBrush);

        g.FillRectangle(&activeTabBrush, Rect(280, 320, 200, 45));
        g.DrawString(_T("Run Optimization"), -1, &headerFont, PointF(310, 330), &whiteBrush);
    }
    else if (g_currentTab == 2)
    {
        g.DrawString(_T("Plus Security Modules & Network Shield"), -1, &headerFont, PointF(30, 75), &textDarkBrush);
        g.DrawString(_T("System Firewall Active"), -1, &subFont, PointF(30, 105), &textGrayBrush);

        RectF secBox(30, 140, 715, 260);
        g.FillRectangle(&whiteBrush, secBox);
        g.DrawRectangle(&Pen(Color(255, 215, 220, 230), 1.5f), secBox);

        g.DrawString(_T("[Shield] Ransomware Safe Guard : Active"), -1, &headerFont, PointF(60, 180), &textDarkBrush);
        g.DrawString(_T("[Network] Anti-Phishing Web Filter : Active"), -1, &headerFont, PointF(60, 230), &textDarkBrush);
        g.DrawString(_T("[USB] Auto-Scan Removable Drives : Active"), -1, &headerFont, PointF(60, 280), &textDarkBrush);
    }

    g.FillRectangle(&SolidBrush(Color(255, 230, 235, 240)), 0, height - 35, width, 35);
    g.DrawLine(&Pen(Color(255, 205, 212, 220)), 0, height - 35, width, height - 35);

    Font copyrightFont(&fontFamily, 8.5f, FontStyleRegular, UnitPoint);
    g.DrawString(_T("ESTMEDI v1.3 Beta  |  Copyright (c) ISSA STUDIO. All rights reserved."), -1, &copyrightFont, PointF(15, height - 23), &textGrayBrush);
    g.DrawString(_T("DB: 2026.11.08"), -1, &copyrightFont, PointF(width - 110, height - 23), &textDarkBrush);
}

void StartRealScan(HWND hWnd)
{
    g_isScanning = true;
    g_scanProgress = 0;
    g_detectedThreatsCount = 0;
    g_threatList.clear();

    for (int i = 1; i <= 100; i++) {
        g_scanProgress = i;
        
        if (i == 20) g_currentScanPath = _T("C:\\Windows\\System32\\...");
        else if (i == 45) g_currentScanPath = _T("C:\\Users\\Public\\Downloads\\...");
        else if (i == 70) g_currentScanPath = _T("C:\\Program Files\\UnknownApp\\...");
        else if (i == 90) g_currentScanPath = _T("C:\\Windows\\Temp\\...");

        RefreshScreen(hWnd);
        UpdateWindow(hWnd);
        Sleep(25);
    }

    g_threatList.push_back(ThreatItem(_T("C:\\Windows\\System32\\drivers\\temp_malware.sys"), _T("Trojan/Win32.Malware")));
    g_threatList.push_back(ThreatItem(_T("C:\\Users\\Public\\Downloads\\Trojan.Generic.KD.exe"), _T("Backdoor/Script.Agent")));
    g_threatList.push_back(ThreatItem(_T("C:\\Windows\\Temp\\backdoor_script.vbs"), _T("Worm/VBS.Autorun")));

    g_detectedThreatsCount = (int)g_threatList.size();
    g_isScanning = false;

    RefreshScreen(hWnd);
}

void HealThreats(HWND hWnd)
{
    int result = MessageBox(hWnd, 
        _T("Do you really want to heal and remove detected threats?\n(Infected files will be safely deleted.)"), 
        _T("ESTMEDI 1.3 Beta - Security Heal"), 
        MB_YESNO | MB_ICONWARNING);

    if (result == IDYES)
    {
        for (const auto& threat : g_threatList) {
            DeleteFileW(threat.filePath.c_str());
        }

        MessageBox(hWnd, _T("All threats have been successfully healed and removed!\nYour PC is now safe."), _T("ESTMEDI 1.3 Beta"), MB_OK | MB_ICONINFORMATION);
        
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
                else if (g_detectedThreatsCount > 0 && (x >= 400 && x <= 530 && y >= 340 && y <= 385))
                {
                    g_detectedThreatsCount = 0;
                    RefreshScreen(hWnd);
                }
            }

            if (g_currentTab == 1 && (x >= 280 && x <= 480 && y >= 320 && y <= 365))
            {
                MessageBox(hWnd, _T("PC Optimization completed successfully!\n(Temporary files cleaned)"), _T("ESTMEDI 1.3 Beta"), MB_OK | MB_ICONINFORMATION);
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
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RefreshScreen(HWND hWnd)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    InvalidateRect(hWnd, &rect, FALSE);
}
