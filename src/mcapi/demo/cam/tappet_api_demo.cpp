#include "cam_tappet_ser_api.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"


void cam_tappet_update_demo() {

    CamTappet* cam = cam_tappet_new();
    

    cam_tappet_resize(cam, 10, 5);


    int xyva_len, tappet_len;
    CamXyva* xyva = cam_tappet_xyva_ptr(cam, &xyva_len);
    Tappet* tappets = cam_tappet_tappet_ptr(cam, &tappet_len);


    for(int i = 0; i < xyva_len; ++i) {
        xyva[i] = { 
            static_cast<double>(i), 
            i * 0.5,                
            i * 2.0,                
            i * 0.1                 
        };
    }

    printf("resize\n");

    cam_tappet_resize(cam, xyva_len + 2, tappet_len);
    

    xyva = cam_tappet_xyva_ptr(cam, &xyva_len);


    xyva[xyva_len - 2] = {15.0, 7.5, 3.0, 0.2};
    xyva[xyva_len - 1] = {20.0, 8.0, 2.5, 0.1};


    printf("XYVA Points(%d):\n", xyva_len);
    for(int i = 0; i < xyva_len; ++i) {
        printf("[%d] X:%.1f Y:%.1f\n", i, xyva[i].dX, xyva[i].dY);
    }


    cam_tappet_delete(cam);
}


void tappet_show_demo(){
    const char* actionStrArr[] = { "nop", "open", "close", "invert"};
    Tappet tappet = { 0, 1, 0, 0, 1, 1.5, 0};

    printf("ID,X,positive pass,negative pass\n");
    if (tappet.ctt == 0) {
        printf("%d,%f,%s,%s\n", tappet.iGroupID, tappet.x, actionStrArr[tappet.cta], actionStrArr[0]);
    }else if(tappet.ctt == 1) {
        printf("%d,%f,%s,%s\n", tappet.iGroupID, tappet.x, actionStrArr[tappet.cta], actionStrArr[tappet.cta]);
    }else if(tappet.ctt == 2) {
        printf("%d,%f,%s,%s\n", tappet.iGroupID, tappet.x, actionStrArr[0], actionStrArr[tappet.cta]);
    }
}


void _cam_file_to_csv_demo(const char* filename, bool is_csv, const char* oFilename) {
    CamTappet* camTp = cam_tappet_new();
    int ret = cam_tappet_parse_camfile(camTp, filename);
    if (ret) return;

    int len;
    auto ptr = cam_tappet_xyva_ptr(camTp, &len);

    printf("XYVA points: %d\n", len);
    // for (int i = 0; i < len; i++) {
        // auto& xy = ptr[i];
        // printf("%f,%f,%f,%f\n", xy.dX, xy.dY, xy.dV, xy.dA);
    // }
    char* outputFn = (char*) malloc(strlen(oFilename) + 5);
    strcpy(outputFn, oFilename);
    const char* fnTail[] = {".ini", ".csv"};
    if (!is_csv){
        // oFilename is  not end with '.ini', then append '.ini'
        if (strstr(outputFn, fnTail[0]) == NULL){
            strcat(outputFn, fnTail[0]);
        }
        cam_tappet_serialize(camTp, outputFn);
    }else {
        if (strstr(outputFn, fnTail[1]) == NULL){
            strcat(outputFn, fnTail[1]);
        }
        cam_array_dump_csv(outputFn, ptr, len);
    }
    free(outputFn);
    cam_tappet_delete(camTp);
}

int cam_file_to_csv_demo(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <filename> [--csv] [--output <output_filename>]\n", argv[0]);
        return -1;
    }
    const char* filename = argv[1];
    bool is_csv = false;
    if (argc > 2 && strcmp(argv[2], "--csv") == 0) {
        is_csv = true;
    }
    const char* output_filename = "output";
    if (argc > 3 && (strcmp(argv[3], "--output") == 0 || strcmp(argv[3], "-o") == 0)) {
        output_filename = argv[4];
    }
    _cam_file_to_csv_demo(filename, is_csv, output_filename);
    return 0;
}

void cam_file_parse_demo() { 
    char* argv[] = {"cam_file_to_csv_demo", "data.cam", "--csv", "--output", "output"};
    cam_file_to_csv_demo(5, argv); 
}