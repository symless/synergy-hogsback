#pragma once

#include <cxxopts.hpp>

class App {
public:
    App(bool (*installServiceHelper)());
    int run(int argc, char* argv[]);

private:
    bool installService();

private:
    bool (*m_installServiceHelper)();
};

extern cxxopts::Options g_options;
