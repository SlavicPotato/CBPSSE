#pragma once

#include "Common/Base.h"
#include "Common/FileSelector.h"

namespace CBP
{
    class UIContext;

    class UIDialogImportExport :
        public UICommon::UIWindowBase,
        public UIFileSelector
    {
    public:
        UIDialogImportExport(UIContext& a_parent);

        void Draw();
        void OnOpen();

    private:

        void DrawExportContextMenu();

        void DoImport(const fs::path& a_path);
        void DoExport(const fs::path& a_path);

        ISerialization::ImportFlags GetFlags() const;

        std::regex m_rFileCheck;
        UIContext& m_parent;
    };

}