#ifndef __VEL_TO_JSON_H_
#define __VEL_TO_JSON_H_
#include "common/thirdlib/nlohmann/json.hpp"
#include "vel/include/VelUnCppWrap.h"

using json = nlohmann::json;

namespace zmc {
void to_json(json& j, const VelocityWrap& p);
void from_json(const json& j, VelocityWrap& p);

std::string to_jsonstr(const VelocityWrap& p);
void from_jsonstr(const std::string& j, VelocityWrap& p);

}  // namespace zmc

#endif