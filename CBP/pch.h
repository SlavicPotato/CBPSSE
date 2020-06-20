#ifndef PCH_H
#define PCH_H

#include <unordered_map>
#include <sstream>
#include <thread>

#include <ShlObj.h>

#include "common/ICriticalSection.h"
#include "common/IMemPool.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64/PluginAPI.h"
#include "skse64/NiNodes.h"
#include "skse64/NiTypes.h"
#include "skse64/NiObjects.h"
#include "skse64/PapyrusVM.h"
#include "skse64/GameReferences.h"

#include "skse64/gamethreads.h"

#include "plugin.h"

#include "common.h"
#include "patching.h"
#include "hook.h"
#include "tlib.h"
#include "logging.h"
#include "addrlib.h"
#include "rtti.h"
#include "skse.h"
#include "cbp.h"

#endif //PCH_H
