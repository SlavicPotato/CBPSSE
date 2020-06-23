#pragma once

namespace CBP
{
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