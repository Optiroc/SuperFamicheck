#ifndef sfcRom_hpp
#define sfcRom_hpp

#include <cstdint>
#include <string>
#include <vector>

struct sfcRom {
    sfcRom(const std::string& path);

    bool                    isValid() const;
    std::string             description(bool silent) const;
    std::string             fix(const std::string& path, bool silent);

private:
    std::string             filepath;
    std::vector<uint8_t>    image;

    void            getHeaderInfo(const std::vector<uint8_t> &header);
    int             scoreHeaderLocation(int location) const;
    uint16_t        calculateChecksum() const;

    int             imageSize;
    int             imageOffset = 0;
    int             headerLocation;

    bool            valid = false;
    bool            hasIssues = false;
    bool            hasSevereIssues = false;

    bool            hasCopierHeader = false;
    bool            hasCorrectTitle = false;
    bool            hasCorrectRamSize = true;
    bool            hasLegalMode = true;
    bool            hasKnownMapper = true;
    bool            hasNewFormatHeader = false;

    uint8_t         correctedMode = 0;
    uint8_t         correctedRomSize = 0;
    uint16_t        correctedChecksum;
    uint16_t        correctedComplement;

    uint8_t         mode;
    uint8_t         mapper;
    bool            fast;
    bool            hasRam = false;

    uint8_t         chipset;
    uint8_t         chipsetSubtype = 0;
    uint8_t         romSize;
    uint8_t         ramSize;
    uint8_t         countryCode;

    uint16_t        checksum;
    uint16_t        complement;

    std::string     title;
    std::string     mapperName;
    std::string     chipSetInfo;
    std::string     makerCode;
    std::string     gameCode;
    std::string     version;
    std::string     country;
};

#endif /* sfcRom_hpp */
