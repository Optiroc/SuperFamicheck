#include "sfcRomInfo.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

using namespace std;

sfcRomInfo::sfcRomInfo(const string& path) {
    filepath = path;
    ifstream file(path, ios::binary|ios::in|ios::ate);
    fileSize = file.tellg();
    file.seekg(0, file.beg);

    if ((fileSize & 0x3ff) == 0x200) {
        hasCopierHeader = true;
        imageOffset = 0x200;
    }
    imageSize = fileSize - imageOffset;

    if (imageSize < 0x8000 || imageSize % 0x8000 != 0) return;

    // Decide header location and read info
    header = vector<char>(0x50, 0);
    if (getHeader(file, 0x7fb0 + imageOffset)) {
        headerLocation = 0x7fb0;
    } else if (imageSize >= 0x10000 && getHeader(file, 0xffb0 + imageOffset)) {
        headerLocation = 0xffb0;
    } else {
        return;
    }

    // ROM image seems valid
    valid = true;

    // Calculate correct checksum
    // Compare with header checksum

    int checksumLocation = headerLocation + 0x2c;
    uint16_t sum = 0;
    char byte;
    int offset = 0;
    while (file.read(&byte, sizeof(char))) {
        if (!(offset >= checksumLocation && offset < checksumLocation + 4)) sum += byte & 0xff;
        ++offset;
    }

    //checksum = sum + 0x1fe; // Checksum + complement adds up to 0xff * 2
    //complement = ~checksum;
}


string sfcRomInfo::description() {
    ostringstream os;
    if (valid) {
        os << setfill('0') << hex;
        os << "ROM info for file \"" << filepath << "\"" << endl << endl;

        os << "  Title       " << title << endl;
        if (!gameCode.empty()) {
            os << "  Game code   " << gameCode << endl;
        }
        os << "  Maker code  " << makerCode << endl;
        os << "  Country     0x" << setw(2) << static_cast<uint16_t>(countryCode) << " (" << country << ")" << endl;
        os << "  Version     " << version << endl;
        os << endl;

        os << dec;
        os << "  ROM size    " << (int)(1 << (romsize)) << "KB" << endl;
        if (ramsize) {
            os << "  RAM size    " << (int)(1 << (ramsize)) << "KB" << endl;
        }
        os << hex;
        os << "  Speed       " << (fast ? "120ns" : "200ns") << endl;

        os << "  Map mode    0x" << setw(2) << static_cast<uint16_t>(mapper);
        if (mapperName.empty()) {
            os << endl;
        } else {
            os << " (" << mapperName << ")" << endl;
        }
        os << "  Chipset     0x" << setw(2) << static_cast<uint16_t>(chipset);
        if (chipsetSubtype) {
            os << "/" << setw(2) << static_cast<uint16_t>(chipsetSubtype);
        }
        if (chipSetInfo.empty()) {
            os << endl;
        } else {
            os << " (" << chipSetInfo << ")" << endl;
        }
        os << endl;


        os << "  Checksum    0x" << setw(4) << checksum << endl;
        os << "  Complement  0x" << setw(4) << complement << endl;

        if (hasCopierHeader) {
            os << endl << "  File has a copier header (0x200 bytes)" << endl;
        }

    } else {
        os << "File \"" << filepath << "\" is not an SFC ROM image" << endl;
    }
    os << endl;
    return os.str();
}


bool sfcRomInfo::fix() {
    if (!valid) return false;

    return valid;
}


bool sfcRomInfo::getHeader(ifstream &file, int location) {
    file.seekg(location, file.beg);
    file.read(&header[0], 0x50);

    // Check mapper byte
    uint8_t mode = header[0x25];
    uint8_t mapper = mode & 0x0f;
    if (((mode & 0xe0) != 0x20) || !(mapper == 0 || mapper == 1 || mapper == 2 || mapper == 3 || mapper == 5)) {
        return false;
    }

    // TODO: Check reset vector for viable opcodes

    this->hasNewFormatHeader = (header[0x2a] == 0x33);

    this->mode = mode;
    this->mapper = mapper;
    this->fast = mode & 0x10;

    this->chipset = header[0x26];
    if (this->hasNewFormatHeader) this->chipsetSubtype = header[0x0f];
    this->romsize = header[0x27];
    this->ramsize = header[0x28];
    this->countryCode = header[0x29];

    this->complement = (uint8_t)(header[0x2c]);
    this->complement += (uint8_t)(header[0x2d]) << 8;
    this->checksum = (uint8_t)(header[0x2e]);
    this->checksum += (uint8_t)(header[0x2f]) << 8;

    this->title = string(&header[0x10], 21);

    switch (this->mapper) {
        case 0x0: this->mapperName = "LoROM"; break;
        case 0x1: this->mapperName = "HiROM"; break;
        case 0x2: this->mapperName = "Super MMC/S-DD1"; break;
        case 0x3: this->mapperName = "Super MMC/SA-1"; break;
        case 0x5: this->mapperName = "ExHiROM"; break;
        default: this->mapperName = string(); break;
    }

    if (this->hasNewFormatHeader) {
        this->makerCode = string(&header[0x00], 2);
        this->gameCode = string(&header[0x02], 4);
    } else {
        ostringstream os;
        os << "0x" << setfill('0') << setw(2) << hex << static_cast<unsigned int>((uint8_t)header[0x2a]);
        this->makerCode = os.str();
        this->gameCode = string();
    }

    this->chipSetInfo = string();
    switch (this->chipset) {
        case 0x00: this->chipSetInfo = "ROM"; break;
        case 0x01: this->chipSetInfo = "ROM/RAM"; break;
        case 0x02: this->chipSetInfo = "ROM/RAM/Battery"; break;
        case 0x03: this->chipSetInfo = "ROM/DSP"; break;
        case 0x04: this->chipSetInfo = "ROM/RAM/DSP"; break;
        case 0x05: this->chipSetInfo = "ROM/RAM/Battery/DSP"; break;
        case 0x13: this->chipSetInfo = "ROM/EXP RAM/MARIO CHIP 1"; break;
        case 0x25: this->chipSetInfo = "ROM/RAM/Battery/OBC-1"; break;
        case 0x32: this->chipSetInfo = "ROM/RAM/Battery/SA-1"; break;
        case 0x34: this->chipSetInfo = "ROM/RAM/SA-1"; break;
        case 0x35: this->chipSetInfo = "ROM/RAM/Battery/SA-1"; break;
        case 0x43: this->chipSetInfo = "ROM/S-DD1"; break;
        case 0x45: this->chipSetInfo = "ROM/RAM/Battery/S-DD1"; break;
        case 0x55: this->chipSetInfo = "ROM/RAM/Battery/S-RTC"; break;
        case 0xe3: this->chipSetInfo = "ROM/SGB"; break;
        case 0xe5: this->chipSetInfo = "ROM/BS-X"; break;

        case 0x14:
            this->chipSetInfo = this->romsize > 0x0a ? "ROM/RAM/GSU-2" : "ROM/RAM/GSU-1";
            break;
        case 0x15:
            this->chipSetInfo = this->romsize > 0x0a ? "ROM/RAM/Battery/GSU-2" : "ROM/RAM/Battery/GSU-1";
            break;
        case 0x1a:
            this->chipSetInfo = "ROM/RAM/Battery/GSU-2-SP1";
            break;

        case 0xf3:
            if (this->chipsetSubtype == 0x10) this->chipSetInfo = "ROM/CX4";
            break;
        case 0xf5:
            if (this->chipsetSubtype == 0x00) this->chipSetInfo = "ROM/RAM/Battery/SPC7110";
            if (this->chipsetSubtype == 0x02) this->chipSetInfo = "ROM/RAM/Battery/ST-018";
            break;
        case 0xf6:
            if (this->chipsetSubtype == 0x01) this->chipSetInfo = "ROM/Battery/ST-010 or ST-011";
            break;
        case 0xf9:
            if (this->chipsetSubtype == 0x00) this->chipSetInfo = "ROM/RAM/Battery/SPC7110/RTC";
            break;

        default: break;
    }

    if (this->gameCode == "XBND") this->chipSetInfo = "ROM/RAM/Battery/XBand Modem";
    if (this->gameCode == "MENU") this->chipSetInfo = "ROM/RAM/Battery/Nintendo Power";

    {
        ostringstream os;
        os << "1." << static_cast<unsigned int>((uint8_t)header[0x2b]);
        this->version = os.str();
    }

    switch (this->countryCode) {
        case 0x00: this->country = "Japan"; break;
        case 0x01: this->country = "USA"; break;
        case 0x02: this->country = "Europe"; break;
        case 0x03: this->country = "Sweden"; break;
        case 0x04: this->country = "Finland"; break;
        case 0x05: this->country = "Denmark"; break;
        case 0x06: this->country = "France"; break;
        case 0x07: this->country = "Holland"; break;
        case 0x08: this->country = "Spain"; break;
        case 0x09: this->country = "Germany"; break;
        case 0x0a: this->country = "Italy"; break;
        case 0x0b: this->country = "China/Hong Kong"; break;
        case 0x0c: this->country = "Indonesia"; break;
        case 0x0d: this->country = "South Korea"; break;
        case 0x0f: this->country = "Canada"; break;
        case 0x10: this->country = "Brazil"; break;
        case 0x11: this->country = "Australia"; break;
        default: this->country = "Unknown"; break;
    }

    return true;
}
