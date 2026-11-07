#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <type_traits>

#include <string>
#include <vector>

#include "EcatMotion.h"
#include "EcatMotionFacade.hpp"
#include "../source/EcatMotionCli.hpp"
#include "MotionRunApi.h"
#include "srv/HttpFileManager.hpp"
#include "common/get_file_path.hpp"
#include "common/zal_version.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"

namespace {

// The facade owns the runtime without exposing a polymorphic base relationship.
static_assert(!std::is_base_of<zmc::EcatMotion, zmc::EcatMotionFacade>::value,
              "Facade must not inherit EcatMotion");
static_assert(!std::is_polymorphic<zmc::EcatMotion>::value,
              "EcatMotion must not contain virtual functions");

// Expose parsed motion options without changing the production interface.
class EcatMotionArgsProbe : public zmc::EcatMotion {
   public:
    unsigned int axis_num() const { return ini_args().axis_num; }
};


// Own and clean up a small local file used by the selection tests.
class ScopedTestFile {
   public:
    explicit ScopedTestFile(const std::string& path) : path_(path) {}
    ~ScopedTestFile() { std::remove(path_.c_str()); }
    bool write(const std::string& bytes) const {
        std::ofstream output(path_.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (!output.is_open()) {
            return false;
        }
        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        output.close();
        return output.good();
    }
    const std::string& path() const { return path_; }
   private:
    std::string path_;
};

// Read fixture bytes so tests can compare selected snapshots with the content hash format.
std::string read_test_file(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        return std::string();
    }
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::string hash_bytes(const std::string& bytes) {
    return zmc::HttpFileManager::hashBytes(
        bytes.empty() ? nullptr : bytes.data(), bytes.size());
}

}  // namespace

TEST(EcatMotionArgs, RunModeDefaultsToHttp) {
    EcatMotionArgsProbe ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(zmc::MODE_HTTP, ctx.run_mode());
    EXPECT_EQ(ZMC_MOTAPP_HTTP_DEFAULT_PORT, ctx.port());
    EXPECT_EQ(0u, ctx.axis_num());
    EXPECT_FALSE(ctx.script_exit());
}

TEST(EcatMotionArgs, AxisCountClampsToLogicalAxisCapacity) {
    EcatMotionArgsProbe ctx;
    std::vector<std::string> args;
    args.push_back("--axis-num=65");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(static_cast<unsigned int>(MC_MOTION_MAX_AXES), ctx.axis_num());
}

TEST(EcatMotionArgs, RunModeTracksCmdOption) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--cmd");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(zmc::MODE_CMD, ctx.run_mode());
}

TEST(EcatMotionArgs, PortOptionOverridesDefault) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--port=9090");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(9090, ctx.port());
}

TEST(EcatMotionArgs, InvalidPortOptionFails) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--port=bad");

    EXPECT_EQ(-1, ctx.parse_args(args));
}

TEST(EcatMotionArgs, CmdModeAcceptsPortOption) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--cmd");
    args.push_back("--port=9090");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(zmc::MODE_CMD, ctx.run_mode());
    EXPECT_EQ(9090, ctx.port());
}

TEST(EcatMotionArgs, ScriptExitOptionIsParsed) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--cmd");
    args.push_back("--script_exit");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_TRUE(ctx.script_exit());
}

TEST(EcatMotionArgs, RpcModeAllowsScriptArgs) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("axis.ini");
    args.push_back("--rpc");
    args.push_back("--script");
    args.push_back("startup.txt");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ(zmc::MODE_RPC, ctx.run_mode());
}

TEST(EcatMotionArgs, PathOptionsAreOptional) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_TRUE(ctx.ini_path().empty());
    EXPECT_TRUE(ctx.eni_path().empty());
    // Channel / netname defaults to ZMC_DEFAULT_NET_NAME.
    EXPECT_EQ(ZMC_DEFAULT_NET_NAME, ctx.net_name());
}

TEST(EcatMotionArgs, DeviceBackgroundInfoDefaults) {
    zmc::EcatMotion ctx;

    EXPECT_TRUE(ctx.eni_name().empty());
    EXPECT_TRUE(ctx.ini_name().empty());
    EXPECT_EQ(ZMC_DEFAULT_NET_NAME, ctx.net_name());
}

TEST(EcatMotionArgs, NamedPathOptionsAreParsed) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back("--ini=axis.ini");
    args.push_back("--eni=motion.xml");
    args.push_back("--netname=zecm1");

    EXPECT_EQ(0, ctx.parse_args(args));
    EXPECT_EQ("axis.ini", ctx.ini_path());
    EXPECT_EQ("motion.xml", ctx.eni_path());
    EXPECT_EQ("zecm1", ctx.net_name());
    EXPECT_TRUE(ctx.ini_name().empty());
    EXPECT_TRUE(ctx.eni_name().empty());
    EXPECT_EQ("zecm1", ctx.net_name());
}

TEST(EcatMotionArgs, CliResolvesNestedEniBeforeEcatMotion) {
    const std::string ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    zmc::PdoPowerIniArgs parsed;
    std::vector<std::string> prepared_args;
    ASSERT_EQ(0, zmc::prepare_ecat_motion_cli_args(std::vector<std::string>{"--ini", ini},
                                                   &parsed, &prepared_args));
    EXPECT_EQ(ini, parsed.iniName);
    EXPECT_NE(std::string::npos, parsed.eniName.find("pse_servo1.xml"));
    ASSERT_EQ(4U, prepared_args.size());
    EXPECT_EQ("--eni", prepared_args[2]);
    EXPECT_EQ(parsed.eniName, prepared_args[3]);

    zmc::EcatMotion cli_prepared;
    ASSERT_EQ(0, cli_prepared.prepare_config(prepared_args));
    EXPECT_EQ(parsed.eniName, cli_prepared.eni_path());

    zmc::EcatMotion service_args;
    ASSERT_EQ(0, service_args.prepare_config(std::vector<std::string>{"--ini", ini}));
    EXPECT_TRUE(service_args.eni_path().empty());

    ASSERT_EQ(0, zmc::prepare_ecat_motion_cli_args(std::vector<std::string>{"--ini=" + ini},
                                                   &parsed, &prepared_args));
    EXPECT_NE(std::string::npos, parsed.eniName.find("pse_servo1.xml"));

    const std::vector<std::string> explicit_eni_args = {"--ini", ini, "--eni", "override.xml"};
    ASSERT_EQ(0, zmc::prepare_ecat_motion_cli_args(explicit_eni_args, &parsed, &prepared_args));
    EXPECT_EQ("override.xml", parsed.eniName);
    EXPECT_EQ(explicit_eni_args, prepared_args);

    ASSERT_EQ(0, zmc::prepare_ecat_motion_cli_args(std::vector<std::string>{ini}, &parsed,
                                                   &prepared_args));
    EXPECT_EQ(ini, parsed.iniName);
    EXPECT_NE(std::string::npos, parsed.eniName.find("pse_servo1.xml"));
    ASSERT_EQ(3U, prepared_args.size());
    EXPECT_EQ("--eni", prepared_args[1]);
    EXPECT_EQ(parsed.eniName, prepared_args[2]);
}

TEST(EcatMotionArgs, CommandLineOverridesIniBusOptions) {
    zmc::EcatMotion ctx;
    std::vector<std::string> args;
    args.push_back(zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini"));
    args.push_back("--eni=override.xml");
    args.push_back("--netname=zecm9");

    ASSERT_EQ(0, ctx.prepare_config(args));
    EXPECT_EQ("override.xml", ctx.eni_path());
    EXPECT_EQ("zecm9", ctx.net_name());
}

TEST(EcatMotionLifecycle, EniSelectSnapshotsHashAndPreservesPreviousOnFailure) {
    ScopedTestFile file("ecat_motion_selected_hash_test.xml");
    ASSERT_TRUE(file.write("foobar"));
    zmc::EcatMotion ctx;
    ASSERT_EQ(0, ctx.eni_select(file.path()));
    EXPECT_EQ(file.path(), ctx.selected_eni_path());
    EXPECT_EQ("343e1662793c64bf6f0d3597ba446f18", ctx.selected_eni_hash());

    ASSERT_TRUE(file.write("a"));
    EXPECT_EQ("343e1662793c64bf6f0d3597ba446f18", ctx.selected_eni_hash());
    ASSERT_EQ(0, ctx.eni_select(file.path()));
    EXPECT_EQ("d228cb696f1a8caf78912b704e4a8964", ctx.selected_eni_hash());

    const std::string previous_path = ctx.selected_eni_path();
    const std::string previous_hash = ctx.selected_eni_hash();
    EXPECT_EQ(static_cast<int>(ZMC_ERR_ENI_FILE_OPEN_FAILED),
              ctx.eni_select(file.path() + ".missing"));
    EXPECT_EQ(previous_path, ctx.selected_eni_path());
    EXPECT_EQ(previous_hash, ctx.selected_eni_hash());
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), ctx.eni_select(""));
}

TEST(EcatMotionLifecycle, EniSelectUpdatesNextConfigWithoutChangingCurrent) {
    zmc::EcatMotion ctx;
    const std::string eni = zmc::_get_file_path("mhelper/data/pse_servo1.xml");
    ASSERT_EQ(0, ctx.eni_select(eni));
    EXPECT_EQ(eni, ctx.selected_eni_path());
    EXPECT_TRUE(ctx.eni_name().empty());
    EXPECT_TRUE(ctx.current_eni_hash().empty());
    EXPECT_TRUE(ctx.eni_path().empty());
}

TEST(EcatMotionLifecycle, AvailableChannelsAfterDeviceOpen) {
    zmc::EcatMotionFacade ctx;
    EXPECT_TRUE(ctx.available_channel_names().empty());
    EXPECT_FALSE(ctx.is_device_open());
    EXPECT_FALSE(ctx.is_channel_open());
    EXPECT_FALSE(ctx.is_ecat_running());
    EXPECT_EQ(0, ctx.openDevice("0"));
    EXPECT_TRUE(ctx.is_device_open());
    EXPECT_TRUE(ctx.is_channel_open());
    const std::vector<std::string> names = ctx.available_channel_names();
    ASSERT_EQ(1u, names.size());
    EXPECT_EQ(ZMC_DEFAULT_NET_NAME, names[0]);
    EXPECT_EQ(0, ctx.closeAll());
    EXPECT_TRUE(ctx.available_channel_names().empty());
}

TEST(EcatMotionLifecycle, OpenDeviceAndCloseDevice) {
    zmc::EcatMotionFacade ctx;
    EXPECT_EQ(0, ctx.openDevice("0"));
    EXPECT_TRUE(ctx.is_device_open());
    EXPECT_EQ(0, ctx.closeAll());
    EXPECT_FALSE(ctx.is_device_open());
}

TEST(EcatMotionLifecycle, OpenDeviceAndChannel) {
    zmc::EcatMotionFacade ctx;
    const int ret = ctx.openDevice("0");
    ASSERT_EQ(0, ret);
    EXPECT_TRUE(ctx.is_device_open());
    EXPECT_TRUE(ctx.is_channel_open());
    EXPECT_FALSE(ctx.is_ecat_running());
    EXPECT_EQ(0, ctx.openChannel(ZMC_DEFAULT_NET_NAME));
    EXPECT_EQ(0, ctx.closeChannel());
    EXPECT_TRUE(ctx.is_channel_open());
    EXPECT_TRUE(ctx.is_device_open());
    EXPECT_EQ(0, ctx.closeAll());
}

TEST(EcatMotionLifecycle, VersionAndAboutMetadata) {
    zmc::EcatMotion ctx;
    EXPECT_EQ(std::string(ZAL_VERSION_STR), ctx.version());
    EXPECT_FALSE(ctx.about_author().empty());
    EXPECT_FALSE(ctx.about_description().empty());
    EXPECT_FALSE(ctx.about_code_date().empty());
}

TEST(EcatMotionLifecycle, FacadeStartupRequiresOpenDevice) {
    zmc::EcatMotionFacade ctx;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_HOST_INVALID_HANDLE),
              ctx.startup(std::string(), std::string()));
    // INI validation precedes the logical-device state check.
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), ctx.startupIni(std::string()));
    const std::string ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    EXPECT_EQ(static_cast<int>(ZMC_ERR_HOST_INVALID_HANDLE), ctx.startupIni(ini));
    ASSERT_EQ(0, ctx.openDevice("0"));
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              ctx.startup(std::string(), std::string()));
}

TEST(EcatMotionLifecycle, FacadeKeepsCommandInterfaceAcrossClose) {
    zmc::EcatMotionFacade ctx;
    zmc::MotionCmdQueueBase* queue = &ctx.command_interface();
    ASSERT_EQ(0, ctx.openDevice("0"));
    ASSERT_EQ(0, ctx.closeAll());
    EXPECT_EQ(queue, &ctx.command_interface());
}

TEST(EcatMotionLifecycle, StartupRequiresEni) {
    zmc::EcatMotion ctx;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              ctx.startup(std::string(), std::string()));
}

TEST(EcatMotionLifecycle, StartupLogLevelsControlEventsAndEniDetails) {
    const std::string eni = zmc::_get_file_path("mhelper/data/pse_servo2.xml");
    for (int level = 0; level <= 2; ++level) {
        zmc::EcatMotion ctx;
        ctx.set_log_level(level);
        testing::internal::CaptureStdout();
        const int ret = ctx.startup(eni, std::string());
        const std::string output = testing::internal::GetCapturedStdout();
        ASSERT_EQ(0, ret);
        EXPECT_EQ(level >= 1, output.find("load ecat eni " + eni) != std::string::npos);
        EXPECT_EQ(level >= 1, output.find("start ecat zecm0") != std::string::npos);
        EXPECT_EQ(level >= 2, output.find("slaves=2 servos=2") != std::string::npos);
        EXPECT_EQ(0, ctx.stop());
    }
}

TEST(EcatMotionLifecycle, StartupWithoutEniFails) {
    zmc::EcatMotion ctx;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), ctx.eni_select(""));
}

TEST(EcatMotionLifecycle, StartupIniUsesExplicitPathsWithoutChangingCandidates) {
    zmc::EcatMotion ctx;
    const std::string ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    const std::string candidate_ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    ASSERT_EQ(0, ctx.ini_select(candidate_ini));
    const std::string candidate_ini_hash = ctx.selected_ini_hash();
    const std::string candidate_eni_path = ctx.selected_eni_path();
    const std::string candidate_eni_hash = ctx.selected_eni_hash();

    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), ctx.startupIni(ini));
    EXPECT_TRUE(ctx.eni_name().empty());
    EXPECT_TRUE(ctx.ini_name().empty());
    EXPECT_EQ(candidate_ini, ctx.selected_ini_path());
    EXPECT_EQ(candidate_ini_hash, ctx.selected_ini_hash());
    EXPECT_EQ(candidate_eni_path, ctx.selected_eni_path());
    EXPECT_EQ(candidate_eni_hash, ctx.selected_eni_hash());
}

TEST(EcatMotionLifecycle, ReadEniJsonTextEmptyPathFails) {
    zmc::EcatMotion ctx;
    std::string out;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_ENI_FILE_OPEN_FAILED), ctx.read_eni_json_text(out));
    EXPECT_TRUE(out.empty());
}

TEST(EcatMotionLifecycle, ReadEniFileEmptyPathFails) {
    zmc::EcatMotion ctx;
    std::string out;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_ENI_FILE_OPEN_FAILED), ctx.read_eni_file(out));
    EXPECT_TRUE(out.empty());
}

TEST(EcatMotionLifecycle, ReadIniFileEmptyPathFails) {
    zmc::EcatMotion ctx;
    std::string out;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_INI_FILE_OPEN), ctx.read_ini_file(out));
    EXPECT_TRUE(out.empty());
}

TEST(EcatMotionLifecycle, ReadEniJsonTextUsesCurrentConfig) {
    zmc::EcatMotion ctx;
    const std::string eni = zmc::_get_file_path("mhelper/data/pse_servo2.xml");
    ASSERT_EQ(0, ctx.eni_select(eni));
    std::string json_text;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_ENI_FILE_OPEN_FAILED), ctx.read_eni_json_text(json_text));
    EXPECT_TRUE(json_text.empty());
    std::string bytes;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_ENI_FILE_OPEN_FAILED), ctx.read_eni_file(bytes));
    EXPECT_TRUE(bytes.empty());
    EXPECT_EQ(eni, ctx.selected_eni_path());
}

TEST(EcatMotionLifecycle, ReadIniFileUsesCurrentConfig) {
    zmc::EcatMotion ctx;
    const std::string ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    ASSERT_EQ(0, ctx.ini_select(ini));
    std::string content;
    EXPECT_EQ(static_cast<int>(ZMC_ERR_INI_FILE_OPEN), ctx.read_ini_file(content));
    EXPECT_TRUE(content.empty());
    EXPECT_EQ(ini, ctx.selected_ini_path());
}

TEST(EcatMotionLifecycle, IniSelectHashSnapshotsUpdateOnReselectAndClearOnClose) {
    ScopedTestFile referenced_eni("ecat_motion_ini_snapshot_referenced_eni.xml");
    ScopedTestFile selected_eni("ecat_motion_ini_snapshot_selected_eni.xml");
    ScopedTestFile ini("ecat_motion_ini_snapshot.ini");
    ASSERT_TRUE(referenced_eni.write("referenced eni"));
    ASSERT_TRUE(selected_eni.write("first selected eni"));
    const std::string ini_bytes =
        "[base]\naxis_num = 1\neni_path = ./ecat_motion_ini_snapshot_referenced_eni.xml\n";
    ASSERT_TRUE(ini.write(ini_bytes));

    zmc::EcatMotion ctx;
    ASSERT_EQ(0, ctx.eni_select(selected_eni.path()));
    ASSERT_EQ(0, ctx.ini_select(ini.path()));
    const std::string first_ini_hash = hash_bytes(read_test_file(ini.path()));
    const std::string first_eni_hash = hash_bytes(read_test_file(selected_eni.path()));
    EXPECT_EQ(ini.path(), ctx.selected_ini_path());
    EXPECT_EQ(selected_eni.path(), ctx.selected_eni_path());
    EXPECT_EQ(first_ini_hash, ctx.selected_ini_hash());
    EXPECT_EQ(first_eni_hash, ctx.selected_eni_hash());
    EXPECT_TRUE(ctx.ini_name().empty());
    EXPECT_TRUE(ctx.eni_name().empty());

    ASSERT_TRUE(referenced_eni.write("changed referenced eni"));
    ASSERT_TRUE(selected_eni.write("second selected eni"));
    ASSERT_TRUE(ini.write(ini_bytes + "; updated\n"));
    EXPECT_EQ(first_ini_hash, ctx.selected_ini_hash());
    EXPECT_EQ(first_eni_hash, ctx.selected_eni_hash());
    ASSERT_EQ(0, ctx.ini_select(ini.path()));
    EXPECT_EQ(hash_bytes(read_test_file(ini.path())), ctx.selected_ini_hash());
    EXPECT_EQ(selected_eni.path(), ctx.selected_eni_path());
    EXPECT_EQ(first_eni_hash, ctx.selected_eni_hash());
    EXPECT_NE(first_ini_hash, ctx.selected_ini_hash());
    ASSERT_EQ(0, ctx.eni_select(selected_eni.path()));
    EXPECT_EQ(hash_bytes(read_test_file(selected_eni.path())), ctx.selected_eni_hash());
    EXPECT_NE(first_eni_hash, ctx.selected_eni_hash());

    ASSERT_EQ(0, ctx.reset());
    EXPECT_TRUE(ctx.selected_ini_path().empty());
    EXPECT_TRUE(ctx.selected_ini_hash().empty());
    EXPECT_TRUE(ctx.selected_eni_path().empty());
    EXPECT_TRUE(ctx.selected_eni_hash().empty());
}

TEST(EcatMotionLifecycle, IniSelectUpdatesOnlyIniCandidate) {
    zmc::EcatMotion ctx;
    const std::string ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    const std::string eni = zmc::_get_file_path("mhelper/data/pse_servo2.xml");
    ASSERT_EQ(0, ctx.eni_select(eni));
    const std::string selected_eni_hash = ctx.selected_eni_hash();
    ASSERT_EQ(0, ctx.ini_select(ini));

    const std::string ini_bytes = read_test_file(ctx.selected_ini_path());
    ASSERT_FALSE(ini_bytes.empty());
    EXPECT_EQ(ini, ctx.selected_ini_path());
    EXPECT_EQ(hash_bytes(ini_bytes), ctx.selected_ini_hash());
    EXPECT_EQ(eni, ctx.selected_eni_path());
    EXPECT_EQ(selected_eni_hash, ctx.selected_eni_hash());

    ScopedTestFile invalid_ini("ecat_motion_missing_eni.ini");
    std::string invalid_ini_bytes = read_test_file(ini);
    const std::string eni_leaf = "pse_servo1.xml";
    const size_t eni_pos = invalid_ini_bytes.rfind(eni_leaf);
    ASSERT_NE(std::string::npos, eni_pos);
    invalid_ini_bytes.replace(eni_pos, eni_leaf.size(), "missing_selected_eni.xml");
    ASSERT_TRUE(invalid_ini.write(invalid_ini_bytes));

    const std::string previous_eni_path = ctx.selected_eni_path();
    const std::string previous_eni_hash = ctx.selected_eni_hash();
    ASSERT_EQ(0, ctx.ini_select(invalid_ini.path()));
    EXPECT_EQ(invalid_ini.path(), ctx.selected_ini_path());
    EXPECT_EQ(hash_bytes(read_test_file(invalid_ini.path())), ctx.selected_ini_hash());
    EXPECT_EQ(previous_eni_path, ctx.selected_eni_path());
    EXPECT_EQ(previous_eni_hash, ctx.selected_eni_hash());

    const std::string selected_ini_path = ctx.selected_ini_path();
    const std::string selected_ini_hash = ctx.selected_ini_hash();
    EXPECT_EQ(static_cast<int>(ZMC_ERR_INI_FILE_OPEN),
              ctx.ini_select(invalid_ini.path() + ".missing"));
    EXPECT_EQ(selected_ini_path, ctx.selected_ini_path());
    EXPECT_EQ(selected_ini_hash, ctx.selected_ini_hash());
    EXPECT_EQ(previous_eni_path, ctx.selected_eni_path());
    EXPECT_EQ(previous_eni_hash, ctx.selected_eni_hash());
}
