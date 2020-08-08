#pragma once

#define PLUGIN_NAME					"CBP"
#define PLUGIN_AUTHOR				"polygonhell, SlavicPotato"

#include "version.h"

#define PLUGIN_BASE_PATH            "Data\\SKSE\\Plugins\\"

constexpr const char* PLUGIN_INI_FILE = PLUGIN_BASE_PATH "CBP.ini";
constexpr const char* PLUGIN_LOG_PATH = "\\My Games\\Skyrim Special Edition\\SKSE\\" PLUGIN_NAME ".log";
constexpr const char* PLUGIN_CBP_CONFIG = PLUGIN_BASE_PATH "CBPConfig.txt";
constexpr const char* PLUGIN_CBP_PROFILE_PATH = PLUGIN_BASE_PATH "CBPProfiles";
constexpr const char* PLUGIN_CBP_DATA_PATH = PLUGIN_BASE_PATH "CBPData";
constexpr const char* PLUGIN_CBP_ACTOR_DATA = PLUGIN_BASE_PATH "CBPData\\Actors.json";
constexpr const char* PLUGIN_CBP_RACE_DATA = PLUGIN_BASE_PATH "CBPData\\Races.json";
constexpr const char* PLUGIN_CBP_GLOBAL_DATA = PLUGIN_BASE_PATH "CBPData\\Globals.json";
constexpr const char* PLUGIN_CBP_CG_DATA = PLUGIN_BASE_PATH "CBPData\\CollisionGroups.json";
constexpr const char* PLUGIN_CBP_NODE_DATA = PLUGIN_BASE_PATH "CBPData\\NodeSettings.json";
constexpr const char* PLUGIN_IMGUI_INI_FILE = PLUGIN_BASE_PATH PLUGIN_NAME "_ImGui.ini";

#define MIN_SKSE_VERSION            RUNTIME_VERSION_1_5_23