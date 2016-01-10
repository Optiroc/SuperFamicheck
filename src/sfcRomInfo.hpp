#ifndef sfcRomInfo_hpp
#define sfcRomInfo_hpp

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

struct sfcRomInfo {
    string filepath;
    vector<uint8_t> image;
    int imageSize;
    int imageOffset = 0;
    int headerLocation;

    bool valid = false;
    bool hasIssues = false;
    bool hasSevereIssues = false;

    bool hasCopierHeader = false;
    bool hasCorrectTitle = false;
    bool hasCorrectRamSize = true;
    bool hasLegalMode = true;
    bool hasKnownMapper = true;
    bool hasNewFormatHeader = false;

    uint8_t correctedMode = 0;
    uint8_t correctedRomSize = 0;
    uint16_t correctedChecksum;
    uint16_t correctedComplement;


    uint8_t mode;
    uint8_t mapper;
    bool fast;
    bool hasRam = false;

    uint8_t chipset;
    uint8_t chipsetSubtype = 0;
    uint8_t romSize;
    uint8_t ramSize;
    uint8_t countryCode;

    uint16_t checksum;
    uint16_t complement;

    string title;
    string mapperName;
    string chipSetInfo;
    string makerCode;
    string gameCode;
    string version;
    string country;


    sfcRomInfo(const string& path);
    string description(bool silent);
    string fix(const string& path, bool silent);

private:
    int scoreHeaderLocation(const vector<uint8_t> &image, int location);
    void getHeaderInfo(const vector<uint8_t> &header);
    uint16_t calculateChecksum(const vector<uint8_t> &image);
};

#endif /* sfcRomInfo_hpp */
