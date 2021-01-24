#pragma once

namespace CBP
{

    template <class T, class P>
    class UIListBase :
        virtual protected UIBase
    {
    public:

        SKMP_FORCEINLINE void QueueListUpdateCurrent() {
            m_listNextUpdateCurrent = true;
        }

        SKMP_FORCEINLINE void QueueListUpdate() {
            m_listNextUpdate = true;
        }

    protected:
        typedef typename T::value_type listValue_t;
        typedef typename T::value_type::second_type::second_type entryValue_t;

        void ListTick();
        void ListReset();
        void ListUpdateCurrent();
        void ListDrawInfo(listValue_t* a_entry);

        UIListBase(float a_itemWidthScalar = -10.5f) noexcept;
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
        float m_itemWidthScalar;
    };

}