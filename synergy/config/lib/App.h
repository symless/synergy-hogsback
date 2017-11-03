#pragma once

#include <cxxopts.hpp>
#include <QApplication>

class App {
public:
    int run(int argc, char* argv[]);
    static cxxopts::Options& options();

private:
    void restart(QApplication& app, std::vector<std::string> args);
#ifdef Q_OS_OSX
    void startService();
    void stopService();
    void installAndStartService();
#endif

private:
    static cxxopts::Options s_options;
};
