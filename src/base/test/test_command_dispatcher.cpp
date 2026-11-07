#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "common/misc/CommandDispatcher.h"

namespace {

// This helper builds a mutable argv view that stays valid during one parse call.
static std::vector<char*> buildArgv(std::vector<std::string>& storage) {
    std::vector<char*> argv;
    argv.reserve(storage.size());
    for (size_t i = 0; i < storage.size(); ++i) {
        argv.push_back(const_cast<char*>(storage[i].c_str()));
    }
    return argv;
}

}  // namespace

using namespace zmc;

TEST(command_dispatcher, parse_vector_hits_vector_handler)
{
    CommandDispatcher dispatcher;
    std::vector<std::string> seen;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("echo", [&](const CommandDispatcher::ArgList& args) -> int {
        seen = args;
        return 12;
    }));

    CommandDispatcher::ArgList tokens;
    tokens.push_back("prog");
    tokens.push_back("echo");
    tokens.push_back("a");
    tokens.push_back("b");

    EXPECT_EQ(12, dispatcher.parse(tokens));
    ASSERT_EQ(static_cast<size_t>(3), seen.size());
    EXPECT_EQ("echo", seen[0]);
    EXPECT_EQ("a", seen[1]);
    EXPECT_EQ("b", seen[2]);
}

TEST(command_dispatcher, parse_argv_hits_argv_handler)
{
    CommandDispatcher dispatcher;
    int seenArgc = -1;
    std::vector<std::string> seenArgs;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("cmd", [&](int argc, char** argv) -> int {
        seenArgc = argc;
        seenArgs.clear();
        for (int i = 0; i < argc; ++i) {
            seenArgs.push_back(argv[i]);
        }
        return 23;
    }));

    std::vector<std::string> storage;
    storage.push_back("prog");
    storage.push_back("cmd");
    storage.push_back("left");
    storage.push_back("right");
    std::vector<char*> argv = buildArgv(storage);

    EXPECT_EQ(23, dispatcher.parse(static_cast<int>(argv.size()), &argv[0]));
    EXPECT_EQ(3, seenArgc);
    ASSERT_EQ(static_cast<size_t>(3), seenArgs.size());
    EXPECT_EQ("cmd", seenArgs[0]);
    EXPECT_EQ("left", seenArgs[1]);
    EXPECT_EQ("right", seenArgs[2]);
}

TEST(command_dispatcher, parse_without_argv0_still_hits_handlers)
{
    CommandDispatcher dispatcher;
    std::vector<std::string> vectorArgs;
    int seenArgc = -1;
    std::vector<std::string> seenArgs;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("vec", [&](const CommandDispatcher::ArgList& args) -> int {
        vectorArgs = args;
        return 17;
    }));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("cmd", [&](int argc, char** argv) -> int {
        seenArgc = argc;
        seenArgs.clear();
        for (int i = 0; i < argc; ++i) {
            seenArgs.push_back(argv[i]);
        }
        return 23;
    }));

    EXPECT_EQ(17, dispatcher.parse(CommandDispatcher::ArgList{"vec", "a", "b"}, true));
    ASSERT_EQ(static_cast<size_t>(2), vectorArgs.size());
    EXPECT_EQ("a", vectorArgs[0]);
    EXPECT_EQ("b", vectorArgs[1]);

    std::vector<std::string> storage;
    storage.push_back("cmd");
    storage.push_back("left");
    storage.push_back("right");
    std::vector<char*> argv = buildArgv(storage);

    EXPECT_EQ(23, dispatcher.parse(static_cast<int>(argv.size()), &argv[0], true));
    EXPECT_EQ(2, seenArgc);
    ASSERT_EQ(static_cast<size_t>(2), seenArgs.size());
    EXPECT_EQ("left", seenArgs[0]);
    EXPECT_EQ("right", seenArgs[1]);
}

TEST(command_dispatcher, empty_input_uses_default_handler)
{
    CommandDispatcher dispatcher;
    int calls = 0;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.setDefaultHandler([&](const CommandDispatcher::ArgList& args) -> int {
        ++calls;
        return static_cast<int>(args.size()) + 31;
    }));

    std::vector<std::string> storage;
    storage.push_back("prog");
    std::vector<char*> argv = buildArgv(storage);

    EXPECT_EQ(31, dispatcher.parse(CommandDispatcher::ArgList()));
    EXPECT_EQ(31, dispatcher.parse(0, static_cast<char**>(NULL)));
    EXPECT_EQ(31, dispatcher.parse(CommandDispatcher::ArgList{"prog"}));
    EXPECT_EQ(31, dispatcher.parse(static_cast<int>(argv.size()), &argv[0]));
    EXPECT_EQ(4, calls);
}

TEST(command_dispatcher, empty_input_argv_default_handler_receives_no_arguments)
{
    CommandDispatcher dispatcher;
    int seenArgc = -1;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.setDefaultHandler([&](int argc, char** argv) -> int {
        seenArgc = argc;
        EXPECT_TRUE(argv == NULL);
        return 41;
    }));

    std::vector<std::string> storage;
    storage.push_back("prog");
    std::vector<char*> argv = buildArgv(storage);

    EXPECT_EQ(41, dispatcher.parse(CommandDispatcher::ArgList{"prog"}));
    EXPECT_EQ(0, seenArgc);
    EXPECT_EQ(41, dispatcher.parse(static_cast<int>(argv.size()), &argv[0]));
    EXPECT_EQ(0, seenArgc);
}

TEST(command_dispatcher, empty_input_without_default_returns_not_found)
{
    CommandDispatcher dispatcher;

    std::vector<std::string> storage;
    storage.push_back("prog");
    std::vector<char*> argv = buildArgv(storage);

    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(CommandDispatcher::ArgList()));
    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(0, static_cast<char**>(NULL)));
    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(CommandDispatcher::ArgList{"prog"}));
    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(static_cast<int>(argv.size()), &argv[0]));
}

TEST(command_dispatcher, miss_does_not_fall_back_to_default_handler)
{
    CommandDispatcher dispatcher;
    int defaultCalls = 0;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.setDefaultHandler([&](const CommandDispatcher::ArgList&) -> int {
        ++defaultCalls;
        return 77;
    }));

    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(CommandDispatcher::ArgList{"missing"}, true));
    EXPECT_EQ(CommandDispatcher::ERR_NOT_FOUND, dispatcher.parse(CommandDispatcher::ArgList{"prog", "missing"}));
    EXPECT_EQ(0, defaultCalls);
}

TEST(command_dispatcher, void_helper_rejects_extra_arguments)
{
    CommandDispatcher dispatcher;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("noop", makeVoidHandler([]() {
    })));

    CommandDispatcher::ArgList tokens;
    tokens.push_back("prog");
    tokens.push_back("noop");
    tokens.push_back("extra");

    EXPECT_EQ(CommandDispatcher::ERR_INVALID_ARG_COUNT, dispatcher.parse(tokens));
}

TEST(command_dispatcher, typed_helpers_convert_and_invoke)
{
    CommandDispatcher dispatcher;
    int intValue = 0;
    double doubleValue = 0.0;
    bool boolValue = false;
    std::string stringValue;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_int", makeIntHandler([&](int value) {
        intValue = value;
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_double", makeDoubleHandler([&](double value) {
        doubleValue = value;
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_bool", makeBoolHandler([&](bool value) {
        boolValue = value;
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_str", makeStringHandler([&](const std::string& value) {
        stringValue = value;
    })));

    EXPECT_EQ(CommandDispatcher::OK, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_int", "42"}));
    EXPECT_EQ(CommandDispatcher::OK, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_double", "3.5"}));
    EXPECT_EQ(CommandDispatcher::OK, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_bool", "YeS"}));
    EXPECT_EQ(CommandDispatcher::OK, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_str", "hello"}));

    EXPECT_EQ(42, intValue);
    EXPECT_DOUBLE_EQ(3.5, doubleValue);
    EXPECT_TRUE(boolValue);
    EXPECT_EQ("hello", stringValue);
}

TEST(command_dispatcher, typed_helpers_report_conversion_errors)
{
    CommandDispatcher dispatcher;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_int", makeIntHandler([](int) {
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_double", makeDoubleHandler([](double) {
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("set_bool", makeBoolHandler([](bool) {
    })));

    EXPECT_EQ(CommandDispatcher::ERR_CONVERT_ARG, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_int", "1x"}));
    EXPECT_EQ(CommandDispatcher::ERR_CONVERT_ARG, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_double", "3.5x"}));
    EXPECT_EQ(CommandDispatcher::ERR_CONVERT_ARG, dispatcher.parse(CommandDispatcher::ArgList{"prog", "set_bool", "maybe"}));
}

TEST(command_dispatcher, helper_return_codes_are_forwarded)
{
    CommandDispatcher dispatcher;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("status", makeIntHandler([](int value) -> int {
        return value + 5;
    })));

    EXPECT_EQ(12, dispatcher.parse(CommandDispatcher::ArgList{"prog", "status", "7"}));
}

TEST(command_dispatcher, repeated_bind_and_default_handler_are_overwritten)
{
    CommandDispatcher dispatcher;

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("dup", makeVoidHandler([]() -> int {
        return 11;
    })));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.bind("dup", makeVoidHandler([]() -> int {
        return 19;
    })));

    ASSERT_EQ(CommandDispatcher::OK, dispatcher.setDefaultHandler([](const CommandDispatcher::ArgList&) -> int {
        return 21;
    }));
    ASSERT_EQ(CommandDispatcher::OK, dispatcher.setDefaultHandler([](const CommandDispatcher::ArgList&) -> int {
        return 29;
    }));

    EXPECT_EQ(19, dispatcher.parse(CommandDispatcher::ArgList{"prog", "dup"}));
    EXPECT_EQ(29, dispatcher.parse(CommandDispatcher::ArgList()));
}

TEST(command_dispatcher, invalid_empty_std_function_bind_is_rejected)
{
    CommandDispatcher dispatcher;
    CommandDispatcher::VectorHandler emptyVectorHandler;
    CommandDispatcher::ArgvHandler emptyArgvHandler;

    EXPECT_EQ(CommandDispatcher::ERR_INVALID_BIND, dispatcher.bind("vec", emptyVectorHandler));
    EXPECT_EQ(CommandDispatcher::ERR_INVALID_BIND, dispatcher.bind("argv", emptyArgvHandler));
    EXPECT_EQ(CommandDispatcher::ERR_INVALID_BIND, dispatcher.setDefaultHandler(emptyVectorHandler));
    EXPECT_EQ(CommandDispatcher::ERR_INVALID_BIND, dispatcher.setDefaultHandler(emptyArgvHandler));
}
