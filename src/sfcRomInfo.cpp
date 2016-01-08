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

uint8_t getByte(const vector<uint8_t> &vec, int offset);
uint16_t getWord(const vector<uint8_t> &vec, int offset);
string sjisToString(uint8_t code);

sfcRomInfo::sfcRomInfo(const string& path) {
    filepath = path;

    // Read file into buffer
    vector<uint8_t> image(0);
    {
        ifstream file(path, ios::binary|ios::ate);
        if (file) {
            int fileSize = file.tellg();
            if ((fileSize & 0x3ff) == 0x200) {
                hasCopierHeader = true;
                imageOffset = 0x200;
            }

            imageSize = fileSize - imageOffset;
            if (imageSize < 0x8000 || imageSize > 0xc00000 || imageSize % 0x8000 != 0) return;

            file.seekg(imageOffset, ios::beg);
            image.resize(imageSize);
            file.read((char*) &image[0], imageSize);
        } else {
            return;
        }
    }

    // Decide header location
    vector<uint8_t> header(0x50, 0);
    vector<int> possibleHeaderLocations = { 0x7fb0, 0xffb0 };

    for (int loc : possibleHeaderLocations) {
        header = vector<uint8_t>(image.begin() + loc, image.begin() + loc + 0x50);
        uint8_t mode = header[0x25];
        uint8_t mapper = mode & 0x0f;
        uint16_t reset = getWord(header, 0x4c);

        // TODO? Set scores for "known mapper", "initial opcode" and "correct title".. valid if at least two tests pass

        if (((mode & 0xe0) == 0x20) && (mapper == 0x0 || mapper == 0x1 || mapper == 0x2 || mapper == 0x3 || mapper == 0x5 || mapper == 0xa)) {

            // If 32K/bank mapper, reset vector must point to upper half
            if (mapper == 0 || mapper == 2 || mapper == 3) {
                if (reset < 0x8000) {
                    continue;
                } else {
                    reset -= 0x8000;
                }
            }

            // Check first opcode
            // 0x18 clc, 0x4c jmp, 0x5c jml, 0x78 sei, 0x9c stz, 0xe2 sep
            uint8_t op = image[reset];
            if (op == 0x18 || op == 0x4c || op == 0x5c || op == 0x78 || op == 0x9c || op == 0xe2) {
                valid = true;
                headerLocation = loc;
                break;
            } else {
                cout << "reset vector: 0x" << setfill('0') << hex << setw(4) << reset << endl;
                cout << "first opcode: 0x" << setfill('0') << hex << setw(2) << static_cast<uint16_t>(op) << endl;
            }
        }
    }
    if (!valid) return;

    getHeaderInfo(header);

    // Fix romSize

    // Calculate correct checksum
    {
        int rsize = (int)((1 << (romSize)) * 1024);
        int chkloc = headerLocation + 0x2c;
        uint16_t sum = 0;
        for (int i = 0; i < rsize; ++i) {
            if (!(i >= chkloc && i < chkloc + 4)) sum += getByte(image, i) & 0xff;
        }

        correctChecksum = sum + 0x1fe;
        correctComplement = ~correctChecksum;
    }

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

        int romSizeKb = (int)(1 << (romSize));
        int imageSizeKb = (int)(imageSize / 1024);
        os << dec;
        os << "  ROM size    " << romSizeKb << "KB";
        if (romSizeKb != imageSizeKb) {
            os << " (Actual size is " << (int)(imageSize / 1024) << "KB)" << endl;
        } else {
            os << endl;
        }
        if (ramSize) {
            os << "  RAM size    " << (int)(1 << (ramSize)) << "KB" << endl;
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


        os << "  Checksum    0x" << setw(4) << checksum << " : " << correctChecksum << endl;
        os << "  Complement  0x" << setw(4) << complement << " : " << correctComplement << endl;

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


void sfcRomInfo::getHeaderInfo(const vector<uint8_t> &header) {
    mode = header[0x25];
    mapper = mode & 0x0f;
    fast = mode & 0x10;

    hasNewFormatHeader = (header[0x2a] == 0x33);

    chipset = header[0x26];
    if (hasNewFormatHeader) chipsetSubtype = header[0x0f];
    romSize = header[0x27];
    ramSize = header[0x28];
    countryCode = header[0x29];

    complement = getWord(header, 0x2c);
    checksum = getWord(header, 0x2e);

    title = string();
    for (int i = 0x10; i < 0x10 + 21; ++i) title += sjisToString(header[i]);

    switch (mapper) {
        case 0x0: mapperName = "LoROM"; break;
        case 0x1: mapperName = "HiROM"; break;
        case 0x2: mapperName = "LoROM/S-DD1"; break;
        case 0x3: mapperName = "LoROM/SA-1"; break;
        case 0x5: mapperName = "Extended HiROM"; break;
        case 0xa: mapperName = "Extended HiROM/SPC7110"; break;
        default: mapperName = string(); break;
    }

    if (hasNewFormatHeader) {
        makerCode = string((char*)&header[0x00], 2);
        gameCode = string((char*)&header[0x02], 4);
    } else {
        ostringstream os;
        os << "0x" << setfill('0') << setw(2) << hex << static_cast<unsigned int>((uint8_t)header[0x2a]);
        makerCode = os.str();
        gameCode = string();
    }

    chipSetInfo = string();
    switch (chipset) {
        case 0x00: chipSetInfo = "ROM"; break;
        case 0x01: chipSetInfo = "ROM/RAM"; break;
        case 0x02: chipSetInfo = "ROM/RAM/Battery"; break;
        case 0x03: chipSetInfo = "ROM/DSP"; break;
        case 0x04: chipSetInfo = "ROM/RAM/DSP"; break;
        case 0x05: chipSetInfo = "ROM/RAM/Battery/DSP"; break;
        case 0x13: chipSetInfo = "ROM/EXP RAM/MARIO CHIP 1"; break;
        case 0x25: chipSetInfo = "ROM/RAM/Battery/OBC-1"; break;
        case 0x32: chipSetInfo = "ROM/RAM/Battery/SA-1"; break;
        case 0x34: chipSetInfo = "ROM/RAM/SA-1"; break;
        case 0x35: chipSetInfo = "ROM/RAM/Battery/SA-1"; break;
        case 0x43: chipSetInfo = "ROM/S-DD1"; break;
        case 0x45: chipSetInfo = "ROM/RAM/Battery/S-DD1"; break;
        case 0x55: chipSetInfo = "ROM/RAM/Battery/S-RTC"; break;
        case 0xe3: chipSetInfo = "ROM/SGB"; break;
        case 0xe5: chipSetInfo = "ROM/BS-X"; break;

        case 0x14:
            chipSetInfo = romSize > 0x0a ? "ROM/RAM/GSU-2" : "ROM/RAM/GSU-1";
            break;
        case 0x15:
            chipSetInfo = romSize > 0x0a ? "ROM/RAM/Battery/GSU-2" : "ROM/RAM/Battery/GSU-1";
            break;
        case 0x1a:
            chipSetInfo = "ROM/RAM/Battery/GSU-2-SP1";
            break;

        case 0xf3:
            if (chipsetSubtype == 0x10) chipSetInfo = "ROM/CX4";
            break;
        case 0xf5:
            if (chipsetSubtype == 0x00) chipSetInfo = "ROM/RAM/Battery/SPC7110";
            if (chipsetSubtype == 0x02) chipSetInfo = "ROM/RAM/Battery/ST-018";
            break;
        case 0xf6:
            if (chipsetSubtype == 0x01) chipSetInfo = "ROM/Battery/ST-010 or ST-011";
            break;
        case 0xf9:
            if (chipsetSubtype == 0x00) chipSetInfo = "ROM/RAM/Battery/SPC7110/RTC";
            break;

        default: break;
    }

    if (gameCode == "XBND") chipSetInfo = "ROM/RAM/Battery/XBand Modem";
    if (gameCode == "MENU") chipSetInfo = "ROM/RAM/Battery/MX15001TFC";

    {
        ostringstream os;
        os << "1." << static_cast<unsigned int>((uint8_t)header[0x2b]);
        version = os.str();
    }

    switch (countryCode) {
        case 0x00: country = "Japan"; break;
        case 0x01: country = "USA"; break;
        case 0x02: country = "Europe"; break;
        case 0x03: country = "Sweden"; break;
        case 0x04: country = "Finland"; break;
        case 0x05: country = "Denmark"; break;
        case 0x06: country = "France"; break;
        case 0x07: country = "Holland"; break;
        case 0x08: country = "Spain"; break;
        case 0x09: country = "Germany"; break;
        case 0x0a: country = "Italy"; break;
        case 0x0b: country = "China/Hong Kong"; break;
        case 0x0c: country = "Indonesia"; break;
        case 0x0d: country = "South Korea"; break;
        case 0x0f: country = "Canada"; break;
        case 0x10: country = "Brazil"; break;
        case 0x11: country = "Australia"; break;
        default:   country = "Unknown"; break;
    }
}


// Get byte with "ROM size out of bounds mirroring"
uint8_t getByte(const vector<uint8_t> &vec, int offset) {
    uint32_t size = static_cast<uint32_t>(vec.size());
    if (offset >= size) {
        uint32_t mask = 1;
        uint32_t t = size;
        while (t >>= 1) {
            mask <<= 1;
            mask += 1;
        }
        mask >>= 1;
        uint32_t base = mask + 1;
        uint32_t offset_mask = size - base - 1;
        return vec[base + (offset & offset_mask)];
    } else {
        return vec[offset];
    }
}

// Get little endian word
uint16_t getWord(const vector<uint8_t> &vec, int offset) {
    return (uint16_t)((vec[offset]) + ((uint8_t)(vec[offset+1]) << 8));
}

// Not full SJIS, but this seems to be what's used in SNES headers
string sjisToString(uint8_t code) {
    if (code >= 0x20 && code <= 0x7e) {
        return string(1, static_cast<char>(code));
    } else {
        switch (code) {
            case 0xa1: return "｡";
            case 0xa2: return "｢";
            case 0xa3: return "｣	";
            case 0xa4: return "､";
            case 0xa5: return "･";
            case 0xa6: return "ｦ";
            case 0xa7: return "ｧ";
            case 0xa8: return "ｨ";
            case 0xa9: return "ｩ";
            case 0xaa: return "ｪ";
            case 0xab: return "ｫ";
            case 0xac: return "ｬ";
            case 0xad: return "ｭ";
            case 0xae: return "ｮ";
            case 0xaf: return "ｯ";

            case 0xb0: return "ｰ";
            case 0xb1: return "ｱ";
            case 0xb2: return "ｲ";
            case 0xb3: return "ｳ";
            case 0xb4: return "ｴ";
            case 0xb5: return "ｵ";
            case 0xb6: return "ｶ";
            case 0xb7: return "ｷ";
            case 0xb8: return "ｸ";
            case 0xb9: return "ｹ";
            case 0xba: return "ｺ";
            case 0xbb: return "ｻ";
            case 0xbc: return "ｼ";
            case 0xbd: return "ｽ";
            case 0xbe: return "ｾ";
            case 0xbf: return "ｿ";

            case 0xc0: return "ﾀ";
            case 0xc1: return "ﾁ";
            case 0xc2: return "ﾂ";
            case 0xc3: return "ﾃ";
            case 0xc4: return "ﾄ";
            case 0xc5: return "ﾅ";
            case 0xc6: return "ﾆ";
            case 0xc7: return "ﾇ";
            case 0xc8: return "ﾈ";
            case 0xc9: return "ﾉ";
            case 0xca: return "ﾊ";
            case 0xcb: return "ﾋ";
            case 0xcc: return "ﾌ";
            case 0xcd: return "ﾍ";
            case 0xce: return "ﾎ";
            case 0xcf: return "ﾏ";

            case 0xd0: return "ﾐ";
            case 0xd1: return "ﾑ";
            case 0xd2: return "ﾒ";
            case 0xd3: return "ﾓ";
            case 0xd4: return "ﾔ";
            case 0xd5: return "ﾕ";
            case 0xd6: return "ﾖ";
            case 0xd7: return "ﾗ";
            case 0xd8: return "ﾘ";
            case 0xd9: return "ﾙ";
            case 0xda: return "ﾚ";
            case 0xdb: return "ﾛ";
            case 0xdc: return "ﾜ";
            case 0xdd: return "ﾝ";

            default:   return string();
        }
    }
}
