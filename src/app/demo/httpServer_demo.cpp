#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
/*
 * @Author: ChenShengyong
 * @Date: 2026-01-21
 * @Description: HTTP REST API server for motion control
 */
#include "AxisFbGraph.hpp"
#include "qcmd/MotionCmdInterface.hpp"
#include "pou/axPouTmplRunner.hpp"
#include <thread>
#include <iostream>
#include <string>
#include "HttpScriptHelper.hpp"

int runHttpServer_demo(int argc, char** argv) {
    using namespace zmc;
    std::string scriptPath;
    const bool hasScript = parse_script_path_from_argv(argc, argv, scriptPath);
    // The HTTP demo honors the same --port flag as the axis_ini run mode.
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    for (int i = 0; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg.rfind("--port=", 0) == 0) {
            const std::string valueStr = arg.substr(7);
            if (!parse_port_value(valueStr, &port)) {
                std::cerr << "Error: invalid --port value: " << valueStr << std::endl;
                return -1;
            }
        }
    }
    auto pRunner = make_pou_config<AxisFbGraphSimu>(argc, argv, "");
    if (!pRunner) {
        std::cerr << "Failed to initialize AxisFbGraphSimu" << std::endl;
        return -1;
    }
    auto& axfbGrp = *pRunner;
    ServerWrap svrWrap;
    // Bind the requested port before probe/run so the CLI port cannot be ignored.
    svrWrap.port = port;
    if (hasScript) {
        std::string scriptContent;
        if (!read_text_file_all(scriptPath, scriptContent)) {
            std::cerr << "Error: failed to open script file: " << scriptPath << std::endl;
            return -1;
        }
        if (scriptContent.empty()) {
            std::cerr << "Error: script file is empty: " << scriptPath << std::endl;
            return -1;
        }
        std::cout << "[http-script] startup script: " << scriptPath << std::endl;
        svrWrap.setStartupScript(scriptContent);
    }
    MotionCmdInterface cmdIf(axfbGrp.fbGrp.getCmdInterface());
    svrWrap.bind(cmdIf);
    if (svrWrap.probePort() != 0) {
        cmdIf.sendExitFlag();
        return -1;
    }
    int httpRet = 0;
    std::thread httpThread([&svrWrap, &httpRet]() { httpRet = svrWrap.run(); });
    std::cout << "Starting motion control loop..." << std::endl;
    axfbGrp.run();
    svrWrap.stop();
    cmdIf.sendExitFlag();
    if (httpThread.joinable()) {
        httpThread.join();
    }
    return httpRet != 0 || (hasScript && svrWrap.hasPresetFailed()) ? -1 : 0;
}
#else
	int runHttpServer_demo(int argc, char** argv) {
		return -1;
	}
#endif
