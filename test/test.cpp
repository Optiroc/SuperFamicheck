#include <string>
#include <catch2/catch_test_macros.hpp>
#include "../src/sfcRom.hpp"

/*
rom0.sfc:
  - 32Kb, code at 0x0000, mapped at 0x8000
  - Reset vector 0x0000
  - Not valid

rom1.sfc:
  - 32Kb
  - Valid sfc
  - ROM size 0x05
  - Map mode 0x00
  - Chipset 0x00
  - Speed 200ns
  - Bad checksum

*/

bool rom_isValid(const std::string& path) {
  sfcRom rom(path);
  return rom.isValid();
}

inline constexpr auto rom0 = "data/rom0.sfc";
inline constexpr auto rom1 = "data/rom1.sfc";

TEST_CASE("sfcRom.isValid") {
  REQUIRE(rom_isValid(rom0) == false);
  REQUIRE(rom_isValid(rom1) == true);
}
