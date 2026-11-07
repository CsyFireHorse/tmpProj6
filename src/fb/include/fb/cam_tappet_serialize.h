#ifndef _CAM_REF_FILE_H_
#define _CAM_REF_FILE_H_

#include <vector>
#include "cam_tappet.h"
#include "mc_ref_def.h"

struct CamTappet{
    int ver_major = 1;
    int ver_minor = 0;
    int ver_patch = 0;
    double xMin;
    double xMax;
    std::vector<CamXyva> xyva;
    std::vector<Tappet> tappets;

    int parse(const char* filename);
    int serialize(const char* filename)const;
    int parseCamFile(const char* filename);
    void toCamRef(MC_CAM_REF cam) const;
    int fromCamRef(const MC_CAM_REF cam);
};

MC_CAM_REF makeCamRef(const CamXyva* pXyva, int len);
// void initCamRef(MC_CAM_REF pCam);

#endif