#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#include <tchar.h>

// DirectX9 인터페이스
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Direct3D 초기화 함수 선언
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 1. 윈도우 클래스 등록
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("EstmediImGuiClass"), NULL };
    ::RegisterClassEx(&wc);

    // 2. 메인 메인 윈도우 생성 (알약 2.5 비율 레퍼런스: 780x520)
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("ESTMEDI 1.3 Beta"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 100, 100, 780, 520, NULL, NULL, wc.hInstance, NULL);

    // 3. Direct3D 초기화
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // 4. ImGui 컨텍스트 생성 및 설정
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL; // 설정 파일 생성 방지

    // Win32 & DirectX9 백엔드 연결
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // 5. 알약 2.5 감성의 녹색/모던 스티일 폰트 및 커스텀 컬러스키머 설정
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 4.0f;
    style.ItemSpacing = ImVec2(10, 8);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = ImVec4(0.96f, 0.97f, 0.98f, 1.00f); // 연한 회색 배경
    colors[ImGuiCol_Header]             = ImVec4(0.35f, 0.65f, 0.15f, 1.00f); // 메인 녹색 포인트
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.42f, 0.75f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.30f, 0.58f, 0.12f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.35f, 0.65f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.42f, 0.75f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.30f, 0.58f, 0.12f, 1.00f);
    colors[ImGuiCol_Text]               = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // 메인 루프
    bool done = false;
    static int currentTab = 0; // 0: 검사하기, 1: PC최적화, 2: 플러스보안

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        // ImGui 프레임 시작
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 메인 프레임 윈도우 구성
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("MainPanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // ----------------------------------------------------
        // [1] 상단 알약 2.5 톤 네비게이션 탭 & 실시간 감시
        // ----------------------------------------------------
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.35f, 0.65f, 0.15f, 1.00f));
        ImGui::BeginChild("TopNav", ImVec2(0, 50), false);
        
        ImGui::SetCursorPos(ImVec2(15, 10));
        if (ImGui::Button(currentTab == 0 ? "[ 검사하기 ]" : "검사하기", ImVec2(100, 30))) currentTab = 0;
        ImGui::SameLine();
        if (ImGui::Button(currentTab == 1 ? "[ PC최적화 ]" : "PC최적화", ImVec2(100, 30))) currentTab = 1;
        ImGui::SameLine();
        if (ImGui::Button(currentTab == 2 ? "[ 플러스보안 ]" : "플러스보안", ImVec2(100, 30))) currentTab = 2;

        // 우측 상단 실시간 감시 상태 표시
        ImGui::SameLine(ImGui::GetWindowWidth() - 170);
        ImGui::SetCursorPosY(15);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "실시간 감시 : ON");
        
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // ----------------------------------------------------
        // [2] 메인 대시보드 및 카드 영역
        // ----------------------------------------------------
        ImGui::SetCursorPosY(65);
        ImGui::Indent(20);
        ImGui::TextSize();
        ImGui::TextColored(ImVec4(0.1f, 0.1f, 0.1f, 1.0f), "PC가 안전하고 쾌적하게 보호되고 있습니다.");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "엔진 버전: v1.3.0 Beta  |  실시간 보안 활성화 중");
        ImGui::Unindent(20);

        ImGui::SetCursorPosY(130);
        
        // 3분할 스마트 카드 (1 & 2: 기능 카드, 3: 스마트 정보 배너)
        ImGui::BeginGroup();
        {
            // 카드 1: 빠른 검사
            ImGui::BeginChild("Card1", ImVec2(230, 260), true);
            ImGui::Text("\n   [ 빠른 검사 ]");
            ImGui::Separator();
            ImGui::Text("\n 주요 시스템 영역과\n 메모리를 신속하게\n 검사합니다.");
            ImGui::SetCursorPosY(200);
            if (ImGui::Button("검사 시작", ImVec2(210, 40))) {
                // 빠른 검사 동작 로직
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 15);

            // 카드 2: PC 최적화
            ImGui::BeginChild("Card2", ImVec2(230, 260), true);
            ImGui::Text("\n   [ PC 최적화 ]");
            ImGui::Separator();
            ImGui::Text("\n 불필요한 임시 파일과\n 시스템 레지스트리를\n 정리합니다.");
            ImGui::SetCursorPosY(200);
            if (ImGui::Button("최적화 실행", ImVec2(210, 40))) {
                // PC 최적화 동작 로직
            }
            ImGui::EndChild();

            ImGui::SameLine(0, 15);

            // 카드 3: 우측 스마트 정보창 (스마트 딥블루 포인트)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.16f, 0.22f, 1.00f));
            ImGui::BeginChild("Card3", ImVec2(230, 260), true);
            ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "\n  ESTMEDI 1.3 BETA");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), 
                "\n - 차세대 보안 엔진 적용\n"
                " - 실시간 행위 기반 감지\n"
                " - 스마트 메모리 최적화\n\n"
                " 베타 테스트 진행 중입니다."
            );
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::EndGroup();

        // ----------------------------------------------------
        // [3] 하단 푸터 (저작권 표시 및 상태바)
        // ----------------------------------------------------
        ImGui::SetCursorPosY(445);
        ImGui::Separator();
        ImGui::SetCursorPosY(455);
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), " ESTMEDI v1.3 Beta  |  Copyright (c) ISSA STUDIO. All rights reserved.");
        ImGui::SameLine(ImGui::GetWindowWidth() - 190);
        if (ImGui::Button("DB 업데이트", ImVec2(170, 22))) {
            // 업데이트 클릭 로직
        }

        ImGui::End();

        // ImGui 렌더링
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(245, 247, 250), 1.0f, 0);
        
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // 종료 정리
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// ----------------------------------------------------
// Direct3D 9 보조 함수 모음 (WinXP~Win11 범용)
// ----------------------------------------------------
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
