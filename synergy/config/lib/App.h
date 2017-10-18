#pragma once

#include <cxxopts.hpp>
#include <QApplication>

class App {
public:
    App(bool (*installServiceHelper)());
    int run(int argc, char* argv[]);

private:
    bool installService();

private:
    bool (*m_installServiceHelper)();
    void restart(QApplication& app, std::vector<std::string> args);
};

extern cxxopts::Options g_options;
