#ifndef PCH_H
#define PCH_H

#include <set>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <thread>
#include <queue>
#include <algorithm>

#include <ShlObj.h>

#include "ext/ICommon.h"
#include "ext/IHook.h"
#include "common/IMemPool.h"
#include "common/ICriticalSection.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64/PluginAPI.h"
#include "skse64/NiNodes.h"
#include "skse64/NiTypes.h"
#include "skse64/NiObjects.h"
#include "skse64/GameReferences.h"
#include "skse64/PapyrusVM.h"
#include "skse64/PapyrusNativeFunctions.h"
#include "skse64/gamethreads.h"
#include "skse64/GameRTTI.h"

#include "plugin.h"
#include "common.h"
#include "patching.h"
#include "logging.h"
#include "skse.h"
#include "tasks.h"
#include "cbp.h"
#include "cbp_papyrus.h"
#include "cbp_configobs.h"

#endif //PCH_H
