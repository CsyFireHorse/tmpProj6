#include "cam_tappet.h"
#include "cam_tappet_serialize.h"
#include "stdio.h"

// Usage example.
void cam_tappet_serialize_demo() {
    CamTappet data;
    data.xMin = 0.0;
    data.xMax = 10.0;
    data.xyva = {{0, 3.5, 6, 7}, {2, -2.987654321, 6.5, 0}, {5.5, 5, 3.5, 0}, {10, 2, 0.5, 0}};
    data.tappets = {{2, 3, 0, 0, 256, 1.5}, {0, 1, 0, 0, 511, 2.5}};
    const char* filename = "out.txt";
    // Serialization example.
    data.serialize(filename);
    // Parsing example.
    CamTappet data2;
    data2.parse(filename);

    const char* filename2 = "out2.txt";
    data2.serialize(filename2);
}
