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
        armorOverrides
    };

    typedef std::pair<const std::string, configComponents_t> actorEntryBaseConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryBaseConf_t> actorListBaseConf_t;

    typedef std::pair<const std::string, configNodes_t> actorEntryNodeConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    class UIBase
    {
    protected:
        bool CollapsingHeader(
            const std::string& a_key,
            const char* a_label,
            bool a_default = true) const;

        void HelpMarker(MiscHelpText a_id) const;

        inline float GetNextTextOffset(const char* a_text)
        {
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

    private:

        float m_posOffset = 0.0f;

        std::unordered_map<std::string, float> m_ctlPositions;

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
            configComponentsValue_t& a_pair
        );

        virtual void OnSimSliderChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val
        ) = 0;

    protected:
        void Propagate(
            configComponents_t& a_dl,
            configComponents_t* a_dg,
            const std::string& a_comp,
            const std::string& a_key,
            float a_val);

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name)
        {
            std::ostringstream ss;
            ss << "UISC#" << Misc::Underlying(ID) << "#" << a_name;
            return ss.str();
        }

        float GetActualSliderValue(const armorCacheValue_t& a_cacheval, float a_baseval);

    private:
        virtual bool ShouldDrawComponent(
            T a_handle,
            const configComponents_t::value_type& a_comp) const;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            T a_handle,
            const std::string& a_comp) const;

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
            const configNode_t& a_data) = 0;

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name)
        {
            std::ostringstream ss;
            ss << "UIND#" << Misc::Underlying(ID) << "#" << a_name;
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
        } state;
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

    class UIProfileEditorSim :
        public UIProfileEditorBase<SimProfile>,
        UISimComponent<int, UIEditorID::kProfileEditorSim>
    {
    public:
        UIProfileEditorSim(const char* a_name) :
            UIProfileEditorBase<SimProfile>(a_name) {}
    private:
        virtual void DrawItem(SimProfile& a_profile);

        virtual void OnSimSliderChange(
            int,
            typename SimProfile::base_type& a_data,
            typename SimProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val);
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
            const NodeProfile::base_type::mapped_type& a_data);
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

    template <class T>
    class UIActorList
    {
    public:
        void ActorListTick();
        void ResetActorList();
    protected:
        using actorListValue_t = typename T::value_type;
        using actorEntryValue_t = typename T::value_type::second_type::second_type;

        UIActorList(bool a_mark);
        virtual ~UIActorList() noexcept = default;

        virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle) = 0;

        actorListValue_t* GetSelectedEntry();

        void DrawActorList(actorListValue_t*& a_entry, const char*& a_curSelName);
        void SetCurrentActor(SKSE::ObjectHandle a_handle);

        inline void ClearActorList() {
            m_actorList.clear();
        }

        T m_actorList;
        SKSE::ObjectHandle m_currentActor;

        uint64_t m_lastCacheUpdateId;

        bool m_firstUpdate;
        bool m_markActor;
    private:
        void UpdateActorList();

        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle) = 0;
        virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle) = 0;

        void FilterSelected(actorListValue_t*& a_entry, const char*& a_curSelName);

        char m_strBuf1[128];

        UIGenericFilter m_filter;

        std::string m_globLabel;
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

    class UINodeConfig :
        UIActorList<actorListNodeConf_t>,
        UIProfileSelector<actorListNodeConf_t::value_type, NodeProfile>,
        UINode<SKSE::ObjectHandle, UIEditorID::kNodeEditor>
    {
    public:
        UINodeConfig();

        void Draw(bool* a_active);
        void Reset();
    private:
        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const NodeProfile::base_type& GetData(const actorListValue_t* a_data) const;

        virtual void ApplyProfile(actorListValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            SKSE::ObjectHandle a_handle,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data);

        virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle);
    };

    typedef std::pair<const std::string, configComponents_t> raceEntry_t;
    typedef std::map<SKSE::FormID, raceEntry_t> raceList_t;

    class UIRaceEditor :
        UIProfileSelector<raceList_t::value_type, SimProfile>,
        UISimComponent<SKSE::FormID, UIEditorID::kRaceEditor>
    {
        using raceListValue_t = raceList_t::value_type;
    public:

        UIRaceEditor() noexcept;

        void Draw(bool* a_active);
        void Reset();

        inline void QueueUpdateRaceList() {
            m_nextUpdateRaceList = true;
        }

        [[nodiscard]] inline bool GetChanged() {
            bool r = m_changed;
            m_changed = false;
            return r;
        }
    private:

        raceListValue_t* GetSelectedEntry();

        void UpdateRaceList();
        void ResetAllRaceValues(SKSE::FormID a_formid, raceListValue_t* a_data);

        virtual void ApplyProfile(raceListValue_t* a_data, const SimProfile& a_profile);
        [[nodiscard]] virtual const configComponents_t& GetData(const raceListValue_t* a_data) const;

        virtual void OnSimSliderChange(
            SKSE::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val);

        inline void MarkChanged() { m_changed = true; }

        struct {
            except::descriptor lastException;
        } state;

        SKSE::FormID m_currentRace;
        raceList_t m_raceList;

        bool m_nextUpdateRaceList;
        bool m_changed;
        bool m_firstUpdate;

        UIGenericFilter m_filter;
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
        UIActorList<actorListBaseConf_t>,
        UIProfileSelector<actorListBaseConf_t::value_type, SimProfile>,
        UIApplyForce<actorListBaseConf_t::value_type>
    {
        using actorListValue_t = actorListBaseConf_t::value_type;

        class UISimComponentActor :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

        private:
            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                const configComponents_t::value_type& a_comp) const;

            virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
                SKSE::ObjectHandle a_handle,
                const std::string& a_comp) const;
        };

        class UISimComponentGlobal :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            virtual void OnSimSliderChange(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair,
                const componentValueDescMap_t::vec_value_type& a_desc,
                float* a_val);

            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                const configComponents_t::value_type& a_comp) const;
        };

    public:

        UIContext() noexcept;

        void Reset(uint32_t a_loadInstance);
        void Draw(bool* a_active);

        inline void QueueUpdateCurrentActor() {
            m_nextUpdateCurrentActor = true;
        }

        [[nodiscard]] inline uint32_t GetLoadInstance() const {
            return m_activeLoadInstance;
        }

        inline void LogNotify() {
            m_log.SetScrollBottom();
        }

    private:

        virtual void ApplyProfile(actorListValue_t* a_data, const SimProfile& m_peComponents);
        [[nodiscard]] virtual const SimProfile::base_type& GetData(const actorListValue_t* a_data) const;

        virtual void ApplyForce(
            actorListValue_t* a_data,
            uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force);

        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle);
        [[nodiscard]] virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle);
        virtual ConfigClass GetActorClass(SKSE::ObjectHandle a_handle);

        void UpdateActorValues(SKSE::ObjectHandle a_handle);
        void UpdateActorValues(actorListValue_t* a_data);

        bool m_nextUpdateCurrentActor;

        uint32_t m_activeLoadInstance;
        long long m_tsNoActors;

        struct {
            struct {
                bool options;
                bool profileSim;
                bool profileNodes;
                bool race;
                bool collisionGroups;
                bool nodeConf;
                bool profiling;
                bool debug;
                bool log;
                bool importDialog;
            } windows;

            except::descriptor lastException;
        } state;

        UIProfileEditorSim m_peComponents;
        UIProfileEditorNode m_peNodes;
        UIRaceEditor m_raceEditor;
        UIOptions m_options;
        UICollisionGroups m_colGroups;
        UINodeConfig m_nodeConfig;
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