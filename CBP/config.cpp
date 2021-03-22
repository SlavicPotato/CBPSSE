#include "pch.h"

#include "config.h"

INIReader IConfigINI::m_confReader;
INIReader IConfigINI::m_confReaderCustom;

int IConfigINI::Load()
{
	m_confReader.Load(PLUGIN_INI_FILE);
	m_confReaderCustom.Load(PLUGIN_INI_CUSTOM_FILE);
	return m_confReader.ParseError();
}

void IConfigINI::Clear()
{
	m_confReader.Clear();
	m_confReaderCustom.Clear();
}