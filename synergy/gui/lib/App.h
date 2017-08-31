#pragma once

#include <cxxopts.hpp>

class App {
public:
    App();
    int run(int argc, char* argv[]);

private:
    cxxopts::Options m_options;
};
