#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#define USE_MULTI_THREADING true
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

void searchRoutine(const fs::path &path, SearchMode mode, const boost::regex &regex);

int main(int argc, char **argv) {
    fstream file;
    file.open("/home/goforbroke/MEGA/Java __ Golang Developer.html");

    string line;
    const boost::regex my_filter("[<]+");
    boost::smatch xResults;

    while (getline(file, line)) {
        const string::const_iterator &first = line.begin();
        const string::const_iterator &last = line.end();
        if (boost::regex_search(first, last, xResults, my_filter)) {
            std::cout << "OK" << std::endl;
        } else {
            std::cout << "FAIL" << std::endl;
        }
    }
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
    *LOGGER << "Multi threading : " << (USE_MULTI_THREADING ? "ON" : "OFF") << endl;

    milliseconds beforeExec = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    const boost::regex my_filter(pattern);
    std::vector<std::thread> ts;

    fs::directory_iterator it(targetDir), end_itr;
    for (; it != end_itr; ++it) {
        if (fs::is_regular_file(it->status())) {
            if (matchFileName(it->path(), my_filter))
                std::cout << it->path().string() << std::endl;
        } else {
            if (USE_MULTI_THREADING) {
                std::thread t = std::thread(&searchRoutine, it->path(), mode, my_filter);
                ts.push_back(std::move(t));
            } else {
                searchRoutine(it->path(), mode, my_filter);
            }
        }
    }

    *LOGGER << "Used routines : " << ts.size() << endl;
    for (auto &t : ts) {
        t.join();
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
    f.open(path.filename().string());
    string line;
    while (getline(f, line)) {
        boost::smatch xResults;
        const string::const_iterator &first = line.begin();
        const string::const_iterator &last = line.end();
        if (boost::regex_search(first, last, xResults, regex)) {
            return true;
        }
    }
    return false;
}

void searchRoutine(const fs::path &path, SearchMode mode, const boost::regex &regex) {
    fs::recursive_directory_iterator it(path), end_itr;
    for (; it != end_itr; ++it) {
        if (!fs::is_regular_file(it->status())) continue;

        switch (mode) {
            case SearchMode::NAME:
                if (!matchFileName(it->path(), regex)) continue;
                break;
            case SearchMode::CONTENT:
                *LOGGER << "File : " << it->path().string() << endl;
                if (!matchFileContent(it->path(), regex))continue;
                break;
            default:
                continue;
        }
        std::cout << it->path().string() << std::endl;
    }
}
