#include "pch.h"

namespace CBP
{
    const D3D11_INPUT_ELEMENT_DESC VertexPositionColorAV::InputElements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    static_assert(sizeof(VertexPositionColorAV) == 32, "Vertex struct/layout mismatch");

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
        bool a_moving,
        bool a_centerOfGravity,
        Game::ObjectHandle a_markedHandle)
    {
        auto& globalConfig = IConfig::GetGlobal();

        Bullet::btTransformEx tf;

        for (const auto& e : a_actorList)
        {
            if (e.second.IsSuspended())
                continue;

            if (a_moving || a_centerOfGravity)
            {
                auto& nl = e.second.GetNodeList();

                int count = nl.size();
                for (int i = 0; i < count; i++)
                {
                    auto& n = nl[i];

                    if (!n->HasMotion())
                        continue;

                    if (a_moving)
                    {
                        auto& tf = n->GetWorldTransform();
                        GenerateSphere(btVector3(tf.pos.x, tf.pos.y, tf.pos.z), a_radius * tf.scale, globalConfig.debugRenderer.colors.movingNodes);
                    }

                    if (a_centerOfGravity)
                    {
                        n->GetParentWorldTransform(tf);
                        GenerateSphere(tf * n->GetCenterOfGravity(), a_radius * tf.getScale(), globalConfig.debugRenderer.colors.movingNodesCOG);
                    }
                }
            }

            if (e.first == a_markedHandle)
            {
                if (e.second.GetHeadTransform(tf))
                {
                    GenerateSphere(
                        tf * btVector3(0.0f, 0.0f, 20.0f),
                        2.0f * tf.getScale(), globalConfig.debugRenderer.colors.actorMarker);
                }
            }
        }
    }

    void Renderer::GenerateMotionConstraints(
        const simActorList_t& a_actorList,
        float a_radius)
    {
        auto& globalConfig = IConfig::GetGlobal();

        Bullet::btTransformEx tf;

        for (const auto& e : a_actorList)
        {
            if (e.second.IsSuspended())
                continue;

            auto& nl = e.second.GetNodeList();

            int count = nl.size();
            for (int i = 0; i < count; i++)
            {
                auto n = nl[i];

                if (!n->HasMotion())
                    continue;

                auto& conf = n->GetConfig();
                auto mc = conf.ex.motionConstraints;

                n->GetParentWorldTransform(tf);

                if ((mc & MotionConstraints::Box) == MotionConstraints::Box)
                {
                    drawBox(
                        conf.fp.vec.maxOffsetN,
                        conf.fp.vec.maxOffsetP,
                        tf,
                        globalConfig.debugRenderer.colors.constraintBox
                    );

                }

                GenerateSphere(tf * n->GetVirtualPos(), a_radius * tf.getScale(), globalConfig.debugRenderer.colors.virtualPosition);

                if ((mc & MotionConstraints::Sphere) == MotionConstraints::Sphere)
                {
                    tf.getOrigin() = tf * conf.fp.vec.maxOffsetSphereOffset;

                    drawSphere(
                        conf.fp.f32.maxOffsetSphereRadius * tf.getScale(),
                        tf,
                        globalConfig.debugRenderer.colors.constraintSphere
                    );
                }

            }
        }
    }

    void Renderer::Clear()
    {
        m_tris.clear();
        m_lines.clear();
    }

    void Renderer::ReleaseGeometry()
    {
        if (!m_tris.empty())
            m_tris.swap(decltype(m_tris)());

        if (!m_lines.empty())
            m_lines.swap(decltype(m_lines)());
    }

    using namespace DirectX;
    using namespace DirectX::SimpleMath;

    void Renderer::Draw()
    {
        D3D11StateBackup _(m_pImmediateContext);

        m_pImmediateContext->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
        m_pImmediateContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
        m_pImmediateContext->RSSetState(m_states->CullCounterClockwise());

        m_effect->Apply(m_pImmediateContext);

        m_pImmediateContext->IASetInputLayout(m_inputLayout.Get());

        m_batch->Begin();

        for (const auto& e : m_lines)
            m_batch->DrawLine(e.v0, e.v1);

        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.debugRenderer.wireframe) {
            for (const auto& e : m_tris) {
                m_batch->DrawLine(e.v0, e.v1);
                m_batch->DrawLine(e.v0, e.v2);
                m_batch->DrawLine(e.v2, e.v1);
            }
        }
        else {
            for (const auto& e : m_tris)
                m_batch->DrawTriangle(e.v0, e.v1, e.v2);
        }

        m_batch->End();


    }

    static const auto s_2 = DirectX::XMVectorReplicate(2.0f);
    static const auto s_1 = DirectX::XMVectorReplicate(1.0f);

    void Renderer::FillScreenPt(VertexType& a_out, const btVector3& a_col)
    {
        a_out.position = DirectX::XMVectorSubtract(DirectX::XMVectorMultiply(a_out.position, s_2), s_1);
        a_out.color = a_col.get128();
    }

    void Renderer::FillScreenPt(VertexType& a_out, const DirectX::XMVECTOR& a_col)
    {
        a_out.position = DirectX::XMVectorSubtract(DirectX::XMVectorMultiply(a_out.position, s_2), s_1);
        a_out.color = a_col;
    }

    bool Renderer::GetScreenPt(const btVector3& a_pos, VertexType& a_out)
    {
        return WorldPtToScreenPt3_Internal(
            g_worldToCamMatrix,
            g_viewPort,
            reinterpret_cast<NiPoint3*>(const_cast<btVector3*>(std::addressof(a_pos))),
            &a_out.position.m128_f32[0],
            &a_out.position.m128_f32[1],
            &a_out.position.m128_f32[2],
            1e-5f);
    }

    bool Renderer::IsValidPosition(const DirectX::XMVECTOR& a_pos)
    {
        return a_pos.m128_f32[0] > -0.05f && a_pos.m128_f32[1] > -0.05f && a_pos.m128_f32[2] > -0.05f &&
            a_pos.m128_f32[0] < 1.05f && a_pos.m128_f32[1] < 1.05f && a_pos.m128_f32[2] < 1.05f;
    }

    template <class T>
    bool Renderer::GetTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const T& a_col, ItemTri& a_out)
    {
        if (!GetScreenPt(v0, a_out.v0))
            return false;
        if (!GetScreenPt(v1, a_out.v1))
            return false;
        if (!GetScreenPt(v2, a_out.v2))
            return false;

        if (!IsValidPosition(a_out.v0.position) &&
            !IsValidPosition(a_out.v1.position) &&
            !IsValidPosition(a_out.v2.position))
        {
            return false;
        }

        FillScreenPt(a_out.v0, a_col);
        FillScreenPt(a_out.v1, a_col);
        FillScreenPt(a_out.v2, a_col);

        return true;
    }

    template <class T>
    bool Renderer::GetLine(const btVector3& v0, const btVector3& v1, const T& a_col, ItemLine& a_out)
    {
        if (!GetScreenPt(v0, a_out.v0))
            return false;
        if (!GetScreenPt(v1, a_out.v1))
            return false;

        if (!IsValidPosition(a_out.v0.position) &&
            !IsValidPosition(a_out.v1.position))
        {
            return false;
        }

        FillScreenPt(a_out.v0, a_col);
        FillScreenPt(a_out.v1, a_col);

        return true;
    }

    // Adapted from https://github.com/DanielChappuis/reactphysics3d/blob/master/src/utils/DebugRenderer.cpp#L122
    void Renderer::GenerateSphere(const btVector3& a_pos, float a_radius, const DirectX::XMVECTOR& a_col)
    {
        btVector3 vertices[(NB_SECTORS_SPHERE + 1) * (NB_STACKS_SPHERE + 1) + (NB_SECTORS_SPHERE + 1)];

        // Vertices
        const float sectorStep = 2 * float(MATH_PI) / NB_SECTORS_SPHERE;
        const float stackStep = float(MATH_PI) / NB_STACKS_SPHERE;

        for (std::uint32_t i = 0; i <= NB_STACKS_SPHERE; i++) {

            const float stackAngle = float(MATH_PI) / 2 - i * stackStep;

            float s, c;
            DirectX::XMScalarSinCos(&s, &c, stackAngle);

            const float radiusCosStackAngle = a_radius * c;
            const float z = a_radius * s;

            for (std::uint32_t j = 0; j <= NB_SECTORS_SPHERE; j++) {

                const float sectorAngle = j * sectorStep;

                DirectX::XMScalarSinCos(&s, &c, sectorAngle);

                const float x = radiusCosStackAngle * c;
                const float y = radiusCosStackAngle * s;

                vertices[i * (NB_SECTORS_SPHERE + 1) + j] = a_pos + btVector3(x, y, z);
            }
        }

        ItemTri item;

        // Faces
        for (std::uint32_t i = 0; i < NB_STACKS_SPHERE; i++) {

            std::uint32_t a1 = i * (NB_SECTORS_SPHERE + 1);
            std::uint32_t a2 = a1 + NB_SECTORS_SPHERE + 1;

            for (std::uint32_t j = 0; j < NB_SECTORS_SPHERE; j++, a1++, a2++) {

                // 2 triangles per sector except for the first and last stacks

                if (i != 0)
                {
                    if (GetTriangle(vertices[a1], vertices[a2], vertices[a1 + 1], a_col, item))
                        m_tris.emplace_back(std::move(item));
                }

                if (i != (NB_STACKS_SPHERE - 1))
                {
                    if (GetTriangle(vertices[a1 + 1], vertices[a2], vertices[a2 + 1], a_col, item))
                        m_tris.emplace_back(std::move(item));
                }
            }
        }
    }

    void Renderer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        ItemLine item;

        if (!GetLine(from, to, color, item))
            return;

        m_lines.emplace_back(std::move(item));
    }

    void Renderer::drawLine(const btVector3& from, const btVector3& to, const DirectX::XMVECTOR& color)
    {
        ItemLine item;

        if (!GetLine(from, to, color, item))
            return;

        m_lines.emplace_back(std::move(item));
    }

    void Renderer::drawBox(const btVector3& bbMin, const btVector3& bbMax, const Bullet::btTransformEx& trans, const DirectX::XMVECTOR& color)
    {
        auto p1 = trans * btVector3(bbMin.x(), bbMin.y(), bbMin.z());
        auto p2 = trans * btVector3(bbMax.x(), bbMin.y(), bbMin.z());
        auto p3 = trans * btVector3(bbMax.x(), bbMax.y(), bbMin.z());
        auto p4 = trans * btVector3(bbMin.x(), bbMax.y(), bbMin.z());
        auto p5 = trans * btVector3(bbMin.x(), bbMin.y(), bbMax.z());
        auto p6 = trans * btVector3(bbMax.x(), bbMin.y(), bbMax.z());
        auto p7 = trans * btVector3(bbMax.x(), bbMax.y(), bbMax.z());
        auto p8 = trans * btVector3(bbMin.x(), bbMax.y(), bbMax.z());

        drawLine(p1, p2, color);
        drawLine(p2, p3, color);
        drawLine(p3, p4, color);
        drawLine(p4, p1, color);
        drawLine(p1, p5, color);
        drawLine(p2, p6, color);
        drawLine(p3, p7, color);
        drawLine(p4, p8, color);
        drawLine(p5, p6, color);
        drawLine(p6, p7, color);
        drawLine(p7, p8, color);
        drawLine(p8, p5, color);
    }

    void Renderer::drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar /*alpha*/)
    {
        ItemTri item;

        if (!GetTriangle(v0, v1, v2, color, item))
            return;

        m_tris.emplace_back(std::move(item));

    }

    void Renderer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
    {
        drawSphere(PointOnB, m_contactPointSphereRadius, color);
        drawLine(PointOnB, PointOnB + normalOnB * m_contactNormalLength, IConfig::GetGlobal().debugRenderer.colors.contactNormal);
    }

    void Renderer::drawContactExtra(btPersistentManifold* manifold, const btManifoldPoint& contactPoint)
    {

        /*auto ob2 = manifold->getBody1();

        auto sc2 = static_cast<SimComponent*>(ob2->getUserPointer());

        btVector3 p = sc2->GetVelocity();
        p.safeNormalize();

        btVector3 p1, p2;

        btPlaneSpace1(p, p1, p2);

        auto e = (p1 + p2).cross(contactPoint.m_normalWorldOnB);
        e.safeNormalize();

        drawLine(contactPoint.getPositionWorldOnB(), contactPoint.getPositionWorldOnB() + e * 30.0f, IConfig::GetGlobal().debugRenderer.colors.constraintSphere);*/
    }

    void Renderer::draw3dText(const btVector3& location, const char* textString)
    {
    }

    void Renderer::reportErrorWarning(const char* warningString)
    {
        gLog.Warning("%s: %s", __FUNCTION__, warningString);
    }

    void Renderer::setDebugMode(int debugMode) {
        m_debugMode = debugMode;
    }

    int Renderer::getDebugMode() const {
        return m_debugMode;
    }

    auto Renderer::getDefaultColors() const -> DefaultColors
    {
        return IConfig::GetGlobal().debugRenderer.btColors;
    }

}