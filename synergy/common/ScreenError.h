#pragma once

#include <stdint.h>

enum class ScreenError: uint32_t {
    kRouterUnreachableNode = 0x10000001,
    kCoreZombieProcess = 0x20000001,
    kCoreDuplicateName = 0x20000002,
    kGeneral = 0x30000000
};
