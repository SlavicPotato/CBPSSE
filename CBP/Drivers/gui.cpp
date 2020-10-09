#include "pch.h"

namespace CBP
{
    DUI DUI::m_Instance;

    DUI::DUI() :
        m_imInitialized(false),
        m_suspended(false),
        m_nextResetIO(false),
        m_uiRenderPerf({ 1000000LL, 0 })
    {
    }

    bool DUI::Initialize()
    {
        DInput::RegisterForKeyEvents(&m_Instance.m_inputEventHandler);

        IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_DUI);
        IEvents::RegisterForEvent(Event::OnExit, OnExit_DUI);
        DRender::AddPresentCallback(Present_Pre);

        return true;
    }


    void DUI::Present_Pre()
    {
        m_Instance.Present_Pre_Impl();
    }

    void DUI::Present_Pre_Impl()
    {
        if (m_suspended)
            return;

        IScopedCriticalSection _(std::addressof(m_lock));

        m_uiRenderPerf.timer.Begin();

        if (m_nextResetIO) {
            m_nextResetIO = false;
            ResetImGuiIO();
        }

        m_keyEvents.ProcessTasks();

        ::ImGui_ImplDX11_NewFrame();
        ::ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        auto it = m_drawCallbacks.begin();
        while (it != m_drawCallbacks.end())
        {
            ImGui::PushID(static_cast<const void*>(it->second));
            bool res = it->second();
            ImGui::PopID();

            if (!res) {
                it = m_drawCallbacks.erase(it);
            }
            else {
                ++it;
            }
        }

        ImGui::Render();
        ::ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (m_drawCallbacks.empty()) 
        {
            ResetImGuiIO();
            Suspend();
        }

        m_uiRenderPerf.timer.End(
            m_uiRenderPerf.current);
    }

    void DUI::OnD3D11PostCreate_DUI(Event, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        m_Instance.info.bufferSize.width = static_cast<float>(info->m_pSwapChainDesc->BufferDesc.Width);
        m_Instance.info.bufferSize.height = static_cast<float>(info->m_pSwapChainDesc->BufferDesc.Height);
        m_Instance.m_WindowHandle = info->m_pSwapChainDesc->OutputWindow;

        RECT rect;
        if (::GetClientRect(info->m_pSwapChainDesc->OutputWindow, &rect) == TRUE)
        {
            m_Instance.m_ioUserData.btsRatio =
            {
                static_cast<float>(info->m_pSwapChainDesc->BufferDesc.Width) / rect.right,
                static_cast<float>(info->m_pSwapChainDesc->BufferDesc.Height) / rect.bottom,
            };
        }
        else
            m_Instance.m_ioUserData.btsRatio = { 1.0f, 1.0f };

        ASSERT(IMGUI_CHECKVERSION());
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = true;
        io.WantSetMousePos = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.DisplaySize = ImVec2(m_Instance.info.bufferSize.width, m_Instance.info.bufferSize.height);
        io.MousePos.x = io.DisplaySize.x / 2.0f;
        io.MousePos.y = io.DisplaySize.y / 2.0f;
        io.UserData = static_cast<void*>(&m_Instance.m_ioUserData);

        if (!m_Instance.conf.imgui_ini.empty())
            io.IniFilename = m_Instance.conf.imgui_ini.c_str();
        else
            io.IniFilename = nullptr;

        ImGui::StyleColorsDark();

        ::ImGui_ImplWin32_Init(info->m_pSwapChainDesc->OutputWindow);
        ::ImGui_ImplDX11_Init(info->m_pDevice, info->m_pImmediateContext);

        m_Instance.m_imInitialized = true;

        m_Instance.pfnWndProc = reinterpret_cast<WNDPROC>(
            ::SetWindowLongPtrA(
                info->m_pSwapChainDesc->OutputWindow,
                GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(WndProc_Hook))
            );

        if (!m_Instance.pfnWndProc)
            m_Instance.Error(
                "[0x%llX] SetWindowLongPtrA failed", info->m_pSwapChainDesc->OutputWindow);

    }

    void DUI::OnExit_DUI(Event, void* data)
    {
        if (m_Instance.m_imInitialized)
        {
            m_Instance.Debug("Shutting down..");

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            m_Instance.m_imInitialized = false;
        }
    }

    LRESULT CALLBACK DUI::WndProc_Hook(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
    {
        LRESULT lr = ::CallWindowProcA(m_Instance.pfnWndProc, hWnd, uMsg, wParam, lParam);

        switch (uMsg)
        {
        case WM_KILLFOCUS:
            QueueResetIO();
            break;
        }

        return lr;
    }

    void DUI::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        if (m_Instance.m_suspended)
            return;

        auto& queue = GetKeyPressQueue();

        switch (keyCode)
        {
        case InputMap::kMacro_MouseButtonOffset:
            queue.AddTask<KeyEventTask>(ev, KeyEventType::MouseButton, 0U);
            break;
        case InputMap::kMacro_MouseButtonOffset + 1:
            queue.AddTask<KeyEventTask>(ev, KeyEventType::MouseButton, 1U);
            break;
        case InputMap::kMacro_MouseButtonOffset + 2:
            queue.AddTask<KeyEventTask>(ev, KeyEventType::MouseButton, 2U);
            break;
        case InputMap::kMacro_MouseWheelOffset:
            queue.AddTask<KeyEventTask>(ev, KeyEventType::MouseWheel, 1.0f);
            break;
        case InputMap::kMacro_MouseWheelOffset + 1:
            queue.AddTask<KeyEventTask>(ev, KeyEventType::MouseWheel, -1.0f);
            break;
        default:
            if (keyCode < InputMap::kMacro_NumKeyboardKeys)
            {
                UINT vkCode;

                switch (keyCode)
                {
                case DIK_LEFT:
                    vkCode = VK_LEFT;
                    break;
                case DIK_RIGHT:
                    vkCode = VK_RIGHT;
                    break;
                case DIK_UP:
                    vkCode = VK_UP;
                    break;
                case DIK_DOWN:
                    vkCode = VK_DOWN;
                    break;
                case DIK_DELETE:
                    vkCode = VK_DELETE;
                    break;
                default:
                    vkCode = MapVirtualKeyW(keyCode, MAPVK_VSC_TO_VK);
                    if (vkCode == 0) {
                        return;
                    }
                }

                WORD Char(0);

                if (ev == KeyEvent::KeyDown)
                {
                    if (GetKeyboardState(keyState)) {
                        if (!ToAscii(vkCode, keyCode, keyState, &Char, 0) != 0) {
                            Char = 0;
                        }
                    }
                }

                queue.AddTask<KeyEventTask>(ev, KeyEventType::Keyboard, vkCode, Char);
            }

            break;
        }
    }

    DUI::KeyEventTask::KeyEventTask(
        KeyEvent a_event, KeyEventType a_eventType, UINT a_val, WCHAR a_k) :
        m_event(a_event),
        m_eventType(a_eventType),
        m_k(a_k)
    {
        b.m_uval = a_val;
    }

    DUI::KeyEventTask::KeyEventTask(
        KeyEvent a_event, KeyEventType a_eventType, float a_val) :
        m_event(a_event),
        m_eventType(a_eventType)
    {
        b.m_fval = a_val;
    }

    DUI::KeyEventTask::KeyEventTask(
        KeyEventType a_eventType) :
        m_event(KeyEvent::None),
        m_eventType(a_eventType)
    {
    }

    void DUI::KeyEventTask::Run()
    {
        auto& io = ImGui::GetIO();

        switch (m_eventType) {
        case KeyEventType::MouseButton:
            io.MouseDown[b.m_uval] = (m_event == KeyEvent::KeyDown ? true : false);
            /*if (!ImGui::IsAnyMouseDown() && ::GetCapture() == m_Instance.m_WindowHandle)
                ::ReleaseCapture();*/
            break;
        case KeyEventType::MouseWheel:
            io.MouseWheel += b.m_fval;
            break;
        case KeyEventType::Keyboard:
            if (m_event == KeyEvent::KeyDown)
            {
                if (b.m_uval < ARRAYSIZE(io.KeysDown))
                    io.KeysDown[b.m_uval] = true;

                if (m_k != 0)
                    io.AddInputCharacterUTF16(m_k);
            }
            else
            {
                if (b.m_uval < ARRAYSIZE(io.KeysDown))
                    io.KeysDown[b.m_uval] = false;
            }
            break;
        }
    }

    void DUI::AddCallback(uint32_t id, const uiDrawCallback_t f)
    {
        IScopedCriticalSection _(std::addressof(m_Instance.m_lock));

        m_Instance.m_drawCallbacks.insert_or_assign(id, f);

        if (m_Instance.m_suspended)
            m_Instance.m_suspended = false;
    }

    void DUI::RemoveCallback(uint32_t id)
    {
        IScopedCriticalSection _(std::addressof(m_Instance.m_lock));

        m_Instance.m_drawCallbacks.erase(id);

        if (m_Instance.m_drawCallbacks.empty())
            m_Instance.Suspend();
    }

    void DUI::ResetImGuiIO()
    {
        auto& io = ImGui::GetIO();

        memset(&io.KeysDown, 0x0, sizeof(io.KeysDown));
        memset(&io.MouseDown, 0x0, sizeof(io.MouseDown));

        io.MouseWheel = 0.0f;
        io.KeyCtrl = false;
        io.KeyShift = false;
        io.KeyAlt = false;

        m_keyEvents.ClearTasks();
    }

    void DUI::Suspend()
    {
        ResetImGuiIO();
        m_suspended = true;
    }

}