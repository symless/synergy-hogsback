#pragma once

// clang-format off
#include <synergy/service/router/utils/copy_ptr.hpp>
#include <synergy/service/router/protocol/v2/Hello_generated.h>
#include <synergy/service/router/protocol/v2/Route_generated.h>
#include <synergy/service/router/protocol/v2/Proxy_generated.h>
#include <synergy/service/router/protocol/v2/Core_generated.h>
// clang-format on

namespace synergy {
namespace protocol {
namespace v2 {

using HelloMessage       = fb::HelloT;
using Route              = fb::RouteT;
using RouteAdvertisement = fb::RouteAdvertisementT;
using ProxyClientConnect = fb::ProxyClientConnectT;
using ProxyServerClaim   = fb::ProxyServerClaimT;
using CoreMessage        = fb::CoreMessageT;

struct RouteRevocation : RouteAdvertisement {};

} // namespace v2
} // namespace protocol
} // namespace synergy
