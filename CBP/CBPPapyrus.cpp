#include "pch.h"

namespace CBP
{
    static IThreadSafeBasicMemPool<ConfigUpdateTask, 30> s_configUpdateTaskPool;

    static UInt32 PP_GetVersion(StaticFunctionTag* base)
    {
        return MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);
    }

    static void PP_UpdateAllActors(StaticFunctionTag* base)
    {
        DCBP::UpdateConfigOnAllActors();
    }

    static bool PP_SetGlobalConfig(StaticFunctionTag* base, BSFixedString sect, BSFixedString key, float val)
    {
        auto cmd = ConfigUpdateTask::Create(sect, key, val);
        if (cmd) {
            DTasks::AddTask(cmd);
            return true;
        }
        return false;
    }

    static bool PP_SetActorConfig(StaticFunctionTag* base, Actor* actor, BSFixedString sect, BSFixedString key, float val)
    {
        if (!actor)
            return false;

        SKSE::ObjectHandle handle;
        if (!SKSE::GetHandle(actor, actor->formType, handle))
            return false;

        auto cmd = ConfigUpdateTask::Create(handle, sect, key, val);
        if (cmd) {
            DTasks::AddTask(cmd);
            return true;
        }

        return false;
    }

    static void PP_ResetAllActors(StaticFunctionTag* base)
    {
        DCBP::ResetActors();
    }

    bool RegisterFuncs(VMClassRegistry* registry)
    {
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, UInt32>("GetVersion", "CBP", PP_GetVersion, registry));
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, void>("UpdateAllActors", "CBP", PP_UpdateAllActors, registry));
        registry->RegisterFunction(
            new NativeFunction0<StaticFunctionTag, void>("ResetAllActors", "CBP", PP_ResetAllActors, registry));
        registry->RegisterFunction(
            new NativeFunction3<StaticFunctionTag, bool, BSFixedString, BSFixedString, float>("SetGlobalConfig", "CBP", PP_SetGlobalConfig, registry));
        registry->RegisterFunction(
            new NativeFunction4<StaticFunctionTag, bool, Actor*, BSFixedString, BSFixedString, float>("SetActorConfig", "CBP", PP_SetActorConfig, registry));

        return true;
    }

    ConfigUpdateTask::ConfigUpdateTask() :
        m_handle(0)
    {
    }

    ConfigUpdateTask* ConfigUpdateTask::Create(
        const BSFixedString& a_sect,
        const BSFixedString& a_key,
        float a_val)
    {
        std::string sect(a_sect.c_str());
        std::string key(a_key.c_str());

        transform(sect.begin(), sect.end(), sect.begin(), ::tolower);
        transform(key.begin(), key.end(), key.begin(), ::tolower);

        auto& tgcd = IConfig::GetThingGlobalConfigDefaults();

        auto it = tgcd.find(sect);
        if (it == tgcd.end())
            return nullptr;

        if (!it->second.Contains(key))
            return nullptr;

        auto cmd = s_configUpdateTaskPool.Allocate();
        if (cmd) {
            cmd->m_sect = std::move(sect);
            cmd->m_key = std::move(key);
            cmd->m_val = a_val;
        }

        return cmd;
    }

    ConfigUpdateTask* ConfigUpdateTask::Create(
        SKSE::ObjectHandle a_handle,
        const BSFixedString& a_sect,
        const BSFixedString& a_key,
        float a_val)
    {
        auto cmd = Create(a_sect, a_key, a_val);
        if (cmd)
            cmd->m_handle = a_handle;

        return cmd;
    }

    void ConfigUpdateTask::Run()
    {
        IScopedCriticalSection m(std::addressof(DCBP::GetLock()));

        if (m_handle)
        {
            auto& ach = IConfig::GetActorConfigHolder();

            auto ith = ach.find(m_handle);
            if (ith == ach.end())
                return;

            auto itt = ith->second.find(m_sect);
            if (itt == ith->second.end())
                return;

            if (itt->second.Set(m_key, m_val)) 
                DCBP::DispatchActorTask(
                    m_handle, UTTask::kActionUpdateConfig);            
        }
        else {
            auto& globalConfig = IConfig::GetGlobalPhysicsConfig();

            auto it = globalConfig.find(m_sect);
            if (it == globalConfig.end())
                return;

            if (it->second.Set(m_key, m_val))
                DCBP::UpdateConfigOnAllActors();            
        }

    }

    void ConfigUpdateTask::Dispose()
    {
        s_configUpdateTaskPool.Free(this);
    }

}