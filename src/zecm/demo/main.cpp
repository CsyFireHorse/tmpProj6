#include "stdio.h"
#include "string.h"

int run_servo_simulator(int argc, char* argv[]);
int main_socket_demo(int argc, char* argv[]);


void printHelp(char** argv){
    printf("Usage: %s <command>\n", argv[0]);
    printf("Commands:\n");
    printf("  servo_simulator\n");
}
int main(int argc, char* argv[]){
    if (argc == 2 && strcmp(argv[1], "--help") == 0){
        printHelp(argv);
        return 0;
    }else if (argc > 1 && strcmp(argv[1], "servo_simulator") == 0){
        return run_servo_simulator(argc-1, argv+1);
    }else if (argc > 1 && strcmp(argv[1], "socket_demo") == 0){
        return main_socket_demo(argc-1, argv+1);
    }else {
        printHelp(argv);
        return 1;
    }
}