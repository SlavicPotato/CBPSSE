#pragma once

namespace CBP
{
    typedef std::unordered_map<std::string, float> configEntry_t;
    typedef std::unordered_map<std::string, configEntry_t> config_t;

    extern config_t config;

    bool CBPLoadConfig();
}