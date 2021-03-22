#pragma once

#include "Base.h"

#include "../../Serialization.h"

namespace CBP
{
    class UIFileSelector :
        virtual protected UIBase
    {
        class SelectedFile
        {
        public:
            SelectedFile();
            SelectedFile(const fs::path& a_path);

            void UpdateInfo();

            fs::path m_path;
            std::string m_filenameStr;
            importInfo_t m_info;
            bool m_infoResult;
        };
    public:
        bool UpdateFileList();
    protected:
        UIFileSelector();

        void DrawFileSelector();
        bool DeleteExport(const fs::path& a_file);

        SKMP_FORCEINLINE const auto& GetSelected() const {
            return m_selected;
        }

        SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastExcept;
        }

    private:
        SelectedItem<SelectedFile> m_selected;
        stl::vector<fs::path> m_files;

        except::descriptor m_lastExcept;
    };

}