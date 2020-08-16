#include "pch.h"

namespace CBP
{

    Renderer::Renderer(
        ID3D11Device* a_pDevice,
        ID3D11DeviceContext* a_pImmediateContext)
        :
        m_pDevice(a_pDevice),
        m_pImmediateContext(a_pImmediateContext)
    {
        m_states = std::make_unique<DirectX::CommonStates>(a_pDevice);
        m_effect = std::make_unique<DirectX::BasicEffect>(a_pDevice);

        m_effect->SetVertexColorEnabled(true);

        DirectX::ThrowIfFailed(
            CreateInputLayoutFromEffect<VertexType>(a_pDevice, m_effect.get(),
                m_inputLayout.ReleaseAndGetAddressOf())
        );
        
        m_batch = std::make_unique<DirectX::PrimitiveBatch<VertexType>>(a_pImmediateContext);
    }

    void Renderer::GenerateLines(const r3d::DebugRenderer& a_dr)
    {
        m_lines.clear();

        for (const auto& line : a_dr.getLines())
        {
            ItemLine item;

            if (!GetScreenPt(line.point1, line.color1, item.pos1))
                continue;
            if (!GetScreenPt(line.point2, line.color2, item.pos2))
                continue;

            m_lines.emplace_back(item);
        }

    }

    void Renderer::GenerateTris(const r3d::DebugRenderer& a_dr)
    {
        m_tris.clear();

        for (const auto& tri : a_dr.getTriangles())
        {
            ItemTri item;

            if (!GetScreenPt(tri.point1, tri.color1, item.pos1))
                continue;
            if (!GetScreenPt(tri.point2, tri.color2, item.pos2))
                continue;
            if (!GetScreenPt(tri.point3, tri.color3, item.pos3))
                continue;

            m_tris.emplace_back(item);
        }
    }

    void Renderer::Update(const r3d::DebugRenderer& a_dr)
    {
        GenerateLines(a_dr);
        GenerateTris(a_dr);
    }

    void Renderer::Clear()
    {
        m_tris.clear();
        m_lines.clear();
    }

    void Renderer::Draw()
    {
        m_pImmediateContext->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
        m_pImmediateContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
        m_pImmediateContext->RSSetState(m_states->CullCounterClockwise());

        m_effect->Apply(m_pImmediateContext);

        m_pImmediateContext->IASetInputLayout(m_inputLayout.Get());

        m_batch->Begin();

        auto& globalConfig = IConfig::GetGlobalConfig();

        for (const auto& e : m_lines)
            m_batch->DrawLine(e.pos1, e.pos2);

        if (globalConfig.debugRenderer.wireframe)
            for (const auto& e : m_tris) {
                m_batch->DrawLine(e.pos1, e.pos2);
                m_batch->DrawLine(e.pos1, e.pos3);
                m_batch->DrawLine(e.pos3, e.pos2);
            }
        else
            for (const auto& e : m_tris)
                m_batch->DrawTriangle(e.pos1, e.pos2, e.pos3);

        m_batch->End();
    }

    bool Renderer::GetScreenPt(const r3d::Vector3& a_pos, r3d::uint32 a_col, VertexType& a_out)
    {
        static_assert(sizeof(a_col) == 0x4);

        static_assert(sizeof(Vector3::x) == sizeof(NiPoint3::x));
        static_assert(sizeof(Vector3::y) == sizeof(NiPoint3::y));
        static_assert(sizeof(Vector3::z) == sizeof(NiPoint3::z));
        static_assert(offsetof(Vector3, x) == offsetof(NiPoint3, x));
        static_assert(offsetof(Vector3, y) == offsetof(NiPoint3, y));
        static_assert(offsetof(Vector3, z) == offsetof(NiPoint3, z));

        if (!WorldPtToScreenPt3_Internal(g_worldToCamMatrix, g_viewPort, (NiPoint3*)(std::addressof(a_pos)), &a_out.position.x, &a_out.position.y, &a_out.position.z, 1e-5f))
            return false;

        if (a_out.position.x < -0.05f || a_out.position.y < -0.05f || a_out.position.z < -0.05f ||
            a_out.position.x > 1.05f || a_out.position.y > 1.05f || a_out.position.z > 1.05f)
            return false;

        a_out.position.x = (a_out.position.x * 2.0f) - 1.0f;
        a_out.position.y = (a_out.position.y * 2.0f) - 1.0f;
        a_out.position.z = (a_out.position.z * 2.0f) - 1.0f;

        const auto b = reinterpret_cast<uint8_t*>(&a_col);

        a_out.color.x = static_cast<float>(b[0]);
        a_out.color.y = static_cast<float>(b[1]);
        a_out.color.z = static_cast<float>(b[2]);
        a_out.color.w = static_cast<float>(b[3]);

        return true;
    }

}