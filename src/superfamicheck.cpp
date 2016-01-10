#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "sfcRomInfo.hpp"
#include "ezOptionParser/ezOptionParser.hpp"

bool fileAvailable(const std::string& path) {
    ifstream file(path, ios::in);
    return file.good();
}

int main(int argc, const char * argv[]) {
    ez::ezOptionParser opt;
    opt.overview = "SuperFamicheck 0.1";
    opt.syntax = "superfamicheck [options...]";

    opt.add("",     // Default
            true,   // Required
            1,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Input ROM image path",
            "-i", "--in"
            );

    opt.add("",     // Default
            false,  // Required
            0,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Fix ROM image",
            "-f", "--fix"
            );

    opt.add("",     // Default
            false,  // Required
            1,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Output ROM image path",
            "-o", "--out"
            );

    opt.add("",     // Default
            false,  // Required
            0,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Silent operation (unless issues found)",
            "-s", "--semisilent"
            );

    opt.add("",     // Default
            false,  // Required
            0,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Silent operation",
            "-S", "--silent"
            );

    opt.add("",     // Default
            false,  // Required
            0,      // Number of args expected
            0,      // Delimiter if expecting multiple args
            "Display instructions",
            "-h", "--help");

    string usage;
    vector<string> badOptions, badArgs;
    opt.parse(argc, argv);
    opt.getUsage(usage);

    if (opt.isSet("-h")) {
        cout << usage;
        return 0;
    }

    bool silent = opt.isSet("-s");
    bool verysilent = opt.isSet("-S");

    if (!opt.gotRequired(badOptions)) {
        for(int i=0; i < badOptions.size(); ++i) {
            cerr << "Missing required option: " << badOptions[i] << endl << endl;
        }
        std::cout << usage;
        return 1;
    }

    if (!opt.gotValid(badOptions, badArgs)) {
        for(int i=0; i < badOptions.size(); ++i) {
            cerr << "Unexpected number of arguments for option: " << badOptions[i] << endl << endl;
        }
        std::cout << usage;
        return 1;
    }

    string inputPath;
    opt.get("-i")->getString(inputPath);
    if (!fileAvailable(inputPath)) {
        cerr << "Cannot open file \"" << inputPath << "\"" << endl;
        return 1;
    }

    sfcRomInfo romInfo(inputPath);

    if (!verysilent) cout << romInfo.description(silent);

    if (romInfo.valid && opt.isSet("-f")) {
        string outputPath = inputPath;
        if (opt.isSet("-o")) opt.get("-o")->getString(outputPath);

        string fixDescripton = romInfo.fix(outputPath, silent);
        if (!verysilent) cout << fixDescripton;
    }

    return 0;
}