/*
 * @Author: ChenSy
 * @Date: 2026-05-28
 * @Description: HTTP client demo for motion API testing
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "common/thirdlib/yhirose/httplib.h"
#include "common/zmc/zmc_predef.h"

static const char* EXE_NAME = "zmc_motapp_demo";

static bool parsePortArg(const std::string& text, int& port) {
    char* end = nullptr;
    long value = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || value <= 0 || value > 65535) {
        return false;
    }
    port = static_cast<int>(value);
    return true;
}

static std::string toUpperAscii(std::string text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = static_cast<char>(text[i] - 'a' + 'A');
        }
    }
    return text;
}

static void printHttpClientUsage() {
    std::cout << "Usage: " << EXE_NAME
              << " http_client [host] [port] [path] [method] [body] [--session ID]\n"
              << "Example: " << EXE_NAME
              << " http_client 127.0.0.1 " ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR
              << " /zmot/api/status_latest\n"
              << "Example: " << EXE_NAME
              << " http_client 127.0.0.1 " ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR
              << " /zmot/api/flow_config POST \"{}\"\n";
}

static httplib::Result sendHttpClientRequest(httplib::Client& cli, const std::string& method,
                                             const std::string& path, const std::string& body) {
    if (method == "GET") {
        return cli.Get(path);
    }
    if (method == "POST") {
        return cli.Post(path, body, "application/json");
    }
    if (method == "PUT") {
        return cli.Put(path, body, "application/json");
    }
    if (method == "PATCH") {
        return cli.Patch(path, body, "application/json");
    }
    if (method == "DELETE") {
        if (body.empty()) {
            return cli.Delete(path);
        }
        return cli.Delete(path, body, "application/json");
    }

    return httplib::Result(nullptr, httplib::Error::Unknown);
}

int runHttpClient_demo(const std::vector<std::string>& inputArgs) {
    // AI modification signature: GPT-6. This demo sends exactly the requested operation.
    std::vector<std::string> args;
    std::string session;
    for (size_t i = 0; i < inputArgs.size(); ++i) {
        if (inputArgs[i] == "--session") {
            if (++i == inputArgs.size()) {
                printHttpClientUsage();
                return -1;
            }
            session = inputArgs[i];
        } else if (inputArgs[i].find("--session=") == 0) {
            session = inputArgs[i].substr(10);
        } else {
            args.push_back(inputArgs[i]);
        }
    }
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        printHttpClientUsage();
        return 0;
    }

    std::string host = "127.0.0.1";
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    std::string path = "/zmot/api/status_latest";
    std::string method = "GET";
    std::string body;

    if (args.size() > 0) {
        host = args[0];
    }
    if (args.size() > 1 && !parsePortArg(args[1], port)) {
        std::cerr << "Invalid port: " << args[1] << std::endl;
        printHttpClientUsage();
        return -1;
    }
    if (args.size() > 2) {
        path = args[2];
    }
    if (args.size() > 3) {
        method = toUpperAscii(args[3]);
    }
    if (args.size() > 4) {
        body = args[4];
    }
    if (args.size() > 5) {
        std::cerr << "Too many arguments." << std::endl;
        printHttpClientUsage();
        return -1;
    }

    httplib::Client cli(host, port);
    if (!session.empty()) cli.set_default_headers({{"X-Zmot-Session", session}});
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(2, 0);
    cli.set_write_timeout(2, 0);

    std::cout << method << " http://" << host << ":" << port << path << std::endl;
    httplib::Result res = sendHttpClientRequest(cli, method, path, body);
    if (!res) {
        std::cerr << "HTTP request failed: " << httplib::to_string(res.error()) << std::endl;
        return -1;
    }

    std::cout << "Status: " << res->status << std::endl;
    std::cout << res->body << std::endl;
    return res->status >= 200 && res->status < 300 ? 0 : -1;
}
