#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <random>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>


using namespace std;

namespace fs = boost::filesystem;

//

void search_routine(fs::path p, const char *pattern) {
//    std::cout << "Thread number: " << number << std::endl;

    const boost::regex my_filter(pattern);
    boost::filesystem::directory_iterator it(p), end_itr;
    for (; it != end_itr; ++it) {
        if (!fs::is_regular_file(it->status())) continue;
        boost::smatch what;
        const string &filename = it->path().filename().string();
        if (!boost::regex_match(filename, what, my_filter)) continue;

        std::cout << it->path().string() << std::endl;
    }
}

int main(int argc, char **argv) {

    if (argc == 1) {
        cerr << "Wrong args number. Usage: gfb_find /target/dir ([a-z0-9]{4})\\.txt log" << endl;
    }

    string targetDir;
    vector<string> patterns;

    if (argc == 2) {
        targetDir = argv[0]; // TODO: get parent dir
        patterns.emplace_back(argv[1]);
    } else if (argc >= 3) {
        targetDir = argv[1];
        for (int i = 2; i < argc; ++i) {
            patterns.emplace_back(argv[i]);
        }
    }

//    fs::directory_iterator it{targetDir}, end;
//    for (; it != end; ++it)
//        cout << it->path() << endl;

    search_routine(fs::path("/home/goforbroke/scripts"), ".*\\.sh");

//    cout << argv[0] << endl;
//    cout << argv[1] << endl;
//    cout << argv[2] << endl;

//    unsigned int concurrentNum = std::thread::hardware_concurrency() * 2;
//
//    const string path = "/home/goforbroke/";
//    const fs::directory_iterator &range = fs::directory_iterator(path);
//
//    std::vector<std::thread> ts;
//    for (unsigned int i = 0; i < concurrentNum; ++i) {
//        std::thread t = std::thread(&search_routine, i, range);
//        ts.push_back(std::move(t));
//    }
//    for (auto &t : ts) {
//        t.join();
//    }
    return 0;
}