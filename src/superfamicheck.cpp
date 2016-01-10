#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "sfcRom.hpp"
#include "ezOptionParser/ezOptionParser.hpp"

bool fileAvailable(const std::string& path) {
    ifstream file(path, ios::in);
    return file.good();
}

int main(int argc, const char * argv[]) {
    ez::ezOptionParser opt;
    opt.overview = "SuperFamicheck 0.2";
    opt.syntax = "superfamicheck rom_file [options...]";

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

    if (!opt.gotExpected(badOptions)) {
        for(int i=0; i < badOptions.size(); ++i) {
            cerr << "Missing argument for option: " << badOptions[i] << endl << endl;
        }
        std::cout << usage;
        return 1;
    }

    if (!opt.gotValid(badOptions, badArgs)) {
        for(int i=0; i < badOptions.size(); ++i) {
            cerr << "Invalid argument for option: " << badOptions[i] << endl << endl;
        }
        std::cout << usage;
        return 1;
    }

    string inputPath = string();
    if (opt.firstArgs.size() > 1 || opt.lastArgs.size() > 0) {
        inputPath = opt.firstArgs.size() > 1 ? *opt.firstArgs.back() : *opt.lastArgs.front();
        if (!fileAvailable(inputPath)) {
            cerr << "Cannot open file \"" << inputPath << "\"" << endl;
            return 1;
        }
    } else {
        cerr << "Missing required argument: rom_file" << endl << endl;
        std::cout << usage;
        return 1;
    }

    sfcRom rom(inputPath);

    if (!verysilent) cout << rom.description(silent);

    if (rom.valid && opt.isSet("-f")) {
        string outputPath = inputPath;
        if (opt.isSet("-o")) opt.get("-o")->getString(outputPath);

        string fixDescripton = rom.fix(outputPath, silent);
        if (!verysilent) cout << fixDescripton;
    }

    return 0;
}
