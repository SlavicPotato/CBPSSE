#pragma once

#define PLUGIN_NAME                 "CBP"
#define PLUGIN_AUTHOR               "polygonhell, SlavicPotato"

#include "version.h"

#define PLUGIN_BASE_PATH            "Data\\SKSE\\Plugins\\"
#define CBP_DATA_BASE_PATH          "Data\\SKSE\\Plugins\\CBP"

constexpr const char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH "CBP.ini";
constexpr const char* PLUGIN_LOG_PATH = "My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
constexpr const char* PLUGIN_CBP_CONFIG = PLUGIN_BASE_PATH "CBPConfig.txt";

constexpr const char* PLUGIN_CBP_PROFILE_PATH_R = "Profiles\\Physics";
constexpr const char* PLUGIN_CBP_PROFILE_NODE_PATH_R = "Profiles\\Node";
constexpr const char* PLUGIN_CBP_GLOBAL_DATA_R = "Settings\\Globals.json";
constexpr const char* PLUGIN_CBP_CG_DATA_R = "Settings\\CollisionGroups.json";
constexpr const char* PLUGIN_CBP_NODE_DATA_R = "Nodes.json";
constexpr const char* PLUGIN_CBP_GLOBPROFILE_DEFAULT_DATA_R = "Default.json";
constexpr const char* PLUGIN_CBP_EXPORTS_PATH_R = "Exports";
constexpr const char* PLUGIN_CBP_TEMP_PROF_PHYS_R = "Templates\\Profiles\\Physics";
constexpr const char* PLUGIN_CBP_TEMP_PROF_NODE_R = "Templates\\Profiles\\Node";
constexpr const char* PLUGIN_CBP_TEMP_PLUG_R = "Templates\\Plugins";
constexpr const char* PLUGIN_CBP_COLLIDER_DATA_R = "ColliderData";
constexpr const char* PLUGIN_IMGUI_INI_FILE_R = "Settings\\ImGui.ini";

#define MIN_SKSE_VERSION            RUNTIME_VERSION_1_5_39