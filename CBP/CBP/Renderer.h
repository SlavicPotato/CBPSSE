#pragma once

namespace CBP
{
    class SKMP_ALIGN(16) Renderer : 
        public btIDebugDraw
    {
        using VertexType = DirectX::VertexPositionColor;

        struct SKMP_ALIGN(16) ItemLine
        {
            VertexType pos1;
            VertexType pos2;
        };

        struct SKMP_ALIGN(16) ItemTri
        {
            VertexType pos1;
            VertexType pos2;
            VertexType pos3;
        };

        int m_debugMode;

    public:
        Renderer(
            ID3D11Device* a_pDevice,
            ID3D11DeviceContext* a_pImmediateContext);

        Renderer() = delete;

        void Draw();
        void GenerateMovingNodes(const simActorList_t& a_actorList, float a_radius, bool a_centerOfGravity, Game::ObjectHandle a_markedHandle);
        void GenerateMovementConstraints(const simActorList_t& a_actorList, float a_radius);

        void Clear();
        void Release();

        SKMP_FORCEINLINE void SetContactPointSphereRadius(btScalar a_val) {
            m_contactPointSphereRadius = a_val;
        }

        SKMP_FORCEINLINE void SetContactNormalLength(btScalar a_val) {
            m_contactNormalLength = a_val;
        }
        
    private:
        static constexpr int NB_SECTORS_SPHERE = 9;
        static constexpr int NB_STACKS_SPHERE = 5;

        static constexpr auto MOVING_NODES_COL = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.85f);
        static constexpr auto ACTOR_MARKER_COL = DirectX::XMFLOAT4(0.921f, 0.596f, 0.203f, 0.85f);

        static constexpr auto CONSTRAINT_BOX_COL = DirectX::XMFLOAT4(0.2f, 0.9f, 0.5f, 0.85f);
        static constexpr auto VIRTUAL_POS_COL = DirectX::XMFLOAT4(0.3f, 0.7f, 0.7f, 0.85f);

        static constexpr auto CONTACT_NORMAL_COL = DirectX::XMFLOAT4(0.0f, 0.749f, 1.0f, 1.0f);

        std::unique_ptr<DirectX::BasicEffect> m_effect;
        std::unique_ptr<DirectX::CommonStates> m_states;
        std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        ID3D11Device* m_pDevice;
        ID3D11DeviceContext* m_pImmediateContext;

        stl::vector<ItemLine> m_lines;
        stl::vector<ItemTri> m_tris;

        btScalar m_contactPointSphereRadius;
        btScalar m_contactNormalLength;

        void GenerateSphere(const NiPoint3& a_pos, float a_radius, const DirectX::XMFLOAT4& a_col);

        SKMP_FORCEINLINE bool GetScreenPt(const btVector3& a_pos, const btVector3 &a_col, VertexType& a_out);
        SKMP_FORCEINLINE bool GetScreenPt(const btVector3& a_pos, const DirectX::XMFLOAT4& a_col, VertexType& a_out);
        SKMP_FORCEINLINE bool GetScreenPt(const NiPoint3& a_pos, const DirectX::XMFLOAT4& a_col, VertexType& a_out);

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
        virtual void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar /*alpha*/) override;
        virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
        virtual void draw3dText(const btVector3& location, const char* textString) override;
        virtual void reportErrorWarning(const char* warningString) override;

        void drawLine(const NiPoint3& from, const NiPoint3& to, const DirectX::XMFLOAT4& color);
        void drawLine(const btVector3& from, const btVector3& to, const DirectX::XMFLOAT4& color);
        void drawBox(const NiPoint3& bbMin, const NiPoint3& bbMax, const NiTransform& trans, const DirectX::XMFLOAT4& color);

    public:
        virtual void setDebugMode(int debugMode) override;
        virtual int getDebugMode() const override;
    };
}