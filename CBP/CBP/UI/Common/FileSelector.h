#pragma once

#include "Base.h"

#include "../../Serialization.h"

namespace CBP
{
    class UIFileSelector :
        virtual protected UIBase
    {
        using storage_type = stl::imap<std::string, fs::path>;

        class SelectedFile
        {
        public:
            SelectedFile();
            SelectedFile(const fs::path& a_root, const storage_type::value_type& a_filename);

            void UpdateInfo();

            fs::path m_fullpath;
            fs::path m_filename;
            std::string m_key;
            importInfo_t m_info;
            bool m_infoResult;
        };

    public:
        bool UpdateFileList(bool a_select = true);

    protected:
        UIFileSelector();
        UIFileSelector(const fs::path& a_root);

        void DrawFileSelector();
        bool DeleteSelected();
        bool DeleteItem(const SelectedFile& a_item);
        bool RenameItem(const SelectedFile& a_item, const fs::path& a_newFileName);
        void SelectItem(const std::string& a_itemDesc);

        SKMP_FORCEINLINE const auto& GetSelected() const {
            return m_selected;
        }

        SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastExcept;
        }

        SKMP_FORCEINLINE bool HasFile(const std::string& a_itemDesc) const {
            return m_files.contains(a_itemDesc);
        }

    private:
        bool Delete(const fs::path& a_file);

        SelectedItem<SelectedFile> m_selected;
        storage_type m_files;
        fs::path m_root;

        except::descriptor m_lastExcept;
    };

}