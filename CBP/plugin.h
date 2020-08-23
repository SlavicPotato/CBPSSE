#pragma once

#define PLUGIN_NAME                 "CBP"
#define PLUGIN_AUTHOR               "polygonhell, SlavicPotato"

#include "version.h"

#define PLUGIN_BASE_PATH            "Data\\SKSE\\Plugins\\"
#define CBP_DATA_BASE_PATH          "Data\\SKSE\\Plugins\\CBP\\"

constexpr const char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH "CBP.ini";
constexpr const char* PLUGIN_LOG_PATH = "\\My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
constexpr const char* PLUGIN_CBP_CONFIG = PLUGIN_BASE_PATH "CBPConfig.txt";
constexpr const char* PLUGIN_CBP_PROFILE_PATH = CBP_DATA_BASE_PATH "Profiles\\Physics";
constexpr const char* PLUGIN_CBP_PROFILE_NODE_PATH = CBP_DATA_BASE_PATH "Profiles\\Node";
constexpr const char* PLUGIN_CBP_GLOBAL_DATA = CBP_DATA_BASE_PATH "Settings\\Globals.json";
constexpr const char* PLUGIN_CBP_CG_DATA = CBP_DATA_BASE_PATH "Settings\\CollisionGroups.json";
constexpr const char* PLUGIN_CBP_NODE_DATA = CBP_DATA_BASE_PATH "Nodes.json";
constexpr const char* PLUGIN_CBP_GLOBPROFILE_DEFAULT_DATA = CBP_DATA_BASE_PATH "Default.json";
constexpr const char* PLUGIN_CBP_EXPORTS_PATH = CBP_DATA_BASE_PATH "Exports";
constexpr const char* PLUGIN_IMGUI_INI_FILE = CBP_DATA_BASE_PATH "Settings\\ImGui.ini";

#define MIN_SKSE_VERSION            RUNTIME_VERSION_1_5_23