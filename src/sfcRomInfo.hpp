#ifndef sfcRomInfo_hpp
#define sfcRomInfo_hpp

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

struct sfcRomInfo {
    string filepath;
    int fileSize;
    int imageSize;
    int imageOffset = 0;
    int headerLocation;
    vector<char> header;

    bool valid = false;
    bool hasCopierHeader = false;
    bool hasNewFormatHeader = false;
    bool hasCorrectChecksum = false;

    uint8_t mode;
    uint8_t mapper;
    bool fast;

    uint8_t chipset;
    uint8_t chipsetSubtype = 0;
    uint8_t romsize;
    uint8_t ramsize;
    uint8_t countryCode;

    uint16_t checksum;
    uint16_t complement;
    uint16_t resetVector;

    string title;
    string mapperName;
    string chipSetInfo;
    string makerCode;
    string gameCode;
    string version;
    string country;

    uint16_t correctChecksum;
    uint16_t correctComplement;

    sfcRomInfo(const string& path);
    string description();
    bool fix();

private:
    bool getHeader(ifstream &file, int location);
};

#endif /* sfcRomInfo_hpp */
