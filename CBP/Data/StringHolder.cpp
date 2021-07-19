#include "pch.h"

#include "StringHolder.h"

namespace Common
{
    StringHolder StringHolder::m_Instance;

    StringHolder::StringHolder()
        :
        reset("Reset"),
        rescan("Rescan"),
        save("Save"),
        nodetree("NodeTree"),
        configgroups("ConfigGroups"),
        filter("Filter"),
        sample("Sample"),
        snew("New"),
        apply("Apply"),
        edit("Edit"),
        reload("Reload"),
        del("Delete"),
        plus("+")
    {
    }
}