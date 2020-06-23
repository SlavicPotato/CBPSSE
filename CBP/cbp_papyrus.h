#pragma once

namespace CBP
{
    constexpr UInt32 CFG_SECTID_BREAST = 0x00000001;
    constexpr UInt32 CFG_SECTID_BUTT = 0x00000002;
    constexpr UInt32 CFG_SECTID_BELLY = 0x00000003;

    class ConfigUpdateTask :
        public TaskDelegate
    {
    public:
        virtual void Run();
        virtual void Dispose();

        static ConfigUpdateTask* Create(BSFixedString &sect, BSFixedString& key, float val);
    private:
        std::string m_sect;
        std::string m_key;
        float m_val;
    };

    class ConfigCommitTask :
        public TaskDelegateStatic
    {
    public:
        virtual void Run();
    };

    bool RegisterFuncs(VMClassRegistry* registry);
}