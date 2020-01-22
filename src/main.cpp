#include <iostream>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <roaring/roaring.hh>

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
//    auto log_worker = LogWorker::createLogWorker();

//    // logger is initialized
//    g3::initializeLogging(log_worker.get());

//    auto sinkHandle = log_worker->addSink(std::make_unique<ColorCoutSink>(),
//                                     &ColorCoutSink::ReceiveLogMessage);
//    LOG(DEBUG) << "g3 log test";

    Roaring r1;
//    for (uint32_t i = 1; i < 10; i++) {
//      r1.add(i);
//    }
    r1.add(5);
    r1.add(1);
    r1.add(8);
    r1.add(9);
    r1.add(3);
    for(Roaring::const_iterator i = r1.begin() ; i != r1.end() ; i++) {
       cout<<*i<<endl;
    }


    return 0;
}
