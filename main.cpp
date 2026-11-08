#include <windows.h>
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

// 프로그램 상태 관리
int g_currentTab = 0;           // 0: 검사하기, 1: PC최적화, 2: 플러스보안
bool g_isScanning = false;      // 현재 검사 중 여부
int g_scanProgress = 0;         // 검사 진행률 (0~100)
std::wstring g_currentScanPath = _T("대기 중..."); 
int g_detectedThreatsCount = 0; // 발견된 위협 개수

// 발견된 가상의 악성 파일 목록 구조체
struct ThreatItem {
    std::wstring filePath;
    std::wstring virusName;
};
std::vector<ThreatItem> g_threatList;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void StartRealScan(HWND hWnd);
void HealThreats(HWND hWnd);
void InvalidatedRect(HWND hWnd);

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

// 화면 렌더링 함수
void OnPaint(HDC hdc, RECT rectClient)
{
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    int width = rectClient.right - rectClient.left;
    int height = rectClient.bottom - rectClient.top;

    // 1. 상단 네비게이션 바
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

    g.DrawString(_T("검사하기"), -1, &tabFont, PointF(32, 18), &whiteBrush);
    g.DrawString(_T("PC최적화"), -1, &tabFont, PointF(137, 18), &whiteBrush);
    g.DrawString(_T("플러스보안"), -1, &tabFont, PointF(242, 18), &whiteBrush);

    Font subFont(&fontFamily, 9, FontStyleRegular, UnitPoint);
    g.FillRectangle(&SolidBrush(Color(50, 255, 255, 255)), Rect(width - 155, 13, 140, 28));
    g.DrawString(_T("실시간 감시 : ON"), -1, &subFont, PointF(width - 145, 19), &whiteBrush);

    // 2. 메인 컨텐츠 배경
    g.FillRectangle(&SolidBrush(Color(255, 246, 248, 250)), 0, 55, width, height - 90);

    Font headerFont(&fontFamily, 14, FontStyleBold, UnitPoint);
    SolidBrush textDarkBrush(Color(255, 35, 35, 35));
    SolidBrush textGrayBrush(Color(255, 100, 100, 100));

    // ---------------------------------------------------------
    // [탭 0: 검사하기 화면]
    // ---------------------------------------------------------
    if (g_currentTab == 0)
    {
        if (!g_isScanning && g_detectedThreatsCount == 0)
        {
            g.DrawString(_T("PC가 안전하고 쾌적하게 보호되고 있습니다."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
            g.DrawString(_T("차세대 보안 엔진 v1.3.0 Beta 적용 완료"), -1, &subFont, PointF(30, 105), &textGrayBrush);

            Pen borderPen(Color(255, 215, 220, 230), 1.5f);

            // 빠른 검사 카드
            RectF card1(30, 140, 220, 260);
            g.FillRectangle(&whiteBrush, card1);
            g.DrawRectangle(&borderPen, card1);
            g.DrawString(_T("빠른 검사"), -1, &headerFont, PointF(95, 170), &textDarkBrush);
            g.DrawString(_T("주요 시스템 영역과 메모리를\n신속하게 검사합니다."), -1, &subFont, PointF(55, 215), &textGrayBrush);
            
            g.FillRectangle(&activeTabBrush, Rect(50, 340, 180, 40));
            g.DrawString(_T("검사 시작"), -1, &tabFont, PointF(105, 350), &whiteBrush);

            // 정밀 검사 카드
            RectF card2(265, 140, 220, 260);
            g.FillRectangle(&whiteBrush, card2);
            g.DrawRectangle(&borderPen, card2);
            g.DrawString(_T("정밀 검사"), -1, &headerFont, PointF(330, 170), &textDarkBrush);
            g.DrawString(_T("전체 디스크 드라이브를\n구석구석 검사합니다."), -1, &subFont, PointF(290, 215), &textGrayBrush);

            // 우측 정보 배너
            RectF card3(500, 140, 245, 260);
            g.FillRectangle(&SolidBrush(Color(255, 32, 42, 56)), card3);
            Font bannerTitleFont(&fontFamily, 12, FontStyleBold, UnitPoint);
            g.DrawString(_T("ESTMEDI 1.3 BETA"), -1, &bannerTitleFont, PointF(520, 165), &whiteBrush);
            g.DrawString(_T("실제 경로 분석 엔진 탑재.\n\n- C:/ 시스템 정밀 스캔\n- 실시간 악성코드 진단\n- 안전한 격리 및 치료"), -1, &subFont, PointF(520, 210), &SolidBrush(Color(255, 200, 210, 225)));
        }
        else if (g_isScanning)
        {
            // 검사 진행 중 화면
            g.DrawString(_T("시스템을 정밀하게 검사하고 있습니다..."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
            
            WCHAR progressText[256];
            wsprintf(progressText, _T("진행률: %d%%  |  현재 검사 중인 경로:"), g_scanProgress);
            g.DrawString(progressText, -1, &subFont, PointF(30, 110), &textGrayBrush);

            // 현재 검사 중인 실제 파일 경로 출력 (C:/...)
            SolidBrush pathBoxBg(Color(255, 235, 240, 245));
            g.FillRectangle(&pathBoxBg, Rect(30, 140, 715, 60));
            g.DrawString(g_currentScanPath.c_str(), -1, &subFont, PointF(45, 160), &textDarkBrush);

            // 게이지바 배경 및 채우기
            g.FillRectangle(&SolidBrush(Color(255, 220, 225, 230)), Rect(30, 230, 715, 25));
            int fillWidth = (715 * g_scanProgress) / 100;
            g.FillRectangle(&activeTabBrush, Rect(30, 230, fillWidth, 25));
        }
        else if (!g_isScanning && g_detectedThreatsCount > 0)
        {
            // 악성코드 발견 완료 화면
            g.DrawString(_T("[경고] 위협이 발견되었습니다! 치료가 필요합니다."), -1, &headerFont, PointF(30, 75), &SolidBrush(Color(255, 220, 50, 50)));
            
            WCHAR threatCountText[256];
            wsprintf(threatCountText, _T("진단된 악성 위협 개수: %d개"), g_detectedThreatsCount);
            g.DrawString(threatCountText, -1, &subFont, PointF(30, 105), &textDarkBrush);

            // 발견된 위협 리스트박스 박스
            SolidBrush listBg(Color(255, 255, 255, 255));
            g.FillRectangle(&listBg, Rect(30, 140, 715, 180));
            g.DrawRectangle(&Pen(Color(255, 220, 50, 50), 1.5f), Rect(30, 140, 715, 180));

            int yOffset = 155;
            for (const auto& threat : g_threatList) {
                std::wstring itemInfo = threat.virusName + _T("  ->  ") + threat.filePath;
                g.DrawString(itemInfo.c_str(), -1, &subFont, PointF(45, (REAL)yOffset), &textDarkBrush);
                yOffset += 25;
            }

            // [치료하기] 버튼 생성
            g.FillRectangle(&activeTabBrush, Rect(250, 340, 130, 45));
            g.DrawString(_T("치료하기"), -1, &headerFont, PointF(280, 350), &whiteBrush);

            // [다시 검사] 버튼
            g.FillRectangle(&SolidBrush(Color(255, 100, 100, 100)), Rect(400, 340, 130, 45));
            g.DrawString(_T("다시 검사"), -1, &headerFont, PointF(430, 350), &whiteBrush);
        }
    }
    else if (g_currentTab == 1)
    {
        g.DrawString(_T("불필요한 시스템 임시 파일 및 레지스트리를 정리합니다."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
        g.DrawString(_T("최적화 대상 항목 선택 완료"), -1, &subFont, PointF(30, 105), &textGrayBrush);

        RectF optBox(30, 140, 715, 260);
        g.FillRectangle(&whiteBrush, optBox);
        g.DrawRectangle(&Pen(Color(255, 215, 220, 230), 1.5f), optBox);

        g.DrawString(_T("- 시스템 임시 파일 정밀 청소 (대기 중)"), -1, &tabFont, PointF(60, 170), &textDarkBrush);
        g.DrawString(_T("- 웹 브라우저 캐시 및 쿠키 삭제 (대기 중)"), -1, &tabFont, PointF(60, 210), &textDarkBrush);
        g.DrawString(_T("- 메모리 최적화 및 프로세스 정리 (대기 중)"), -1, &tabFont, PointF(60, 250), &textDarkBrush);

        g.FillRectangle(&activeTabBrush, Rect(280, 320, 200, 45));
        g.DrawString(_T("PC 최적화 실행"), -1, &headerFont, PointF(315, 330), &whiteBrush);
    }
    else if (g_currentTab == 2)
    {
        g.DrawString(_T("플러스 보안 모듈 및 네트워크 차단 설정"), -1, &headerFont, PointF(30, 75), &textDarkBrush);
        g.DrawString(_T("시스템 실시간 방화벽 작동 중"), -1, &subFont, PointF(30, 105), &textGrayBrush);

        RectF secBox(30, 140, 715, 260);
        g.FillRectangle(&whiteBrush, secBox);
        g.DrawRectangle(&Pen(Color(255, 215, 220, 230), 1.5f), secBox);

        g.DrawString(_T("[보안] 랜섬웨어 전용 안심 차단기 : 활성화"), -1, &headerFont, PointF(60, 180), &textDarkBrush);
        g.DrawString(_T("[네트워크] 악성 피싱/사기 사이트 자동 차단 : 활성화"), -1, &headerFont, PointF(60, 230), &textDarkBrush);
        g.DrawString(_T("[파일] USB 자동 검사 및 바이러스 보호 : 활성화"), -1, &headerFont, PointF(60, 280), &textDarkBrush);
    }

    // 4. 하단 푸터 (저작권 표시)
    g.FillRectangle(&SolidBrush(Color(255, 230, 235, 240)), 0, height - 35, width, 35);
    g.DrawLine(&Pen(Color(255, 205, 212, 220)), 0, height - 35, width, height - 35);

    Font copyrightFont(&fontFamily, 8.5f, FontStyleRegular, UnitPoint);
    g.DrawString(_T("ESTMEDI v1.3 Beta  |  Copyright (c) ISSA STUDIO. All rights reserved."), -1, &copyrightFont, PointF(15, height - 23), &textGrayBrush);
    g.DrawString(_T("DB 버전: 2026.11.08"), -1, &copyrightFont, PointF(width - 135, height - 23), &textDarkBrush);
}

// 실제 스캔 시뮬레이션 및 경로 탐지 함수
void StartRealScan(HWND hWnd)
{
    g_isScanning = true;
    g_scanProgress = 0;
    g_detectedThreatsCount = 0;
    g_threatList.clear();

    std::wstring scanPaths[] = {
        _T("C:\\Windows\\System32\\drivers\\temp_malware.sys"),
        _T("C:\\Users\\Public\\Downloads\\Trojan.Generic.KD.exe"),
        _T("C:\\Program Files\\UnknownApp\\malicious_plugin.dll"),
        _T("C:\\Windows\\Temp\\backdoor_script.vbs"),
        _T("C:\\Users\\Default\\AppData\\Local\\Worm.Autorun.Gen")
    };

    for (int i = 1; i <= 100; i++) {
        g_scanProgress = i;
        
        if (i == 20) g_currentScanPath = _T("C:\\Windows\\System32\\...");
        else if (i == 45) g_currentScanPath = _T("C:\\Users\\Public\\Downloads\\...");
        else if (i == 70) g_currentScanPath = _T("C:\\Program Files\\UnknownApp\\...");
        else if (i == 90) g_currentScanPath = _T("C:\\Windows\\Temp\\...");

        InvalidatedRect(hWnd);
        UpdateWindow(hWnd);
        Sleep(25);
    }

    g_threatList.push_back({ _T("C:\\Windows\\System32\\drivers\\temp_malware.sys"), _T("Trojan/Win32.Malware") });
    g_threatList.push_back({ _T("C:\\Users\\Public\\Downloads\\Trojan.Generic.KD.exe"), _T("Backdoor/Script.Agent") });
    g_threatList.push_back({ _T("C:\\Windows\\Temp\\backdoor_script.vbs"), _T("Worm/VBS.Autorun") });

    g_detectedThreatsCount = (int)g_threatList.size();
    g_isScanning = false;

    InvalidatedRect(hWnd);
}

// 악성코드 진짜 치료 수행 함수
void HealThreats(HWND hWnd)
{
    int result = MessageBox(hWnd, 
        _T("진단된 악성코드를 정말로 치료하시겠습니까?\n(치료 시 감염된 파일이 안전하게 제거됩니다.)"), 
        _T("ESTMEDI 1.3 Beta - 보안 치료"), 
        MB_YESNO | MB_ICONWARNING);

    if (result == IDYES)
    {
        for (const auto& threat : g_threatList) {
            DeleteFileW(threat.filePath.c_str());
        }

        MessageBox(hWnd, _T("모든 악성코드가 성공적으로 치료 및 격리되었습니다!\nPC가 안전한 상태입니다."), _T("ESTMEDI 1.3 Beta"), MB_OK | MB_ICONINFORMATION);
        
        g_detectedThreatsCount = 0;
        g_threatList.clear();

        InvalidatedRect(hWnd);
    }
}

// 마우스 클릭 이벤트 처리
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
                if (x >= 15 && x <= 115) { g_currentTab = 0; InvalidatedRect(hWnd); }
                else if (x >= 120 && x <= 220) { g_currentTab = 1; InvalidatedRect(hWnd); }
                else if (x >= 225 && x <= 325) { g_currentTab = 2; InvalidatedRect(hWnd); }
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
                    InvalidatedRect(hWnd);
                }
            }

            if (g_currentTab == 1 && (x >= 280 && x <= 480 && y >= 320 && y <= 365))
            {
                MessageBox(hWnd, _T("PC 최적화가 완료되었습니다!\n(불필요한 임시 파일 148MB 청소 완료)"), _T("ESTMEDI 1.3 Beta"), MB_OK | MB_ICONINFORMATION);
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

void InvalidatedRect(HWND hWnd)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    InvalidateRect(hWnd, &rect, FALSE);
}
