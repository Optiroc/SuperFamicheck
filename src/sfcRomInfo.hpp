#ifndef sfcRomInfo_hpp
#define sfcRomInfo_hpp

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

struct sfcRomInfo {
    string filepath;
    int imageSize;
    int imageOffset = 0;
    int headerLocation;
    bool hasNewFormatHeader = false;

    bool valid = false;
    bool hasCopierHeader = false;
    bool hasIssues = false;

    uint8_t mode;
    uint8_t mapper;
    bool fast;

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

    bool correctTitle = false;
    uint8_t correctMode = 0;
    uint8_t correctRomSize = 0;
    uint16_t correctChecksum;
    uint16_t correctComplement;

    sfcRomInfo(const string& path);
    string description();
    bool fix();

private:
    int scoreHeaderLocation(const vector<uint8_t> &image, int location);
    void getHeaderInfo(const vector<uint8_t> &header);
};

#endif /* sfcRomInfo_hpp */
