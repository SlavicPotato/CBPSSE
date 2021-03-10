#pragma once

namespace CBP
{

    struct SKMP_ALIGN(16) VertexPositionColorAV
    {
        VertexPositionColorAV() = default;

        VertexPositionColorAV(const VertexPositionColorAV&) = default;
        VertexPositionColorAV& operator=(const VertexPositionColorAV&) = default;

        VertexPositionColorAV(VertexPositionColorAV&&) = default;
        VertexPositionColorAV& operator=(VertexPositionColorAV&&) = default;

        VertexPositionColorAV(DirectX::XMVECTOR const& iposition, DirectX::XMVECTOR const& icolor) noexcept
            :
            position(iposition),
            color(icolor)
        { 
        }

        VertexPositionColorAV(DirectX::FXMVECTOR iposition, DirectX::XMFLOAT4 icolor) noexcept :
            position(iposition)
        {
            this->color = DirectX::XMLoadFloat4(&icolor);
        }

        DirectX::XMVECTOR position;
        DirectX::XMVECTOR color;

        static constexpr unsigned int InputElementCount = 2;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

    class SKMP_ALIGN(32) Renderer : 
        public btIDebugDraw
    {
        using VertexType = VertexPositionColorAV;

        struct SKMP_ALIGN_AUTO ItemLine
        {
            VertexType v0;
            VertexType v1;
        };

        struct SKMP_ALIGN_AUTO ItemTri
        {
            VertexType v0;
            VertexType v1;
            VertexType v2;
        };

        int m_debugMode;

    public:
        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO()

        Renderer(
            ID3D11Device* a_pDevice,
            ID3D11DeviceContext* a_pImmediateContext);

        Renderer() = delete;

        void Draw();
        void GenerateMovingNodes(const simActorList_t& a_actorList, float a_radius, bool a_moving, bool a_centerOfGravity, Game::ObjectHandle a_markedHandle);
        void GenerateMotionConstraints(const simActorList_t& a_actorList, float a_radius);

        void Clear();
        void ReleaseGeometry();

        SKMP_FORCEINLINE void SetContactPointSphereRadius(btScalar a_val) {
            m_contactPointSphereRadius = a_val;
        }

        SKMP_FORCEINLINE void SetContactNormalLength(btScalar a_val) {
            m_contactNormalLength = a_val;
        }
        
    private:
        static inline constexpr int NB_SECTORS_SPHERE = 9;
        static inline constexpr int NB_STACKS_SPHERE = 5;

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

        void GenerateSphere(const btVector3& a_pos, float a_radius, const DirectX::XMVECTOR& a_col);

        SKMP_FORCEINLINE void FillScreenPt(VertexType& a_out, const btVector3& a_col);
        SKMP_FORCEINLINE void FillScreenPt(VertexType& a_out, const DirectX::XMVECTOR& a_col);

        SKMP_FORCEINLINE bool GetScreenPt(const btVector3& a_pos, VertexType& a_out);

        template <class T>
        SKMP_FORCEINLINE bool GetTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const T& a_col, ItemTri& a_out);
        template <class T>
        SKMP_FORCEINLINE bool GetLine(const btVector3& v0, const btVector3& v1, const T& a_col, ItemLine& a_out);

        SKMP_FORCEINLINE bool IsValidPosition(const DirectX::XMVECTOR& a_pos);

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
        virtual void drawTriangle(const btVector3& v0, const btVector3& v1, const btVector3& v2, const btVector3& color, btScalar /*alpha*/) override;
        virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
        virtual void drawContactExtra(btPersistentManifold* manifold, const btManifoldPoint& contactPoint) override;
        virtual void draw3dText(const btVector3& location, const char* textString) override;
        virtual void reportErrorWarning(const char* warningString) override;

        SKMP_FORCEINLINE void drawLine(const btVector3& from, const btVector3& to, const DirectX::XMVECTOR& color);
        SKMP_FORCEINLINE void drawBox(const btVector3& bbMin, const btVector3& bbMax, const Bullet::btTransformEx& trans, const DirectX::XMVECTOR& color);

        virtual DefaultColors getDefaultColors() const override;

    public:
        virtual void setDebugMode(int debugMode) override;
        virtual int getDebugMode() const override;
    };
}