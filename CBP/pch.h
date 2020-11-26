#ifndef PCH_H
#define PCH_H

#include <ext/ICommon.h>
#include <ext/IHook.h>
#include <ext/ITasks.h>
#include <ext/JITASM.h>
#include <ext/INIReader.h>
#include <common/IMemPool.h>
#include <common/ICriticalSection.h>
#include <skse64/PluginAPI.h>
#include <skse64_common/skse_version.h>
#include <skse64_common/BranchTrampoline.h>
#include <skse64/PluginAPI.h>
#include <skse64/NiNodes.h>
#include <skse64/NiTypes.h>
#include <skse64/NiObjects.h>
#include <skse64/GameReferences.h>
#include <skse64/GameData.h>
#include <skse64/GameMenus.h>
#include <skse64/GameExtraData.h>
#include <skse64/PapyrusVM.h>
#include <skse64/PapyrusNativeFunctions.h>
#include <skse64/PapyrusEvents.h>
#include <skse64/gamethreads.h>
#include <skse64/GameRTTI.h>
#include <skse64/NiRenderer.h>
#include <skse64/NiExtraData.h>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <algorithm>
#include <regex>
#include <bitset>
#include <functional>
#include <numbers>
#include <queue>

#include <ShlObj.h>

#include <d3d11.h>
#include <dxgi1_5.h>

#include <wrl/client.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <btBulletCollisionCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include <Inc/CommonStates.h>
#include <Inc/SimpleMath.h>
#include <Inc/PrimitiveBatch.h>
#include <Inc/Effects.h>
#include <Inc/VertexTypes.h>
#include <Inc/DirectXHelpers.h>
#include <Src/PlatformHelpers.h>

#include <boost/any.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/archive/binary_oarchive.hpp> 
#include <boost/archive/binary_iarchive.hpp> 

#include <boost/serialization/unordered_map.hpp> 
#include <boost/serialization/string.hpp> 
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

#include <boost/algorithm/string.hpp>

#include <json/json.h>

#include <imgui.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace fs = std::filesystem;

//#define _CBP_ENABLE_DEBUG

#include "plugin.h"
#include "paths.h"
#include "Common/Data.h"
#include "game.h"
#include "skse.h"
#include "config.h"
#include "drivers/tasks.h"
#include "drivers/events.h"
#include "drivers/data.h"
#include "drivers/input.h"
#include "drivers/render.h"
#include "drivers/gui.h"
#include "Common/Serialization.h"
#include "Common/ProfileManager.h"
#include "Common/UIData.h"
#include "Common/UICommon.h"
#include "cbp/Data.h"
#include "cbp/Config.h"
#include "cbp/Serialization.h"
#include "cbp/Profile.h"
#include "cbp/Template.h"
#include "cbp/SimComponent.h"
#include "cbp/SimObject.h"
#include "cbp/Collision.h"
#include "cbp/Armor.h"
#include "cbp/UI.h"
#include "cbp/Papyrus.h"
#include "cbp/Renderer.h"
#include "cbp/Profiling.h"
#include "cbp/Controller.h"
#include "cbp/GameEventHandlers.h"
#include "drivers/cbp.h"

#endif //PCH_H
