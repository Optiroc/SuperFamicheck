#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct sfcRom {
    sfcRom(const std::string& path);

    std::string description(bool silent) const;
    std::string fix(const std::string& path, bool silent);

    bool isValid = false;
    bool hasIssues = false;
    bool hasSevereIssues = false;

    bool hasCopierHeader = false;
    bool hasCorrectTitle = false;
    bool hasCorrectRamSize = false;
    bool hasCorrectChecksum = false;
    bool hasLegalMode = false;
    bool hasKnownMapper = false;
    bool hasNewFormatHeader = false;

    std::string title;
    std::string mapperName;
    std::string chipSetInfo;
    std::string makerCode;
    std::string gameCode;
    std::string version;
    std::string country;

    uint8_t mode = 0;
    uint8_t mapper = 0;
    bool fast = false;
    bool hasRam = false;

    uint8_t chipset = 0;
    uint8_t chipsetSubtype = 0;
    uint8_t romSize = 0;
    uint8_t ramSize = 0;
    uint8_t countryCode = 0;

    uint16_t checksum = 0;
    uint16_t complement = 0;

    size_t imageSize = 0;
    size_t imageOffset = 0;
    size_t headerLocation = 0;

    uint8_t correctedMode = 0;
    uint8_t correctedRomSize = 0;
    uint16_t correctedChecksum = 0;
    uint16_t correctedComplement = 0;

  private:
    std::string filepath;
    std::vector<uint8_t> image;

    void getHeaderInfo(const std::vector<uint8_t>& header);
    int scoreHeaderLocation(size_t location) const;
    uint16_t calculateChecksum() const;
};
