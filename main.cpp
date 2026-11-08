#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken;
HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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

    // 알약 2.5 비율 레퍼런스 크기 (780x520)
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

    // 1. 상단 알약 2.5 그린 테마 네비게이션 바
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

    // 선택된 탭 강조 배경
    SolidBrush activeTabBrush(Color(255, 60, 130, 0));
    g.FillRectangle(&activeTabBrush, 15, 8, 100, 38);

    g.DrawString(_T("검사하기"), -1, &tabFont, PointF(32, 18), &whiteBrush);
    g.DrawString(_T("PC최적화"), -1, &tabFont, PointF(135, 18), &whiteBrush);
    g.DrawString(_T("플러스보안"), -1, &tabFont, PointF(240, 18), &whiteBrush);

    // 우측 상단 실시간 감시 상태 표시
    Font subFont(&fontFamily, 9, FontStyleRegular, UnitPoint);
    g.FillRectangle(&SolidBrush(Color(50, 255, 255, 255)), Rect(width - 155, 13, 140, 28));
    g.DrawString(_T("실시간 감시 : ON"), -1, &subFont, PointF(width - 145, 19), &whiteBrush);

    // 2. 메인 컨텐츠 바탕
    g.FillRectangle(&SolidBrush(Color(255, 246, 248, 250)), 0, 55, width, height - 90);

    Font headerFont(&fontFamily, 14, FontStyleBold, UnitPoint);
    SolidBrush textDarkBrush(Color(255, 35, 35, 35));
    SolidBrush textGrayBrush(Color(255, 100, 100, 100));

    g.DrawString(_T("PC가 안전하고 쾌적하게 보호되고 있습니다."), -1, &headerFont, PointF(30, 75), &textDarkBrush);
    g.DrawString(_T("차세대 보안 엔진 v1.3.0 Beta 적용 완료"), -1, &subFont, PointF(30, 105), &textGrayBrush);

    // 3. 3분할 스마트 카드 영역
    Pen borderPen(Color(255, 215, 220, 230), 1.5f);

    // 카드 1: 빠른 검사
    RectF card1(30, 140, 220, 260);
    g.FillRectangle(&whiteBrush, card1);
    g.DrawRectangle(&borderPen, card1);
    g.DrawString(_T("빠른 검사"), -1, &headerFont, PointF(95, 170), &textDarkBrush);
    g.DrawString(_T("주요 시스템 영역과 메모리를\n신속하게 검사합니다."), -1, &subFont, PointF(55, 220), &textGrayBrush);

    // 카드 2: PC 최적화
    RectF card2(265, 140, 220, 260);
    g.FillRectangle(&whiteBrush, card2);
    g.DrawRectangle(&borderPen, card2);
    g.DrawString(_T("PC 최적화"), -1, &headerFont, PointF(330, 170), &textDarkBrush);
    g.DrawString(_T("불필요한 임시 파일과 레지스트리를\n깔끔하게 정리합니다."), -1, &subFont, PointF(285, 220), &textGrayBrush);

    // 카드 3: 스마트 정보 배너 (딥블루 포인트)
    RectF card3(500, 140, 245, 260);
    g.FillRectangle(&SolidBrush(Color(255, 32, 42, 56)), card3);
    Font bannerTitleFont(&fontFamily, 12, FontStyleBold, UnitPoint);
    g.DrawString(_T("ESTMEDI 1.3 BETA"), -1, &bannerTitleFont, PointF(520, 165), &whiteBrush);
    g.DrawString(_T("새로운 차세대 보안 엔진.\n\n- 실시간 행위 기반 감지\n- 스마트 메모리 최적화\n- 가벼운 리소스 점유"), -1, &subFont, PointF(520, 210), &SolidBrush(Color(255, 200, 210, 225)));

    // 4. 하단 푸터 (저작권 표시)
    g.FillRectangle(&SolidBrush(Color(255, 230, 235, 240)), 0, height - 35, width, 35);
    g.DrawLine(&Pen(Color(255, 205, 212, 220)), 0, height - 35, width, height - 35);

    Font copyrightFont(&fontFamily, 8.5f, FontStyleRegular, UnitPoint);
    g.DrawString(_T("ESTMEDI v1.3 Beta  |  Copyright (c) ISSA STUDIO. All rights reserved."), -1, &copyrightFont, PointF(15, height - 23), &textGrayBrush);
    g.DrawString(_T("DB 버전: 2026.11.08"), -1, &copyrightFont, PointF(width - 135, height - 23), &textDarkBrush);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rect;
            GetClientRect(hWnd, &rect);

            // 더블 버퍼링 적용 (깜빡임 방지)
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

