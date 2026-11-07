#include "AxisManager.h"
#include <algorithm>
#include <cassert>
#include "fbt_axis_opaque.h"

namespace zmc {


////////////////// SgAxisArr //////////////
struct SgAxisArrData {
    std::vector<AXIS_REF> axArr;
    std::vector<std::string> nameArr;
    void clean(){
        axArr.clear();
        nameArr.clear();
    }

    bool addAxis(AXIS_REF axis, const std::string& name){
        if (axis == nullptr)
            return false;
        if (!((const AxisOpaque*)axis)->is_valid())
            return false;
        if (axArr.size() >= MAX_AXIS_COUNT || axis == nullptr || name.size() == 0){
            return false;
        }
        if (std::find(nameArr.begin(), nameArr.end(), name) != nameArr.end()){
            return false;
        }
        axArr.push_back(axis);
        nameArr.push_back(name);
        return true;
    }
};

static SgAxisArrData& getSgAxisArrData(){
    static SgAxisArrData data;
    return data;
}

std::vector<AXIS_REF>& SgAxisArr::getInstance() {
    return getSgAxisArrData().axArr;
}

bool SgAxisArr::addAxis(void* value, const char* name) {
    std::string nameStr(name);
    if (!name){
        nameStr = std::to_string(getSgAxisArrData().axArr.size() + 1);
    }
    return getSgAxisArrData().addAxis((AXIS_REF) value, nameStr);
}

void* SgAxisArr::getAxisByIndex(uint32_t index){
    if (index >= getInstance().size())
        return nullptr;
    return getInstance()[index];
}

int SgAxisArr::getIndexByAxis(void* axis) {
    if (!axis) {
        return -1;
    }
    auto& arr = getInstance();
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == static_cast<AXIS_REF>(axis)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void* SgAxisArr::getAxisByName(const char* name){
    auto& data = getSgAxisArrData();
    for (int i = 0; i < data.nameArr.size(); i++){
        if (data.nameArr[i] == name){
            return data.axArr[i];
        }
    }
    return nullptr;
}
std::string SgAxisArr::getAxisName(uint32_t index){
    auto& data = getSgAxisArrData();
    if (index >= data.nameArr.size()){
        return "";
    }
    return data.nameArr[index];
}
std::vector<std::string> SgAxisArr::getAllAxisNames(){
    return getSgAxisArrData().nameArr;
}

void* SgAxisArr::getAxisDefault(){
    return nullptr;
}
uint32_t SgAxisArr::getSize() {
    return getInstance().size();
}

// replace axis name by index
bool SgAxisArr::replaceAxisName(uint32_t index, const char* newName) {
    auto& data = getSgAxisArrData();
    if (!newName || index >= data.nameArr.size() || std::string(newName).size() == 0) {
        return false;
    }
    // check name is unique
    for (uint32_t i = 0; i < data.nameArr.size(); ++i) {
        if (i != index && data.nameArr[i] == newName) {
            return false;
        }
    }
    data.nameArr[index] = newName;
    return true;
}

void SgAxisArr::clear(){
    getSgAxisArrData().clean();
}

} // namespace zmc
