#pragma once

namespace CBP
{
    class Renderer
    {
        using VertexType = DirectX::VertexPositionColor;

        struct ItemLine
        {
            VertexType pos1;
            VertexType pos2;
        };

        struct ItemTri
        {
            VertexType pos1;
            VertexType pos2;
            VertexType pos3;
        };

    public:
        Renderer(
            ID3D11Device* a_pDevice,
            ID3D11DeviceContext* a_pImmediateContext);

        Renderer() = delete;

        void Draw();
        void Update(const r3d::DebugRenderer& a_dr);
        void UpdateMovingNodes(const simActorList_t& a_actorList, float a_radius);
        void Clear();

    private:
        static constexpr int NB_SECTORS_SPHERE = 18;
        static constexpr int NB_STACKS_SPHERE = 10;

        static constexpr DirectX::XMFLOAT4 MOVING_NODES_COL = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.9f);
        static constexpr float MOVING_NODES_RAD = 0.5f;

        std::unique_ptr<DirectX::BasicEffect> m_effect;
        std::unique_ptr<DirectX::CommonStates> m_states;
        std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        ID3D11Device* m_pDevice;
        ID3D11DeviceContext* m_pImmediateContext;

        std::vector<ItemLine> m_lines;
        std::vector<ItemTri> m_tris;

        void GenerateLines(const r3d::DebugRenderer& a_dr);
        void GenerateTris(const r3d::DebugRenderer& a_dr);
        void GenerateMovingNodes(const simActorList_t& a_actorList, float a_radius);

        void GenerateSphere(const NiPoint3& a_pos, float a_radius, const DirectX::XMFLOAT4& a_col);

        __forceinline bool GetScreenPt(const r3d::Vector3& a_pos, r3d::uint32 a_col, VertexType& a_out);
        __forceinline bool GetScreenPt2(const NiPoint3& a_pos, const DirectX::XMFLOAT4& a_col, VertexType& a_out);
    };
}