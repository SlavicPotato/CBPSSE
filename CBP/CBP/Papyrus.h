#pragma once

namespace CBP
{
    class ConfigUpdateTask :
        public TaskDelegate
    {
    public:
        ConfigUpdateTask();

        virtual void Run();
        virtual void Dispose();

        static ConfigUpdateTask* Create(const BSFixedString& sect, const BSFixedString& key, float val);
        static ConfigUpdateTask* Create(Game::ObjectHandle handle, const BSFixedString& sect, const BSFixedString& key, float val);
    private:
        std::string m_sect;
        std::string m_key;
        float m_val;
        Game::ObjectHandle m_handle;
    };

    bool RegisterFuncs(VMClassRegistry* registry);
}