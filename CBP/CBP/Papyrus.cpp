#include "pch.h"

#include "Papyrus.h"
#include "Config.h"

#include "Drivers/cbp.h"
#include "Drivers/tasks.h"

namespace CBP
{
    static IThreadSafeBasicMemPool<ConfigUpdateTask, 30> s_configUpdateTaskPool;

    static UInt32 PP_GetVersion(StaticFunctionTag*)
    {
        return MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);
    }

    static void PP_UpdateAllActors(StaticFunctionTag*)
    {
        DCBP::UpdateConfigOnAllActors();
    }

    static bool PP_SetGlobalConfig(StaticFunctionTag*, BSFixedString sect, BSFixedString key, bool isFemale, float val)
    {
        auto cmd = ConfigUpdateTask::Create(isFemale ? CBP::ConfigGender::Female : CBP::ConfigGender::Male, sect, key, val);
        if (cmd) {
            DTasks::AddTask(cmd);
            return true;
        }
        return false;
    }

    static bool PP_SetActorConfig(StaticFunctionTag*, Actor* actor, BSFixedString sect, BSFixedString key, bool isFemale, float val)
    {
        if (!actor)
            return false;

        Game::ObjectHandle handle;
        if (!handle.Get(actor))
            return false;

        auto cmd = ConfigUpdateTask::Create(handle, isFemale ? CBP::ConfigGender::Female : CBP::ConfigGender::Male, sect, key, val);
        if (cmd) {
            DTasks::AddTask(cmd);
            return true;
        }

        return false;
    }

    static void PP_ResetAllActors(StaticFunctionTag*)
    {
        DCBP::ResetActors();
    }
    
    static void PP_OpenUI(StaticFunctionTag*, bool a_open)
    {
        DCBP::OpenUI(a_open);
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
            new NativeFunction4<StaticFunctionTag, bool, BSFixedString, BSFixedString, bool, float>("SetGlobalConfig", "CBP", PP_SetGlobalConfig, registry));
        registry->RegisterFunction(
            new NativeFunction5<StaticFunctionTag, bool, Actor*, BSFixedString, BSFixedString, bool, float>("SetActorConfig", "CBP", PP_SetActorConfig, registry));
        registry->RegisterFunction(
            new NativeFunction1<StaticFunctionTag, void, bool>("OpenUI", "CBP", PP_OpenUI, registry));


        registry->SetFunctionFlags("CBP", "OpenUI", VMClassRegistry::kFunctionFlag_NoWait);
        registry->SetFunctionFlags("CBP", "ResetAllActors", VMClassRegistry::kFunctionFlag_NoWait);
        registry->SetFunctionFlags("CBP", "UpdateAllActors", VMClassRegistry::kFunctionFlag_NoWait);

        return true;
    }

    ConfigUpdateTask::ConfigUpdateTask() :
        m_handle(0)
    {
    }

    ConfigUpdateTask* ConfigUpdateTask::Create(
        ConfigGender a_gender,
        const BSFixedString& a_sect,
        const BSFixedString& a_key,
        float a_val)
    {
        std::string sect(a_sect.c_str());
        std::string key(a_key.c_str());

        auto& tgcd = IConfig::GetTemplateBase<configComponents_t>();

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
            cmd->m_gender = a_gender;
        }

        return cmd;
    }

    ConfigUpdateTask* ConfigUpdateTask::Create(
        Game::ObjectHandle a_handle,
        ConfigGender a_gender,
        const BSFixedString& a_sect,
        const BSFixedString& a_key,
        float a_val)
    {
        auto cmd = Create(a_gender, a_sect, a_key, a_val);
        if (cmd) {
            cmd->m_handle = a_handle;
        }

        return cmd;
    }

    void ConfigUpdateTask::Run()
    {
        IScopedLock _(DCBP::GetLock());

        if (m_handle)
        {
            auto& ach = IConfig::GetActorPhysicsHolder();

            auto ith = ach.find(m_handle);
            if (ith == ach.end())
                return;

            auto itt = ith->second(m_gender).find(m_sect);
            if (itt == ith->second(m_gender).end())
                return;

            if (itt->second.Set(m_key, m_val))
                DCBP::DispatchActorTask(
                    m_handle, ControllerInstruction::Action::UpdateConfig);
        }
        else {
            auto& globalConfig = IConfig::GetGlobalPhysics()(m_gender);

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