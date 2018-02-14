#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include <string>

class WampClient;

class RpcLogSink final: public spdlog::sinks::sink {
public:
    RpcLogSink (WampClient& rpcClient, std::string const& log);
    RpcLogSink (RpcLogSink&&) noexcept = delete;
    RpcLogSink& operator= (RpcLogSink&&) noexcept = delete;

protected:
    void log (spdlog::details::log_msg const& msg) override;
    void flush() override;

private:
    WampClient* m_rpcClient = nullptr;
    std::string m_rpcLogFunction;
};
