#pragma once

enum class ProcessMode {
    kServer,
    kClient,
    kUnknown
};

static inline char const*
processModeToString (ProcessMode const mode) {
    switch (mode) {
        case ProcessMode::kServer:
            return "server";
        case ProcessMode::kClient:
            return "client";
        case ProcessMode::kUnknown:
            return "unknown";
    }
}
