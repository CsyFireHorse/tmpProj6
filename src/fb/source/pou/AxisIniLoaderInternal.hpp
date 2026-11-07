#ifndef AXIS_INI_LOADER_INTERNAL_HPP
#define AXIS_INI_LOADER_INTERNAL_HPP

#include <string>
#include "AxisIniLoader.h"
#include "common/thirdlib/inih/INIReader.h"

// Check whether a parsed reader contains the requested section.
bool axis_ini_has_section(const INIReader& reader, const std::string& section);

// Load one axis section from an already parsed reader.
int axis_ini_load_axis_from_reader(const INIReader& reader, const char* source, AxisCiaConfig& cfg,
                                   const std::string& section);

// Load [base] from an already parsed reader.
int axis_ini_load_base_from_reader(const INIReader& reader, const char* source, bool is_content,
                                   AxisCommonConfig* cfg);

#endif