#pragma once

#include "Common/Base.h"
#include "Render/Backup.h"

namespace CBP
{

    class UIContext;
    struct ColliderData;

    class UICollisionGeometryManager :
        public UICommon::UIProfileEditorBase<ColliderProfile>,
        virtual UIBase
    {
        using resolutionDesc_t = std::pair<std::string, float>;

        static inline constexpr float DEFAULT_ZOOM = 50.0f;
        static inline constexpr float DEFAULT_FOV = 75.0f;
        static inline constexpr float DEFAULT_NEAR_PLANE = 0.1f;
        static inline constexpr float DEFAULT_FAR_PLANE = 1000.0f;

        struct SKMP_ALIGN_AUTO matrices_t
        {
            SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

            matrices_t(
                const DirectX::SimpleMath::Matrix& a_world,
                const DirectX::SimpleMath::Vector3& a_eyePos,
                const DirectX::SimpleMath::Vector3& a_targetPos,
                const DirectX::SimpleMath::Matrix& a_proj
            ) :
                m_world(a_world),
                m_eyePos(a_eyePos),
                m_targetPos(a_targetPos),
                m_proj(a_proj)
            {}

            DirectX::SimpleMath::Matrix m_world;
            DirectX::SimpleMath::Matrix m_proj;
            DirectX::SimpleMath::Vector3 m_eyePos;
            DirectX::SimpleMath::Vector3 m_targetPos;
        };

        class ShapeBase
        {
        protected:

            using setStateFunc_t = std::function<void __cdecl()>;

        public:

            enum class Light
            {
                kAmbient,
                kDiffuse,
                kSpecular
            };

            virtual ~ShapeBase() noexcept = default;

            virtual void Initialize(
                const ColliderData* a_data,
                ID3D11Device* a_device,
                ID3D11DeviceContext* a_context) = 0;

            virtual void __vectorcall Draw(
                setStateFunc_t a_setCustomState) const = 0;

            virtual void EnableLighting(bool a_switch) = 0;

            virtual void __vectorcall SetLightColor(
                Light a_which,
                int a_index,
                DirectX::XMVECTOR a_color) = 0;

            virtual void __vectorcall SetMaterialColor(
                DirectX::XMVECTOR a_color) = 0;

            virtual DirectX::BasicEffect* GetEffect() = 0;

            virtual void LoadGeometry(
                const ColliderData* a_data,
                ID3D11Device* a_device) = 0;

            SKMP_FORCEINLINE auto GetNumVertices() const {
                return m_numVertices;
            }

            SKMP_FORCEINLINE auto GetNumIndices() const {
                return m_numIndices;
            }

        protected:

            std::size_t m_numVertices;
            std::size_t m_numIndices;
        };


        template <class T>
        class Shape :
            public ShapeBase
        {
        protected:

            using vertexType_t = T;
            using vertexVector_t = stl::vector<vertexType_t>;
            using indexVector_t = stl::vector<std::uint32_t>;

        public:

            Shape() noexcept = default;
            virtual ~Shape() noexcept = default;

            virtual void Initialize(
                const ColliderData* a_data,
                ID3D11Device* a_device,
                ID3D11DeviceContext* a_context) override;

            virtual void __vectorcall Draw(
                setStateFunc_t a_setCustomState) const override;

            virtual void EnableLighting(bool a_switch) override
            {
                m_effect->SetLightingEnabled(a_switch);
            }

            virtual void __vectorcall SetLightColor(
                Light a_which,
                int a_index,
                DirectX::XMVECTOR a_color);

            virtual void __vectorcall SetMaterialColor(
                DirectX::XMVECTOR a_color) override;

            virtual DirectX::BasicEffect* GetEffect() override
            {
                return m_effect.get();
            }

            virtual void LoadGeometry(
                const ColliderData* a_data,
                ID3D11Device* a_device) override;

        protected:

            virtual void CreateGeometry(
                const ColliderData* a_data,
                vertexVector_t& a_vertices,
                indexVector_t& a_indices) const = 0;

            virtual void ApplyEffectSettings(DirectX::BasicEffect* a_effect) const;

            Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

            Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

            UINT m_indexCount;

            std::unique_ptr<DirectX::BasicEffect> m_effect;

            Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        };

        class ShapePosition :
            public Shape<DirectX::VertexPosition>
        {
        public:

            using Shape<DirectX::VertexPosition>::Shape;

            virtual ~ShapePosition() noexcept = default;

        private:
            virtual void CreateGeometry(
                const ColliderData* a_data,
                vertexVector_t& a_vertices,
                indexVector_t& a_indices) const override;

            virtual void ApplyEffectSettings(DirectX::BasicEffect* a_effect) const override;
        };

        class ShapeNormal :
            public Shape<DirectX::VertexPositionNormal>
        {
        public:
            using Shape<DirectX::VertexPositionNormal>::Shape;

            virtual ~ShapeNormal() noexcept = default;

        private:

            virtual void CreateGeometry(
                const ColliderData* a_data,
                vertexVector_t& a_vertices,
                indexVector_t& a_indices) const override;

            virtual void ApplyEffectSettings(DirectX::BasicEffect* a_effect) const override;

        };

        class SKMP_ALIGN(16) Model
        {
        public:

            SKMP_DECLARE_ALIGNED_ALLOCATOR(16);

            Model() = delete;

            Model(
                const ColliderData * a_data,
                ID3D11Device * a_device,
                ID3D11DeviceContext * a_context,
                const DirectX::XMFLOAT3 & a_bufferSize,
                float a_textureResolution);

            void Draw() const;

            SKMP_FORCEINLINE auto GetTexture() {
                return m_shaderResourceView.Get();
            }

            SKMP_FORCEINLINE const auto& GetBufferSize() const {
                return m_bufferSize;
            }

            SKMP_FORCEINLINE auto GetNumVertices() const {
                return m_shape->GetNumVertices();
            }

            SKMP_FORCEINLINE auto GetNumIndices() const {
                return m_shape->GetNumIndices();
            }

            SKMP_FORCEINLINE void EnableLighting(bool a_switch) {
                m_shape->EnableLighting(a_switch);
            }

            SKMP_FORCEINLINE void __vectorcall SetLightColor(
                ShapeBase::Light a_which,
                int a_index,
                DirectX::XMVECTOR a_color)
            {
                m_shape->SetLightColor(a_which, a_index, a_color);
            }

            SKMP_FORCEINLINE void __vectorcall SetMaterialColor(
                DirectX::XMVECTOR a_color)
            {
                m_shape->SetMaterialColor(a_color);
            }

            SKMP_FORCEINLINE DirectX::SimpleMath::Vector3 __vectorcall GetEyePos() const {
                return m_eyePos;
            }

            SKMP_FORCEINLINE DirectX::SimpleMath::Vector3 __vectorcall GetTargetPos() const {
                return m_targetPos;
            }

            SKMP_FORCEINLINE const auto& GetWorldMatrix() const {
                return m_world;
            }

            SKMP_FORCEINLINE const auto& GetViewMatrix() const {
                return m_view;
            }

            SKMP_FORCEINLINE const auto& GetProjectionMatrix() const {
                return m_proj;
            }

            void GetMatrices(std::unique_ptr<matrices_t>&a_out) const;
            void SetMatrices(const std::unique_ptr<matrices_t>&a_in);

            SKMP_FORCEINLINE void LoadGeometry(
                const ColliderData * a_data)
            {
                return m_shape->LoadGeometry(a_data, m_device.Get());
            }

            void ResetPos();
            void SetZoom(float a_val);
            void SetFOV(float a_val);
            void Pan(const ImVec2 & a_delta);
            void Rotate(const ImVec2 & a_delta);

        private:

            void UpdateViewMatrix();

            DirectX::SimpleMath::Vector3 m_eyePos;
            DirectX::SimpleMath::Vector3 m_targetPos;

            DirectX::SimpleMath::Matrix m_world;
            DirectX::SimpleMath::Matrix m_view;
            DirectX::SimpleMath::Matrix m_proj;

            std::unique_ptr<DirectX::CommonStates> m_states;
            std::unique_ptr<ShapeBase> m_shape;

            Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
            Microsoft::WRL::ComPtr<ID3D11Texture2D>	m_renderTargetViewTexture;
            Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
            Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

            Microsoft::WRL::ComPtr<ID3D11Device> m_device;
            Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;

            DirectX::XMFLOAT3 m_bufferSize;
            float m_textureResolution;

            mutable D3D11StateBackupImpl m_backup;
        };

        class DragController
        {

            using func_t = std::function<void(const ImVec2& a_delta)>;

        public:

            DragController() = delete;
            DragController(std::uint8_t a_button, func_t a_onDrag);

            void Update(bool a_isHovered);

        private:

            bool m_dragging;
            std::uint8_t m_button;
            ImVec2 m_lastMousePos;
            ImVec2 m_avgMouseDir;
            func_t m_func;
        };

        class ReleaseModelTask :
            public TaskDelegate
        {
        public:
            ReleaseModelTask(std::unique_ptr<Model>&& a_model);

            virtual void Run() {};
            virtual void Dispose() {
                delete this;
            };

        private:

            std::unique_ptr<Model> m_ref;
        };

    public:
        UICollisionGeometryManager(UIContext& a_parent, const char* a_name);
        virtual ~UICollisionGeometryManager() noexcept = default;

        void DrawModel();
        void OnOpen();
        void OnClose();

    private:

        static void DrawCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd);

        virtual ProfileManager<ColliderProfile>& GetProfileManager() const override;

        virtual UICommon::WindowLayoutData GetWindowDimensions() const override;
        virtual void DrawItem(ColliderProfile& a_profile) override;
        virtual void DrawOptions(ColliderProfile& a_profile) override;
        virtual bool InitializeProfile(ColliderProfile& a_profile) override;

        virtual bool AllowCreateNew() const override;
        virtual bool AllowSave() const override;
        virtual void OnItemSelected(const std::string& a_item) override;
        virtual void OnReload(const ColliderProfile& a_profile) override;
        virtual void OnProfileSave(const std::string& a_item) override;
        virtual bool OnDeleteWarningOverride(const std::string& a_key, std::string& a_msg) override;

        void SetResolution(const resolutionDesc_t& a_res);
        void AutoSelectResolution(const char*& a_curSelName);
        void DrawResolutionCombo();
        void CreateInfoStrings();

        void Load(const std::string& a_item, bool a_force = false);
        void Load(const ColliderProfile& a_profile, bool a_force = false);

        void QueueModelRelease();
        void QueueModelReload();

        std::unique_ptr<Model> m_model;

        SelectedItem<resolutionDesc_t> m_resolution;
        const stl::vector<resolutionDesc_t> m_resList;

        float m_zoom;

        DragController m_dragRotate;
        DragController m_dragPan;

        struct
        {
            std::string m_vertices;
            std::string m_indices;
        } m_infoStrings;

        std::unique_ptr<matrices_t> m_matrixBackup;

        UIContext& m_parent;
    };

}