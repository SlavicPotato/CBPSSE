#pragma once

#include "Config.h"

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
        static ConfigUpdateTask* Create(Game::VMHandle handle, ConfigGender a_gender, const BSFixedString& sect, const BSFixedString& key, float val);
    private:
        stl::fixed_string m_sect;
        stl::fixed_string m_key;
        float m_val;
        Game::VMHandle m_handle;
        ConfigGender m_gender;
    };

    bool RegisterFuncs(VMClassRegistry* registry);
}