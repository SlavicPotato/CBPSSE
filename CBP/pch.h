#ifndef PCH_H
#define PCH_H

#include <unordered_map>
#include <sstream>
#include <thread>
#include <algorithm>

#include <ShlObj.h>

#include "common/ICriticalSection.h"
#include "common/IMemPool.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/BranchTrampoline.h"

#include "skse64/PluginAPI.h"

#include "ext/ICommon.h"
#include "skse64-m/NiNodes.h"
#include "skse64-m/NiTypes.h"
#include "skse64-m/NiObjects.h"
#include "skse64-m/GameReferences.h"
#include "skse64-m/PapyrusVM.h"
#include "skse64-m/PapyrusNativeFunctions.h"
#include "skse64-m/gamethreads.h"
#include "skse64-m/GameRTTI.h"

#include "plugin.h"
#include "common.h"
#include "patching.h"
#include "hook.h"
#include "logging.h"
#include "skse.h"
#include "cbp.h"

#endif //PCH_H
