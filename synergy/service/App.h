#pragma once

#include <cxxopts.hpp>

class App
{
public:
    int run(int argc, char* argv[]);
    static cxxopts::Options& options();

private:
    static cxxopts::Options s_options;
};
