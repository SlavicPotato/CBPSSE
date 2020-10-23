#include "pch.h"

namespace CBP
{
    Renderer::Renderer(
        ID3D11Device* a_pDevice,
        ID3D11DeviceContext* a_pImmediateContext)
        :
        m_pDevice(a_pDevice),
        m_pImmediateContext(a_pImmediateContext),
        m_contactPointSphereRadius(1.5f),
        m_contactNormalLength(3.0f)
    {
        m_states = std::make_unique<DirectX::CommonStates>(a_pDevice);
        m_effect = std::make_unique<DirectX::BasicEffect>(a_pDevice);

        m_effect->SetVertexColorEnabled(true);

        DirectX::ThrowIfFailed(
            DirectX::CreateInputLayoutFromEffect<VertexType>(a_pDevice, m_effect.get(),
                m_inputLayout.ReleaseAndGetAddressOf())
        );

        m_batch = std::make_unique<DirectX::PrimitiveBatch<VertexType>>(a_pImmediateContext);
    }

    void Renderer::GenerateMovingNodes(
        const simActorList_t& a_actorList,
        float a_radius,
        bool a_centerOfMass,
        Game::ObjectHandle a_markedHandle)
    {
        for (const auto& e : a_actorList)
        {
            if (e.second.IsSuspended())
                continue;

            if (a_centerOfMass) {
                for (const auto& n : e.second)
                    if (n.second.HasMovement())
                        GenerateSphere(n.second.GetCenterOfMass(), a_radius * n.second.GetNodeScale(), MOVING_NODES_COL);
            }
            else {
                for (const auto& n : e.second)
                    if (n.second.HasMovement())
                        GenerateSphere(n.second.GetPos(), a_radius * n.second.GetNodeScale(), MOVING_NODES_COL);
            }

            if (e.first == a_markedHandle)
            {
                auto ht = e.second.GetHeadTransform();
                if (ht)
                    GenerateSphere(
                        *ht * NiPoint3(0.0f, 0.0f, 20.0f),
                        2.0f, ACTOR_MARKER_COL);
            }
        }
    }

    void Renderer::UpdateMovingNodes(
        const simActorList_t& a_actorList,
        float a_radius,
        bool a_centerOfMass,
        Game::ObjectHandle a_markedHandle
    )
    {
        GenerateMovingNodes(a_actorList, a_radius, a_centerOfMass, a_markedHandle);
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

        auto& globalConfig = IConfig::GetGlobal();

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

    
    bool Renderer::GetScreenPt(const btVector3& a_pos, const btVector3& a_col, VertexType& a_out)
    {
        NiPoint3 tmp(a_pos.x(), a_pos.y(), a_pos.z());

        if (!WorldPtToScreenPt3_Internal(
            g_worldToCamMatrix,
            g_viewPort,
            &tmp,
            &a_out.position.x,
            &a_out.position.y,
            &a_out.position.z,
            1e-5f))
        {
            return false;
        }

        if (a_out.position.x < -0.05f || a_out.position.y < -0.05f || a_out.position.z < -0.05f ||
            a_out.position.x > 1.05f || a_out.position.y > 1.05f || a_out.position.z > 1.05f)
            return false;

        a_out.position.x = (a_out.position.x * 2.0f) - 1.0f;
        a_out.position.y = (a_out.position.y * 2.0f) - 1.0f;
        a_out.position.z = (a_out.position.z * 2.0f) - 1.0f;

        a_out.color.x = a_col.x();
        a_out.color.y = a_col.y();
        a_out.color.z = a_col.z();
        a_out.color.w = 1.0f;

        return true;
    }

    bool Renderer::GetScreenPt(const NiPoint3& a_pos, const DirectX::XMFLOAT4& a_col, VertexType& a_out)
    {
        if (!WorldPtToScreenPt3_Internal(
            g_worldToCamMatrix,
            g_viewPort,
            const_cast<NiPoint3*>(std::addressof(a_pos)),
            &a_out.position.x,
            &a_out.position.y,
            &a_out.position.z,
            1e-5f))
        {
            return false;
        }

        if (a_out.position.x < -0.05f || a_out.position.y < -0.05f || a_out.position.z < -0.05f ||
            a_out.position.x > 1.05f || a_out.position.y > 1.05f || a_out.position.z > 1.05f)
            return false;

        a_out.position.x = (a_out.position.x * 2.0f) - 1.0f;
        a_out.position.y = (a_out.position.y * 2.0f) - 1.0f;
        a_out.position.z = (a_out.position.z * 2.0f) - 1.0f;

        a_out.color = a_col;

        return true;
    }

    // Adapted from https://github.com/DanielChappuis/reactphysics3d/blob/master/src/utils/DebugRenderer.cpp#L122
    void Renderer::GenerateSphere(const NiPoint3& a_pos, float a_radius, const DirectX::XMFLOAT4& a_col)
    {
        NiPoint3 vertices[(NB_SECTORS_SPHERE + 1) * (NB_STACKS_SPHERE + 1) + (NB_SECTORS_SPHERE + 1)];

        // Vertices
        const float sectorStep = 2 * float(MATH_PI) / NB_SECTORS_SPHERE;
        const float stackStep = float(MATH_PI) / NB_STACKS_SPHERE;

        for (uint32_t i = 0; i <= NB_STACKS_SPHERE; i++) {

            const float stackAngle = float(MATH_PI) / 2 - i * stackStep;
            const float radiusCosStackAngle = a_radius * std::cos(stackAngle);
            const float z = a_radius * std::sin(stackAngle);

            for (uint32_t j = 0; j <= NB_SECTORS_SPHERE; j++) {

                const float sectorAngle = j * sectorStep;
                const float x = radiusCosStackAngle * std::cos(sectorAngle);
                const float y = radiusCosStackAngle * std::sin(sectorAngle);

                vertices[i * (NB_SECTORS_SPHERE + 1) + j] = a_pos + NiPoint3(x, y, z);
            }
        }

        // Faces
        for (uint32_t i = 0; i < NB_STACKS_SPHERE; i++) {

            uint32_t a1 = i * (NB_SECTORS_SPHERE + 1);
            uint32_t a2 = a1 + NB_SECTORS_SPHERE + 1;

            for (uint32_t j = 0; j < NB_SECTORS_SPHERE; j++, a1++, a2++) {

                // 2 triangles per sector except for the first and last stacks

                if (i != 0)
                {
                    ItemTri item;

                    if (GetScreenPt(vertices[a1], a_col, item.pos1) &&
                        GetScreenPt(vertices[a2], a_col, item.pos2) &&
                        GetScreenPt(vertices[a1 + 1], a_col, item.pos3))
                    {
                        m_tris.emplace_back(item);
                    }

                }

                if (i != (NB_STACKS_SPHERE - 1))
                {
                    ItemTri item;

                    if (GetScreenPt(vertices[a1 + 1], a_col, item.pos1) &&
                        GetScreenPt(vertices[a2], a_col, item.pos2) &&
                        GetScreenPt(vertices[a2 + 1], a_col, item.pos3))
                    {
                        m_tris.emplace_back(item);
                    }
                }
            }
        }
    }

    void Renderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        ItemLine item;

        if (!GetScreenPt(from, color, item.pos1))
            return;
        if (!GetScreenPt(to, color, item.pos2))
            return;

        m_lines.emplace_back(std::move(item));
    }

    void Renderer::drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar /*alpha*/)
    {
        ItemTri item;

        if (!GetScreenPt(v0, color, item.pos1))
            return;
        if (!GetScreenPt(v1, color, item.pos2))
            return;
        if (!GetScreenPt(v2, color, item.pos3))
            return;

        m_tris.emplace_back(std::move(item));
    }

    void Renderer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) 
    {
        drawSphere(PointOnB, m_contactPointSphereRadius, color);
        drawLine(PointOnB, PointOnB + normalOnB * m_contactNormalLength, color);
    }

    void Renderer::draw3dText(const btVector3& location, const char* textString)
    {
    }

    void Renderer::reportErrorWarning(const char* warningString) 
    {
    }

    void Renderer::setDebugMode(int debugMode) {
        m_debugMode = debugMode;
    }

    int Renderer::getDebugMode() const { 
        return m_debugMode; 
    }

}