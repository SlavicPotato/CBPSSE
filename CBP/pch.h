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

#include <wrl/client.h>

#include <reactphysics3d/reactphysics3d.h>

#include <Inc/CommonStates.h>
#include <Inc/SimpleMath.h>
#include <Inc/PrimitiveBatch.h>
#include <Inc/Effects.h>
#include <Inc/VertexTypes.h>
#include <Inc/DirectXHelpers.h>
#include <Src/PlatformHelpers.h>

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

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

namespace r3d = reactphysics3d;

#include "plugin.h"
#include "common.h"
#include "logging.h"
#include "skse.h"
#include "game.h"
#include "config.h"
#include "tasks.h"
#include "events.h"
#include "input.h"
#include "render.h"
#include "gui.h"
#include "CBPConfig.h"
#include "CBPData.h"
#include "CBPSerialization.h"
#include "CBPProfile.h"
#include "CBPThing.h"
#include "CBPSimObj.h"
#include "CBPCollision.h"
#include "CBPUI.h"
#include "CBPPapyrus.h"
#include "CBPRenderer.h"
#include "CBPProfiling.h"
#include "cbp.h"

#endif //PCH_H
