#pragma once

#include "Base.h"
#include "RaceList.h"
#include "Profile.h"

namespace CBP
{
    typedef std::pair<const stl::fixed_string, configComponentsGenderRoot_t> raceEntryPhysConf_t;
    typedef std::map<Game::FormID, raceEntryPhysConf_t> raceListPhysConf_t;

    typedef std::pair<const stl::fixed_string, configNodesGenderRoot_t> raceEntryNodeConf_t;
    typedef std::map<Game::FormID, raceEntryNodeConf_t> raceListNodeConf_t;

    template <class T, class N>
    class UIRaceEditorBase :
        virtual protected UIBase,
        public UIRaceList<T>,
        public UIProfileSelector<typename T::value_type, N>
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