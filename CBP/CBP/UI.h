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
        UIProfileEditorPhysics(UIContext &a_parent, const char* a_name) :
            UICommon::UIProfileEditorBase<PhysicsProfile>(a_name),
            m_ctxParent(a_parent)
        {}
    private:

        virtual ProfileManager<PhysicsProfile>& GetProfileManager() const;

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
        UIProfileEditorNode(UIContext& a_parent, const char* a_name) :
            UICommon::UIProfileEditorBase<NodeProfile>(a_name),
            m_ctxParent(a_parent)
        {}

    private:

        virtual ProfileManager<NodeProfile>& GetProfileManager() const;

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
        void Draw(bool* a_active);

    private:
        void DrawKeyOptions(
            const char* a_desc,
            const keyDesc_t& a_dmap,
            UInt32 a_key);
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
        UIProfileSelector<actorListNodeConf_t::value_type, NodeProfile>,
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
        UIRaceEditorNode(UIContext &a_parent) noexcept;

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
        UIRaceEditorPhysics(UIContext &a_parent) noexcept;

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

        virtual bool HasCollisions(
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

    class UIDialogImport :
        public UIFileSelector
    {
    public:
        UIDialogImport() = default;

        bool Draw(bool* a_active);
        void OnOpen();

    };

    class UIDialogExport :
        UIBase
    {
    public:
        UIDialogExport();

        bool Draw();
        void Open();

    private:

        bool OnFileInput();

        fs::path m_path;
        fs::path m_lastTargetPath;
        std::regex m_rFileCheck;

        char m_buf[32];
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

            virtual bool HasCollisions(
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

            virtual bool HasCollisions(
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
                bool importDialog;
                bool nodeMap;
                bool armorOverride;
            } windows;

            struct {
                bool openExportDialog;
                bool openImportDialog;
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
        UIDialogImport m_importDialog;
        UIDialogExport m_exportDialog;
        UILog m_log;
        UINodeMap m_nodeMap;
        UIArmorOverrideEditor m_armorOverride;

#ifdef _CBP_ENABLE_DEBUG
        UIDebugInfo m_debug;
#endif

        UISimComponentActor m_scActor;
        UISimComponentGlobal m_scGlobal;
    };

}