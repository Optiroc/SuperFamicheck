#include <filesystem>
#include <iostream>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "../src/sfcRom.hpp"

//
// Simple test ROMs
//

// rom0.sfc:
//  - 32Kb, code at 0x0000, mapped at 0x8000
//  - Reset vector 0x0000
//  - Not valid
inline constexpr auto rom0 = "data/public/rom0.sfc";

// rom1.sfc:
//  - 32Kb
//  - Valid sfc
//  - ROM size 0x05
//  - Map mode 0x00
//  - Chipset 0x00
//  - Speed 200ns
//  - Bad checksum
inline constexpr auto rom1 = "data/public/rom1.sfc";

//
// ROMs not stored in repo
//

// Tales of Phantasia
//  ROM size    0x0d (8192KB, actual size 6144KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x05 (Extended HiROM)
//  Chipset     0x02 (ROM, RAM, Battery)
//  Speed       120ns
//  Checksum    0x7c57
//  Complement  0x83a8
inline constexpr auto rom_atvj = "data/private/atvj.sfc";

// Tales of Phantasia with bad checksum
inline constexpr auto rom_atvj_bad = "data/private/atvj-bad.sfc";

// Daikaiju Monogatari II
//  ROM size    0x0d (8192KB, actual size 5120KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x05 (Extended HiROM)
//  Chipset     0x55 (ROM, RAM, S-RTC, Battery)
//  Speed       120ns
//  Checksum    0x8528
//  Complement  0x7ad7
inline constexpr auto rom_dkm2 = "data/private/dkm2.sfc";

// Metroid 3
//  ROM size    0x0c (4096KB, actual size 3072KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x00 (LoROM)
//  Chipset     0x02 (ROM, RAM, Battery)
//  Speed       120ns
//  Checksum    0xf8df
//  Complement  0x0720
inline constexpr auto rom_m3 = "data/private/m3.sfc";

// Momotaro Dentetsu Happy
//  ROM size    0x0c (4096KB, actual size 3072KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x0a (Extended HiROM/SPC7110)
//  Chipset     0xf5 (ROM, RAM, SPC7110, Battery)
//  Speed       120ns
//  Checksum    0xe28c
//  Complement  0x1d73
inline constexpr auto rom_mdh = "data/private/mdh.sfc";

// Super Mario World
//  ROM size    0x09 (512KB)
//  RAM size    0x01 (2KB)
//  Map mode    0x00 (LoROM)
//  Chipset     0x02 (ROM, RAM, Battery)
//  Speed       200ns
//  Checksum    0xa0da
//  Complement  0x5f25
inline constexpr auto rom_smw = "data/private/smw.sfc";

// Super Mario World 2
//  ROM size    0x0b (2048KB)
//  RAM size    0x00 (1KB)
//  Map mode    0x00 (LoROM)
//  Chipset     0x15 (ROM, RAM, GSU-2, Battery)
//  Speed       200ns
//  Checksum    0x132c
//  Complement  0xecd3
inline constexpr auto rom_smw2 = "data/private/smw2.sfc";

// Star Ocean
//  ROM size    0x0d (8192KB, actual size 6144KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x02 (LoROM/S-DD1)
//  Chipset     0x45 (ROM, RAM, S-DD1, Battery)
//  Speed       120ns
//  Checksum    0x13b8
//  Complement  0xec47
inline constexpr auto rom_so = "data/private/so.sfc";

// Super Power League 4
//  ROM size    0x0b (2048KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x0a (Extended HiROM/SPC7110)
//  Chipset     0xf5 (ROM, RAM, SPC7110, Battery)
//  Speed       120ns
//  Checksum    0x01aa
//  Complement  0xfe55
inline constexpr auto rom_spl4 = "data/private/spl4.sfc";

// Tengai Makyou Zero
//  ROM size    0x0d (8192KB, actual size 5120KB)
//  RAM size    0x03 (8KB)
//  Map mode    0x0a (Extended HiROM/SPC7110)
//  Chipset     0xf9 (ROM, RAM, SPC7110, RTC, Battery)
//  Speed       120ns
//  Checksum    0xde89
//  Complement  0x2176
inline constexpr auto rom_tmz = "data/private/tmz.sfc";

bool file_exists(const std::string& path) {
  std::filesystem::path p { path };
  return std::filesystem::is_regular_file(p);
}

bool rom_isValid(const std::string& path, bool expected) {
  if (!file_exists(path)) {
    std::cout << "File '" << path << "' not found, skipping\n";
    return expected;
  }
  sfcRom rom(path);
  return rom.isValid;
}

bool rom_hasCorrectChecksum(const std::string& path, bool expected) {
  if (!file_exists(path)) {
    std::cout << "File '" << path << "' not found, skipping\n";
    return expected;
  }
  sfcRom rom(path);
  return rom.hasCorrectChecksum;
}

TEST_CASE("sfcRom.isValid") {
  REQUIRE(rom_isValid(rom0, false) == false);
  REQUIRE(rom_isValid(rom1, true) == true);
  REQUIRE(rom_isValid(rom_atvj, true) == true);
  REQUIRE(rom_isValid(rom_atvj_bad, true) == true);
  REQUIRE(rom_isValid(rom_dkm2, true) == true);
  REQUIRE(rom_isValid(rom_m3, true) == true);
  REQUIRE(rom_isValid(rom_mdh, true) == true);
  REQUIRE(rom_isValid(rom_smw, true) == true);
  REQUIRE(rom_isValid(rom_smw2, true) == true);
  REQUIRE(rom_isValid(rom_so, true) == true);
  REQUIRE(rom_isValid(rom_spl4, true) == true);
  REQUIRE(rom_isValid(rom_tmz, true) == true);
}

TEST_CASE("sfcRom.hasCorrectChecksum") {
  REQUIRE(rom_hasCorrectChecksum(rom0, false) == false);
  REQUIRE(rom_hasCorrectChecksum(rom1, false) == false);
  REQUIRE(rom_hasCorrectChecksum(rom_atvj, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_atvj_bad, false) == false);
  REQUIRE(rom_hasCorrectChecksum(rom_dkm2, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_m3, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_mdh, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_smw, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_smw2, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_so, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_spl4, true) == true);
  REQUIRE(rom_hasCorrectChecksum(rom_tmz, true) == true);
}
