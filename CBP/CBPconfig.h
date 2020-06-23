#pragma once

namespace CBP
{
    typedef std::unordered_map<std::string, float> configEntry_t;
    typedef std::unordered_map<std::string, configEntry_t> config_t;

    typedef std::set<std::string> vKey_t;
    typedef std::unordered_map<std::string, vKey_t> vSection_t;

    extern config_t config;

    bool LoadConfig();
    bool SaveConfig();
    bool IsConfigOption(std::string& sect, std::string& key);
}