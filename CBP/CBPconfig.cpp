#include "pch.h"

#include "CBPconfig.h"

namespace CBP
{
    config_t config;

    static vKey_t keysPhysics = {
        "stiffness",
        "stiffness2",
        "damping",
        "maxoffset",
        "timetick",
        "linearx",
        "lineary",
        "linearz",
        "rotational",
        "timescale",
        "gravitybias",
        "gravitycorrection",
        "cogoffset"
    };

    static vKey_t keysTuning = {
        "reloadonchange"
    };

    static vSection_t validSections = {
        {"breast", keysPhysics},
        {"belly", keysPhysics},
        {"butt", keysPhysics},
        {"tuning", keysTuning}
    };

    bool IsConfigOption(std::string& sect, std::string& key)
    {
        auto it1 = validSections.find(sect);
        if (it1 != validSections.end()) {
            auto it2 = it1->second.find(key);
            if (it2 != it1->second.end()) {
                return true;
            }
        }

        return false;
    }

    bool LoadConfig()
    {
        char buffer[1024];
        FILE* fh;

        if (fopen_s(&fh, PLUGIN_CBP_CONFIG, "r") != 0) {
            _ERROR("Failed to open config");
            return false;
        }

        config.clear();

        do {
            auto str = fgets(buffer, 1023, fh);
            //logger.error("str %s\n", str);
            if (str && strlen(str) > 1) {
                if (str[0] != '#') {
                    char* next_tok = nullptr;

                    char* tok0 = strtok_s(str, ".", &next_tok);
                    char* tok1 = strtok_s(NULL, " ", &next_tok);
                    char* tok2 = strtok_s(NULL, " ", &next_tok);

                    if (tok0 && tok1 && tok2) {
                        std::string sect(tok0);
                        std::string key(tok1);

                        transform(sect.begin(), sect.end(), sect.begin(), ::tolower);
                        transform(key.begin(), key.end(), key.begin(), ::tolower);

                        if (!IsConfigOption(sect, key)) {
                            continue;
                        }

                        config[sect][key] = atof(tok2);
                    }
                }
            }
        } while (!feof(fh));

        fclose(fh);

        return true;
    }

    bool SaveConfig()
    {
        if (!config.size()) {
            return false;
        }

        std::ofstream fs(PLUGIN_CBP_CONFIG, std::ios::trunc);
        if (!fs.is_open()) {
            return false;
        }
        
        typedef std::vector<std::pair<std::string, float>> configEntryS_t;
        typedef std::pair<std::string, configEntryS_t> configEntryPairS_t;
        typedef std::vector<configEntryPairS_t> configS_t;

        configS_t tmp;

        for (const auto& it1 : config) {
            auto ce = configEntryS_t();

            for (const auto& it2 : it1.second) {
                ce.push_back(it2);
            }

            std::sort(ce.begin(), ce.end(), [=](auto& a, auto& b) {
                return a.first < b.first; });

            tmp.push_back(configEntryPairS_t(it1.first, ce));
        }

        std::sort(tmp.begin(), tmp.end(), [=](auto& a, auto& b) {
            return a.first < b.first; });

        for (const auto& it1 : tmp) {
            for (const auto& it2 : it1.second) {
                fs << it1.first << "." << it2.first << " " << it2.second << std::endl;
            }
            fs << std::endl;
        }

        return true;
    }
}