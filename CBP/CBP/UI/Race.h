#pragma once

namespace CBP
{
    typedef std::pair<const std::string, configComponents_t> raceEntryPhysConf_t;
    typedef stl::map<Game::FormID, raceEntryPhysConf_t> raceListPhysConf_t;

    typedef std::pair<const std::string, configNodes_t> raceEntryNodeConf_t;
    typedef stl::map<Game::FormID, raceEntryNodeConf_t> raceListNodeConf_t;

    template <class T, class N>
    class UIRaceEditorBase :
        virtual protected UIBase,
        public UIRaceList<T>,
        protected UIProfileSelector<typename T::value_type, N>
    {
    public:

        [[nodiscard]] SKMP_FORCEINLINE bool GetChanged() {
            bool r = m_changed;
            m_changed = false;
            return r;
        }

        virtual void Reset();

    protected:
        UIRaceEditorBase() noexcept;
        virtual ~UIRaceEditorBase() noexcept = default;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid) = 0;

        SKMP_FORCEINLINE void MarkChanged() { m_changed = true; }

        bool m_changed;
    };

    template <class T, class N>
    UIRaceEditorBase<T, N>::UIRaceEditorBase() noexcept :
        UIRaceList<T>(),
        m_changed(false)
    {
    }

    template <class T, class N>
    void UIRaceEditorBase<T, N>::Reset() {

        UIListBase<T, Game::FormID>::ListReset();
        m_changed = false;
    }

}