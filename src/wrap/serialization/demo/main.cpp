/*
 * @Author: ChenShengyong
 * @Date: 2024-08-02 16:42:28
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-08-05 10:30:57
 */
#include "stdio.h"
#include "string.h"

void vel_2_json_demo();
void ServoSgFaker_2_json_demo();
void vel_load_demo();

static int printHelp(int argc, char** argv) {
    printf(
        " --version\n"
        " vel_dump\n"
        " vel_load\n"
        " servo\n"
        " --help\n");
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return printHelp(argc, argv);
    } else if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        return printHelp(argc, argv);
    } else if (argc >= 2 && strcmp(argv[1], "vel_dump") == 0) {
        vel_2_json_demo();
    } else if (argc >= 2 && strcmp(argv[1], "vel_load") == 0) {
        vel_load_demo();
    } else if (argc >= 2 && strcmp(argv[1], "servo") == 0) {
        ServoSgFaker_2_json_demo();
    }
    return 0;
}