#include "pch.h"

#include "UIProfiling.h"

#include "CBP/Profiling.h"
#include "CBP/Renderer.h"
#include "CBP/BoneCast.h"

#include "Drivers/cbp.h"
#include "Drivers/gui.h"

namespace CBP
{
    using namespace UICommon;

    UIProfiling::UIProfiling() :
        m_lastUID(0),
        m_plotUpdateTime("Time/frame", ImVec2(0, 30.0f), false, 200),
        m_plotFramerate("Timer", ImVec2(0, 30.0f), false, 200),
        m_lastVMIUpdate(IPerfCounter::Query() - 1000000LL)
    {
    }

    void UIProfiling::Initialize()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        m_plotUpdateTime.SetRes(globalConfig.profiling.plotValues);
        m_plotFramerate.SetRes(globalConfig.profiling.plotValues);

        m_plotUpdateTime.SetHeight(globalConfig.profiling.plotHeight);
        m_plotFramerate.SetHeight(globalConfig.profiling.plotHeight);

        m_plotUpdateTime.SetShowAvg(globalConfig.profiling.showAvg);
        m_plotFramerate.SetShowAvg(globalConfig.profiling.showAvg);
    }

    void UIProfiling::Draw()
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(0.0f, 400.0f, 400.0f, true);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Stats##CBP", GetOpenState()))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (globalConfig.profiling.enableProfiling)
            {
                auto& profiler = DCBP::GetProfiler();
                auto& stats = profiler.Current();

                auto dr = DCBP::GetRenderer();
                bool drEnabled = dr && globalConfig.debugRenderer.enabled;

                ImGui::Columns(2, nullptr, false);

                ImGui::TextWrapped("Time/frame:");
                HelpMarker(MiscHelpText::timePerFrame);
                ImGui::TextWrapped("Step rate:");
                ImGui::TextWrapped("Sim. rate:");
                HelpMarker(MiscHelpText::simRate);
                ImGui::TextWrapped("Timer:");
                HelpMarker(MiscHelpText::frameTimer);
                ImGui::TextWrapped("Actors:");
                ImGui::TextWrapped("UI:");

                if (drEnabled)
                {
                    ImGui::TextWrapped("DR Gen:");
                    ImGui::TextWrapped("DR Draw:");
                }

                ImGui::TextWrapped("BoneCast cache:");
                ImGui::Spacing();

#if defined(SKMP_MEMDBG)
                ImGui::TextWrapped("Mem:");
#endif

                ImGui::NextColumn();

                bool tWarn(stats.avgStepsPerUpdate > 1.0f);

                if (tWarn)
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);

                ImGui::TextWrapped("%lld \xC2\xB5s", stats.avgTime);

                if (tWarn)
                    ImGui::PopStyleColor();

                ImGui::TextWrapped("%.1f/s (%.1f/frame)", stats.avgStepRate, stats.avgStepsPerUpdate);
                ImGui::TextWrapped("%.1f", stats.avgStepsPerUpdate > _EPSILON
                    ? stats.avgStepRate / stats.avgStepsPerUpdate : 0.0);
                ImGui::TextWrapped("%.4f", stats.avgFrameTime);
                ImGui::TextWrapped("%u", stats.avgActorCount);
                ImGui::TextWrapped("%lld \xC2\xB5s", DUI::GetPerf());

                if (drEnabled)
                {
                    ImGui::TextWrapped("%lld \xC2\xB5s", dr->GetGenerateTime());
                    ImGui::TextWrapped("%lld \xC2\xB5s", dr->GetDrawTime());
                }

                ImGui::TextWrapped("%zu kb", IBoneCast::GetCacheSize() / std::size_t(1024));
                ImGui::Spacing();

#if defined(SKMP_MEMDBG)
                ImGui::TextWrapped("%.2f MB", static_cast<double>(mem::g_allocatedSize.load()) / (1024.0 * 1024.0));
#endif

                ImGui::Columns(1);

                if (globalConfig.profiling.enablePlot)
                {
                    ImGui::Spacing();

                    if (globalConfig.profiling.animatePlot)
                    {
                        auto currentUID = profiler.GetUID();
                        if (currentUID != m_lastUID)
                        {
                            m_plotUpdateTime.Update(stats.avgTime);
                            m_plotFramerate.Update(stats.avgFrameTime >= _EPSILON ?
                                1.0f / stats.avgFrameTime : 0.0f);

                            m_lastUID = currentUID;
                        }
                    }

                    ImGui::PushItemWidth(ImGui::GetFontSize() * -6.0f);

                    m_plotUpdateTime.Draw();
                    m_plotFramerate.Draw();

                    ImGui::PopItemWidth();
                }

                if (globalConfig.debugRenderer.enabled)
                {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                    ImGui::TextWrapped("WARNING: Disable debug renderer for accurate measurement");
                    ImGui::PopStyleColor();
                    ImGui::Spacing();
                }

                ImGui::Separator();
            }

            static const std::string chKey("Stats#Settings");

            if (CollapsingHeader(chKey, "Settings"))
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

                if (Checkbox("Enabled", &globalConfig.profiling.enableProfiling))
                    if (globalConfig.profiling.enableProfiling)
                        DCBP::ResetProfiler();

                if (SliderInt("Interval (ms)", &globalConfig.profiling.profilingInterval, 10, 10000))
                    DCBP::SetProfilerInterval(static_cast<long long>(
                        globalConfig.profiling.profilingInterval) * 1000);

                Checkbox("Enable plots", &globalConfig.profiling.enablePlot);

                ImGui::SameLine();
                if (Checkbox("Show avg", &globalConfig.profiling.showAvg))
                {
                    m_plotUpdateTime.SetShowAvg(globalConfig.profiling.showAvg);
                    m_plotFramerate.SetShowAvg(globalConfig.profiling.showAvg);
                }

                ImGui::SameLine();
                Checkbox("Animate", &globalConfig.profiling.animatePlot);

                if (SliderInt("Plot values", &globalConfig.profiling.plotValues, 10, 2000))
                {
                    m_plotUpdateTime.SetRes(globalConfig.profiling.plotValues);
                    m_plotFramerate.SetRes(globalConfig.profiling.plotValues);
                }

                if (SliderFloat("Plot height", &globalConfig.profiling.plotHeight, 10.0f, 200.0f))
                {
                    m_plotUpdateTime.SetHeight(globalConfig.profiling.plotHeight);
                    m_plotFramerate.SetHeight(globalConfig.profiling.plotHeight);
                }

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

}