#pragma once

namespace SKSE
{   
    [[nodiscard]] bool ResolveHandle(SKSESerializationInterface* intfc, Game::VMHandle a_handle, Game::VMHandle& a_out);
    [[nodiscard]] bool ResolveRaceForm(SKSESerializationInterface* intfc, Game::FormID a_formID, Game::FormID& a_out);
}

#include <ext/ISKSE.h>

class ISKSE :
    public ISKSEBase<
    SKSEInterfaceFlags::kMessaging |
    SKSEInterfaceFlags::kSerialization |
    SKSEInterfaceFlags::kTrampoline |
    SKSEInterfaceFlags::kTask |
    SKSEInterfaceFlags::kPapyrus,
    128,
    256>
{
public:

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
        return m_Instance;
    }

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetBranchTrampoline() {
        return m_Instance.GetTrampoline(TrampolineID::kBranch);
    }

    [[nodiscard]] SKMP_FORCEINLINE static auto& GetLocalTrampoline() {
        return m_Instance.GetTrampoline(TrampolineID::kLocal);
    }

private:
    ISKSE() = default;

    virtual void OnLogOpen() override;
    virtual const char* GetLogPath() const override;
    virtual const char* GetPluginName() const override;
    virtual UInt32 GetPluginVersion() const override;
    virtual bool CheckRuntimeVersion(UInt32 a_version) const override;
    //virtual bool CheckInterfaceVersion(UInt32 a_interfaceID, UInt32 a_interfaceVersion, UInt32 a_compiledInterfaceVersion) const override;

    static ISKSE m_Instance;
};
