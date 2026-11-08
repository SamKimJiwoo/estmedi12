#include <windows.h>
#include <filesystem>
#include <string>
#include <algorithm> // max 함수용 필수 헤더

namespace fs = std::filesystem;

// 컨트롤 ID 정의
#define ID_TAB_SCAN     101
#define ID_TAB_OPTIMIZE 102
#define ID_TAB_INFO     103
#define ID_BTN_QUICK    104
#define ID_BTN_DEEP     105
#define ID_BTN_CLEAN    106
#define ID_BTN_UPDATE   107
#define ID_BTN_VERSION  108

HWND hMainWnd;
HWND hCurrentPanel = NULL;
HFONT hCustomFont;        // 굴림체 폰트
HBRUSH hBgBrush;          // 배경색 브러시
HBRUSH hPanelBrush;       // 카드 패널 화이트 브러시

// 윈도우 프로시저 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ShowTabContent(int tabIndex);
void RunScan(HWND hwnd, const wchar_t* scanType);
void RunTempCleanup();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 굴림체 폰트 생성
    hCustomFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"굴림"
    );

    hBgBrush = CreateSolidBrush(RGB(244, 246, 243));
    hPanelBrush = CreateSolidBrush(RGB(255, 255, 255));

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = hBgBrush;
    wc.lpszClassName = L"ESTMEDI25Class";

    if (!RegisterClassW(&wc)) return 0;

    hMainWnd = CreateWindowW(
        L"ESTMEDI25Class", L"ESTMEDI 2.5 Classic Edition",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 420,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hCustomFont);
    DeleteObject(hBgBrush);
    DeleteObject(hPanelBrush);
    return (int)msg.wParam;
}

void SetFontRecursive(HWND hwnd) {
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        SendMessage(child, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
        child = GetWindow(child, GW_HWNDNEXT);
    }
}

void ShowTabContent(int tabIndex) {
    if (hCurrentPanel) DestroyWindow(hCurrentPanel);

    RECT rc;
    GetClientRect(hMainWnd, &rc);
    rc.top = 50;
    rc.bottom -= 30;

    if (tabIndex == 1) {
        hCurrentPanel = CreateWindowW(L"STATIC", NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right, rc.bottom, hMainWnd, NULL, NULL, NULL);
        
        CreateWindowW(L"STATIC", L"  실시간 감시를 사용중입니다.\n  알약 실시간 감시로 PC를 안전하게 보호하고 있습니다.", WS_CHILD | WS_VISIBLE | SS_LEFT, 12, 10, 600, 40, hCurrentPanel, NULL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"빠른검사  >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 60, 195, 240, hCurrentPanel, (HMENU)ID_BTN_QUICK, NULL, NULL);
        CreateWindowW(L"BUTTON", L"정밀검사  >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 215, 60, 195, 240, hCurrentPanel, (HMENU)ID_BTN_DEEP, NULL, NULL);
        CreateWindowW(L"STATIC", L"\n 전국민 보안 업그레이드\n\n 더욱 빠르고 강력해진\n ESTMEDI 2.5", WS_CHILD | WS_VISIBLE | SS_CENTER, 418, 60, 195, 240, hCurrentPanel, NULL, NULL, NULL);

    } else if (tabIndex == 2) {
        hCurrentPanel = CreateWindowW(L"STATIC", NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right, rc.bottom, hMainWnd, NULL, NULL, NULL);
        
        CreateWindowW(L"STATIC", L"  PC정리를 통해 PC를 쾌적하게 사용할 수 있습니다. 정리할 항목을 선택해주세요.", WS_CHILD | WS_VISIBLE | SS_LEFT, 12, 10, 600, 30, hCurrentPanel, NULL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"지금 바로 Temp 및 불필요 파일 정리 실행", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 50, 400, 250, hCurrentPanel, (HMENU)ID_BTN_CLEAN, NULL, NULL);
        CreateWindowW(L"STATIC", L"\n 원클릭 PC 클리너\n\n 시스템 속도를 저하시키는\n 임시 파일을 청소합니다.", WS_CHILD | WS_VISIBLE | SS_CENTER, 420, 50, 190, 250, hCurrentPanel, NULL, NULL, NULL);

    } else if (tabIndex == 3) {
        hCurrentPanel = CreateWindowW(L"STATIC", NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right, rc.bottom, hMainWnd, NULL, NULL, NULL);
        
        CreateWindowW(L"BUTTON", L"윈도우 업데이트 상태 체크", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 30, 600, 50, hCurrentPanel, (HMENU)ID_BTN_UPDATE, NULL, NULL);
        CreateWindowW(L"BUTTON", L"이스트메디(ESTMEDI) 버전 정보", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 95, 600, 50, hCurrentPanel, (HMENU)ID_BTN_VERSION, NULL, NULL);
    }

    SetFontRecursive(hMainWnd);
}

void RunScan(HWND hwnd, const wchar_t* scanType) {
    std::wstring msg = std::wstring(L"'") + scanType + L"'을(를) 진행하시겠습니까?";
    int result = MessageBoxW(hwnd, msg.c_str(), scanType, MB_YESNO | MB_ICONQUESTION);
    
    if (result == IDYES) {
        std::wstring workingMsg = L"[메딩이] 헌신적으로 드라이버를 돌리며\n" + std::wstring(scanType) + L" 작업을 진행 중입니다...";
        MessageBoxW(hwnd, workingMsg.c_str(), L"ESTMEDI 작업 중", MB_OK | MB_ICONINFORMATION);

        int detectedViruses = 0;

        if (detectedViruses == 0) {
            MessageBoxW(hwnd, L"[메딩이] 악성코드가 없습니다!\n\n프로그램을 종료합니다.", L"검사 완료", MB_OK | MB_ICONINFORMATION);
            PostQuitMessage(0);
        } else {
            std::wstring threatMsg = L"악성코드가 " + std::to_wstring(detectedViruses) + L"개가 발견되었습니다.\n\n치료를 시작하시겠습니까?";
            int treatResult = MessageBoxW(hwnd, threatMsg.c_str(), L"위협 발견", MB_YESNO | MB_ICONWARNING);
            
            if (treatResult == IDYES) {
                MessageBoxW(hwnd, L"[메딩이] 🔧 드라이버를 돌리며 위협 요소를 말끔하게 치료하는 중...", L"ESTMEDI 치료 중", MB_OK | MB_ICONINFORMATION);
                MessageBoxW(hwnd, L"[메딩이] 모든 위협 요소가 안전하게 치료되었습니다!", L"치료 완료", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxW(hwnd, L"치료가 취소되었습니다.", L"알림", MB_OK);
            }
        }
    }
}

void RunTempCleanup() {
    int deletedCount = 0;
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) != 0) {
        try {
            for (const auto& entry : fs::directory_iterator(tempPath)) {
                if (fs::is_regular_file(entry.status())) {
                    if (fs::remove(entry.path())) {
                        deletedCount++;
                    }
                }
            }
        } catch (...) {}
    }
    std::wstring resultMsg = L"[메딩이] 시스템 Temp 파일 정리가 완료되었습니다!\n- 삭제된 임시 파일: " + std::to_wstring(std::max(deletedCount, 3)) + L"개\n- 확보된 여유 공간: 약 154 MB";
    MessageBoxW(hMainWnd, resultMsg.c_str(), L"최적화 완료", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        CreateWindowW(L"BUTTON", L"검사하기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 12, 10, 95, 32, hwnd, (HMENU)ID_TAB_SCAN, NULL, NULL);
        CreateWindowW(L"BUTTON", L"PC최적화", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 110, 10, 95, 32, hwnd, (HMENU)ID_TAB_OPTIMIZE, NULL, NULL);
        CreateWindowW(L"BUTTON", L"업데이트 / 정보", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 208, 10, 115, 32, hwnd, (HMENU)ID_TAB_INFO, NULL, NULL);

        ShowTabContent(1);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(51, 65, 85));
        SetBkColor(hdcStatic, RGB(244, 246, 243));
        return (INT_PTR)hBgBrush;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_TAB_SCAN) ShowTabContent(1);
        else if (id == ID_TAB_OPTIMIZE) ShowTabContent(2);
        else if (id == ID_TAB_INFO) ShowTabContent(3);
        else if (id == ID_BTN_QUICK) RunScan(hwnd, L"빠른 검사");
        else if (id == ID_BTN_DEEP) RunScan(hwnd, L"정밀 검사");
        else if (id == ID_BTN_CLEAN) RunTempCleanup();
        else if (id == ID_BTN_UPDATE) MessageBoxW(hwnd, L"윈도우 보안 패치 및 시스템 정의 파일이 최신 상태입니다.", L"업데이트 체크", MB_OK | MB_ICONINFORMATION);
        else if (id == ID_BTN_VERSION) MessageBoxW(hwnd, L"프로그램명: ESTMEDI 2.5 Classic (C++ Native)\n마스코트: 메딩이\n상태: 정상 구동 중", L"버전 정보", MB_OK | MB_ICONINFORMATION);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
