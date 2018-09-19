#include <iostream>
#include <thread>
#include <fstream>
#include <future>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <future>
#include "threads.h"

#define ARGS_ERROR_MSG "Wrong args number. Usage: gfb_find /target/dir -name ([a-z0-9]{4})\\.txt"
#define MODE_ERROR_MSG "Unexpected mode! Try to use \"-name\" or \"-content\""

using namespace std;
using namespace std::chrono;

namespace fs = boost::filesystem;

enum class SearchMode {
    NAME,
    CONTENT,
};

fstream *LOGGER;

bool matchFileName(const fs::path &path, const boost::regex &regex);

bool matchFileContent(const fs::path &path, const boost::regex &regex);

void searchRoutine(const fs::path &path, const SearchMode &mode, const boost::regex &regex);

void writeRoutine(Channel<string> &ch, __useconds_t delay, int index) {
    while (true) {
        ch << ("hello " + std::to_string(index));
        usleep(delay);
    }
}

void readRoutine(Channel<string> &ch) {
    string s;
    while (true) {
        ch >> s;
        cout << s << endl;
    }
}

int main(int argc, char **argv) {
    Channel<string> ch(5);

    std::vector<thread> tasks;

    tasks.emplace_back(writeRoutine, std::ref(ch), 500, 1);
    tasks.emplace_back(writeRoutine, std::ref(ch), 300, 2);
    tasks.emplace_back(writeRoutine, std::ref(ch), 250, 3);

    tasks.emplace_back(readRoutine, std::ref(ch));

    for (thread & t : tasks)
        t.join();

    return 0;
}

int main2(int argc, char **argv) {
    if (argc != 4) {
        cerr << argc << " - " << ARGS_ERROR_MSG << endl;
        return -1;
    }

    fs::path progLocation(argv[0]);
    string progDir = progLocation.parent_path().string();

    LOGGER = new fstream;
    LOGGER->open(progDir + "/gfb_find.log", fstream::out);

    string targetDir = argv[1];
    SearchMode mode;
    char *modeStr = argv[2];
    if (0 == strcmp("-name", modeStr))
        mode = SearchMode::NAME;
    else if (0 == strcmp("-content", modeStr))
        mode = SearchMode::CONTENT;
    else {
        cerr << MODE_ERROR_MSG << endl;
        return -1;
    }
    string pattern = argv[3];

    *LOGGER << "Target dir : " << targetDir << endl;
    *LOGGER << "Mode : " << modeStr << endl;
    *LOGGER << "Pattern : " << pattern << endl;

    milliseconds beforeExec = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    boost::regex my_filter(pattern);
    Semaphore threadLimiter(std::thread::hardware_concurrency());
    std::vector<std::future<void>> tasks;

    tasks.emplace_back(std::async([&threadLimiter, targetDir, &mode, &my_filter] {
        CriticalSection cs(threadLimiter);
        searchRoutine(fs::path(targetDir), mode, my_filter);
    }));

    fs::directory_iterator it(targetDir), end_itr;
    for (; it != end_itr; ++it) {
        if (!fs::is_regular_file(it->status())) {
            const boost::filesystem::path &path = it->path();
            tasks.emplace_back(std::async([&threadLimiter, &path, &mode, &my_filter] {
                CriticalSection cs(threadLimiter);
                searchRoutine(path, mode, my_filter);
            }));
        }
    }

    *LOGGER << "Used routines : " << tasks.size() << endl;
    for (auto &task : tasks) {
        task.get();
    }

    milliseconds afterExec = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    *LOGGER << "Spend time : " << (afterExec - beforeExec).count() << " milliseconds" << endl;

    LOGGER->close();

    return 0;
}

bool matchFileName(const fs::path &path, const boost::regex &regex) {
    static boost::smatch what;
    const string &filename = path.filename().string();
    return boost::regex_match(filename, what, regex);
}

bool matchFileContent(const fs::path &path, const boost::regex &regex) {
    fstream f;
    f.open(path.string());
    string line;
    boost::smatch xResults;
    while (getline(f, line)) {
        const string::const_iterator &first = line.begin();
        const string::const_iterator &last = line.end();
        if (boost::regex_search(first, last, xResults, regex)) {
            return true;
        }
    }
    return false;
}

void searchRoutine(const fs::path &path, const SearchMode &mode, const boost::regex &regex) {
    fs::recursive_directory_iterator it(path), end_itr;
    for (; it != end_itr; ++it) {
        if (!fs::is_regular_file(it->status()))
            continue;
        switch (mode) {
            case SearchMode::NAME:
                if (!matchFileName(it->path(), regex))
                    continue;
                break;
            case SearchMode::CONTENT:
                if (!matchFileContent(it->path(), regex))
                    continue;
                break;
            default:
                continue;
        }
        std::cout << it->path().string() << std::endl;
    }
}
