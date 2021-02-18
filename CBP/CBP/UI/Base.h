
#define sshex(size) std::uppercase << std::setfill('0') << std::setw(size) << std::hex


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
        timePerFrame,
        rotation,
        controllerStats
    };

    typedef std::pair<const std::string, configComponents_t> actorEntryPhysConf_t;
    typedef stl::map<Game::ObjectHandle, actorEntryPhysConf_t> actorListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> actorEntryNodeConf_t;
    typedef stl::map<Game::ObjectHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    typedef std::pair<const std::string, actorCacheEntry_t> actorEntryCache_t;
    typedef stl::map<Game::ObjectHandle, actorEntryCache_t> actorListCache_t;

    typedef stl::vector<std::pair<const std::string, const configNode_t*>> nodeConfigList_t;

    typedef std::function<void(configComponent32_t&, const configPropagate_t&)> propagateFunc_t;


    class UIBase :
        virtual protected UICommon::UIWindow,
        virtual protected UICommon::UIAlignment,
        virtual protected UICommon::UICollapsibles,
        virtual protected UICommon::UIControls
    {
    protected:

        void HelpMarker(MiscHelpText a_id) const;
        void HelpMarker(const std::string& a_text) const;
        void HelpMarker(const char* a_text) const;

        template <typename T>
        void SetGlobal(T& a_member, T const a_value) const;

    protected:

        static const keyDesc_t m_comboKeyDesc;
        static const keyDesc_t m_keyDesc;

    private:

        virtual UIData::UICollapsibleStates& GetCollapsibleStatesData() const;
        virtual void OnCollapsibleStatesUpdate() const;
        virtual void OnControlValueChange() const;

        static const char* GetHelpText(MiscHelpText a_code);
    };

    template <typename T>
    void UIBase::SetGlobal(T& a_member, T const a_value) const
    {
        static_assert(std::is_fundamental_v<T> || std::is_same_v<T, Game::ObjectHandle> || std::is_same_v<T, Game::FormID>);

        a_member = a_value;
        DCBP::MarkGlobalsForSave();
    }


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

    template <UIEditorID ID>
    UIMainItemFilter<ID>::UIMainItemFilter(MiscHelpText a_helpText) :
        m_dataFilter(true, "Regex")
    {
    }

    template <UIEditorID ID>
    void UIMainItemFilter<ID>::DrawItemFilter()
    {
        ImGui::PushID(static_cast<const void*>(std::addressof(m_dataFilter)));

        if (Tree(GetGCSID("Filter"), "Filter", false))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

            m_dataFilter.Draw();

            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        ImGui::PopID();
    }


};
