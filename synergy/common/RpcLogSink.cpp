#include <synergy/common/RpcLogSink.h>
#include <synergy/common/WampClient.h>

RpcLogSink::RpcLogSink (WampClient& rpcClient, std::string const& log):
    m_rpcClient (&rpcClient),
    m_rpcLogFunction (fmt::format("synergy.log.{}", log)) {
}

void
RpcLogSink::log (spdlog::details::log_msg const& msg) {
    if (m_rpcClient->isConnected()) {
        m_rpcClient->call<void> (m_rpcLogFunction.c_str(), msg.formatted.str());
    }
}

void
RpcLogSink::flush() {
}
