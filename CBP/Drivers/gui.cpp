#include "pch.h"

namespace CBP
{
    DUI DUI::m_Instance;

    DUI::DUI() :
        m_imInitialized(false),
        m_nextResetIO(false),
        m_suspended(true),
        state({ 0, 0, 0, false, false }),
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

        auto it = m_drawTasks.begin();
        while (it != m_drawTasks.end())
        {
            ImGui::PushID(static_cast<const void*>(it->second));
            bool res = it->second->Run();
            ImGui::PopID();

            if (!res)
            {
                OnTaskRemove(it->second);
                it = m_drawTasks.erase(it);
            }
            else {
                ++it;
            }
        }

        ImGui::Render();
        ::ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (m_drawTasks.empty())
            Suspend();

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
                        if (ToAscii(vkCode, keyCode, keyState, &Char, 0) == 0) {
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
                if (b.m_uval < std::size(io.KeysDown))
                    io.KeysDown[b.m_uval] = true;

                if (m_k != 0)
                    io.AddInputCharacterUTF16(m_k);
            }
            else
            {
                if (b.m_uval < std::size(io.KeysDown))
                    io.KeysDown[b.m_uval] = false;
            }
            break;
        }
    }

    static UInt32 s_controlDisableFlags =
        USER_EVENT_FLAG::kAll;

    static UInt8 s_byChargenDisableFlags =
        PlayerCharacter::kDisableSaving |
        PlayerCharacter::kDisableWaiting;

    void DUI::LockControls(bool a_switch)
    {
        state.controlsLocked = a_switch;
        DTasks::AddTask([a_switch]()
            {
                auto im = InputManager::GetSingleton();

                if (im)
                    im->EnableControls(s_controlDisableFlags, !a_switch);

                auto player = *g_thePlayer;
                if (player)
                {
                    if (a_switch)
                        player->byCharGenFlag |= s_byChargenDisableFlags;
                    else
                        player->byCharGenFlag &= ~s_byChargenDisableFlags;
                }

            });
    }

    void DUI::FreezeTime(bool a_switch)
    {
        state.timeFrozen = a_switch;
        DTasks::AddTask([a_switch]()
            {
                auto unk = Game::Unk00::GetSingleton();
                unk->SetGlobalTimeMultiplier(a_switch ? 1e-006f : 1.0f, true);
            });
    }

    void DUI::AddTask(uint32_t id, UIRenderTaskBase* a_task)
    {
        IScopedCriticalSection _(std::addressof(m_Instance.m_lock));

        auto it = m_Instance.m_drawTasks.emplace(id, a_task);
        if (!it.second)
            return;

        a_task->m_state.holdsLock = a_task->m_options.lock;
        a_task->m_state.holdsFreeze = a_task->m_options.freeze;

        if (a_task->m_state.holdsLock)
            m_Instance.state.lockCounter++;

        if (a_task->m_state.holdsFreeze)
            m_Instance.state.freezeCounter++;

        if (m_Instance.m_suspended)
            m_Instance.m_suspended = false;

        if (!m_Instance.state.controlsLocked && m_Instance.state.lockCounter > 0)
            m_Instance.LockControls(true);

        if (!m_Instance.state.timeFrozen && m_Instance.state.freezeCounter > 0)
            m_Instance.FreezeTime(true);
    }

    void DUI::RemoveTask(uint32_t id)
    {
        IScopedCriticalSection _(std::addressof(m_Instance.m_lock));

        auto it = m_Instance.m_drawTasks.find(id);
        if (it == m_Instance.m_drawTasks.end())
            return;

        m_Instance.OnTaskRemove(it->second);

        m_Instance.m_drawTasks.erase(it);

        if (m_Instance.m_drawTasks.empty())
            m_Instance.Suspend();
    }

    void DUI::EvaluateTaskState()
    {
        DTasks::AddTask(&m_Instance.m_evalTaskState);
    }

    void DUI::EvaluateTaskStateTask::Run()
    {
        m_Instance.EvaluateTaskStateImpl();
    }

    void DUI::EvaluateTaskStateImpl()
    {
        IScopedCriticalSection _(std::addressof(m_lock));

        for (auto& e : m_drawTasks)
        {
            if (e.second->m_options.lock != e.second->m_state.holdsLock)
            {
                e.second->m_state.holdsLock = e.second->m_options.lock;

                if (e.second->m_state.holdsLock)
                    state.lockCounter++;
                else
                    state.lockCounter--;
            }

            if (e.second->m_options.freeze != e.second->m_state.holdsFreeze)
            {
                e.second->m_state.holdsFreeze = e.second->m_options.freeze;

                if (e.second->m_state.holdsFreeze)
                    state.freezeCounter++;
                else
                    state.freezeCounter--;
            }
        }

        if (state.controlsLocked)
        {
            if (state.lockCounter == 0)
                LockControls(false);
        }
        else
        {
            if (state.lockCounter > 0)
                LockControls(true);
        }

        if (state.timeFrozen)
        {
            if (state.freezeCounter == 0)
                FreezeTime(false);
        }
        else
        {
            if (state.freezeCounter > 0)
                FreezeTime(true);
        }
    }

    void DUI::OnTaskRemove(UIRenderTaskBase* a_task)
    {
        if (a_task->m_state.holdsLock)
            state.lockCounter--;

        if (a_task->m_state.holdsFreeze)
            state.freezeCounter--;

        if (state.controlsLocked && state.lockCounter == 0)
            LockControls(false);

        if (state.timeFrozen && state.freezeCounter == 0)
            FreezeTime(false);
    }

    void DUI::ResetImGuiIO()
    {
        auto& io = ImGui::GetIO();

        memset(io.KeysDown, 0x0, sizeof(io.KeysDown));
        memset(io.MouseDown, 0x0, sizeof(io.MouseDown));

        io.MouseWheel = 0.0f;
        io.KeyCtrl = false;
        io.KeyShift = false;
        io.KeyAlt = false;

        m_keyEvents.ClearTasks();
    }

    void DUI::Suspend()
    {
        ResetImGuiIO();

        if (state.controlsLocked)
        {
            state.lockCounter = 0;
            LockControls(false);
        }
        if (state.timeFrozen)
        {
            state.freezeCounter = 0;
            FreezeTime(false);
        }
        m_suspended = true;
    }

    bool UIRenderTaskBase::RunEnableChecks()
    {
        if (Game::InPausedMenu()) {
            Game::Debug::Notification("UI unavailable while in menu");
            return false;
        }

        if (!m_options.enableChecks)
            return true;

        auto player = *g_thePlayer;
        if (!player)
            return false;

        if (player->IsInCombat()) {
            Game::Debug::Notification("UI unavailable while in combat");
            return false;
        }

        auto pl = Game::ProcessLists::GetSingleton();
        if (pl && pl->GuardsPursuing(player)) {
            Game::Debug::Notification("UI unavailable while pursued by guards");
            return false;
        }

        auto tm = MenuTopicManager::GetSingleton();
        if (tm && tm->GetDialogueTarget() != nullptr) {
            Game::Debug::Notification("UI unavailable while in a conversation");
            return false;
        }

        if (player->unkBDA & PlayerCharacter::FlagBDA::kAIDriven) {
            Game::Debug::Notification("UI unavailable while player is AI driven");
            return false;
        }

        if (player->byCharGenFlag & PlayerCharacter::ByCharGenFlag::kHandsBound) {
            Game::Debug::Notification("UI unavailable while player hands are bound");
            return false;
        }

        return true;
    }
}