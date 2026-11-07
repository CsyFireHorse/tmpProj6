#ifndef ZML_HTTP_CONTEXT_HPP
#define ZML_HTTP_CONTEXT_HPP

/* Shared by the HTTP C API implementations. GPT-6 Codex, 2026-11-05. */
namespace httplib { class Client; }

struct ZML_Context {
    httplib::Client* client;
    int connectionState;
};

#endif /* ZML_HTTP_CONTEXT_HPP */
