#include "pch.h"

#include "CBPconfig.h"

namespace CBP
{
	config_t config;

	bool CBPLoadConfig() {
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

						config[sect][key] = atof(tok2);
					}
				}
			}
		} while (!feof(fh));

		fclose(fh);

		return true;
	}
}