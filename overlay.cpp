#include "pch.h"
#include "overlay.h"
#include "menu.h"
#include "config.h"
#include "theme.h"
#include "HackManager.h"

#include <d3d11.h>
#include <dxgi.h>

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "kiero.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* PresentFn)(IDXGISwapChain*, UINT, UINT);

namespace
{
    HMODULE g_module = nullptr;
    PresentFn g_origPresent = nullptr;
    ID3D11Device* g_device = nullptr;
    ID3D11DeviceContext* g_context = nullptr;
    ID3D11RenderTargetView* g_renderTarget = nullptr;
    HWND g_window = nullptr;
    WNDPROC g_origWndProc = nullptr;
    UINT g_width = 0;
    UINT g_height = 0;

    volatile long g_initialized = 0;
    volatile long g_tornDown = 0;
    bool g_cursorVisible = false;
}

volatile long g_showMenu = 1;
volatile long g_wantUnload = 0;

namespace
{
    bool CreateRenderTarget(IDXGISwapChain* swapChain)
    {
        ID3D11Texture2D* backBuffer = nullptr;
        if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
            return false;
        HRESULT hr = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTarget);
        backBuffer->Release();
        return SUCCEEDED(hr);
    }

    void ReleaseRenderTarget()
    {
        if (g_renderTarget)
        {
            g_renderTarget->Release();
            g_renderTarget = nullptr;
        }
    }

    void SetCursorVisible(bool visible)
    {
        if (g_cursorVisible == visible)
            return;

        if (visible)
            while (ShowCursor(TRUE) < 0) {}
        else
            while (ShowCursor(FALSE) >= 0) {}

        g_cursorVisible = visible;
    }

    bool IsMouseMessage(UINT msg)
    {
        switch (msg)
        {
        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            return true;
        default:
            return false;
        }
    }

    LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool InitOverlay(IDXGISwapChain* swapChain)
    {
        if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_device)))
            return false;
        g_device->GetImmediateContext(&g_context);

        DXGI_SWAP_CHAIN_DESC desc{};
        swapChain->GetDesc(&desc);
        g_window = desc.OutputWindow;
        g_width = desc.BufferDesc.Width;
        g_height = desc.BufferDesc.Height;

        if (!CreateRenderTarget(swapChain))
            return false;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        LoadFonts();
        ApplyClassicTheme();

        ImGui_ImplWin32_Init(g_window);
        ImGui_ImplDX11_Init(g_device, g_context);

        g_origWndProc = (WNDPROC)SetWindowLongPtrW(g_window, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

        InterlockedExchange((volatile long*)&g_initialized, 1);
        return true;
    }

    void HandleResize(IDXGISwapChain* swapChain)
    {
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swapChain->GetDesc(&desc)))
            return;
        if (desc.BufferDesc.Width == g_width && desc.BufferDesc.Height == g_height)
            return;
        g_width = desc.BufferDesc.Width;
        g_height = desc.BufferDesc.Height;
        ReleaseRenderTarget();
        CreateRenderTarget(swapChain);
    }

    void Teardown()
    {
        if (InterlockedExchange((volatile long*)&g_tornDown, 1) != 0)
            return;

        SetCursorVisible(false);

        if (g_window && g_origWndProc)
        {
            SetWindowLongPtrW(g_window, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);
            g_origWndProc = nullptr;
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        ReleaseRenderTarget();
        if (g_context) { g_context->Release(); g_context = nullptr; }
        if (g_device) { g_device->Release(); g_device = nullptr; }
    }

    LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_KEYDOWN && wParam == VK_INSERT)
        {
            InterlockedExchange((volatile long*)&g_showMenu, g_showMenu ? 0 : 1);
            return 0;
        }
        if (msg == WM_KEYDOWN && wParam == VK_END)
        {
            InterlockedExchange((volatile long*)&g_wantUnload, 1);
            return 0;
        }

        if (g_showMenu && !g_wantUnload)
        {
            ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();
            if (IsMouseMessage(msg) && io.WantCaptureMouse)
                return 1;
            if ((msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR) && io.WantCaptureKeyboard)
                return 1;
        }

        return CallWindowProcW(g_origWndProc, hWnd, msg, wParam, lParam);
    }

    HRESULT __stdcall PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
    {
        if (!g_tornDown)
        {
            if (g_wantUnload)
            {
                Teardown();
                return g_origPresent(swapChain, syncInterval, flags);
            }

            if (!g_initialized)
            {
                if (!InitOverlay(swapChain))
                    return g_origPresent(swapChain, syncInterval, flags);
            }
            else
            {
                HandleResize(swapChain);
            }

            SetCursorVisible(g_showMenu != 0);
            if (g_showMenu)
                ClipCursor(nullptr);

            CHackManager::Get().SyncFromConfig(g_config);
            CHackManager::Get().Update();

            if (g_showMenu || g_config.watermark)
            {
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();

                ImGuiIO& io = ImGui::GetIO();
                RECT clientRc{};
                if (g_window && GetClientRect(g_window, &clientRc) && clientRc.right > 0 && clientRc.bottom > 0)
                {
                    io.DisplaySize = ImVec2((float)clientRc.right, (float)clientRc.bottom);
                    io.DisplayFramebufferScale = ImVec2((float)g_width / clientRc.right, (float)g_height / clientRc.bottom);
                }
                else
                {
                    io.DisplaySize = ImVec2((float)g_width, (float)g_height);
                    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
                }

                ImGui::NewFrame();

                RenderUI();

                ImGui::Render();
                g_context->OMSetRenderTargets(1, &g_renderTarget, nullptr);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
        }

        return g_origPresent(swapChain, syncInterval, flags);
    }

    DWORD WINAPI MainThread(LPVOID)
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
            kiero::bind(8, (void**)&g_origPresent, (void*)PresentHook);

        while (!g_wantUnload)
            Sleep(80);

        while (!g_tornDown)
            Sleep(20);

        Sleep(400);
        kiero::shutdown();
        FreeLibraryAndExitThread(g_module, 0);
        return 0;
    }
}

void Overlay_Start(HMODULE hModule)
{
    DisableThreadLibraryCalls(hModule);
    g_module = hModule;
    CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
}
