#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

using namespace std;
using namespace g3;

struct ColorCoutSink {

enum FG_Color {YELLOW = 33, RED = 31, GREEN=32, WHITE = 97};

    FG_Color GetColor(const LEVELS level) const
    {
        if (level.value == WARNING.value) { return YELLOW; }
        if (level.value == DEBUG.value) { return GREEN; }
        if (g3::internal::wasFatal(level)) { return RED; }

        return WHITE;
    }
    void ReceiveLogMessage(g3::LogMessageMover logEntry)
    {
        auto level = logEntry.get()._level;
        auto color = GetColor(level);

        std::cout << "\033[" << color << "m" << logEntry.get().toString() << "\033[m" << std::endl;
    }
};

int main()
{

    auto log_worker = LogWorker::createLogWorker();

    // logger is initialized
    g3::initializeLogging(log_worker.get());

    auto sinkHandle = log_worker->addSink(std::make_unique<ColorCoutSink>(),
                                     &ColorCoutSink::ReceiveLogMessage);
    LOG(DEBUG) << "g3 log test";

    return 0;
}
