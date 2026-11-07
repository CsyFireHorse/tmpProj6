#include <gtest/gtest.h>
#include <string>
#include "common/utils/fnv1a128.h"

// GPT-6 Codex, 2026-10-22. Algorithm vectors belong to the base utility tests.
namespace zmc {
namespace {
struct Fnv128TestVector {
    const char* data;
    size_t size;
    const char* digest;
};

TEST(Fnv1a128Test, MatchesRfcVectorsWithIncrementalInput) {
    const char binary[] = {
        'H', 'e', 'l', 'l', 'o', '!', 0x01, static_cast<char>(0xff), static_cast<char>(0xed)};
    const char embedded_nul[] = {'a', '\0'};
    const Fnv128TestVector vectors[] = {
        {"", 0U, "6c62272e07bb014262b821756295c58d"},
        {"a", 1U, "d228cb696f1a8caf78912b704e4a8964"},
        {"foobar", 6U, "343e1662793c64bf6f0d3597ba446f18"},
        {binary, sizeof(binary), "74202c600b051c165b1acafed10d1419"},
        {embedded_nul, sizeof(embedded_nul), "0880954519ab1be95aa0733055b70e0c"}};
    const size_t chunk_sizes[] = {1U, 3U, 2U};

    for (size_t vector = 0; vector < sizeof(vectors) / sizeof(vectors[0]); ++vector) {
        char digest[33];
        const U128 one_shot = fnv1a_128(vectors[vector].data, vectors[vector].size);
        fnv1a_128_digest(&one_shot, digest);
        EXPECT_STREQ(vectors[vector].digest, digest);
        EXPECT_EQ('\0', digest[32]);

        U128 bytewise;
        fnv1a_128_init(&bytewise);
        fnv1a_128_update(&bytewise, vectors[vector].data, 0U);
        for (size_t i = 0; i < vectors[vector].size; ++i) {
            fnv1a_128_update(&bytewise, vectors[vector].data + i, 1U);
        }
        fnv1a_128_digest(&bytewise, digest);
        EXPECT_STREQ(vectors[vector].digest, digest);

        U128 uneven;
        fnv1a_128_init(&uneven);
        size_t offset = 0;
        size_t chunk = 0;
        while (offset < vectors[vector].size) {
            size_t length = chunk_sizes[chunk++ % 3U];
            const size_t remaining = vectors[vector].size - offset;
            if (length > remaining) length = remaining;
            fnv1a_128_update(&uneven, vectors[vector].data + offset, length);
            offset += length;
        }
        fnv1a_128_digest(&uneven, digest);
        EXPECT_STREQ(vectors[vector].digest, digest);
        fnv1a_128_digest(&uneven, digest);
        EXPECT_STREQ(vectors[vector].digest, digest);
        EXPECT_EQ(32U, std::string(digest).size());
        EXPECT_EQ(std::string::npos, std::string(digest).find_first_not_of("0123456789abcdef"));
    }
}
}  // namespace
}  // namespace zmc
