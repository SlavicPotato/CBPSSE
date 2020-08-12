#pragma once

namespace CBP
{
    template <typename K, typename V>
    class KVStorage
    {
        typedef std::unordered_map<K, V> keyMap_t;
        typedef std::vector<typename keyMap_t::value_type> keyVec_t;

        using iterator = typename keyVec_t::iterator;
        using const_iterator = typename keyVec_t::const_iterator;
    public:

        KVStorage(const keyVec_t& a_in) :
            m_vec(a_in)
        {
            _init();
        }

        KVStorage(keyVec_t&& a_in) :
            m_vec(std::forward<keyVec_t>(a_in))
        {
            _init();
        }

        iterator begin() = delete;
        iterator end() = delete;

        [[nodiscard]] inline const_iterator begin() const noexcept {
            return m_vec.begin();
        }
        [[nodiscard]] inline const_iterator end() const noexcept {
            return m_vec.end();
        }

        [[nodiscard]] inline const keyMap_t* operator->() const {
            return std::addressof(m_map);
        }

    private:
        inline void _init() {
            for (const auto& p : m_vec)
                m_map.emplace(p);
        }

        keyMap_t m_map;
        keyVec_t m_vec;
    };

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

    enum UIEditorID : int {
        kProfileEditor = 0,
        kRaceEditor,
        kMainEditor
    };

    enum MiscHelpText
    {
        kHT_timeStep,
        kHT_timeScale,
        kHT_colMaxPenetrationDepth
    };

    class UIBase
    {
    protected:
        inline bool CollapsingHeader(const std::string &a_key, const char *a_label)
        {
            auto& globalConfig = IConfig::GetGlobalConfig();
            bool* cs = globalConfig.GetColStateAddr(a_key);
            *cs = ImGui::CollapsingHeader(a_label,
                *cs ? ImGuiTreeNodeFlags_DefaultOpen : 0);
            return *cs;
        }

        inline void HelpMarker(MiscHelpText a_id) {
            ImGui::SameLine();
            UICommon::HelpMarker(m_helpText.at(a_id));
        }
    private:
        static std::unordered_map<MiscHelpText, const char*> m_helpText;
    };

    template <class T, int ID>
    class UISimComponent :
        UIBase
    {
    public:
        UISimComponent() = default;
        virtual ~UISimComponent() = default;

        void DrawSimComponents(
            T a_handle,
            configComponents_t& a_data);

        virtual void AddSimComponentSlider(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) = 0;

    protected:
        void Propagate(
            configComponents_t& a_dl,
            configComponents_t* a_dg,
            const std::string& a_comp,
            std::string a_key,
            float a_val);

        [[nodiscard]] inline std::string GetCSID(
            const std::string& a_name)
        {
            std::ostringstream ss;
            ss << "UISC#" << ID << "#" << a_name;
            return ss.str();
        }
    private:
        virtual bool ShouldDrawComponent(T m_handle, const configComponents_t::value_type& a_comp) {
            return true;
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

        [[nodiscard]] bool Test(const std::string& a_haystack);

        inline bool IsOpen() {
            return m_searchOpen;
        }

        [[nodiscard]] inline explicit operator bool() const noexcept {
            return m_filter.Has();
        }

        [[nodiscard]] inline const auto& operator*() const noexcept {
            return m_filter;
        }

        [[nodiscard]] inline const auto* operator->() const {
            return std::addressof(m_filter);
        }
    private:
        char m_filterBuf[128];

        UISelectedItem<std::string> m_filter;
        bool m_searchOpen;
    };

    template <class T>
    class UIComponentBase
    {
    protected:
        [[nodiscard]] virtual const configComponents_t& GetComponentData(const T* a_data) const = 0;
    };

    template <class T>
    class UIProfileSelector :
        UIComponentBase<T>
    {
    protected:
        UIProfileSelector() = default;
        virtual ~UIProfileSelector() = default;

        void DrawProfileSelector(T* a_data);

        virtual void ApplyProfile(
            T* a_data,
            const Profile& a_profile) = 0;

    private:
        UISelectedItem<std::string> m_selectedProfile;
        std::exception m_lastException;
    };

    template <class T>
    class UIApplyForce :
        UIBase,
        UIComponentBase<T>
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

    class UIProfileEditor :
        UISimComponent<int, UIEditorID::kProfileEditor>
    {
    public:

        void Draw(bool* a_active);
    private:

        virtual void AddSimComponentSlider(
            int,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        struct {
            UISelectedItem<std::string> selected;
            char new_input[60];
            char ren_input[60];
            std::exception lastException;
        } state;

        UIGenericFilter m_filter;
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

    typedef std::pair<const std::string, configComponents_t> actorEntryBaseConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryBaseConf_t> actorListBaseConf_t;

    typedef std::pair<const std::string, nodeConfigHolder_t> actorEntryNodeConf_t;
    typedef std::map<SKSE::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    template <class T>
    class UIActorList
    {
    public:
        void UpdateActorList(const simActorList_t& a_list);

    protected:
        using actorListValue_t = typename T::value_type;
        using actorEntryValue_t = typename T::value_type::second_type::second_type;

        UIActorList();
        virtual ~UIActorList() noexcept = default;

        actorListValue_t* GetSelectedEntry();

        void DrawActorList(actorListValue_t*& a_entry, const char*& a_curSelName);
        void SetCurrentActor(SKSE::ObjectHandle a_handle);

        T m_actorList;
        SKSE::ObjectHandle m_currentActor;

        bool m_nextUpdateActorList;

    private:
        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle) = 0;
        virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle) = 0;

        void FilterSelected(actorListValue_t*& a_entry, const char*& a_curSelName);

        char m_strBuf1[128];

        UIGenericFilter m_filter;

        std::string m_globLabel;
    };

    class UICollisionGroups
    {
    public:
        UICollisionGroups() = default;

        void Draw(bool* a_active);
    private:
        UISelectedItem<uint64_t> m_selected;
        uint64_t m_input;
    };

    class UINodeConfig :
        UIActorList<actorListNodeConf_t>
    {
    public:
        void Draw(bool* a_active);
        void Reset();
    private:
        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle);
        virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle);

        void UpdateActorRecord(
            actorListValue_t* a_entry,
            const std::string& a_node,
            const actorEntryValue_t::mapped_type& a_rec);
    };

    typedef std::pair<const std::string, configComponents_t> raceEntry_t;
    typedef std::map<SKSE::FormID, raceEntry_t> raceList_t;

    class UIRaceEditor :
        UIProfileSelector<raceList_t::value_type>,
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

        virtual void ApplyProfile(raceListValue_t* a_data, const Profile& m_profile);
        [[nodiscard]] virtual const configComponents_t& GetComponentData(const raceListValue_t* a_data) const;

        virtual void AddSimComponentSlider(
            SKSE::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        inline void MarkChanged() { m_changed = true; }

        struct {
            std::exception lastException;
        } state;

        SKSE::FormID m_currentRace;
        raceList_t m_raceList;

        bool m_nextUpdateRaceList;
        bool m_changed;

        UIGenericFilter m_filter;
    };

    class UIProfiling :
        UIBase
    {
    public:
        void Draw(bool* a_active);
    };

    class UIContext :
        UIActorList<actorListBaseConf_t>,
        UIProfileSelector<actorListBaseConf_t::value_type>,
        UIApplyForce<actorListBaseConf_t::value_type>
    {
        using actorListValue_t = actorListBaseConf_t::value_type;

        class UISimComponentActor :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            virtual void AddSimComponentSlider(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);
        private:
            virtual bool ShouldDrawComponent(
                SKSE::ObjectHandle a_handle,
                const configComponents_t::value_type& a_comp);

        };

        class UISimComponentGlobal :
            public UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>
        {
        public:
            virtual void AddSimComponentSlider(
                SKSE::ObjectHandle a_handle,
                configComponents_t& a_data,
                configComponentsValue_t& a_pair);
        };

    public:

        UIContext() noexcept;

        void Reset(uint32_t a_loadInstance);
        void Draw(bool* a_active);

        inline void QueueUpdateActorList() {
            m_nextUpdateActorList = true;
        }

        inline void QueueUpdateCurrentActor() {
            m_nextUpdateCurrentActor = true;
        }

        [[nodiscard]] inline uint32_t GetLoadInstance() const {
            return m_activeLoadInstance;
        }

    private:

        virtual void ApplyProfile(actorListValue_t* a_data, const Profile& m_profile);
        [[nodiscard]] virtual const configComponents_t& GetComponentData(const actorListValue_t* a_data) const;

        virtual void ApplyForce(
            actorListValue_t* a_data,
            uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force);

        virtual void ResetAllActorValues(SKSE::ObjectHandle a_handle);
        void UpdateActorValues(SKSE::ObjectHandle a_handle);
        void UpdateActorValues(actorListValue_t* a_data);

        virtual const actorEntryValue_t& GetData(SKSE::ObjectHandle a_handle);

        bool m_nextUpdateCurrentActor;

        uint32_t m_activeLoadInstance;
        long long m_tsNoActors;

        struct {
            struct {
                bool options;
                bool profile;
                bool race;
                bool collisionGroups;
                bool nodeConf;
                bool profiling;
            } windows;

            std::exception lastException;
        } state;

        UIProfileEditor m_profile;
        UIRaceEditor m_raceEditor;
        UIOptions m_options;
        UICollisionGroups m_colGroups;
        UINodeConfig m_nodeConfig;
        UIProfiling m_profiling;

        UISimComponentActor m_scActor;
        UISimComponentGlobal m_scGlobal;
    };

}