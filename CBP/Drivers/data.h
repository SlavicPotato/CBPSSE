#pragma once

#include "Data/PluginInfo.h"
#include "Events/Events.h"

namespace CBP
{
    class DData :
        public ILog
    {
    public:

        static void Initialize();

        static void MessageHandler(Event, void* args);

        [[nodiscard]] SKMP_FORCEINLINE static bool HasPluginList() {
            return m_Instance.m_pluginData.IsPopulated();
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetPluginMap() {
            return m_Instance.m_pluginData.GetIndexMap();
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetPluginData() {
            return m_Instance.m_pluginData;
        }

        FN_NAMEPROC("Data")

    private:
        IPluginInfo m_pluginData;

        static DData m_Instance;
    };
}