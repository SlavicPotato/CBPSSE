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
        applyForce
    };

    typedef std::pair<const std::string, configComponents_t> actorEntryPhysConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryPhysConf_t> actorListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> actorEntryNodeConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

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
        inline void SetGlobal(T& a_member, T const a_value);

        inline bool CheckboxGlobal(const char* a_label, bool* a_member);

        inline bool SliderFloatGlobal(
            const char* a_label,
            float* a_member,
            float a_min,
            float a_max,
            const char* a_fmt = "%.3f");

        inline bool SliderFloat3Global(
            const char* a_label,
            float* a_member,
            float a_min,
            float a_max,
            const char* a_fmt = "%.3f");

        inline bool SliderIntGlobal(
            const char* a_label,
            int* a_member,
            int a_min,
            int a_max,
            const char* a_fmt = "%d");

        void SetWindowDimensions(float a_offsetX = 0.0f, float a_sizeX = -1.0f, float a_sizeY = -1.0f);

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
    };

    template <class T, UIEditorID ID>
    class UISimComponent :
        virtual protected UIBase
    {
    public:
        UISimComponent() = default;
        virtual ~UISimComponent() = default;

        void DrawSimComponents(
            T a_handle,
            configComponents_t& a_data);

        void DrawSliders(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const configNode_t* a_nodeConfig
        );

        virtual void OnSimSliderChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val,
            size_t a_size
        ) = 0;

        virtual void OnColliderShapeChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) = 0;

    protected:
        void Propagate(
            configComponents_t& a_dl,
            configComponents_t* a_dg,
            const std::string& a_comp,
            const std::string& a_key,
            float* a_val,
            size_t a_size);

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name)
        {
            std::ostringstream ss;
            ss << "UISC#" << Enum::Underlying(ID) << "#" << a_name;
            return ss.str();
        }

        [[nodiscard]] inline std::string GetCSSID(
            const std::string& a_name, const char* a_group)
        {
            std::ostringstream ss;
            ss << "UISC#" << Enum::Underlying(ID) << "#" << a_name << "#" << a_group;
            return ss.str();
        }

        float GetActualSliderValue(const armorCacheValue_t& a_cacheval, float a_baseval);

    private:
        virtual bool ShouldDrawComponent(
            T a_handle,
            const configNode_t* a_nodeConfig) const;

        virtual bool HasMovement(
            const configNode_t* a_nodeConfig) const;

        virtual bool HasCollisions(
            const configNode_t* a_nodeConfig) const;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            T a_handle,
            const std::string& a_comp) const;

        virtual const configNode_t* GetNodeConfig(
            T a_handle,
            const std::string& a_node) const = 0;

        void DrawMirrorContextMenu(
            T a_handle,
            configComponents_t& a_data,
            configComponents_t::value_type& a_entry);

        __forceinline bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue);

        __forceinline bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue,
            const armorCacheEntry_t::mapped_type* a_cacheEntry);

        __forceinline void DrawColliderShapeCombo(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_entry);


        char m_scBuffer1[64 + std::numeric_limits<float>::digits];
    };

    template <class T, UIEditorID ID>
    class UINode :
        virtual protected UIBase
    {
    public:
        UINode() = default;
        virtual ~UINode() = default;

        void DrawNodes(
            T a_handle,
            configNodes_t& a_data);

    protected:

        virtual void UpdateNodeData(
            T a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) = 0;

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name)
        {
            std::ostringstream ss;
            ss << "UIND#" << Enum::Underlying(ID) << "#" << a_name;
            return ss.str();
        }
    };

    class UIGenericFilter
    {
    public:
        UIGenericFilter();

    public:
        void Draw();
        void DrawButton();
        void Clear();
        void Toggle();

        [[nodiscard]] bool Test(const std::string& a_haystack) const;

        inline bool IsOpen() {
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
    private:
        char m_filterBuf[128];

        UISelectedItem<std::string> m_filter;
        bool m_searchOpen;
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
    protected:
        UIProfileBase() = default;
        virtual ~UIProfileBase() = default;

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
        UIProfileBase<P>
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
            const NiPoint3& a_force) = 0;

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

        inline constexpr const char* GetName() const {
            return m_name;
        }
    protected:

        virtual void DrawItem(T& a_profile) = 0;
    private:

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

        virtual void OnSimSliderChange(
            int,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val,
            size_t a_size);

        virtual void OnColliderShapeChange(
            int a_handle,
            PhysicsProfile::base_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        );

        virtual const configNode_t* GetNodeConfig(
            int a_handle,
            const std::string& a_node) const;
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

        inline void QueueUpdateCurrent() {
            m_nextUpdateCurrent = true;
        }

        inline void QueueUpdateList() {
            m_nextUpdateList = true;
        }

    protected:
        void ListTick();

        virtual void ResetList();
        virtual void UpdateCurrent();

        typedef typename T::value_type listValue_t;
        typedef typename T::value_type::second_type::second_type entryValue_t;

        UIListBase();
        virtual ~UIListBase() noexcept = default;

        virtual listValue_t* GetSelectedEntry() = 0;
        virtual void DrawList(listValue_t*& a_entry, const char*& a_curSelName) = 0;
        virtual void SetCurrentItem(P a_handle) = 0;
        virtual void UpdateList() = 0;
        virtual void ResetAllValues(P a_handle) = 0;
        virtual void FilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        [[nodiscard]] virtual const entryValue_t& GetData(P a_formid) = 0;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const = 0;

        bool m_firstUpdate;
        bool m_nextUpdateCurrent;
        bool m_nextUpdateList;

        T m_list;
        P m_current;

        char m_strBuf1[128];
        UIGenericFilter m_filter;
    };

    template <class T>
    class UIActorList :
        public UIListBase<T, SKSE::ObjectHandle>
    {
    public:
        void ActorListTick();
        virtual void ResetList();
    protected:
        using listValue_t = typename UIListBase<T, SKSE::ObjectHandle>::listValue_t;
        using entryValue_t = typename UIListBase<T, SKSE::ObjectHandle>::entryValue_t;

        UIActorList(bool a_mark);
        virtual ~UIActorList() noexcept = default;

        virtual listValue_t* GetSelectedEntry();
        virtual void DrawList(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void SetCurrentItem(SKSE::ObjectHandle a_handle);

        virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle) = 0;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() = 0;
    private:
        virtual void UpdateList();
        virtual void FilterSelected(listValue_t*& a_entry, const char*& a_curSelName);

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

        virtual listValue_t* GetSelectedEntry();
        virtual void DrawList(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void SetCurrentItem(SKSE::FormID a_formid);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() = 0;

    private:
        virtual void UpdateList();
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
        virtual void ResetAllValues(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const;

        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            SKSE::ObjectHandle a_handle,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data,
            bool a_reset);

        [[nodiscard]] virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig();
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

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid) = 0;

        inline void MarkChanged() { m_changed = true; }

        bool m_changed;

        struct {
            except::descriptor lastException;
        } state;

    };

    class UIRaceEditorNode :
        public UIRaceEditorBase<raceListNodeConf_t, NodeProfile>,
        UINode<SKSE::FormID, UIEditorID::kRaceNodeEditor>
    {
    public:
        UIRaceEditorNode() noexcept;

        void Draw(bool* a_active);

    private:

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) const;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig();

        virtual void ResetAllValues(SKSE::FormID a_formid);
        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            SKSE::FormID a_formid,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data,
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
        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::FormID a_formid);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) const;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig();

        virtual void ResetAllValues(SKSE::FormID a_formid);

        virtual void OnSimSliderChange(
            SKSE::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val,
            size_t a_size);

        virtual void OnColliderShapeChange(
            SKSE::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc);

        virtual const configNode_t* GetNodeConfig(
            SKSE::FormID a_handle,
            const std::string& a_node) const;

        virtual bool ShouldDrawComponent(
            SKSE::FormID a_handle,
            const configNode_t* a_nodeConfig) const;

        virtual bool HasMovement(
            const configNode_t* a_nodeConfig) const;

        virtual bool HasCollisions(
            const configNode_t* a_nodeConfig) const;

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
        UIFileSelector(const fs::path& a_path);

        void DrawFileSelector();
        bool DeleteExport(const fs::path& a_file);

        inline const auto& GetSelected() const {
            return m_selected;
        }

        inline void SetPath(const fs::path& a_path) {
            m_root = a_path;
            m_rootString = a_path.string();
        }

        inline const auto& GetLastException() const {
            return m_lastExcept;
        }

        inline const auto& GetRoot() const {
            return m_root;
        }

        inline const auto& GetRootStr() const {
            return m_rootString;
        }

    private:
        UISelectedItem<SelectedFile> m_selected;
        std::vector<fs::path> m_files;
        fs::path m_root;
        std::string m_rootString;

        except::descriptor m_lastExcept;
    };

    class UIDialogImport :
        public UIFileSelector
    {
    public:
        UIDialogImport(const fs::path& a_path);

        bool Draw(bool* a_active);
        void OnOpen();

    };

    class UIDialogExport :
        UIBase
    {
    public:
        UIDialogExport(const fs::path& a_path);

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
        UIProfileSelector<actorListPhysConf_t::value_type, PhysicsProfile>,
        UIApplyForce<actorListPhysConf_t::value_type>
    {
        //using actorListValue_t = actorListBaseConf_t::value_type;

        class UISimComponentActor :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
        private:
            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val,
                size_t a_size);

            virtual void OnColliderShapeChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                const configNode_t* a_nodeConfig) const;

            virtual bool HasMovement(
                const configNode_t* a_nodeConfig) const;

            virtual bool HasCollisions(
                const configNode_t* a_nodeConfig) const;

            virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
                SKSE::ObjectHandle a_handle,
                const std::string& a_comp) const;

            virtual const configNode_t* GetNodeConfig(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node) const;
        };

        class UISimComponentGlobal :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        private:
            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val,
                size_t a_size);

            virtual void OnColliderShapeChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc
            );

            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                const configNode_t* a_nodeConfig) const;

            virtual bool HasMovement(
                const configNode_t* a_nodeConfig) const;

            virtual bool HasCollisions(
                const configNode_t* a_nodeConfig) const;

            virtual const configNode_t* GetNodeConfig(
                SKSE::ObjectHandle a_handle,
                const std::string& a_node) const;
        };

    public:

        UIContext() noexcept;

        void Reset(uint32_t a_loadInstance);
        void Draw(bool* a_active);

        [[nodiscard]] inline uint32_t GetLoadInstance() const {
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
            const NiPoint3& a_force);

        virtual void ResetAllValues(SKSE::ObjectHandle a_handle);

        [[nodiscard]] virtual const entryValue_t& GetData(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) const;

        [[nodiscard]] virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig();

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