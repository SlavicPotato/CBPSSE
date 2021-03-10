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

        static ConfigUpdateTask* Create(ConfigGender a_gender, const BSFixedString& sect, const BSFixedString& key, float val);
        static ConfigUpdateTask* Create(Game::ObjectHandle handle, ConfigGender a_gender, const BSFixedString& sect, const BSFixedString& key, float val);
    private:
        std::string m_sect;
        std::string m_key;
        float m_val;
        Game::ObjectHandle m_handle;
        ConfigGender m_gender;
    };

    bool RegisterFuncs(VMClassRegistry* registry);
}