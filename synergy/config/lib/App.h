#pragma once

#include <cxxopts.hpp>
#include <QApplication>

class App {
public:
    int run(int argc, char* argv[]);

private:
    void restart(QApplication& app, std::vector<std::string> args);
#ifdef Q_OS_OSX
    void stopService();
    void installAndStartService();
#endif
};

extern cxxopts::Options g_options;
