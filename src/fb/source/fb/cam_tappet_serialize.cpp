
#include "stdint.h"
#include "mc_cam_table.h"
#include "cam_tappet_serialize.h"
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void zmc_tappet_init(Tappet* tappet){
    tappet->ctt = 0;
    tappet->cta = 0;
    tappet->dwDelay = 0;
    tappet->dwDuration = 0;
    tappet->x = 0;
    tappet->iGroupID = 0;
    tappet->dwActive = 0xffffffff;
}

int _cam_tappet_parse(CamTappet* pCt, const char* filename) {
    std::ifstream is(filename);
    if (!is.is_open()) {
       return -1; 
    }
    std::string line;
    line.reserve(256); 

    int key_flag = 0;
    const char* ptr = nullptr;
    const char* end = nullptr;
    auto& fb = *pCt;
    while (std::getline(is, line)) {
        // trim
        ptr = line.c_str();
        while (*ptr && isspace(*ptr)) ptr++;  // skip leading spaces
        end = ptr + strlen(ptr);
        while (end > ptr && isspace(*(end - 1))) end--;  // skip trailing spaces
        if (ptr == end) continue;

        // section judge
        if (*ptr == '[') {
            if (strncmp(ptr, "[Cam]", 5) == 0) {
                key_flag = 1;
                continue;
            }
            if (strncmp(ptr, "[Tappet]", 8) == 0) {
                key_flag = 2;
                continue;
            }
            if (strncmp(ptr, "[Base]", 6) == 0) {
                key_flag = 3;
                continue;
            }
            key_flag = 0;
            continue;
        }

        if (key_flag == 1) {
            CamXyva f;
            char* next = nullptr;
            f.dX = strtod(ptr, &next);
            f.dY = strtod(next + 1, &next);
            f.dV = strtod(next + 1, &next);
            f.dA = strtod(next + 1, nullptr);
            fb.xyva.push_back(f);
        } else if (key_flag == 2) {
            Tappet b;
            char* next = nullptr;
            b.iGroupID = strtoul(ptr, &next, 10);
            b.ctt = strtoul(next + 1, &next, 10);
            b.cta = strtoul(next + 1, &next, 10);
            b.x = strtod(next + 1, &next);
            b.dwDelay = strtoul(next + 1, &next, 10);
            b.dwDuration = strtoul(next + 1, nullptr, 10);
            fb.tappets.push_back(b);
        } else if (key_flag == 3) {
            char* next = nullptr;
            fb.ver_major = strtoul(ptr, &next, 10);
            fb.ver_minor = strtoul(next + 1, &next, 10);
            fb.ver_patch = strtoul(next + 1, &next, 10);
            int xyvaLen, tappetLen;
            xyvaLen = strtoul(next + 1, &next, 10);
            fb.xMin = strtod(next + 1, &next);
            fb.xMax = strtod(next + 1, &next);
            tappetLen = strtoul(next + 1, &next, 10);
            // fb. = strtoul(next + 1, &next, 10);
            // b.dwDelay = strtoul(next + 1, &next, 10);
        }
    }
    return 0;
}


int _cam_tappet_serialize(const CamTappet* pCt, const char* filename) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        return -1;
    }

    // std::string buf;
    // buf.reserve(fb.cam.size() * 128);

    // ofs << std::fixed << std::setprecision(3);

    const auto& fb = *pCt;
    ofs << "[Base]\n";
    {
        char line[128];
        int len = snprintf(line, sizeof(line), "%d,%d,%d,%d,%f,%f,%d\n", fb.ver_major, fb.ver_minor, fb.ver_patch, fb.xyva.size(), fb.xMin, fb.xMax, fb.tappets.size());
        ofs.write(line, len);
    }
    ofs << "[Cam]\n";
    for (const auto& f : fb.xyva) {
        char line[128];
        int len = snprintf(line, sizeof(line), "%.15g,%.15g,%.15g,%.15g\n", f.dX, f.dY, f.dV, f.dA);
        ofs.write(line, len);
    }
    if (fb.tappets.size())
        ofs << "[Tappet]\n";
    for (const auto& b : fb.tappets) {
        char line[128];
        int len = snprintf(line, sizeof(line), "%hu,%u,%u,%.15g,%d,%d\n", b.iGroupID, b.ctt, b.cta, b.x, b.dwDelay, b.dwDuration);
        ofs.write(line, len);
    }
    return 0;
}

// struct CamFile {
//     int ver_major;
//     int ver_minor;
//     int ver_patch;
//     int arrLen;
//     double xMin;
//     double xMax;
//     int tappetLen;
//     std::vector<CamXyva> xyva;
//     std::vector<Tappet> tappets;
// };
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
int _parseCamFile(const std::string& filename, CamTappet& cam) {

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "cant open file: " << filename << std::endl;
        cam = {};  
        return -1; 
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());


    std::string processed;
    // for (uint8_t c : content) {
    //     if (c == 0 ||  c > 0x81) {  
    //         if (!processed.empty() && processed.back() != ' ') processed.push_back(' ');
    //     } else {
    //         processed.push_back(c);
    //     }
    // }
    size_t contentLength = content.size();
    for (size_t i = 0; i < contentLength; i += 20) {
        size_t blockSize = MIN(20, contentLength - i);
        std::string block(content.substr(i, blockSize));
        

        size_t nullPos = block.find('\0');
        if (nullPos != std::string::npos) {
            block = block.substr(0, nullPos); 
        }
        
 
        // size_t lastNonNull = block.find_last_not_of('\0');
        // if (lastNonNull != std::string::npos) {
        //     block = block.substr(0, lastNonNull + 1);
        // } else {
        //     block.clear(); 
        // }


        if (!processed.empty() && !block.empty()) {
            processed.push_back(' ');
        }
        processed.append(block);
    }


    // parse data item
    std::istringstream iss(processed);
    CamTappet& cf = cam;
    std::string token;

    auto readNumberInt = [&](int& val) {
        iss >> token;
        if (token.find("ULINT#") == 0)  // remove ULINT# prefix
            val = std::stoi(token.substr(6));
        else
            val = static_cast<int>(std::stod(token));
    };
    auto readNumberDouble = [&](double& val) {
        iss >> token;
        if (token.find("ULINT#") == 0)
            val = static_cast<double>(std::stoi(token.substr(6)));
        else
            val = std::stod(token);
    };

    readNumberInt(cf.ver_major);
    readNumberInt(cf.ver_minor);
    readNumberInt(cf.ver_patch);
    int arrLen, tappetLen;
    readNumberInt(arrLen);
    readNumberDouble(cf.xMin);
    readNumberDouble(cf.xMax);
    readNumberInt(tappetLen);

    cf.xyva.reserve(arrLen);
    cf.tappets.reserve(tappetLen);
    // parse XYVA array
    bool isCombined = false;  // check if current number string combine prev number string
    for (int i = 0; i < arrLen; ++i) {
        DoubleArr4 xy;
        // printf("%d\n", i);
        for (int j = 0; j < 4; ++j) {
            if (!isCombined)
                iss >> token;
            else {
                isCombined = false;
            }
            if (token.size() > 20) {
                auto token2 = token.substr(0, 20);
                printf("token: %s, token2: %s\n", token.c_str(), token2.c_str());
                xy.arr[j] = std::stod(token2);
                isCombined = true;
                token = token.substr(20);
                continue;
            }
            xy.arr[j] = std::stod(token);
        }
        cf.xyva.push_back((CamXyva&)xy);
    }
    // parse tappets array

    // CAMTAPPETTYPE       ?                   ?                iGroupID            x       action 
    // DINT#1              ULINT#0             ULINT#0             DINT#1              8.91  DINT#0
    // DINT#2              ULINT#0             ULINT#0             DINT#1              1.234 DINT#1
    // DINT#0              ULINT#0             ULINT#0             DINT#1              1.234 DINT#0
    // DINT#1              ULINT#0             ULINT#0             DINT#1              2.345 DINT#1
    for (int i = 0; i < tappetLen; ++i) {
        Tappet t;
        std::string temp;

        // parse 1: CAMTAPPETTYPE (ctt)
        iss >> temp;
        if (temp.find("DINT#") == 0) t.ctt = static_cast<uint8_t>(std::stoi(temp.substr(5)));

        // parse 2: ?
        iss >> temp;
        // parse 3: ?
        iss >> temp;

        // parse 4: iGroupID
        iss >> temp;
        if (temp.find("DINT#") == 0) t.iGroupID = static_cast<uint16_t>(std::stoi(temp.substr(5)));

        // parse 5: x position
        iss >> token;
        t.x = std::stod(token);

        // parse 6: action (cta)
        iss >> temp;
        if (temp.find("DINT#") == 0) t.cta = static_cast<uint8_t>(std::stoi(temp.substr(5)));

        cf.tappets.push_back(t);
    }
    return 0;
}

int CamTappet::parse(const char* filename){
    return _cam_tappet_parse(this, filename);
}

int CamTappet::serialize(const char* filename)const{
    return _cam_tappet_serialize(this, filename);
}

int CamTappet::parseCamFile(const char* filename){
    return _parseCamFile(filename, *this);
}



MC_CAM_REF makeCamRef(const CamXyva* pXyva, int len){
    MC_CAM_REF pCam = new MC_CAM_TABLE();
    pCam->wCamStructID = 0xDC34;
    pCam->byType = mcCamTypeXYVA;
    pCam->nElements = len;
    pCam->pce = (uint8_t*) pXyva;
    pCam->pt = nullptr;
    pCam->nTappets = 0;

    pCam->xStart = pXyva[0].dX;  
    pCam->xEnd = pXyva[len-1].dX;  
    return pCam;
}

inline void _cam_tappet_to_cam_ref(const CamTappet* pCt, MC_CAM_REF cam){
    const auto& ct = *pCt;
    cam->pce = (ZMC_BYTE*) ct.xyva.data();
    cam->nElements = ct.xyva.size();
    
    cam->pt = (SMC_CAMTappet*) ct.tappets.data();
    cam->nTappets = ct.tappets.size();

    cam->xStart = ct.xMin;
    cam->xEnd = ct.xMax;
    cam->wCamStructID = 0xDC34;
    cam->byType = mcCamTypeXYVA;
    // cam->strCAMName = (char*) filename;
}

void CamTappet::toCamRef(MC_CAM_REF cam)const{
    _cam_tappet_to_cam_ref(this, cam);
}
int CamTappet::fromCamRef(const MC_CAM_REF camRef){
    auto& cam = *camRef;
    if (!camRef ) return -1;
    if (cam.wCamStructID !=0xDC34 || cam.byType != mcCamTypeXYVA){
        return -2;
    }
    auto pce = (CamXyva*) cam.pce;
    if (cam.pce && cam.nElements)
        xyva.assign(pce, pce + cam.nElements);
    else
        return -3;
    auto pt = (Tappet*) cam.pt;
    if (cam.pt && cam.nTappets)
        tappets.assign(pt, pt + cam.nTappets);
    static_assert(sizeof(SMC_CAMTappet) == sizeof(Tappet), "size not equal");

    xMin = cam.xStart;
    xMax = cam.xEnd;
    return 0;
}

int _cam_array_dump_csv(const char* filename, const CamXyva* xyvaList, int xyvaLen) {

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "cant create file " << filename << std::endl;
        return -1;
    }

    // write CSV header
    outFile << "X,Y,V,A\n";

    // write data
    for (int i = 0; i < xyvaLen; i++) {
        const auto& xy = xyvaList[i];
        outFile << xy.dX << "," << xy.dY << "," << xy.dV << "," << xy.dA << "\n";
    }
    return 0;
}