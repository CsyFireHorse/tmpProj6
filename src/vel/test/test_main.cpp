#include <gtest/gtest.h>

int main(int argc, char** argv) {
// printf("Running main() from %s\n", __FILE__);
#ifdef _DEBUG
    if (argc == 1) {
#define LEN(x) sizeof(x) / sizeof(*x)
        char* argv2[] = {"", "--gtest_filter=**"};
        int argc2 = LEN(argv2);
        testing::InitGoogleTest(&argc2, argv2);
        return RUN_ALL_TESTS();
    }
#endif
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}