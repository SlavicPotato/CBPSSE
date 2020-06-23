#include "pch.h"

namespace CBP
{
    static ConfigCommitTask g_confCommitTask;

    static IThreadSafeBasicMemPool<ConfigUpdateTask, 256> s_configUpdateTaskPool;

    static UInt32 PP_GetVersion(StaticFunctionTag* base)
    {
        return MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);
    }

    static void PP_ReloadConfig(StaticFunctionTag* base)
    {
        QueueConfigReload();
    }

    static bool PP_UpdateConfig(StaticFunctionTag* base, BSFixedString sect, BSFixedString key, float val)
    {
        auto cmd = ConfigUpdateTask::Create(sect, key, val);
        if (cmd != NULL) {
            SKSE::g_taskInterface->AddTask(cmd);
            return true;
        }
        return false;
    }

    static void PP_CommitConfig(StaticFunctionTag* base)
    {
        SKSE::g_taskInterface->AddTask(&g_confCommitTask);
    }

    bool RegisterFuncs(VMClassRegistry* registry)
    {
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, UInt32>("GetVersion", "CBP", PP_GetVersion, registry));
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, void>("Reload", "CBP", PP_ReloadConfig, registry));
        registry->RegisterFunction(
            new NativeFunction3<StaticFunctionTag, bool, BSFixedString, BSFixedString, float>("Update", "CBP", PP_UpdateConfig, registry));
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, void>("Save", "CBP", PP_CommitConfig, registry));

        return true;
    }

    ConfigUpdateTask* ConfigUpdateTask::Create(BSFixedString& _sect, BSFixedString& _key, float val)
    {
        std::string sect(_sect.c_str());
        std::string key(_key.c_str());

        transform(sect.begin(), sect.end(), sect.begin(), ::tolower);
        transform(key.begin(), key.end(), key.begin(), ::tolower);

        if (!IsConfigOption(sect, key)) {
            return NULL;
        }

        auto cmd = s_configUpdateTaskPool.Allocate();
        if (cmd != NULL) {
            cmd->m_sect = sect;
            cmd->m_key = key;
            cmd->m_val = val;
        }

        return cmd;
    }

    void ConfigUpdateTask::Run()
    {
        config[m_sect][m_key] = m_val;
        g_updateTask.UpdateConfig();
    }

    void ConfigUpdateTask::Dispose()
    {
        s_configUpdateTaskPool.Free(this);
    }

    void ConfigCommitTask::Run()
    {
        if (!SaveConfig()) {
            _ERROR("%s: Unable to save config", __FUNCTION__);
        }
    }
}