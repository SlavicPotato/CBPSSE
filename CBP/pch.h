#ifndef PCH_H
#define PCH_H

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <queue>
#include <algorithm>
#include <regex>

#include <ShlObj.h>

#include <d3d11.h>
#include <dxgi1_5.h>

#include "json/json.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "ext/ICommon.h"
#include "ext/IHook.h"
#include "ext/ITasks.h"

#include "common/IMemPool.h"
#include "common/ICriticalSection.h"
#include "skse64/PluginAPI.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64/PluginAPI.h"
#include "skse64/NiNodes.h"
#include "skse64/NiTypes.h"
#include "skse64/NiObjects.h"
#include "skse64/GameReferences.h"
#include "skse64/GameData.h"
#include "skse64/GameMenus.h"
#include "skse64/PapyrusVM.h"
#include "skse64/PapyrusNativeFunctions.h"
#include "skse64/PapyrusEvents.h"
#include "skse64/gamethreads.h"
#include "skse64/GameRTTI.h"
#include "skse64/NiRenderer.h"

#include "INIReader.h"

#include "plugin.h"
#include "common.h"
#include "logging.h"
#include "skse.h"
#include "game.h"
#include "config.h"
#include "tasks.h"
#include "events.h"
#include "input.h"
#include "gui.h"
#include "CBPConfig.h"
#include "CBPData.h"
#include "CBPProfile.h"
#include "CBPThing.h"
#include "CBPSimObj.h"
#include "CBPUI.h"
#include "CBPPapyrus.h"
#include "CBPSerialization.h"
#include "cbp.h"

#endif //PCH_H
