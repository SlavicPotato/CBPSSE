#include "pch.h"

#include "render.h"
#include "events.h"

namespace CBP
{
    DRender DRender::m_Instance;

    bool DRender::Initialize()
    {
        if (!m_Instance.m_presentCallbacks.size())
            return true;

        if (!Hook::CheckDst5<0xE8>(m_Instance.CreateD3D11) ||
            !Hook::CheckDst5<0xE8>(m_Instance.UnkPresent))
        {
            m_Instance.Error("Unable to hook, one or more invalid targets");
            return false;
        }

        ASSERT(Hook::Call5(
            ISKSE::GetBranchTrampoline(),
            m_Instance.CreateD3D11,
            reinterpret_cast<uintptr_t>(CreateD3D11_Hook),
            m_Instance.CreateD3D11_O));

        ASSERT(Hook::Call5(
            ISKSE::GetBranchTrampoline(),
            m_Instance.UnkPresent,
            reinterpret_cast<uintptr_t>(Present_Pre),
            m_Instance.UnkPresent_O));

        return true;
    }


    void DRender::CreateD3D11_Hook()
    {
        m_Instance.CreateD3D11_O();

        auto renderManager = BSRenderManager::GetSingleton();
        ASSERT(renderManager != nullptr);
        ASSERT(renderManager->swapChain != nullptr);

        DXGI_SWAP_CHAIN_DESC sd;
        if (renderManager->swapChain->GetDesc(&sd) != S_OK) {
            m_Instance.Error("IDXGISwapChain::GetDesc failed");
            m_Instance.m_presentCallbacks.clear();
            return;
        }

        m_Instance.m_device = renderManager->forwarder;
        m_Instance.m_context = renderManager->context;
        m_Instance.m_swapChain = renderManager->swapChain;

        m_Instance.m_bufferSize.x = static_cast<float>(sd.BufferDesc.Width);
        m_Instance.m_bufferSize.y = static_cast<float>(sd.BufferDesc.Height);
        m_Instance.m_bufferSize.z = m_Instance.m_bufferSize.x / m_Instance.m_bufferSize.y;

        D3D11CreateEventPost evd_post(
            std::addressof(sd), 
            renderManager->forwarder, 
            renderManager->context, 
            renderManager->swapChain);

        IEvents::TriggerEvent(Event::OnD3D11PostCreate, static_cast<void*>(&evd_post));
    }

    void DRender::Present_Pre(uint32_t p1)
    {
        m_Instance.UnkPresent_O(p1);

        for (const auto& f : m_Instance.m_presentCallbacks)
        {
            f();
        }
    }
}