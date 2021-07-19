#pragma once

#include "Events/Events.h"
#include "UIData.h"

#include "Profile/Manager.h"

#include "Data/StringHolder.h"

namespace UICommon
{
    class UIAlignment
    {
    protected:

        float GetNextTextOffset(const stl::fixed_string& a_text, bool a_clear = false);
        void ClearTextOffset();
        bool ButtonRight(const stl::fixed_string& a_text, bool a_disabled = false);

    private:
        float m_posOffset = 0.0f;
        std::unordered_map<stl::fixed_string, float> m_ctlPositions;

    };

    struct SKMP_ALIGN(16) WindowLayoutData
    {
        SKMP_FORCEINLINE WindowLayoutData(
            float a_offset,
            float a_width,
            float a_height,
            bool a_centered)
            :
            m_offset(a_offset),
            m_width(a_width),
            m_height(a_height),
            m_centered(a_centered)
        {
        }

        float m_offset;
        float m_width;
        float m_height;
        bool m_centered;
    };

    class UIWindow
    {
    protected:
        void SetWindowDimensions(
            float a_offsetX = 0.0f,
            float a_sizeX = -1.0f,
            float a_sizeY = -1.0f,
            bool a_centered = false);

        SKMP_FORCEINLINE void SetWindowDimensions(const WindowLayoutData& a_data) {
            SetWindowDimensions(a_data.m_offset, a_data.m_width, a_data.m_height, a_data.m_centered);
        }

        bool CanClip() const;

    private:

        struct
        {
            ImVec2 sizeMin;
            ImVec2 sizeMax;
            ImVec2 pos;
            ImVec2 pivot;
            ImVec2 size;
            bool initialized = false;
        } m_sizeData;

    };

    template <int _ID>
    class UICollapsibleIDCache
    {

    public:

        UICollapsibleIDCache() = delete;
        UICollapsibleIDCache(
            const stl::fixed_string& a_prefix)
            :
            m_prefix(a_prefix)
        {
        }

        const stl::fixed_string& Get(const stl::fixed_string& a_key)
        {
            auto it = m_data.find(a_key);
            if (it == m_data.end())
            {
                std::string store(m_prefix);
                store += "#";
                store += std::to_string(_ID);
                store += "#";
                store += a_key;

                return m_data.emplace(a_key, store).first->second;
            }

            return it->second;
        }

    private:

        std::unordered_map<stl::fixed_string, stl::fixed_string> m_data;

        stl::fixed_string m_prefix;
    };

    template <int _ID>
    class UICollapsibleIDCache2 
    {
    public:

        UICollapsibleIDCache2() = delete;
        UICollapsibleIDCache2(
            const stl::fixed_string& a_prefix)
            :
            m_prefix(a_prefix)
        {
        }

        const stl::fixed_string& Get(
            const stl::fixed_string& a_key,
            const stl::fixed_string& a_key2)
        {
            auto& r = m_data.try_emplace(a_key).first->second;

            auto it = r.find(a_key2);
            if (it == r.end())
            {
                std::string store(m_prefix);
                store += "#";
                store += std::to_string(_ID);
                store += "#";
                store += a_key;
                store += "#";
                store += a_key2;

                return r.emplace(a_key2, store).first->second;
            }

            return it->second;
        }

    private:

        std::unordered_map<stl::fixed_string, std::unordered_map<stl::fixed_string, stl::fixed_string>> m_data;

        stl::fixed_string m_prefix;
    };

    class UICollapsibles
    {
    protected:

        bool CollapsingHeader(
            const stl::fixed_string& a_key,
            const char* a_label,
            bool a_default = true) const;

        bool Tree(
            const stl::fixed_string& a_key,
            const char* a_label,
            bool a_default = true,
            bool a_framed = false) const;

    private:
        virtual UIData::UICollapsibleStates& GetCollapsibleStatesData() const = 0;
        virtual void OnCollapsibleStatesUpdate() const;
    };

    class UIControls
    {
    protected:

        template <typename... Args>
        bool SliderFloat(
            const char* a_label,
            float* a_value,
            float a_min,
            float a_max,
            Args ...a_args) const;

        template <typename... Args>
        bool SliderFloat3(
            const char* a_label,
            float* a_value,
            float a_min,
            float a_max,
            Args ...a_args) const;

        template <typename... Args>
        bool SliderInt(
            const char* a_label,
            int* a_value,
            int a_min,
            int a_max,
            Args ...a_args) const;

        template <typename... Args>
        bool Checkbox(Args... a_args) const;

        bool ColorEdit4(const char* a_label, float a_col[4], ImGuiColorEditFlags a_flags = 0);

    private:
        virtual void OnControlValueChange() const;
    };

    template <typename... Args>
    bool UIControls::SliderFloat(
        const char* a_label,
        float* a_value,
        float a_min,
        float a_max,
        Args ...a_args) const
    {
        bool res = ImGui::SliderFloat(
            a_label, a_value, a_min, a_max, std::forward<Args>(a_args)...);

        if (res)
        {
            *a_value = std::clamp(*a_value, a_min, a_max);

            OnControlValueChange();
        }

        return res;
    }

    template <typename... Args>
    bool UIControls::SliderFloat3(
        const char* a_label,
        float* a_value,
        float a_min,
        float a_max,
        Args ... a_args) const
    {
        bool res = ImGui::SliderFloat3(
            a_label, a_value, a_min, a_max, std::forward<Args>(a_args)...);

        if (res)
        {
            for (std::uint32_t i = 0; i < 3; i++)
                a_value[i] = std::clamp(a_value[i], a_min, a_max);

            OnControlValueChange();
        }

        return res;
    }


    template <typename... Args>
    bool UIControls::SliderInt(
        const char* a_label,
        int* a_value,
        int a_min,
        int a_max,
        Args ...a_args) const
    {
        bool res = ImGui::SliderInt(
            a_label, a_value, a_min, a_max, std::forward<Args>(a_args)...);

        if (res)
        {
            *a_value = std::clamp(*a_value, a_min, a_max);

            OnControlValueChange();
        }

        return res;
    }

    template <typename... Args>
    bool UIControls::Checkbox(Args... a_args) const
    {
        bool res = ImGui::Checkbox(std::forward<Args>(a_args)...);

        if (res)
            OnControlValueChange();

        return res;
    }


    template <typename T>
    class UIFilterBase :
        UIAlignment
    {
    public:
        void Draw();
        void DrawButton();
        void Clear();
        void Toggle();

        virtual bool Test(const std::string& a_haystack) const = 0;

        SKMP_FORCEINLINE bool IsOpen() const noexcept {
            return m_searchOpen;
        }

        [[nodiscard]] SKMP_FORCEINLINE explicit operator bool() const noexcept {
            return m_filter.Has();
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& operator*() const noexcept {
            return m_filter;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto* operator->() const noexcept {
            return std::addressof(m_filter);
        }

        SKMP_FORCEINLINE void NextSetFocus() {
            m_nextSetFocus = true;
        }

        SKMP_FORCEINLINE bool Has() const noexcept {
            return m_filter.Has();
        }

    protected:

        UIFilterBase();
        UIFilterBase(bool a_isOpen);
        UIFilterBase(bool a_isOpen, const char* a_label);

        virtual void ProcessInput() = 0;
        virtual const char* GetHelpText() {
            return nullptr;
        };

        const char* m_label;
        char m_filterBuf[128];

        SelectedItem<T> m_filter;
        bool m_searchOpen;
        bool m_nextSetFocus;
    };

    template <typename T>
    UIFilterBase<T>::UIFilterBase() :
        m_searchOpen(false),
        m_label("Filter"),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase(bool a_isOpen) :
        m_searchOpen(a_isOpen),
        m_label("Filter"),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase(bool a_isOpen, const char* a_label) :
        m_searchOpen(a_isOpen),
        m_label(a_label),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    void UIFilterBase<T>::DrawButton()
    {
        ImGui::PushID(static_cast<const void*>(std::addressof(m_searchOpen)));

        if (ImGui::Button(m_searchOpen ? "<" : ">")) {
            m_searchOpen = !m_searchOpen;
            if (m_searchOpen)
                m_nextSetFocus = true;
        }

        ImGui::PopID();
    }

    template <typename T>
    void UIFilterBase<T>::Draw()
    {
        if (!m_searchOpen)
            return;

        ImGui::PushID(static_cast<const void*>(std::addressof(m_filter)));

        ProcessInput();

        ImGui::PopID();
    }

    template <typename T>
    void UIFilterBase<T>::Toggle()
    {
        m_searchOpen = !m_searchOpen;
    }

    template <typename T>
    void UIFilterBase<T>::Clear()
    {
        m_filter.Clear();
        m_filterBuf[0] = 0x0;
    }

    class UIGenericFilter :
        public UIFilterBase<std::string>
    {
    public:

        UIGenericFilter();
        UIGenericFilter(bool a_isOpen);
        UIGenericFilter(bool a_isOpen, const char* a_label);

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

        [[nodiscard]] virtual bool Test(const std::string& a_haystack) const;

    protected:
        virtual void ProcessInput();

    private:

        SelectedItem<except::descriptor> m_lastException;
    };

    enum class UIPopupType : std::uint32_t
    {
        Confirm,
        Input,
        Message
    };

    class UIPopupAction
    {
        friend class UIPopupQueue;

        using func_type = std::function<void(const UIPopupAction&)>;

    public:

        UIPopupAction() = delete;

        UIPopupAction(const UIPopupAction&) = delete;
        UIPopupAction(UIPopupAction&&) = delete;

        UIPopupAction& operator=(const UIPopupAction&) = delete;
        UIPopupAction& operator=(UIPopupAction&&) = delete;

        template<typename... Args>
        UIPopupAction(
            UIPopupType a_type,
            const char* a_key,
            const char* a_fmt,
            Args... a_args) noexcept
            :
            m_type(a_type),
            m_key(a_key)
        {
            m_input[0x0] = 0x0;
            _snprintf_s(m_buf, _TRUNCATE, a_fmt, std::forward<Args>(a_args)...);
        }

        /*SKMP_FORCEINLINE void call(const func_type& a_func) {
            m_func = a_func;
        }*/

        SKMP_FORCEINLINE void call(func_type a_func) noexcept {
            m_func = std::move(a_func);
        }

        SKMP_FORCEINLINE const auto& GetInput() const noexcept {
            return m_input;
        }

    private:

        std::string m_key;

        char m_buf[512];
        char m_input[512];

        UIPopupType m_type;
        func_type m_func;

    };

    class UIPopupQueue
    {
    public:

        UIPopupQueue() = default;

        UIPopupQueue(const UIPopupQueue&) = delete;
        UIPopupQueue(UIPopupQueue&&) = delete;

        UIPopupQueue& operator=(const UIPopupQueue&) = delete;
        UIPopupQueue& operator=(UIPopupQueue&&) = delete;

        void Run(float a_fontScale);

        template <class... Args>
        SKMP_FORCEINLINE decltype(auto) push(Args&&... a_v) {
            return m_queue.emplace(std::forward<Args>(a_v)...);
        }

        SKMP_FORCEINLINE void clear() {
            m_queue.swap(decltype(m_queue)());
        }

    private:
        std::queue<UIPopupAction> m_queue;
    };

    class UIWindowBase
    {
    public:

        UIWindowBase() :
            m_openState(false)
        {
        };

        UIWindowBase(const UIWindowBase&) = delete;
        UIWindowBase(UIWindowBase&&) = delete;
        UIWindowBase& operator=(const UIWindowBase&) = delete;
        UIWindowBase& operator=(UIWindowBase&&) = delete;

        SKMP_FORCEINLINE void SetOpenState(bool a_state) {
            m_openState = a_state;
        }

        SKMP_FORCEINLINE bool* GetOpenState() {
            return std::addressof(m_openState);
        }

        SKMP_FORCEINLINE bool IsWindowOpen() {
            return m_openState;
        }

    private:
        bool m_openState;
    };

    template <class T, class C>
    class UIDataBase
    {
    protected:

        [[nodiscard]] virtual const C& GetData(const T* a_data) = 0;
    };

    template <class T>
    class UIProfileBase :
        GenericEventSink<ProfileManagerEvent<T>>
    {
    public:

        void InitializeProfileBase();

        const T* GetCurrentProfile() const;

    protected:

        UIProfileBase() = default;
        virtual ~UIProfileBase() noexcept;

        void DrawCreateNew(float a_fontScale);

        virtual ProfileManager<T>& GetProfileManager() const = 0;
        virtual bool InitializeProfile(T& a_profile);
        virtual bool AllowCreateNew() const;
        virtual bool AllowSave() const;
        virtual void OnItemSelected(const stl::fixed_string& a_item);

        virtual void OnProfileAdd(
            const stl::fixed_string& a_name,
            T &a_profile);

        virtual void OnProfileSave(
            const stl::fixed_string& a_name,
            T& a_profile);

        virtual void OnProfileDelete(const stl::fixed_string& a_item);

        virtual void OnProfileRename(
            const stl::fixed_string& a_oldName, 
            const stl::fixed_string& a_newName);

        virtual void Receive(const ProfileManagerEvent<T>& a_evn) override;

        void SetSelected(const stl::fixed_string& a_item);

        struct {
            char new_input[60];
            SelectedItem<stl::fixed_string> selected;
            except::descriptor lastException;
        } m_state;

    };

    template <class T>
    UIProfileBase<T>::~UIProfileBase() noexcept
    {
        //GetProfileManager().RemoveSink(this);
    }

    template <class T>
    void UIProfileBase<T>::InitializeProfileBase()
    {
        GetProfileManager().AddSink(this);
    }

    template <class T>
    const T* UIProfileBase<T>::GetCurrentProfile() const
    {
        if (!m_state.selected)
            return nullptr;

        auto& data = GetProfileManager();

        auto it = data.Find(*m_state.selected);
        if (it != data.End())
            return std::addressof(it->second);

        return nullptr;
    }

    template <class T>
    void UIProfileBase<T>::DrawCreateNew(float a_fontScale)
    {
        if (UICommon::TextInputDialog("New profile", "Enter the profile name:",
            m_state.new_input, sizeof(m_state.new_input), a_fontScale))
        {
            if (StrHelpers::strlen(m_state.new_input))
            {
                auto& pm = GetProfileManager();

                T profile;

                if (pm.CreateProfile(m_state.new_input, profile, false))
                {
                    if (InitializeProfile(profile))
                    {
                        if (profile.Save())
                        {
                            stl::fixed_string name(profile.Name());
                            if (pm.AddProfile(std::move(profile)))
                            {
                                m_state.selected = name;
                            }
                            else {
                                m_state.lastException = pm.GetLastException();
                                ImGui::OpenPopup("Add Error");
                            }
                        }
                        else {
                            m_state.lastException = profile.GetLastException();
                            ImGui::OpenPopup("Save Error");
                        }
                    }
                    else {
                        ImGui::OpenPopup("Init Error");
                    }
                }
                else {
                    m_state.lastException = pm.GetLastException();
                    ImGui::OpenPopup("Create Error");
                }
            }
        }

        UICommon::MessageDialog("Init Error", "Could not initialize the profile");
        UICommon::MessageDialog("Create Error", "Could not create the profile\n\n%s", m_state.lastException.what());
        UICommon::MessageDialog("Save Error", "Could not save the profile\n\n%s", m_state.lastException.what());
        UICommon::MessageDialog("Add Error", "Could not add the profile\n\n%s", m_state.lastException.what());
    }

    template <class T>
    bool UIProfileBase<T>::InitializeProfile(T& a_profile)
    {
        return true;
    }

    template <class T>
    bool UIProfileBase<T>::AllowCreateNew() const
    {
        return true;
    }

    template <class T>
    bool UIProfileBase<T>::AllowSave() const
    {
        return true;
    }

    template <class T>
    void UIProfileBase<T>::OnItemSelected(
        const stl::fixed_string& a_item)
    {
    }

    template <class T>
    void UIProfileBase<T>::OnProfileAdd(
        const stl::fixed_string& a_name,
        T& a_profile)
    {
    }
    
    template <class T>
    void UIProfileBase<T>::OnProfileSave(
        const stl::fixed_string& a_name,
        T& a_profile)
    {
    }
    
    template <class T>
    void UIProfileBase<T>::OnProfileDelete(
        const stl::fixed_string& a_item)
    {
    }
    
    template <class T>
    void UIProfileBase<T>::OnProfileRename(
        const stl::fixed_string& a_oldName, 
        const stl::fixed_string& a_newName)
    {
    }

    template <class T>
    void UIProfileBase<T>::Receive(const ProfileManagerEvent<T>& a_evn)
    {
        switch (a_evn.m_type)
        {
        case ProfileManagerEvent<T>::EventType::kProfileAdd:
            OnProfileAdd(*a_evn.m_profile, *a_evn.m_data);
            break;
        case ProfileManagerEvent<T>::EventType::kProfileDelete:
            OnProfileDelete(*a_evn.m_profile);
            if (m_state.selected == *a_evn.m_profile) {
                m_state.selected.Clear();
            }            
            break;
        case ProfileManagerEvent<T>::EventType::kProfileSave:
            OnProfileSave(*a_evn.m_profile, *a_evn.m_data);
            break;
        case ProfileManagerEvent<T>::EventType::kProfileRename:
            OnProfileRename(*a_evn.m_oldProfile, *a_evn.m_profile);
            if (m_state.selected == *a_evn.m_oldProfile) {
                m_state.selected = *a_evn.m_profile;
            }
            break;
        }
    }

    template <class T>
    void UIProfileBase<T>::SetSelected(
        const stl::fixed_string& a_item)
    {
        m_state.selected = a_item;
        OnItemSelected(a_item);
    }

    template <class T, class P>
    class UIProfileSelectorBase :
        virtual protected UIAlignment,
        UIDataBase<T, typename P::base_type>,
        public UIProfileBase<P>
    {
    protected:
        UIProfileSelectorBase();
        virtual ~UIProfileSelectorBase() noexcept = default;

        void DrawProfileSelector(T* a_data, float a_fontScale);

        virtual void ApplyProfile(
            T* a_data,
            const P& a_profile) = 0;

        virtual void DrawProfileSelectorOptions(T* a_data);


    };

    template<class T, class P>
    UIProfileSelectorBase<T, P>::UIProfileSelectorBase()        
    {

    }

    template<class T, class P>
    void UIProfileSelectorBase<T, P>::DrawProfileSelector(T* a_data, float a_fontScale)
    {
        auto& pm = GetProfileManager();
        auto& data = pm.Data();

        ImGui::PushID(std::addressof(pm));

        const char* curSelName = nullptr;
        if (m_state.selected) {
            if (data.find(*m_state.selected) != data.end())
                curSelName = m_state.selected->c_str();
            else
                m_state.selected.Clear();
        }

        if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (const auto& e : data)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                bool selected = m_state.selected == e.first;
                if (selected) {
                    if (ImGui::IsWindowAppearing())
                        ImGui::SetScrollHereY();
                }

                if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                    m_state.selected = e.first;
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        auto wcm = ImGui::GetWindowContentRegionMax();

        auto& sh = Common::StringHolder::GetSingleton();

        ImGui::SameLine(wcm.x - GetNextTextOffset(sh.snew, true));
        if (ButtonRight(sh.snew, !AllowCreateNew())) {
            ImGui::OpenPopup("New profile");
            m_state.new_input[0] = 0;
        }

        DrawCreateNew(a_fontScale);

        if (m_state.selected)
        {
            auto& profile = data.at(*m_state.selected);

            ImGui::SameLine(wcm.x - GetNextTextOffset(sh.apply));
            if (ButtonRight(sh.apply)) {
                ImGui::OpenPopup("Apply from profile");
            }

            if (UICommon::ConfirmDialog(
                "Apply from profile",
                "Load data from profile '%s'?\n\nCurrent values will be lost.\n\n",
                profile.Name().c_str()))
            {
                ApplyProfile(a_data, profile);
            }

            if (AllowSave())
            {
                ImGui::SameLine(wcm.x - GetNextTextOffset(sh.save));
                if (ButtonRight(sh.save)) {
                    ImGui::OpenPopup("Save to profile");
                }
            }

            if (UICommon::ConfirmDialog(
                "Save to profile",
                "Save current values to profile '%s'?\n\n",
                profile.Name().c_str()))
            {
                auto& data = GetData(a_data);
                if (!profile.Save(data, true)) {
                    m_state.lastException = profile.GetLastException();
                    ImGui::OpenPopup("Save to profile error");
                }
            }

            UICommon::MessageDialog("Save to profile error",
                "Error saving to profile '%s'\n\n%s", profile.Name().c_str(),
                m_state.lastException.what());

            DrawProfileSelectorOptions(a_data);
        }

        ImGui::PopID();
    }

    template<class T, class P>
    void UIProfileSelectorBase<T, P>::DrawProfileSelectorOptions(T* a_data)
    {
    }

    template <class T>
    class UIProfileEditorBase :
        public UICommon::UIWindowBase,
        virtual protected UIAlignment,
        virtual protected UIWindow,
        public UIProfileBase<T>
    {
    public:
        UIProfileEditorBase(const char* a_name);
        virtual ~UIProfileEditorBase() noexcept = default;

        void Draw(float a_scale);

        SKMP_FORCEINLINE const stl::fixed_string& GetName() const {
            return m_name;
        }

    protected:

        virtual WindowLayoutData GetWindowDimensions() const;

        virtual void DrawItem(T& a_profile) = 0;
        virtual void OnReload(const T& a_profile);

        virtual bool OnDeleteWarningOverride(const std::string& a_key, std::string& a_msg);

    private:

        virtual ProfileManager<T>& GetProfileManager() const = 0;

        virtual void DrawOptions(T& a_profile);

        struct {
            char ren_input[60];
        } ex_state;

        UICommon::UIGenericFilter m_filter;

        stl::fixed_string m_name;
        std::string m_currentDeleteWarningText;
    };

    template <class T>
    UIProfileEditorBase<T>::UIProfileEditorBase(const char* a_name) :
        m_name(a_name)
    {
    }

    template <class T>
    void UIProfileEditorBase<T>::Draw(float a_scale)
    {
        SetWindowDimensions(GetWindowDimensions());

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin(m_name.c_str(), GetOpenState()))
        {
            ImGui::SetWindowFontScale(a_scale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            auto& data = GetProfileManager().Data();

            const char* curSelName = nullptr;
            if (m_state.selected)
            {
                if (data.find(*m_state.selected) != data.end())
                {
                    curSelName = m_state.selected->c_str();

                    if (!m_filter.Test(*m_state.selected))
                    {
                        for (const auto& e : data)
                        {
                            if (!m_filter.Test(e.first))
                                continue;

                            SetSelected(e.first);
                            curSelName = e.first.c_str();

                            break;
                        }
                    }
                }
                else {
                    m_state.selected.Clear();
                }
            }
            else {
                if (!data.empty()) {
                    SetSelected(data.begin()->first);
                    curSelName = m_state.selected->c_str();
                }
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * -9.0f);

            if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (const auto& e : data)
                {
                    if (!m_filter.Test(e.first))
                        continue;

                    ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                    bool selected = e.first == *m_state.selected;
                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.second.Name().c_str(), selected))
                        SetSelected(e.first);

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            m_filter.DrawButton();

            auto& sh = Common::StringHolder::GetSingleton();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset(sh.snew, true));
            if (ButtonRight(sh.snew, !AllowCreateNew())) {
                ImGui::OpenPopup("New profile");
                m_state.new_input[0] = 0;
            }

            m_filter.Draw();

            ImGui::PopItemWidth();

            DrawCreateNew(a_scale);

            if (m_state.selected)
            {
                auto& profile = data.at(*m_state.selected);

                if (AllowSave())
                {
                    if (ImGui::Button("Save")) {
                        if (!profile.Save()) {
                            ImGui::OpenPopup("Save");
                            m_state.lastException = profile.GetLastException();
                        }
                    }

                    ImGui::SameLine();
                }

                if (ImGui::Button("Delete"))
                {
                    if (!OnDeleteWarningOverride(*m_state.selected, m_currentDeleteWarningText)) {
                        m_currentDeleteWarningText = "Are you sure you want to delete profile '%s'?";
                    }

                    ImGui::OpenPopup("Delete");
                }

                ImGui::SameLine();
                if (ImGui::Button("Rename")) {
                    ImGui::OpenPopup("Rename");
                    _snprintf_s(ex_state.ren_input, _TRUNCATE, "%s", (*m_state.selected).c_str());
                }

                ImGui::SameLine();
                if (ImGui::Button("Reload")) {
                    if (!profile.Load())
                    {
                        ImGui::OpenPopup("Reload");
                        m_state.lastException = profile.GetLastException();
                    }
                    else
                    {
                        OnReload(profile);
                    }
                }

                ImGui::PushID("__pe_options");
                DrawOptions(profile);
                ImGui::PopID();

                if (UICommon::ConfirmDialog(
                    "Delete",
                    m_currentDeleteWarningText.c_str(), curSelName))
                {
                    auto& pm = GetProfileManager();
                    if (pm.DeleteProfile(*m_state.selected)) {
                        m_state.selected.Clear();
                    }
                    else {
                        m_state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Delete failed");
                    }
                }
                else if (UICommon::TextInputDialog("Rename", "Enter the new profile name:",
                    ex_state.ren_input, sizeof(ex_state.ren_input), a_scale))
                {
                    auto& pm = GetProfileManager();
                    std::string newName(ex_state.ren_input);

                    if (pm.RenameProfile(*m_state.selected, newName)) {
                        m_state.selected = newName;
                    }
                    else {
                        m_state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Rename failed");
                    }
                }
                else {

                    UICommon::MessageDialog("Save", "Saving profile '%s' to '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.PathStr().c_str(), m_state.lastException.what());

                    UICommon::MessageDialog("Reload", "Loading profile '%s' from '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.PathStr().c_str(), m_state.lastException.what());

                    ImGui::Separator();

                    DrawItem(profile);
                }

                UICommon::MessageDialog("Delete failed",
                    "Could not delete the profile\n\n%s", m_state.lastException.what());
                UICommon::MessageDialog("Rename failed",
                    "Could not rename the profile\n\n%s", m_state.lastException.what());
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    template <class T>
    WindowLayoutData UIProfileEditorBase<T>::GetWindowDimensions() const
    {
        return WindowLayoutData(400.0f, -1.0f, -1.0f, false);
    }

    template <class T>
    void UIProfileEditorBase<T>::DrawOptions(T& a_profile)
    {
    }

    template <class T>
    void UIProfileEditorBase<T>::OnReload(const T& a_profile)
    {
    }

    template <class T>
    bool UIProfileEditorBase<T>::OnDeleteWarningOverride(
        const std::string& a_key,
        std::string& a_msg)
    {
        return false;
    }

    class UITextFileEditor
    {
    public:
        UITextFileEditor(const fs::path& a_path) :
            m_path(a_path)
        {
        }

    private:

        fs::path m_path;
    };


    void HelpMarker(const char* desc, float a_scale);

    template<typename... Args>
    bool ConfirmDialog(const char* name, const char* text, Args... args)
    {
        bool ret = false;
        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
            ImGui::Text(text, args...);
            ImGui::PopTextWrapPos();
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = true;
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        return ret;
    }

    template<typename... Args>
    int ConfirmDialog2(const char* name, float a_fontScale, const char* text, Args... args)
    {
        int ret = 0;
        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFontScale(a_fontScale);
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
            ImGui::Text(text, args...);
            ImGui::PopTextWrapPos();
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = 1;
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = -1;
            }
            ImGui::EndPopup();
        }

        return ret;
    }

    template<typename... Args>
    void MessageDialog(const char* name, const char* text, Args... args)
    {
        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
            ImGui::Text(text, args...);
            ImGui::PopTextWrapPos();
            ImGui::Separator();

            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    template<typename... Args>
    int MessageDialog2(const char* name, float a_fontScale, const char* text, Args... args)
    {
        auto& io = ImGui::GetIO();

        int ret(0);

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFontScale(a_fontScale);
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
            ImGui::Text(text, args...);
            ImGui::PopTextWrapPos();
            ImGui::Separator();

            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = -1;
            }

            ImGui::EndPopup();
        }

        return ret;
    }

    template<typename... Args>
    bool TextInputDialog(const char* a_name, const char* a_text, char* a_buf, std::size_t a_size, float a_scale, Args... args)
    {
        bool ret = false;

        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(a_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFontScale(a_scale);

            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);
            ImGui::Text(a_text, args...);
            ImGui::PopTextWrapPos();

            ImGui::Spacing();

            if (!ImGui::IsAnyItemActive())
                ImGui::SetKeyboardFocusHere();

            if (ImGui::InputText("##TextInputDialog", a_buf, a_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
                ImGui::CloseCurrentPopup();
                ret = true;
            }

            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = true;
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        return ret;
    }

    template<typename... Args>
    int TextInputDialog2(const char* a_name, const char* a_text, char* a_buf, std::size_t a_size, float a_scale, Args... args)
    {
        int ret(0);

        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(a_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::SetWindowFontScale(a_scale);

            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);
            ImGui::Text(a_text, args...);
            ImGui::PopTextWrapPos();

            ImGui::Spacing();

            if (!ImGui::IsAnyItemActive())
                ImGui::SetKeyboardFocusHere();

            if (ImGui::InputText("##TextInputDialog2", a_buf, a_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
                ImGui::CloseCurrentPopup();
                ret = 1;
            }

            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = 1;
            }

            ImGui::SameLine();
            ImGui::SetItemDefaultFocus();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                ret = -1;
            }

            ImGui::EndPopup();
        }

        return ret;
    }



}