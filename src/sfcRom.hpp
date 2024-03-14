#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct sfcRom {
  sfcRom(const std::string& path);

  std::string description(bool silent) const;
  std::string fix(const std::string& path, bool silent);

  bool isValid() const { return _isValid; }
  bool hasIssues() const { return _hasIssues; }
  bool hasCopierHeader() const { return _hasCopierHeader; }
  bool hasCorrectTitle() const { return _hasCorrectTitle; }
  bool hasCorrectRamSize() const { return _hasCorrectRamSize; }
  bool hasLegalMode() const { return _hasLegalMode; }
  bool hasKnownMapper() const { return _hasKnownMapper; }
  bool hasNewFormatHeader() const { return _hasNewFormatHeader; }

private:
  std::string filepath;
  std::vector<uint8_t> image;

  void getHeaderInfo(const std::vector<uint8_t>& header);
  int scoreHeaderLocation(size_t location) const;
  uint16_t calculateChecksum() const;

  size_t imageSize;
  size_t imageOffset = 0;
  size_t headerLocation = 0;

  bool _isValid = false;
  bool _hasIssues = false;
  bool _hasSevereIssues = false;

  bool _hasCopierHeader = false;
  bool _hasCorrectTitle = false;
  bool _hasCorrectRamSize = true;
  bool _hasLegalMode = true;
  bool _hasKnownMapper = true;
  bool _hasNewFormatHeader = false;

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

  std::string title;
  std::string mapperName;
  std::string chipSetInfo;
  std::string makerCode;
  std::string gameCode;
  std::string version;
  std::string country;
};
