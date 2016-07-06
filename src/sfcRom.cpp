#include "sfcRom.hpp"

#include <cstdint>
#include <climits>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

bool validResetOpcode(uint8_t op);
bool validInterruptOpcode(uint8_t op);
string sjisToString(uint8_t code);

uint16_t getWord(const vector<uint8_t> &vec, int offset);
void putWord(vector<uint8_t> &vec, int offset, uint16_t value);

sfcRom::sfcRom(const string& path) {
    filepath = path;
    int issues = 0;

    // Read file into buffer
    image = vector<uint8_t>(0);
    {
        ifstream file(path, ios::binary|ios::ate);
        if (file) {
            int fileSize = file.tellg();
            if ((fileSize & 0x3ff) == 0x200) {
                hasCopierHeader = true;
                imageOffset = 0x200;
                ++issues;
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

    // Review possible header locations and pick best match
    {
        vector<unsigned int> possibleHeaderLocations = { 0xffb0, 0x7fb0, 0x40ffb0 };
        vector<pair<int, unsigned int>> scoredHeaderLocations = {};

        for (int loc : possibleHeaderLocations) {
            int score = scoreHeaderLocation(image, loc);
            if (score > -2) scoredHeaderLocations.emplace_back(score, loc);
        }

        int top = INT_MIN;
        for (auto p : scoredHeaderLocations) {
            if (p.first >= top) {
                headerLocation = p.second;
                top = p.first;
            }
        }

        if (headerLocation == 0) {
            return;
        } else {
            valid = true;
        }
    }


    // So, we're probably dealing with an SFC ROM image
    getHeaderInfo(vector<uint8_t>(image.begin() + headerLocation, image.begin() + headerLocation + 0x50));

    // Check title
    {
        hasCorrectTitle = true;
        for (int i = 0; i < 21; ++i) {
            if (sjisToString(image[headerLocation + 0x10 + i]).empty()) hasCorrectTitle = false;
        }
        if (!hasCorrectTitle) {
            ++issues;
            hasSevereIssues = true;
        }
    }

    // Check ROM make up
    {
        if ((mode & 0xe0) != 0x20) {
            correctedMode = headerLocation >= 0x8000 ? 0x21 : 0x20;
            hasLegalMode = false;
            hasSevereIssues = true;
            ++issues;
        }

        if (mapperName.empty()) {
            hasKnownMapper = false;
            ++issues;
        }
    }

    // Check ROM size
    {
        if (imageSize > (1 << (romSize + 10)) || imageSize <= (1 << (romSize + 9))) {
            uint32_t pot = imageSize;
            int pot_n = 0;
            while (pot >>= 1) {
                if (pot & 1) ++pot_n;
            }
            correctedRomSize = (pot_n > 1) ? 1 : 0;
            uint32_t t = imageSize >> 10;
            while (t >>= 1) ++correctedRomSize;
            if (correctedRomSize != romSize) ++issues;
        }
    }


    // Check RAM size
    {
        if ((hasRam && ramSize > 0x0f) || (!hasRam && ramSize != 0)) {
            hasCorrectRamSize = false;
            ++issues;
        }
    }

    // Calculate checksum
    {
        correctedChecksum = calculateChecksum(image);
        correctedComplement = ~correctedChecksum;
        if (checksum != correctedChecksum) ++issues;
    }

    if (issues || hasSevereIssues) hasIssues = true;
}


string sfcRom::description(bool silent) {
    ostringstream os;
    if (valid) {
        os << setfill('0') << hex;

        if (!silent) {
            os << "ROM info for file \"" << filepath << "\"" << '\n' << '\n';

            uint32_t headerAt = headerLocation + (hasNewFormatHeader ? 0 : 0x10);
            os << "  Header at   0x" << setw(4) << headerAt << '\n';
            os << "  Title       " << title << '\n';
            if (!gameCode.empty()) {
                os << "  Game code   " << gameCode << '\n';
            }
            os << "  Maker code  " << makerCode << '\n';
            os << "  Country     0x" << setw(2) << static_cast<uint16_t>(countryCode) << " (" << country << ")" << '\n';
            os << "  Version     " << version << '\n';
            os << '\n';

            int romSizeKb = (int)(1 << romSize);
            int imageSizeKb = (int)(imageSize >> 10);
            if (romSize > 0x0f || romSize < 0x05) {
                os << "  ROM size    BAD! (Actual size " << dec << imageSizeKb << hex << "KB)" << '\n';
            } else {
                os << "  ROM size    0x" << setw(2) << static_cast<uint16_t>(romSize) << " (" << dec << romSizeKb << hex << "KB";
                if (romSizeKb != imageSizeKb) {
                    os << ", actual size " << dec << imageSizeKb << hex << "KB)" << '\n';
                } else {
                    os << ")" << '\n';
                }
            }

            if (hasRam) {
                if (ramSize > 0x0d) {
                    os << "  RAM size    BAD! (0x" << setw(2) << static_cast<uint16_t>(ramSize) << ")" << '\n';
                } else {
                    os << "  RAM size    0x" << setw(2) << static_cast<uint16_t>(ramSize);
                    os << " (" << dec << (int)(1 << (ramSize)) << hex << "KB)" << '\n';
                }
            }

            if (hasLegalMode) {
                os << "  Map mode    0x" << setw(2) << static_cast<uint16_t>(mapper);
                if (mapperName.empty()) {
                    os << '\n';
                } else {
                    os << " (" << mapperName << ")" << '\n';
                }
            } else {
                os << "  Map mode    BAD! (ROM makeup 0x" << setw(2) << static_cast<uint16_t>(mode) << ")" << '\n';
            }

            os << "  Chipset     0x" << setw(2) << static_cast<uint16_t>(chipset);
            if (chipsetSubtype) {
                os << "/" << setw(2) << static_cast<uint16_t>(chipsetSubtype);
            }
            if (chipSetInfo.empty()) {
                os << '\n';
            } else {
                os << " (" << chipSetInfo << ")" << '\n';
            }

            os << "  Speed       " << (fast ? "120ns" : "200ns") << '\n';
            os << '\n';

            os << "  Checksum    0x" << setw(4) << checksum << '\n';
            os << "  Complement  0x" << setw(4) << complement << '\n';

            os << '\n';
        }

        if (hasIssues) {
            if (silent) {
                os << "Issues with \"" << filepath << "\":" << '\n';
            } else {
                if (hasSevereIssues) {
                    os << "Severe issues were found:" << '\n';
                } else {
                    os << "The following issues were found:" << '\n';
                }
            }

            if (hasCorrectTitle == false) {
                os << "  ROM title contains illegal characters" << '\n';
            }
            if (!hasLegalMode) {
                os << "  ROM makeup 0x" << setw(2) << static_cast<uint16_t>(mode) << " is not allowed";
                os << ", best guess is 0x" << setw(2) << static_cast<uint16_t>(correctedMode) << '\n';
            } else if (!hasKnownMapper) {
                os << "  ROM makeup 0x" << setw(2) << static_cast<uint16_t>(mode) << " is an unknown type" << '\n';
            }
            if (correctedRomSize && romSize != correctedRomSize) {
                os << "  ROM size should be 0x" << setw(2) << static_cast<uint16_t>(correctedRomSize) << '\n';
            }
            if (!hasCorrectRamSize) {
                if (hasRam) {
                    os << "  RAM size specified too large" << '\n';
                } else {
                    os << "  RAM size should be 0x00" << '\n';
                }
            }
            if (checksum != correctedChecksum || complement != correctedComplement) {
                os << "  Checksum/complement should be 0x" << setw(4) << correctedChecksum << "/0x" << setw(4) << correctedComplement << '\n';
            }
            if (hasCopierHeader) {
                os << "  File has a copier header (0x200 bytes)" << '\n';
            }

            if (!silent) os << '\n';
        }

    } else {
        os << "File \"" << filepath << "\" is not an SFC ROM image" << '\n';
    }
    return os.str();
}


string sfcRom::fix(const string& path, bool silent) {
    if (!valid) return string();
    ostringstream os;

    int fixedIssues = 0;
    if (checksum != correctedChecksum || complement != correctedComplement) ++fixedIssues;

    os << "Writing ROM image to file \"" << path << "\"" << '\n';

    if (hasCopierHeader) {
        os << "  Removed copier header" << '\n';
        ++fixedIssues;
    }

    if (!hasCorrectTitle) {
        //TODO
    }

    if (!hasLegalMode) {
        os << "  Fixed ROM makeup" << '\n';
        image[headerLocation + 0x25] = correctedMode;
        ++fixedIssues;
    }

    if (correctedRomSize && romSize != correctedRomSize) {
        os << "  Fixed ROM size" << '\n';
        image[headerLocation + 0x27] = correctedRomSize;
        ++fixedIssues;
    }

    if (fixedIssues) {
        os << "  Fixed checksum" << '\n';
        correctedChecksum = calculateChecksum(image);
        correctedComplement = ~correctedChecksum;
        putWord(image, headerLocation + 0x2c, correctedComplement);
        putWord(image, headerLocation + 0x2e, correctedChecksum);
    }

    if (fixedIssues || path != filepath) {
        ofstream file(path, ios::binary|ios::trunc);
        if (file && file.good()) {
            file.write((char*)&image[0], image.size() * sizeof(uint8_t));
        } else {
            ostringstream fail;
            fail << "Cannot open file \"" << path << "\" for writing" << '\n';
            return fail.str();
        }
    } else {
        return string();
    }

    os << '\n';
    return os.str();
}

int sfcRom::scoreHeaderLocation(const vector<uint8_t> &image, int loc) {
    if (image.size() < loc + 0x50) return -100;
    int score = 0;
    vector<uint8_t> header = vector<uint8_t>(image.begin() + loc, image.begin() + loc + 0x50);
    uint16_t reset = getWord(header, 0x4c);

    // If 32K/bank mapper, reset vector must point to upper half
    if ((loc & 0xffff) < 0x8000) {
        if (reset < 0x8000) {
            return -100;
        } else {
            score += 1;
            reset -= 0x8000;
        }
    }

    // Correct rom makeup byte?
    {
        uint8_t mode = header[0x25];
        uint8_t mapper = mode & 0x0f;
        if (((mode & 0xe0) == 0x20) && (mapper == 0x0 || mapper == 0x1 || mapper == 0x2 || mapper == 0x3 || mapper == 0x5 || mapper == 0xa)) {
            score += 2;
        }
    }

    // Correct ROM & RAM size bytes?
    {
        if (header[0x27] >= 0x05 && header[0x27] <= 0x0f) score += 2;
        if (header[0x28] >= 0x0a) score += 1;
    }

    // Proper title characters?
    {
        int validChars = 0;
        for (int i = 0; i < 21; ++i) {
            if (!sjisToString(header[0x10 + i]).empty()) ++validChars;
        }
        if (validChars == 21) score += 2;
    }

    // Reasonable reset opcode?
    if (validResetOpcode(image[reset])) {
        score += 2;
    } else {
        score -= 4;
    }

    return score;
}

void sfcRom::getHeaderInfo(const vector<uint8_t> &header) {
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

    title = u8"";
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
        case 0x01: chipSetInfo = "ROM/RAM"; hasRam = true; break;
        case 0x02: chipSetInfo = "ROM/RAM/Battery"; hasRam = true; break;
        case 0x03: chipSetInfo = "ROM/DSP"; break;
        case 0x04: chipSetInfo = "ROM/RAM/DSP"; hasRam = true; break;
        case 0x05: chipSetInfo = "ROM/RAM/Battery/DSP"; hasRam = true; break;
        case 0x13: chipSetInfo = "ROM/EXP RAM/MARIO CHIP 1"; break;
        case 0x25: chipSetInfo = "ROM/RAM/Battery/OBC-1"; hasRam = true; break;
        case 0x32: chipSetInfo = "ROM/RAM/Battery/SA-1"; hasRam = true; break;
        case 0x34: chipSetInfo = "ROM/RAM/SA-1"; hasRam = true; break;
        case 0x35: chipSetInfo = "ROM/RAM/Battery/SA-1"; hasRam = true; break;
        case 0x43: chipSetInfo = "ROM/S-DD1"; break;
        case 0x45: chipSetInfo = "ROM/RAM/Battery/S-DD1"; hasRam = true; break;
        case 0x55: chipSetInfo = "ROM/RAM/Battery/S-RTC"; hasRam = true; break;
        case 0xe3: chipSetInfo = "ROM/SGB"; break;
        case 0xe5: chipSetInfo = "ROM/BS-X"; break;

        case 0x14:
            chipSetInfo = romSize > 0x0a ? "ROM/RAM/GSU-2" : "ROM/RAM/GSU-1";
            hasRam = true;
            break;
        case 0x15:
            chipSetInfo = romSize > 0x0a ? "ROM/RAM/Battery/GSU-2" : "ROM/RAM/Battery/GSU-1";
            hasRam = true;
            break;
        case 0x1a:
            chipSetInfo = "ROM/RAM/Battery/GSU-2-SP1";
            hasRam = true;
            break;

        case 0xf3:
            if (chipsetSubtype == 0x10) chipSetInfo = "ROM/CX4";
            break;
        case 0xf5:
            if (chipsetSubtype == 0x00) chipSetInfo = "ROM/RAM/Battery/SPC7110";
            if (chipsetSubtype == 0x02) chipSetInfo = "ROM/RAM/Battery/ST-018";
            hasRam = true;
            break;
        case 0xf6:
            if (chipsetSubtype == 0x01) chipSetInfo = "ROM/Battery/ST-010 or ST-011";
            break;
        case 0xf9:
            if (chipsetSubtype == 0x00) chipSetInfo = "ROM/RAM/Battery/SPC7110/RTC";
            hasRam = true;
            break;

        default: break;
    }

    if (gameCode == "XBND") { chipSetInfo = "ROM/RAM/Battery/XBand Modem"; hasRam = true; }
    if (gameCode == "MENU") { chipSetInfo = "ROM/RAM/Battery/MX15001TFC"; hasRam = true; }

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

uint16_t sfcRom::calculateChecksum(const vector<uint8_t> &image) {
    int rsize = (int)(1 << (correctedRomSize != 0 ? correctedRomSize + 10 : romSize + 10));
    if (mode == 0x3a) rsize = imageSize;

    // Set up base and mask for out of bounds mirroring
    uint32_t isize = static_cast<uint32_t>(image.size());
    uint32_t mask = 1;
    uint32_t t = isize;
    while (t >>= 1) {
        mask <<= 1;
        mask += 1;
    }
    mask >>= 1;
    uint32_t base = mask + 1;
    uint32_t offset_mask = isize - base - 1;

    // Add sum
    int chkloc = headerLocation + 0x2c;
    uint16_t sum = 0;

    for (int offset = 0; offset < rsize; ++offset) {
        if (!(offset >= chkloc && offset < chkloc + 4)) {
            if (offset < isize) {
                sum += image[offset];
            } else {
                sum += image[base + (offset & offset_mask)];
            }
        }
    }
    return sum + 0x1fe;
}


// Get/put little endian word
uint16_t getWord(const vector<uint8_t> &vec, int offset) {
    return (uint16_t)((vec[offset]) + ((uint8_t)(vec[offset+1]) << 8));
}

void putWord(vector<uint8_t> &vec, int offset, uint16_t value) {
    vec[offset] = (uint8_t)(value & 0xff);
    vec[offset+1] = (uint8_t)(value >> 8);
}

// Opcodes used on reset
bool validResetOpcode(uint8_t op) {
    switch (op) {
        case 0x18: // clc
        case 0x38: // sec
        case 0x4c: // jmp abs
        case 0x5c: // jml abs
        case 0x78: // sei
        case 0x80: // bra rel
        case 0x9c: // stz abs
        case 0xa0: // ldy #imm
        case 0xa9: // lda #imm
        case 0xc2: // rep
        case 0xd4: // pei (zp)
        case 0xd8: // cld
        case 0xdc: // jmp [abs long]
        case 0xe2: // sep
        case 0xe6: // inc zp
        case 0xea: // nop
            return true;
        default:
            return false;
    }
}

// SJIS subset used in SNES header
string sjisToString(uint8_t code) {
    if (code >= 0x20 && code <= 0x7e) {
        return string(1, static_cast<char>(code));
    } else {
        switch (code) {
            case 0xa1: return "\uff61";
            case 0xa2: return "\uff62";
            case 0xa3: return "\uff63";
            case 0xa4: return "\uff64";
            case 0xa5: return "\uff65";
            case 0xa6: return "\uff66";
            case 0xa7: return "\uff67";
            case 0xa8: return "\uff68";
            case 0xa9: return "\uff69";
            case 0xaa: return "\uff6a";
            case 0xab: return "\uff6b";
            case 0xac: return "\uff6c";
            case 0xad: return "\uff6d";
            case 0xae: return "\uff6e";
            case 0xaf: return "\uff6f";

            case 0xb0: return "\uff70";
            case 0xb1: return "\uff71";
            case 0xb2: return "\uff72";
            case 0xb3: return "\uff73";
            case 0xb4: return "\uff74";
            case 0xb5: return "\uff75";
            case 0xb6: return "\uff76";
            case 0xb7: return "\uff77";
            case 0xb8: return "\uff78";
            case 0xb9: return "\uff79";
            case 0xba: return "\uff7a";
            case 0xbb: return "\uff7b";
            case 0xbc: return "\uff7c";
            case 0xbd: return "\uff7d";
            case 0xbe: return "\uff7e";
            case 0xbf: return "\uff7f";

            case 0xc0: return "\uff80";
            case 0xc1: return "\uff81";
            case 0xc2: return "\uff82";
            case 0xc3: return "\uff83";
            case 0xc4: return "\uff84";
            case 0xc5: return "\uff85";
            case 0xc6: return "\uff86";
            case 0xc7: return "\uff87";
            case 0xc8: return "\uff88";
            case 0xc9: return "\uff89";
            case 0xca: return "\uff8a";
            case 0xcb: return "\uff8b";
            case 0xcc: return "\uff8c";
            case 0xcd: return "\uff8d";
            case 0xce: return "\uff8e";
            case 0xcf: return "\uff8f";

            case 0xd0: return "\uff90";
            case 0xd1: return "\uff91";
            case 0xd2: return "\uff92";
            case 0xd3: return "\uff93";
            case 0xd4: return "\uff94";
            case 0xd5: return "\uff95";
            case 0xd6: return "\uff96";
            case 0xd7: return "\uff97";
            case 0xd8: return "\uff98";
            case 0xd9: return "\uff99";
            case 0xda: return "\uff9a";
            case 0xdb: return "\uff9b";
            case 0xdc: return "\uff9c";
            case 0xdd: return "\uff9d";
            case 0xde: return "\uff9e";
            case 0xdf: return "\uff9f";

            default:   return string();
        }
    }
}
