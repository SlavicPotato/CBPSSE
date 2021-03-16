#pragma once

#include "UI/Base.h"
#include "UI/Node.h"
#include "UI/SimComponent.h"
#include "UI/List.h"
#include "UI/ActorList.h"
#include "UI/RaceList.h"
#include "UI/Profile.h"
#include "UI/Force.h"
#include "UI/Race.h"

namespace CBP
{
    class UIContext;

    class UIProfileEditorPhysics :
        public UICommon::UIProfileEditorBase<PhysicsProfile>,
        UISimComponent<int, UIEditorID::kProfileEditorPhys>
    {
    public:

        UIProfileEditorPhysics(UIContext& a_parent, const char* a_name);

        virtual ~UIProfileEditorPhysics() noexcept = default;

    private:

        virtual ProfileManager<PhysicsProfile>& GetProfileManager() const override;

        virtual void DrawItem(PhysicsProfile& a_profile);
        virtual void DrawOptions(PhysicsProfile& a_profile);

        void DrawGroupOptions(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            nodeConfigList_t& a_nodeConfig);

        virtual void OnSimSliderChange(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnMotionConstraintChange(
            int,
            PhysicsProfile::base_type::config_type&,
            PhysicsProfile::base_type::value_type&,
            const componentValueDescMap_t::vec_value_type&) override;

        virtual void OnComponentUpdate(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair) override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(int) const override;

        virtual bool ShouldDrawComponent(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual void UpdateNodeData(
            int,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;
        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        //UISelectedItem<std::string> m_selectedConfGroup;

        UIContext& m_ctxParent;
    };

    class UIProfileEditorNode :
        public UICommon::UIProfileEditorBase<NodeProfile>,
        UINode<int, UIEditorID::kProfileEditorNode>
    {
    public:
        UIProfileEditorNode(UIContext& a_parent, const char* a_name);

        virtual ~UIProfileEditorNode() noexcept = default;

    private:

        virtual ProfileManager<NodeProfile>& GetProfileManager() const override;

        virtual void DrawItem(NodeProfile& a_profile);
        virtual void UpdateNodeData(
            int,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data,
            bool a_reset);


        virtual void DrawOptions(NodeProfile& a_profile) override;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };

    class UIOptions :
        UIBase
    {
    public:
        UIOptions(UIContext& a_parent);

        void Draw(bool* a_active);

    private:
        void DrawKeyOptions(
            const char* a_desc,
            const keyDesc_t& a_dmap,
            UInt32 a_key);

        UIContext& m_parent;
    };

    class UICollisionGroups :
        virtual protected UIBase
    {
    public:
        UICollisionGroups() = default;

        void Draw(bool* a_active);
    private:
        SelectedItem<uint64_t> m_selected;
        uint64_t m_input;
    };

    class UIActorEditorNode :
        public UIActorList<actorListNodeConf_t>,
        public UIProfileSelector<actorListNodeConf_t::value_type, NodeProfile>,
        UINode<Game::ObjectHandle, UIEditorID::kNodeEditor>
    {
    public:
        UIActorEditorNode(UIContext& a_parent) noexcept;

        void Draw(bool* a_active);
        void Reset();
    private:
        virtual void ListResetAllValues(Game::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data);

        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            Game::ObjectHandle a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual void DrawBoneCastButtons(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        void OnListChangeCurrentItem(const SelectedItem<Game::ObjectHandle>& a_oldHandle, Game::ObjectHandle a_newHandle) override;

        UIContext& m_ctxParent;
    };


    class UIRaceEditorNode :
        public UIRaceEditorBase<raceListNodeConf_t, NodeProfile>,
        public UINode<Game::FormID, UIEditorID::kRaceNodeEditor>
    {
    public:
        UIRaceEditorNode(UIContext& a_parent) noexcept;

        void Draw(bool* a_active);

    private:

        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const;

        virtual void ListResetAllValues(Game::FormID a_formid);
        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            Game::FormID a_formid,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };

    class UIRaceEditorPhysics :
        public UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>,
        public UISimComponent<Game::FormID, UIEditorID::kRacePhysicsEditor>
    {
    public:
        UIRaceEditorPhysics(UIContext& a_parent) noexcept;

        void Draw(bool* a_active);
    private:

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) override;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const override;

        virtual void ListResetAllValues(Game::FormID a_formid);

        virtual void DrawConfGroupNodeMenu(
            Game::FormID a_handle,
            nodeConfigList_t& a_nodeList
        ) override;

        virtual void OnSimSliderChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) override;

        virtual void OnMotionConstraintChange(
            Game::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnComponentUpdate(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(
            Game::FormID a_handle) const override;

        virtual void UpdateNodeData(
            Game::FormID a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual bool ShouldDrawComponent(
            Game::FormID a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasMotion(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasCollision(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;
        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

        virtual const PhysicsProfile* GetSelectedProfile() const override;

        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };

    class UIPlot
    {
    public:
        UIPlot(
            const char* a_label,
            const ImVec2& a_size,
            bool a_avg,
            int a_res);

        void Update(float a_value);
        void Draw();
        void SetRes(int a_res);
        void SetHeight(float a_height);

        SKMP_FORCEINLINE void SetShowAvg(bool a_switch) {
            m_avg = a_switch;
        }

    private:
        stl::vector<float> m_values;

        float m_plotScaleMin;
        float m_plotScaleMax;

        char m_strBuf1[16 + std::numeric_limits<float>::digits];

        const char* m_label;
        ImVec2 m_size;
        bool m_avg;
        int m_res;
    };

    class UIProfiling :
        UIBase
    {

    public:
        UIProfiling();

        void Initialize();

        void Draw(bool* a_active);
    private:
        std::uint32_t m_lastUID;

        UIPlot m_plotUpdateTime;
        UIPlot m_plotFramerate;

        long long m_lastVMIUpdate;
    };

#ifdef _CBP_ENABLE_DEBUG
    class UIDebugInfo :
        UIBase
    {
    public:
        void Draw(bool* a_active);
    private:
        const char* ParseFloat(float v);
        std::string TransformToStr(const NiTransform& a_transform);

        bool m_sized = false;
        char m_buffer[64];
    };
#endif

    class UIFileSelector :
        virtual protected UIBase
    {
        class SelectedFile
        {
        public:
            SelectedFile();
            SelectedFile(const fs::path& a_path);

            void UpdateInfo();

            fs::path m_path;
            std::string m_filenameStr;
            importInfo_t m_info;
            bool m_infoResult;
        };
    public:
        bool UpdateFileList();
    protected:
        UIFileSelector();

        void DrawFileSelector();
        bool DeleteExport(const fs::path& a_file);

        SKMP_FORCEINLINE const auto& GetSelected() const {
            return m_selected;
        }

        SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastExcept;
        }

    private:
        SelectedItem<SelectedFile> m_selected;
        stl::vector<fs::path> m_files;

        except::descriptor m_lastExcept;
    };

    class UIDialogImportExport :
        public UIFileSelector
    {
    public:
        UIDialogImportExport(UIContext& a_parent);

        void Draw(bool* a_active);
        void OnOpen();

    private:

        void DrawExportContextMenu();

        void DoImport(const fs::path& a_path);
        void DoExport(const fs::path& a_path);

        ISerialization::ImportFlags GetFlags() const;

        std::regex m_rFileCheck;
        UIContext& m_parent;
    };

    class UILog :
        UIBase
    {
    public:
        UILog();

        void Draw(bool* a_active);

        SKMP_FORCEINLINE void SetScrollBottom() {
            m_doScrollBottom = true;
        }

    private:
        bool m_doScrollBottom;
        std::int8_t m_initialScroll;
    };

    class UINodeMap :
        ILog,
        public UIActorList<actorListCache_t>
    {
    public:
        UINodeMap(UIContext& a_parent);

        void Draw(bool* a_active);
        void Reset();

    private:

        void DrawMenuBar(bool* a_active);
        void DrawNodeTree(const nodeRefEntry_t& a_entry);
        void DrawConfigGroupMap();
        void DrawTreeContextMenu(const nodeRefEntry_t& a_entry);

        void AddNode(const std::string& a_node, const std::string& a_confGroup);
        void AddNodeNewGroup(const std::string& a_node);
        void AddNodeNew();
        void RemoveNode(const std::string& a_node);

        virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const override;
        virtual configGlobalActor_t& GetActorConfig() const override;
        virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const override;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) override;

        virtual void ListResetAllValues(Game::ObjectHandle a_handle) override;

        virtual void ListUpdate() override;
        virtual listValue_t* ListGetSelected() override;
        virtual void ListSetCurrentItem(Game::ObjectHandle a_handle) override;

        [[nodiscard]] SKMP_FORCEINLINE std::string GetCSID(
            const std::string& a_name) const
        {
            std::ostringstream ss;
            ss << "UINM#" << a_name;
            return ss.str();
        }

        bool m_update;
        UICommon::UIGenericFilter m_filter;
        UIContext& m_parent;

        entryValue_t m_dummyEntry;
    };

    class UIArmorOverrideEditor :
        UIBase
    {
        using entry_type = std::pair<std::string, armorCacheEntrySorted_t>;

    public:

        UIArmorOverrideEditor(UIContext& a_parent) noexcept;
        virtual ~UIArmorOverrideEditor() noexcept = default;

        void Draw(bool* a_active);

        void SetCurrentOverrides(const armorOverrideResults_t& a_overrides);

    private:

        void DrawSliders(entry_type& a_entry);
        void DrawToolbar(entry_type& a_entry);
        void DrawOverrideList();
        void DrawGroup(entry_type& a_entry, entry_type::second_type::value_type& a_e);
        void DrawAddSliderContextMenu(entry_type::second_type::value_type& a_e);
        void DrawAddGroupContextMenu(entry_type& a_e);
        void DrawSliderOverrideModeSelector(entry_type::second_type::mapped_type::value_type& a_entry);

        void SetCurrentEntry(const std::string& a_path, const armorCacheEntry_t& a_entry);
        bool SetCurrentEntry(const std::string& a_path, bool a_fromDisk = false);

        void RemoveGroup(const std::string& a_path, const std::string& a_group);
        void DoSave(const entry_type& a_entry);

        static const char* OverrideModeToDesc(std::uint32_t a_mode);

        SelectedItem<armorOverrideResults_t> m_currentOverrides;
        SelectedItem<entry_type> m_currentEntry;

        UICommon::UIPopupQueue m_popupPostGroup;

        UIContext& m_parent;
    };

    class UICollisionGeometryManager :
        virtual private UIBase,
        public UICommon::UIProfileEditorBase<ColliderProfile>
    {
        using resolutionDesc_t = std::pair<std::string, float>;

        static inline constexpr float DEFAULT_ZOOM = 50.0f;
        static inline constexpr float DEFAULT_FOV = 90.0f;
        static inline constexpr float DEFAULT_NEAR_PLANE = 0.1f;
        static inline constexpr float DEFAULT_FAR_PLANE = 1000.0f;

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
                const DirectX::XMFLOAT3 &a_bufferSize,
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
            
            SKMP_FORCEINLINE  DirectX::SimpleMath::Matrix __vectorcall GetWorldMatrix() const {
                return m_world;
            }

            void __vectorcall SetViewData(
                DirectX::SimpleMath::Matrix &a_world,
                DirectX::SimpleMath::Vector3 a_eyePos,
                DirectX::SimpleMath::Vector3 a_targetPos);

            void ResetPos();
            void SetZoom(float a_val);
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
            DragController(int a_key, func_t a_onDrag);

            void Update(bool a_isHovered);

        private:

            bool m_dragging;
            int m_key;
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

        virtual void GetWindowDimensions(float& a_offset, float& a_width, float& a_height, bool& a_centered) const override;
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

        void Load(const std::string& a_item);
        void Load(const ColliderProfile& a_profile);

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

        UIContext& m_parent;
    };


    class SKMP_ALIGN(32) UIContext :
        virtual UIBase,
        public UIActorList<actorListPhysConf_t>,
        public UIProfileSelector<actorListPhysConf_t::value_type, PhysicsProfile>,
        UIApplyForce<actorListPhysConf_t::value_type>,
        ILog
    {
        class UISimComponentActor :
            public UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            UISimComponentActor(UIContext& a_parent);

            virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        private:
            virtual void DrawConfGroupNodeMenu(
                Game::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            ) override;

            virtual void OnSimSliderChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val) override;

            virtual void OnColliderShapeChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            ) override;

            virtual void OnMotionConstraintChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc) override;

            virtual void OnComponentUpdate(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair) override;

            virtual bool ShouldDrawComponent(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                const configGroupMap_t::value_type& a_cgdata,
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool HasMotion(
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool HasCollision(
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool HasBoneCast(
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
                Game::ObjectHandle a_handle,
                const std::string& a_comp) const override;

            virtual bool GetNodeConfig(
                const configNodes_t& a_nodeConf,
                const configGroupMap_t::value_type& cg_data,
                nodeConfigList_t& a_out) const override;

            virtual const configNodes_t& GetNodeData(
                Game::ObjectHandle a_handle) const override;

            virtual void UpdateNodeData(
                Game::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset) override;

            virtual void DrawBoneCastButtons(
                Game::ObjectHandle a_handle,
                const std::string& a_nodeName,
                configNode_t& a_conf) override;

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;

            virtual const PhysicsProfile* GetSelectedProfile() const override;
            virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

            UIContext& m_ctxParent;
        };

        class UISimComponentGlobal :
            public UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            UISimComponentGlobal(UIContext& a_parent);

            virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

        private:
            virtual void DrawConfGroupNodeMenu(
                Game::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            ) override;

            virtual void OnSimSliderChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val) override;

            virtual void OnColliderShapeChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            ) override;

            virtual void OnMotionConstraintChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc) override;

            virtual void OnComponentUpdate(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair) override;

            virtual bool ShouldDrawComponent(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                const configGroupMap_t::value_type& a_cgdata,
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool HasMotion(
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool HasCollision(
                const nodeConfigList_t& a_nodeConfig) const override;

            virtual bool GetNodeConfig(
                const configNodes_t& a_nodeConf,
                const configGroupMap_t::value_type& cg_data,
                nodeConfigList_t& a_out) const override;

            virtual const configNodes_t& GetNodeData(
                Game::ObjectHandle a_handle) const override;

            virtual void UpdateNodeData(
                Game::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset) override;

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;

            virtual const PhysicsProfile* GetSelectedProfile() const override;
            virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

            UIContext& m_ctxParent;
        };

    public:
        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

        UIContext() noexcept;
        virtual ~UIContext() noexcept = default;

        void Initialize();

        void Reset(std::uint32_t a_loadInstance);
        void OnOpen();
        void OnClose();
        void Draw(bool* a_active);

        void QueueListUpdateAll();


        [[nodiscard]] SKMP_FORCEINLINE std::uint32_t GetLoadInstance() const noexcept {
            return m_activeLoadInstance;
        }

        SKMP_FORCEINLINE void LogNotify() {
            m_log.SetScrollBottom();
        }

        SKMP_FORCEINLINE auto& GetPopupQueue() {
            return m_popup;
        }

        SKMP_FORCEINLINE auto& GetWindowStates() {
            return m_state.windows;
        }

        SKMP_FORCEINLINE void ClearPopupQueue() {
            return m_popup.clear();
        }

        void UpdateStyle();

    private:

        void DrawMenuBar(bool* a_active, const listValue_t * a_entry);

        virtual void ApplyProfile(listValue_t * a_data, const PhysicsProfile & m_peComponents) override;

        virtual void ApplyForce(
            listValue_t * a_data,
            std::uint32_t a_steps,
            const std::string & a_component,
            const NiPoint3 & a_force) const override;

        virtual void ListResetAllValues(Game::ObjectHandle a_handle) override;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t * a_data) override;

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const override;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const override;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const override;

        void OnListChangeCurrentItem(const SelectedItem<Game::ObjectHandle> &a_oldHandle, Game::ObjectHandle a_newHandle) override;

        std::uint32_t m_activeLoadInstance;
        long long m_tsNoActors;

        struct {
            struct {
                bool options;
                bool profileSim;
                bool profileNodes;
                bool race;
                bool raceNode;
                bool collisionGroups;
                bool actorNode;
                bool profiling;
                bool debug;
                bool log;
                bool importExportDialog;
                bool nodeMap;
                bool armorOverride;
                bool geometryManager;
            } windows;

            struct {
                bool openIEDialog;
            } menu;

            except::descriptor lastException;
        } m_state;

        UICommon::UIPopupQueue m_popup;

        UIProfileEditorPhysics m_pePhysics;
        UIProfileEditorNode m_peNodes;
        UIRaceEditorPhysics m_racePhysicsEditor;
        UIRaceEditorNode m_raceNodeEditor;
        UIActorEditorNode m_actorNodeEditor;
        UIOptions m_options;
        UICollisionGroups m_colGroups;
        UIProfiling m_profiling;
        UIDialogImportExport m_ieDialog;
        UILog m_log;
        UINodeMap m_nodeMap;
        UIArmorOverrideEditor m_armorOverride;
        UICollisionGeometryManager m_geometryManager;

#ifdef _CBP_ENABLE_DEBUG
        UIDebugInfo m_debug;
#endif

        UISimComponentActor m_scActor;
        UISimComponentGlobal m_scGlobal;
    };

}