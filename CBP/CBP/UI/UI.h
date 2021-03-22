#pragma once

#include "CBP/Config.h"
#include "CBP/Data.h"
#include "CBP/Profile.h"
#include "CBP/ArmorCache.h"

#include "Common/Base.h"
#include "Common/ActorList.h"
#include "Common/Profile.h"
#include "Common/Force.h"

#include "Common/UIData.h"
#include "Common/UICommon.h"

#ifdef _CBP_ENABLE_DEBUG
#include "UIDebugInfo.h"
#endif

namespace CBP
{
    class UIProfileEditorPhysics;
    class UIProfileEditorNode;
    class UIRaceEditorPhysics;
    class UIRaceEditorNode;
    class UIActorEditorNode;
    class UIOptions;
    class UICollisionGroups;
    class UIProfiling;
    class UIDialogImportExport;
    class UILog;
    class UINodeMap;
    class UIArmorOverrideEditor;
    class UICollisionGeometryManager;
    class UISimComponentActor;
    class UISimComponentGlobal;
#ifdef _CBP_ENABLE_DEBUG
    class UIDebugInfo;
#endif

    class UIContext :
        public UICommon::UIWindowBase,
        public UIActorList<actorListPhysConf_t, true>,
        public UIProfileSelector<actorListPhysConf_t::value_type, PhysicsProfile>,
        public UIMainItemFilter<UIEditorID::kMainEditor>,
        UIApplyForce<actorListPhysConf_t::value_type>,
        ILog
    {
    public:

        UIContext();
        virtual ~UIContext() noexcept = default;

        void Initialize();

        void Reset(std::uint32_t a_loadInstance);
        void OnOpen();
        void OnClose();
        void Draw();
        void QueueListUpdateAll();

        [[nodiscard]] SKMP_FORCEINLINE std::uint32_t GetLoadInstance() const noexcept {
            return m_activeLoadInstance;
        }

        [[nodiscard]] SKMP_FORCEINLINE UIMainItemFilter<UIEditorID::kMainEditor>& GetFilter() noexcept {
            return static_cast<UIMainItemFilter<UIEditorID::kMainEditor>&>(*this);
        }

        void LogNotify();

        SKMP_FORCEINLINE auto& GetPopupQueue() {
            return m_popup;
        }

        SKMP_FORCEINLINE void ClearPopupQueue() {
            return m_popup.clear();
        }

        void UpdateStyle();

    private:

        void DrawMenuBar(bool* a_active, const listValue_t* a_entry);

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& m_peComponents) override;

        virtual void ApplyForce(
            listValue_t* a_data,
            std::uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force) const override;

        virtual void ListResetAllValues(Game::ObjectHandle a_handle) override;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) override;

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const override;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const override;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const override;

        [[nodiscard]] virtual std::string GetGCSID(const std::string& a_name) const override;

        virtual void OnListChangeCurrentItem(const SelectedItem<Game::ObjectHandle>& a_oldHandle, Game::ObjectHandle a_newHandle) override;

        std::uint32_t m_activeLoadInstance;

        struct {
            struct {
                bool openIEDialog;
            } menu;

            except::descriptor lastException;
        } m_state;

        UICommon::UIPopupQueue m_popup;

        std::unique_ptr<UIProfileEditorPhysics> m_pePhysics;
        std::unique_ptr<UIProfileEditorNode> m_peNodes;
        std::unique_ptr<UIRaceEditorPhysics> m_racePhysicsEditor;
        std::unique_ptr<UIRaceEditorNode> m_raceNodeEditor;
        std::unique_ptr<UIActorEditorNode> m_actorNodeEditor;
        std::unique_ptr<UIOptions> m_options;
        std::unique_ptr<UICollisionGroups> m_colGroups;
        std::unique_ptr<UIProfiling> m_profiling;
        std::unique_ptr<UIDialogImportExport> m_ieDialog;
        std::unique_ptr<UILog> m_log;
        std::unique_ptr<UINodeMap> m_nodeMap;
        std::unique_ptr<UIArmorOverrideEditor> m_armorOverride;
        std::unique_ptr<UICollisionGeometryManager> m_geometryManager;

#ifdef _CBP_ENABLE_DEBUG
        std::unique_ptr<UIDebugInfo> m_debug;
#endif

        std::unique_ptr<UISimComponentActor> m_scActor;
        std::unique_ptr<UISimComponentGlobal> m_scGlobal;
    };

}