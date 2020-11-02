#pragma once

namespace CBP
{
    typedef KVStorage<UInt32, const char*> keyDesc_t;

    enum class MiscHelpText : int
    {
        timeTick,
        maxSubSteps,
        timeScale,
        colMaxPenetrationDepth,
        showAllActors,
        profileSelect,
        clampValues,
        syncMinMax,
        rescanActors,
        resetConfOnActor,
        resetConfOnRace,
        showEDIDs,
        playableOnly,
        colGroupEditor,
        importDialog,
        exportDialog,
        simRate,
        armorOverrides,
        offsetMin,
        offsetMax,
        applyForce,
        showNodes,
        dataFilterPhys,
        dataFilterNode,
        frameTimer,
        timePerFrame
    };

    typedef std::pair<const std::string, configComponents_t> actorEntryPhysConf_t;
    typedef std::map<Game::ObjectHandle, actorEntryPhysConf_t> actorListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> actorEntryNodeConf_t;
    typedef std::map<Game::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    typedef std::vector<std::pair<const std::string, const configNode_t*>> nodeConfigList_t;

    class UIBase :
        virtual protected UICommon::UIWindow,
        virtual protected UICommon::UIAlignment,
        virtual protected UICommon::UICollapsibles,
        virtual protected UICommon::UIControls
    {
    protected:

        void HelpMarker(MiscHelpText a_id) const;
        void HelpMarker(const std::string& a_text) const;

        template <typename T>
        inline void SetGlobal(T& a_member, T const a_value) const;

    protected:

        static const keyDesc_t m_comboKeyDesc;
        static const keyDesc_t m_keyDesc;

    private:

        virtual UIData::UICollapsibleStates& GetCollapsibleStatesData() const;
        virtual void OnCollapsibleStatesUpdate() const;
        virtual void OnControlValueChange() const;

        static const std::unordered_map<MiscHelpText, const char*> m_helpText;
    };

    template <typename T>
    class UINodeCommon :
        virtual protected UIBase
    {
    protected:
        virtual void UpdateNodeData(
            T a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) = 0;

        /*void DrawConfigGroupNodeItems(
            T a_handle,
            const std::string& a_confGroup,
            configNodes_t& a_data
        );*/

        void DrawNodeItem(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf
        );
    };

    template <typename T, UIEditorID ID>
    class UINodeConfGroupMenu :
        protected UINodeCommon<T>
    {
    protected:
        virtual void DrawConfGroupNodeMenu(
            T a_handle,
            nodeConfigList_t& a_nodeList
        );

        void DrawConfGroupNodeMenuImpl(
            T a_handle,
            nodeConfigList_t& a_nodeList
        );
    };

    template <UIEditorID ID>
    class UIMainItemFilter :
        virtual protected UIBase
    {
    protected:

        UIMainItemFilter(MiscHelpText a_helpText);

        void DrawItemFilter();

        virtual std::string GetGCSID(
            const std::string& a_name) const = 0;

        UICommon::UIRegexFilter m_dataFilter;
    };

    template <class T, UIEditorID ID>
    class UISimComponent :
        virtual protected UIBase,
        protected UINodeConfGroupMenu<T, ID>,
        UIMainItemFilter<ID>
    {
    public:
        void DrawSimComponents(
            T a_handle,
            configComponents_t& a_data);

    protected:
        UISimComponent();
        virtual ~UISimComponent() noexcept = default;

        void DrawSliders(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            nodeConfigList_t& a_nodeConfig
        );

        virtual void OnSimSliderChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val
        ) = 0;

        virtual void OnColliderShapeChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) = 0;

        virtual void OnComponentUpdate(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) = 0;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const = 0;

        virtual const PhysicsProfile* GetSelectedProfile() const;

        virtual void DrawGroupOptions(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            nodeConfigList_t& a_nodeConfig);

        void Propagate(
            configComponents_t& a_dl,
            configComponents_t* a_dg,
            const configComponentsValue_t& a_pair,
            std::function<void(configComponent_t&)> a_func) const;

        [[nodiscard]] inline virtual std::string GetGCSID(
            const std::string& a_name) const;

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name) const
        {
            std::ostringstream ss;
            ss << "UISC#" << Enum::Underlying(ID) << "#" << a_name;
            return ss.str();
        }

        [[nodiscard]] inline std::string GetCSSID(
            const std::string& a_name, const char* a_group) const
        {
            std::ostringstream ss;
            ss << "UISC#" << Enum::Underlying(ID) << "#" << a_name << "#" << a_group;
            return ss.str();
        }

        float GetActualSliderValue(const armorCacheValue_t& a_cacheval, float a_baseval) const;

        inline void MarkCurrentForErase() {
            m_eraseCurrent = true;
        }

    private:

        virtual bool ShouldDrawComponent(
            T a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasMovement(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasCollisions(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            T a_handle,
            const std::string& a_comp) const;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const = 0;

        virtual const configNodes_t &GetNodeData(
            T a_handle) const = 0;

        void DrawComponentTab(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            nodeConfigList_t& a_nodeConfig
        );

        void DrawMirrorContextMenu(
            T a_handle,
            configComponents_t& a_data,
            configComponents_t::value_type& a_entry);

        bool CopyFromSelectedProfile(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        __forceinline bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue,
            bool a_scalar);

        __forceinline bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue,
            const armorCacheEntry_t::mapped_type* a_cacheEntry,
            bool a_scalar);

        __forceinline void DrawColliderShapeCombo(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_entry);

        char m_scBuffer1[64 + std::numeric_limits<float>::digits];
        bool m_eraseCurrent;
    };

    template <class T, UIEditorID ID>
    class UINode :
        virtual protected UIBase,
        UINodeCommon<T>,
        UIMainItemFilter<ID>
    {
    public:
        UINode();
        virtual ~UINode() = default;

        void DrawNodes(
            T a_handle,
            configNodes_t& a_data);

    protected:

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name) const
        {
            std::ostringstream ss;
            ss << "UIND#" << Enum::Underlying(ID) << "#" << a_name;
            return ss.str();
        }

        [[nodiscard]] virtual std::string GetGCSID(
            const std::string& a_name) const;

    };

    template <class T, class P>
    class UIProfileSelector :
        public UICommon::UIProfileSelectorBase<T, P>
    {
    protected:
        virtual ProfileManager<P>& GetProfileManager() const;
    };

    template <class T>
    class UIApplyForce :
        virtual protected UIBase,
        UICommon::UIDataBase<T, configComponents_t>
    {
        static constexpr float FORCE_MIN = -1000.0f;
        static constexpr float FORCE_MAX = 1000.0f;
    protected:
        UIApplyForce() = default;
        virtual ~UIApplyForce() = default;

        void DrawForceSelector(T* a_data, configForceMap_t& a_forceData);

        virtual void ApplyForce(
            T* a_data,
            uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force) const = 0;

    private:
        struct {
            SelectedItem<std::string> selected;
        } m_forceState;

        static const std::string m_chKey;
    };

    class UIProfileEditorPhysics :
        public UICommon::UIProfileEditorBase<PhysicsProfile>,
        UISimComponent<int, UIEditorID::kProfileEditorPhys>
    {
    public:
        UIProfileEditorPhysics(const char* a_name) :
            UICommon::UIProfileEditorBase<PhysicsProfile>(a_name) {}
    private:

        virtual ProfileManager<PhysicsProfile>& GetProfileManager() const;

        virtual void DrawItem(PhysicsProfile& a_profile);
        virtual void DrawOptions(PhysicsProfile& a_profile);

        void DrawGroupOptions(
            int,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            nodeConfigList_t& a_nodeConfig);

        virtual void OnSimSliderChange(
            int,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val);

        virtual void OnColliderShapeChange(
            int a_handle,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        );

        virtual void OnComponentUpdate(
            int a_handle,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair);

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const;

        virtual const configNodes_t& GetNodeData(
            int a_handle) const;

        virtual bool ShouldDrawComponent(
            int a_handle,
            PhysicsProfile::base_type& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const;

        virtual void UpdateNodeData(
            int a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

        //UISelectedItem<std::string> m_selectedConfGroup;
    };

    class UIProfileEditorNode :
        public UICommon::UIProfileEditorBase<NodeProfile>,
        UINode<int, UIEditorID::kProfileEditorNode>
    {
    public:
        UIProfileEditorNode(const char* a_name) :
            UICommon::UIProfileEditorBase<NodeProfile>(a_name) {}
    private:

        virtual ProfileManager<NodeProfile>& GetProfileManager() const;

        virtual void DrawItem(NodeProfile& a_profile);
        virtual void UpdateNodeData(
            int,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data,
            bool a_reset);
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
            UInt32& a_out);
    };

    template <class T, class P>
    class UIListBase :
        virtual protected UIBase
    {
    public:

        inline void QueueListUpdateCurrent() {
            m_listNextUpdateCurrent = true;
        }

        inline void QueueListUpdate() {
            m_listNextUpdate = true;
        }

    protected:
        typedef typename T::value_type listValue_t;
        typedef typename T::value_type::second_type::second_type entryValue_t;

        void ListTick();
        void ListReset();
        void ListUpdateCurrent();
        void ListDrawInfo(listValue_t* a_entry);

        UIListBase() noexcept;
        virtual ~UIListBase() noexcept = default;

        virtual void ListDraw(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry) = 0;
        virtual listValue_t* ListGetSelected() = 0;
        virtual void ListSetCurrentItem(P a_handle) = 0;
        virtual void ListUpdate() = 0;
        virtual void ListResetAllValues(P a_handle) = 0;
        [[nodiscard]] virtual const entryValue_t& GetData(P a_formid) = 0;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) = 0;

        bool m_listFirstUpdate;
        bool m_listNextUpdateCurrent;
        bool m_listNextUpdate;

        T m_listData;
        P m_listCurrent;

        char m_listBuf1[128];
        UICommon::UIGenericFilter m_listFilter;
    };

    template <class T>
    class UIActorList :
        public UIListBase<T, Game::ObjectHandle>
    {
    public:
        void ActorListTick();
        virtual void ListReset();
    protected:
        using listValue_t = typename UIListBase<T, Game::ObjectHandle>::listValue_t;
        using entryValue_t = typename UIListBase<T, Game::ObjectHandle>::entryValue_t;

        UIActorList(bool a_mark);
        virtual ~UIActorList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListDraw(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListSetCurrentItem(Game::ObjectHandle a_handle);

        virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const = 0;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const = 0;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const = 0;
    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);

        uint64_t m_lastCacheUpdateId;

        bool m_markActor;
    };

    template <class T>
    class UIRaceList :
        public UIListBase<T, Game::FormID>
    {
    protected:
        using listValue_t = typename UIListBase<T, Game::FormID>::listValue_t;
        using entryValue_t = typename UIListBase<T, Game::FormID>::entryValue_t;

        UIRaceList();
        virtual ~UIRaceList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListSetCurrentItem(Game::FormID a_formid);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const = 0;

    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);
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
        UIActorEditorNode() noexcept;

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

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const;
    };

    typedef std::pair<const std::string, configComponents_t> raceEntryPhysConf_t;
    typedef std::map<Game::FormID, raceEntryPhysConf_t> raceListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> raceEntryNodeConf_t;
    typedef std::map<Game::FormID, raceEntryNodeConf_t> raceListNodeConf_t;

    template <class T, class N>
    class UIRaceEditorBase :
        virtual protected UIBase,
        public UIRaceList<T>,
        protected UIProfileSelector<typename T::value_type, N>
    {
    public:

        [[nodiscard]] inline bool GetChanged() {
            bool r = m_changed;
            m_changed = false;
            return r;
        }

        virtual void Reset();

    protected:
        UIRaceEditorBase() noexcept;
        virtual ~UIRaceEditorBase() noexcept = default;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid) = 0;

        inline void MarkChanged() { m_changed = true; }

        bool m_changed;
    };

    class UIRaceEditorNode :
        public UIRaceEditorBase<raceListNodeConf_t, NodeProfile>,
        UINode<Game::FormID, UIEditorID::kRaceNodeEditor>
    {
    public:
        UIRaceEditorNode() noexcept;

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
    };

    class UIRaceEditorPhysics :
        public UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>,
        UISimComponent<Game::FormID, UIEditorID::kRacePhysicsEditor>
    {
    public:
        UIRaceEditorPhysics() noexcept;

        void Draw(bool* a_active);
    private:

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const;

        virtual void ListResetAllValues(Game::FormID a_formid);

        virtual void DrawConfGroupNodeMenu(
            Game::FormID a_handle,
            nodeConfigList_t& a_nodeList
        );

        virtual void OnSimSliderChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val);

        virtual void OnColliderShapeChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc);

        virtual void OnComponentUpdate(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const;

        virtual const configNodes_t& GetNodeData(
            Game::FormID a_handle) const;

        virtual void UpdateNodeData(
            Game::FormID a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual bool ShouldDrawComponent(
            Game::FormID a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasMovement(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasCollisions(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

        virtual const PhysicsProfile* GetSelectedProfile() const;

    };

    class UIPlot
    {
    public:
        UIPlot(
            const char* a_label,
            const ImVec2& a_size,
            bool a_avg,
            uint32_t a_res);

        void Update(float a_value);
        void Draw();
        void SetRes(int a_res);
        void SetHeight(float a_height);

        inline void SetShowAvg(bool a_switch) {
            m_avg = a_switch;
        }

    private:
        std::vector<float> m_values;

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
        uint32_t m_lastUID;

        UIPlot m_plotUpdateTime;
        UIPlot m_plotFramerate;
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

        inline const auto& GetSelected() const {
            return m_selected;
        }

        inline const auto& GetLastException() const {
            return m_lastExcept;
        }

    private:
        SelectedItem<SelectedFile> m_selected;
        std::vector<fs::path> m_files;

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

        inline void SetScrollBottom() {
            m_doScrollBottom = true;
        }

    private:
        bool m_doScrollBottom;
        int8_t m_initialScroll;
    };

    class UIContext :
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

        private:
            virtual void DrawConfGroupNodeMenu(
                Game::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            );

            virtual void OnSimSliderChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

            virtual void OnColliderShapeChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual void OnComponentUpdate(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);

            virtual bool ShouldDrawComponent(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                const configGroupMap_t::value_type& a_cgdata,
                const nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasMovement(
                const nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasCollisions(
                const nodeConfigList_t& a_nodeConfig) const;

            virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
                Game::ObjectHandle a_handle,
                const std::string& a_comp) const;

            virtual bool GetNodeConfig(
                const configNodes_t& a_nodeConf,
                const configGroupMap_t::value_type& cg_data,
                nodeConfigList_t& a_out) const;

            virtual const configNodes_t& GetNodeData(
                Game::ObjectHandle a_handle) const;

            virtual void UpdateNodeData(
                Game::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset);

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

            virtual const PhysicsProfile* GetSelectedProfile() const;

            UIContext& m_ctxParent;
        };

        class UISimComponentGlobal :
            public UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            UISimComponentGlobal(UIContext& a_parent);

        private:
            virtual void DrawConfGroupNodeMenu(
                Game::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            );

            virtual void OnSimSliderChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

            virtual void OnColliderShapeChange(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual void OnComponentUpdate(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);

            virtual bool ShouldDrawComponent(
                Game::ObjectHandle a_handle,
                configComponents_t& a_data,
                const configGroupMap_t::value_type& a_cgdata,
                const nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasMovement(
                const nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasCollisions(
                const nodeConfigList_t& a_nodeConfig) const;

            virtual bool GetNodeConfig(
                const configNodes_t& a_nodeConf,
                const configGroupMap_t::value_type& cg_data,
                nodeConfigList_t& a_out) const;

            virtual const configNodes_t& GetNodeData(
                Game::ObjectHandle a_handle) const;

            virtual void UpdateNodeData(
                Game::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset);

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

            virtual const PhysicsProfile* GetSelectedProfile() const;

            UIContext& m_ctxParent;
        };

    public:

        UIContext() noexcept;

        void Initialize();

        void Reset(uint32_t a_loadInstance);
        void Draw(bool* a_active);

        [[nodiscard]] inline uint32_t GetLoadInstance() const noexcept {
            return m_activeLoadInstance;
        }

        inline void LogNotify() {
            m_log.SetScrollBottom();
        }

    private:

        void DrawMenuBar(bool* a_active, const listValue_t* a_entry);

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& m_peComponents);

        virtual void ApplyForce(
            listValue_t* a_data,
            uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force) const;

        virtual void ListResetAllValues(Game::ObjectHandle a_handle);

        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data);

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const;

        uint32_t m_activeLoadInstance;
        long long m_tsNoActors;

        struct {
            struct {
                bool options;
                bool profileSim;
                bool profileNodes;
                bool race;
                bool raceNode;
                bool collisionGroups;
                bool nodeConf;
                bool profiling;
                bool debug;
                bool log;
                bool importDialog;
            } windows;

            struct {
                bool openExportDialog;
                bool openImportDialog;
            } menu;

            except::descriptor lastException;
        } m_state;

        UICommon::UIPopupQueue<UICommon::UIPopupData> m_popup;

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

#ifdef _CBP_ENABLE_DEBUG
        UIDebugInfo m_debug;
#endif

        UISimComponentActor m_scActor;
        UISimComponentGlobal m_scGlobal;
    };

}