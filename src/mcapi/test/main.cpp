#include <gtest/gtest.h>

#define LEN(x) sizeof(x) / sizeof(*(x))
int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);

    if (argc == 1) {
        char* argv2[] = {"", "--gtest_filter=**"};
        int argc2 = LEN(argv2);
        testing::InitGoogleTest(&argc2, argv2);
    } else {
        testing::InitGoogleTest(&argc, argv);
    }
    return RUN_ALL_TESTS();
}