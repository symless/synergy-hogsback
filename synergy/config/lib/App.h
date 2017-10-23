#pragma once

#include <cxxopts.hpp>
#include <QApplication>

class App {
public:
    int run(int argc, char* argv[]);

private:
    bool installService();

private:
    void restart(QApplication& app, std::vector<std::string> args);
};

extern cxxopts::Options g_options;
