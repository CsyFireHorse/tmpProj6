#include "servoFak_to_json.h"
#include "vel_to_json.h"
#include "vel/VelCppWrap.h"
#include "vel/vel_serialization.hpp"

namespace zmc {

void to_json(json& j, const VelocityWrap& p) { to_dict(j, p); }
void from_json(const json& j, VelocityWrap& p) { from_dict(j, p); }

std::string to_jsonstr(const VelocityWrap& p){
    json js;
    to_json(js, p);
    return js.dump(2);
}

void from_jsonstr(const std::string& jstr, VelocityWrap& p){
    json js = json::parse(jstr);
    from_json(js, p);
}

}  // namespace ns