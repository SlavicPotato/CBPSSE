#pragma once

namespace CBP
{

    typedef KVStorage<UInt32, const char*> keyDesc_t;

    template <class T>
    class UISelectedItem
    {
    public:
        UISelectedItem() noexcept :
            m_isSelected(false) {}

        virtual ~UISelectedItem() = default;

        inline void Set(const T& a_rhs) {
            m_isSelected = true;
            m_item = a_rhs;
        }

        inline void Set(T&& a_rhs) {
            m_isSelected = true;
            m_item = std::forward<T>(a_rhs);
        }

        inline T& operator=(const T& a_rhs) {
            m_isSelected = true;
            return (m_item = a_rhs);
        }

        inline T& operator=(T&& a_rhs) {
            m_isSelected = true;
            return (m_item = std::forward<T>(a_rhs));
        }

        inline void Clear() noexcept {
            m_isSelected = false;
        }

        [[nodiscard]] inline const T& Get() const noexcept {
            return m_item;
        }

        [[nodiscard]] inline const T& operator*() const noexcept {
            return m_item;
        }

        [[nodiscard]] inline const T* operator->() const {
            return std::addressof(m_item);
        }

        [[nodiscard]] inline T* operator->() {
            return std::addressof(m_item);
        }

        [[nodiscard]] inline bool Has() const noexcept {
            return m_isSelected;
        }

        [[nodiscard]] inline explicit operator bool() const noexcept {
            return m_isSelected;
        }

    private:
        T m_item;
        bool m_isSelected;
    };

    enum class MiscHelpText : int
    {
        none,
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
        dataFilterNode
    };

    typedef std::pair<const std::string, configComponents_t> actorEntryPhysConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryPhysConf_t> actorListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> actorEntryNodeConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    typedef std::vector<std::pair<const std::string, const configNode_t*>> nodeConfigList_t;

    class UIBase
    {
    protected:
        bool CollapsingHeader(
            const std::string& a_key,
            const char* a_label,
            bool a_default = true) const;

        bool Tree(
            const std::string& a_key,
            const char* a_label,
            bool a_default = true) const;

        void HelpMarker(MiscHelpText a_id) const;
        void HelpMarker(const std::string& a_text) const;

        inline float GetNextTextOffset(const char* a_text, bool a_clear = false)
        {
            if (a_clear)
                ClearTextOffset();

            auto it = m_ctlPositions.find(a_text);
            if (it != m_ctlPositions.end())
                return (m_posOffset += it->second + (m_posOffset == 0.0f ? 0.0f : 5.0f));

            return (m_posOffset += ImGui::CalcTextSize(a_text).x + 5.0f);
        }

        inline void ClearTextOffset() {
            m_posOffset = 0.0f;
        }

        inline bool ButtonRight(const char* a_text)
        {
            bool res = ImGui::Button(a_text);
            m_ctlPositions[a_text] = ImGui::GetItemRectSize().x;
            return res;
        }

        template <typename T>
        inline void SetGlobal(T& a_member, T const a_value) const;

        inline bool CheckboxGlobal(const char* a_label, bool* a_member) const;

        inline bool SliderFloatGlobal(
            const char* a_label,
            float* a_member,
            float a_min,
            float a_max,
            const char* a_fmt = "%.3f") const;

        inline bool SliderFloat3Global(
            const char* a_label,
            float* a_member,
            float a_min,
            float a_max,
            const char* a_fmt = "%.3f") const;

        inline bool SliderIntGlobal(
            const char* a_label,
            int* a_member,
            int a_min,
            int a_max,
            const char* a_fmt = "%d") const;

        void SetWindowDimensions(float a_offsetX = 0.0f, float a_sizeX = -1.0f, float a_sizeY = -1.0f);

        inline bool CanClip() const;

    private:

        float m_posOffset = 0.0f;

        std::unordered_map<std::string, float> m_ctlPositions;

        struct
        {
            ImVec2 sizeMin;
            ImVec2 sizeMax;
            ImVec2 pos;
            ImVec2 size;
            bool initialized = false;
        } m_sizeData;

        static const std::unordered_map<MiscHelpText, const char*> m_helpText;

    protected:
        static const keyDesc_t m_comboKeyDesc;
        static const keyDesc_t m_keyDesc;
    };

    template <typename T>
    class UIFilterBase :
        virtual protected UIBase
    {
    public:
        void Draw();
        void DrawButton();
        void Clear();
        void Toggle();

        virtual bool Test(const std::string& a_haystack) const = 0;

        inline bool IsOpen() const noexcept {
            return m_searchOpen;
        }

        [[nodiscard]] inline explicit operator bool() const noexcept {
            return m_filter.Has();
        }

        [[nodiscard]] inline const auto& operator*() const noexcept {
            return m_filter;
        }

        [[nodiscard]] inline const auto* operator->() const noexcept {
            return std::addressof(m_filter);
        }

        inline void NextSetFocus() {
            m_nextSetFocus = true;
        }

    protected:

        UIFilterBase();
        UIFilterBase(bool a_isOpen);
        UIFilterBase(bool a_isOpen, const char* a_label);
        UIFilterBase(bool a_isOpen, const char* a_label, MiscHelpText a_helpText);

        virtual void ProcessInput() = 0;

        const char* m_label;
        char m_filterBuf[128];

        UISelectedItem<T> m_filter;
        bool m_searchOpen;
        bool m_nextSetFocus;

        MiscHelpText m_helpText;
    };

    class UIGenericFilter :
        public UIFilterBase<std::string>
    {
    public:

        UIGenericFilter();
        UIGenericFilter(bool a_isOpen);
        UIGenericFilter(bool a_isOpen, const char* a_label);
        UIGenericFilter(bool a_isOpen, const char* a_label, MiscHelpText a_helpText);

        [[nodiscard]] virtual bool Test(const std::string& a_haystack) const;

    protected:
        virtual void ProcessInput();
    };

    class UIRegexFilter :
        public UIFilterBase<std::regex>
    {
    public:

        UIRegexFilter();
        UIRegexFilter(bool a_isOpen);
        UIRegexFilter(bool a_isOpen, const char* a_label);
        UIRegexFilter(bool a_isOpen, const char* a_label, MiscHelpText a_helpText);

        [[nodiscard]] virtual bool Test(const std::string& a_haystack) const;

    protected:
        virtual void ProcessInput();

    private:

        UISelectedItem<except::descriptor> m_lastException;
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

        UIRegexFilter m_dataFilter;
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

    private:
        virtual bool ShouldDrawComponent(
            T a_handle,
            nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasMovement(
            nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasCollisions(
            nodeConfigList_t& a_nodeConfig) const;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            T a_handle,
            const std::string& a_comp) const;

        virtual bool GetNodeConfig(
            T a_handle,
            const std::string& a_node,
            nodeConfigList_t &a_out) const = 0;

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

    template <class T, class C>
    class UIDataBase
    {
    protected:
        [[nodiscard]] virtual const C& GetData(const T* a_data) const = 0;
    };

    template <class T>
    class UIProfileBase
    {
    public:
        const T* GetCurrentProfile() const;
    protected:
        UIProfileBase() = default;
        virtual ~UIProfileBase() noexcept = default;

        void DrawCreateNew();

        struct {
            char new_input[60];
            UISelectedItem<std::string> selected;
            except::descriptor lastException;
        } m_state;
    };

    template <class T, class P>
    class UIProfileSelector :
        virtual protected UIBase,
        UIDataBase<T, typename P::base_type>,
        public UIProfileBase<P>
    {
    protected:
        UIProfileSelector() = default;
        virtual ~UIProfileSelector() = default;

        void DrawProfileSelector(T* a_data);

        virtual void ApplyProfile(
            T* a_data,
            const P& a_profile) = 0;

    private:
    };

    template <class T>
    class UIApplyForce :
        virtual protected UIBase,
        UIDataBase<T, configComponents_t>
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
            UISelectedItem<std::string> selected;
        } m_forceState;
    };

    template <class T>
    class UIProfileEditorBase :
        virtual protected UIBase,
        UIProfileBase<T>
    {
    public:
        UIProfileEditorBase(const char* a_name);
        virtual ~UIProfileEditorBase() noexcept = default;

        void Draw(bool* a_active);

        inline const char* GetName() const {
            return m_name;
        }
    protected:

        virtual void DrawItem(T& a_profile) = 0;
    private:

        virtual void DrawOptions() const;

        struct {
            char ren_input[60];
        } ex_state;

        UIGenericFilter m_filter;

        const char* m_name;
    };

    class UIProfileEditorPhysics :
        public UIProfileEditorBase<PhysicsProfile>,
        UISimComponent<int, UIEditorID::kProfileEditorPhys>
    {
    public:
        UIProfileEditorPhysics(const char* a_name) :
            UIProfileEditorBase<PhysicsProfile>(a_name) {}
    private:
        virtual void DrawItem(PhysicsProfile& a_profile);
        virtual void DrawOptions() const;

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
            int a_handle,
            const std::string& a_node,
            nodeConfigList_t& a_out) const;

        virtual void UpdateNodeData(
            int a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const;
    };

    class UIProfileEditorNode :
        public UIProfileEditorBase<NodeProfile>,
        UINode<int, UIEditorID::kProfileEditorNode>
    {
    public:
        UIProfileEditorNode(const char* a_name) :
            UIProfileEditorBase<NodeProfile>(a_name) {}
    private:
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
        [[nodiscard]] virtual const entryValue_t& GetData(P a_formid) const = 0;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const = 0;

        bool m_listFirstUpdate;
        bool m_listNextUpdateCurrent;
        bool m_listNextUpdate;

        T m_listData;
        P m_listCurrent;

        char m_listBuf1[128];
        UIGenericFilter m_listFilter;
    };

    template <class T>
    class UIActorList :
        public UIListBase<T, SKSE::ObjectHandle>
    {
    public:
        void ActorListTick();
        virtual void ListReset();
    protected:
        using listValue_t = typename UIListBase<T, SKSE::ObjectHandle>::listValue_t;
        using entryValue_t = typename UIListBase<T, SKSE::ObjectHandle>::entryValue_t;

        UIActorList(bool a_mark);
        virtual ~UIActorList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListDraw(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListSetCurrentItem(SKSE::ObjectHandle a_handle);

        virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle) const = 0;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const = 0;
    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);

        uint64_t m_lastCacheUpdateId;

        bool m_markActor;
        std::string m_globLabel;
    };

    template <class T>
    class UIRaceList :
        protected UIListBase<T, SKSE::FormID>
    {
    protected:
        using listValue_t = typename UIListBase<T, SKSE::FormID>::listValue_t;
        using entryValue_t = typename UIListBase<T, SKSE::FormID>::entryValue_t;

        UIRaceList();
        virtual ~UIRaceList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListSetCurrentItem(SKSE::FormID a_formid);

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
        UISelectedItem<uint64_t> m_selected;
        uint64_t m_input;
    };

    class UIActorEditorNode :
        public UIActorList<actorListNodeConf_t>,
        UIProfileSelector<actorListNodeConf_t::value_type, NodeProfile>,
        UINode<SKSE::ObjectHandle, UIEditorID::kNodeEditor>
    {
    public:
        UIActorEditorNode() noexcept;

        void Draw(bool* a_active);
        void Reset();
    private:
        virtual void ListResetAllValues(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::ObjectHandle a_handle) const;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const;

        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            SKSE::ObjectHandle a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        [[nodiscard]] virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
    };

    typedef std::pair<const std::string, configComponents_t> raceEntryPhysConf_t;
    typedef std::map<SKSE::FormID, raceEntryPhysConf_t> raceListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> raceEntryNodeConf_t;
    typedef std::map<SKSE::FormID, raceEntryNodeConf_t> raceListNodeConf_t;

    template <class T, class N>
    class UIRaceEditorBase :
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

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid) const = 0;

        inline void MarkChanged() { m_changed = true; }

        bool m_changed;
    };

    class UIRaceEditorNode :
        public UIRaceEditorBase<raceListNodeConf_t, NodeProfile>,
        UINode<SKSE::FormID, UIEditorID::kRaceNodeEditor>
    {
    public:
        UIRaceEditorNode() noexcept;

        void Draw(bool* a_active);

    private:

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid) const;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) const;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const;

        virtual void ListResetAllValues(SKSE::FormID a_formid);
        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            SKSE::FormID a_formid,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);
    };

    class UIRaceEditorPhysics :
        public UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>,
        UISimComponent<SKSE::FormID, UIEditorID::kRacePhysicsEditor>
    {
    public:
        UIRaceEditorPhysics() noexcept;

        void Draw(bool* a_active);
    private:

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile);
        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid) const;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) const;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const;

        virtual void ListResetAllValues(SKSE::FormID a_formid);

        virtual void DrawConfGroupNodeMenu(
            SKSE::FormID a_handle,
            nodeConfigList_t& a_nodeList
        );

        virtual void OnSimSliderChange(
            SKSE::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val);

        virtual void OnColliderShapeChange(
            SKSE::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc);

        virtual void OnComponentUpdate(
            SKSE::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        virtual bool GetNodeConfig(
            SKSE::FormID a_handle,
            const std::string& a_node,
            nodeConfigList_t& a_out) const;

        virtual void UpdateNodeData(
            SKSE::FormID a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual bool ShouldDrawComponent(
            SKSE::FormID a_handle,
            nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasMovement(
            nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasCollisions(
            nodeConfigList_t& a_nodeConfig) const;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

        virtual const PhysicsProfile* GetSelectedProfile() const;

    };

    class UIProfiling :
        UIBase
    {
    public:
        void Draw(bool* a_active);
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
        UISelectedItem<SelectedFile> m_selected;
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
        UILog() :
            m_doScrollBottom(true),
            m_initialScroll(false)
        {
        }

        void Draw(bool* a_active);

        inline void SetScrollBottom() {
            m_doScrollBottom = true;
        }

    private:
        bool m_doScrollBottom;
        bool m_initialScroll;
    };

    class UIContext :
        virtual UIBase,
        public UIActorList<actorListPhysConf_t>,
        public UIProfileSelector<actorListPhysConf_t::value_type, PhysicsProfile>,
        UIApplyForce<actorListPhysConf_t::value_type>
    {
        class UISimComponentActor :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            UISimComponentActor(UIContext& a_parent);

        private:
            virtual void DrawConfGroupNodeMenu(
                SKSE::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            );

            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

            virtual void OnColliderShapeChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual void OnComponentUpdate(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);

            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasMovement(
                nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasCollisions(
                nodeConfigList_t& a_nodeConfig) const;

            virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
                SKSE::ObjectHandle a_handle,
                const std::string& a_comp) const;

            virtual bool GetNodeConfig(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node,
                nodeConfigList_t& a_out) const;

            virtual void UpdateNodeData(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset);

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

            virtual const PhysicsProfile* GetSelectedProfile() const;

            UIContext& m_ctxParent;
        };

        class UISimComponentGlobal :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            UISimComponentGlobal(UIContext& a_parent);

        private:
            virtual void DrawConfGroupNodeMenu(
                SKSE::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeList
            );

            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

            virtual void OnColliderShapeChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual void OnComponentUpdate(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);

            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                nodeConfigList_t& a_nodeConfig) const;

            virtual bool HasMovement(
                nodeConfigList_t &a_nodeConfig) const;

            virtual bool HasCollisions(
                nodeConfigList_t& a_nodeConfig) const;

            virtual bool GetNodeConfig(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node,
                nodeConfigList_t& a_out) const;

            virtual void UpdateNodeData(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node,
                const configNode_t& a_data,
                bool a_reset);

            virtual configGlobalSimComponent_t& GetSimComponentConfig() const;

            virtual const PhysicsProfile* GetSelectedProfile() const;

            UIContext& m_ctxParent;
        };

    public:

        UIContext() noexcept;

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

        virtual void ListResetAllValues(SKSE::ObjectHandle a_handle);

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::ObjectHandle a_handle) const;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const;

        [[nodiscard]] virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;

        void UpdateActorValues(SKSE::ObjectHandle a_handle);
        void UpdateActorValues(listValue_t* a_data);

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
                bool saveAllFailed;
                bool saveToDefaultGlob;
            } menu;

            except::descriptor lastException;
        } m_state;

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