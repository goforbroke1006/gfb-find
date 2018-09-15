#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <random>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <chrono>

#define ARGS_ERROR_MSG "Wrong args number. Usage: gfb_find /target/dir ([a-z0-9]{4})\\.txt"

using namespace std;
using namespace std::chrono;

namespace fs = boost::filesystem;

bool match_file_name(const fs::path &_path, const boost::regex &regex) {
    static boost::smatch what;
    const string &filename = _path.filename().string();
    return boost::regex_match(filename, what, regex);
}

void search_routine(const fs::path &path, const boost::regex &regex) {
    fs::recursive_directory_iterator it(path), end_itr;
    for (; it != end_itr; ++it) {
        if (!fs::is_regular_file(it->status())) continue;
        if (!match_file_name(it->path(), regex)) continue;
        std::cout << it->path().string() << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc == 1 || argc > 3) {
        cerr << ARGS_ERROR_MSG << endl;
        return -1;
    }

    fs::path progLocation(argv[0]);
    string progDir = progLocation.parent_path().string();

    fstream LOGGER;
    LOGGER.open(progDir + "/gfb_find.log", fstream::out);

    string targetDir;
    string pattern;

    if (argc == 2) {
        targetDir = progDir;
        pattern = argv[1];
    } else if (argc == 3) {
        targetDir = argv[1];
        pattern = argv[2];
    }

    LOGGER << "Target dir : " << targetDir << endl;
    LOGGER << "Pattern : " << pattern << endl;

    milliseconds beforeExec = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    const boost::regex my_filter(pattern);
    std::vector<std::thread> ts;

    fs::directory_iterator it(targetDir), end_itr;
    for (; it != end_itr; ++it) {
        if (fs::is_regular_file(it->status())) {
            if (match_file_name(it->path(), my_filter))
                std::cout << it->path().string() << std::endl;
        } else {
//            search_routine(it->path(), my_filter);
            std::thread t = std::thread(&search_routine, it->path(), my_filter);
            ts.push_back(std::move(t));
        }
    }

    LOGGER << "Used routines : " << ts.size() << endl;
    for (auto &t : ts) {
        t.join();
    }

    milliseconds afterExec = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    LOGGER << "Spend time : " << (afterExec - beforeExec).count() << " milliseconds" << endl;

    LOGGER.close();

    return 0;
}