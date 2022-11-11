#include "otf_cmap.h"

namespace txt {

namespace {

inline uint32_t read_u16(const uint8_t* data, size_t offset) {
  return ((uint32_t)data[offset]) << 8 | ((uint32_t)data[offset + 1]);
}

inline uint32_t read_u32(const uint8_t* data, size_t offset) {
  return ((uint32_t)data[offset]) << 24 | ((uint32_t)data[offset + 1]) << 16 |
         ((uint32_t)data[offset + 2]) << 8 | ((uint32_t)data[offset + 3]);
}

inline uint32_t make_tag(const char c[5]) {
  return ((uint32_t)c[0] << 24) | ((uint32_t)c[1] << 16) |
         ((uint32_t)c[2] << 8) | (uint32_t)c[3];
}

constexpr size_t kNGroupsOffset = 12;
constexpr size_t kFirstGroupOffset = 16;
constexpr size_t kGroupSize = 12;
constexpr size_t kStartCharCodeOffset = 0;
constexpr size_t kEndCharCodeOffset = 4;
constexpr size_t kStartGlyphOffset = 8;
constexpr size_t kMaxNGroups = 0xfffffff0 / kGroupSize;
constexpr uint32_t kMaxUnicodeCodepoint = 0x10FFFF;

// Get the cmap table with format 12. See
// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#format-12-segmented-coverage
// for details.
bool get_cmap_format12(std::vector<glyph_group_t>& groups,
                       const uint8_t* data,
                       size_t size) {
  // For all values < kMaxNGroups, kFirstGroupOffset + group_num * kGroupSize
  // fits in 32 bits.
  if (kFirstGroupOffset > size) {
    return false;
  }
  uint32_t group_num = read_u32(data, kNGroupsOffset);
  if (group_num >= kMaxNGroups ||
      kFirstGroupOffset + group_num * kGroupSize > size) {
    return false;
  }
  for (uint32_t i = 0; i < group_num; i++) {
    uint32_t group_offset = kFirstGroupOffset + i * kGroupSize;
    uint32_t start = read_u32(data, group_offset + kStartCharCodeOffset);
    uint32_t end = read_u32(data, group_offset + kEndCharCodeOffset);
    if (end < start) {
      // invalid group range: size must be positive
      return false;
    }

    // No need to read outside of Unicode code point range.
    if (start > kMaxUnicodeCodepoint) {
      return true;
    }

    uint32_t start_glyph = read_u32(data, group_offset + kStartGlyphOffset);
    if (end > kMaxUnicodeCodepoint) {
      groups.push_back({start, kMaxUnicodeCodepoint, start_glyph});
      return true;
    }
    groups.push_back({start, end, start_glyph});
  }
  return true;
}

constexpr size_t kSegCountOffset = 6;
constexpr size_t kEndCountOffset = 14;
constexpr size_t kHeaderSize4 = 16;
// total size of array elements for one segment
constexpr size_t kSegmentSize = 8;

// Table format 4, see
// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#format-4-segment-mapping-to-delta-values
// for details.
bool get_cmap_format4(std::vector<glyph_group_t>& groups,
                      const uint8_t* data,
                      size_t size) {
  if (kEndCountOffset > size) {
    return false;
  }
  size_t seg_count = read_u16(data, kSegCountOffset) >> 1;
  if (kHeaderSize4 + seg_count * kSegmentSize > size) {
    return false;
  }
  for (size_t i = 0; i < seg_count; i++) {
    uint32_t end = read_u16(data, kEndCountOffset + 2 * i);
    uint32_t start = read_u16(data, kHeaderSize4 + 2 * (seg_count + i));
    if (end < start) {
      // invalid segment range: size must be positive
      return false;
    }
    uint32_t range_offset =
        read_u16(data, kHeaderSize4 + 2 * (3 * seg_count + i));
    if (range_offset == 0) {
      int16_t delta = read_u16(data, kHeaderSize4 + 2 * (2 * seg_count + i));
      if (start + delta <= 0) {
        start = -delta + 1;
      }
      if (end < start) {
        continue;
      }
      groups.push_back({start, end, start + delta});
    } else {
      for (uint32_t j = start; j < end + 1; j++) {
        uint32_t actual_range_offset =
            kHeaderSize4 + 6 * seg_count + range_offset + (i + j - start) * 2;
        if (actual_range_offset + 2 > size) {
          // invalid range_offset is considered a "warning" by OpenType
          // Sanitizer
          continue;
        }
        uint32_t glyph_id = read_u16(data, actual_range_offset);
        if (glyph_id != 0) {
          groups.push_back({j, j, glyph_id});
        }
      }
    }
  }
  return true;
}

constexpr uint8_t kLowestPriority = 255;

uint8_t get_table_priority(uint16_t platform_id, uint16_t encoding_id) {
  if (platform_id == 3 && encoding_id == 10) {
    return 0;
  }
  if (platform_id == 0 && encoding_id == 6) {
    return 1;
  }
  if (platform_id == 0 && encoding_id == 4) {
    return 2;
  }
  if (platform_id == 3 && encoding_id == 1) {
    return 3;
  }
  if (platform_id == 0 && encoding_id == 3) {
    return 4;
  }
  if (platform_id == 0 && encoding_id == 2) {
    return 5;
  }
  if (platform_id == 0 && encoding_id == 1) {
    return 6;
  }
  if (platform_id == 0 && encoding_id == 0) {
    return 7;
  }
  // Tables other than above are not supported.
  return kLowestPriority;
}

// Only support for Unicode & Windows platform, which support Unicode charset.
// Macintosh & ISO platform is deprecated according to the otf-spec.
// See
// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#platform-ids
// for details.
bool is_valid_platform(uint16_t platform_id) {
  return platform_id == 0 || platform_id == 3;
}

constexpr size_t kHeaderSize = 4;
constexpr size_t kNumTablesOffset = 2;
constexpr size_t kTableSize = 8;
constexpr size_t kPlatformIdOffset = 0;
constexpr size_t kEncodingIdOffset = 2;
constexpr size_t kOffsetOffset = 4;
constexpr size_t kFormatOffset = 0;
constexpr uint32_t kInvalidOffset = UINT32_MAX;

std::unique_ptr<cmap_t> get_cmap(const uint8_t* cmap_data, size_t cmap_size) {
  if (kHeaderSize > cmap_size) {
    return nullptr;
  }
  uint32_t num_tables = read_u16(cmap_data, kNumTablesOffset);
  if (kHeaderSize + num_tables * kTableSize > cmap_size) {
    return nullptr;
  }

  uint32_t best_table_offset = kInvalidOffset;
  uint16_t best_table_format = 0;
  uint8_t best_table_priority = kLowestPriority;
  // Find the best cmap table
  for (uint32_t i = 0; i < num_tables; ++i) {
    const uint32_t table_head_offset = kHeaderSize + i * kTableSize;
    const uint16_t platform_id =
        read_u16(cmap_data, table_head_offset + kPlatformIdOffset);
    // Not a valid platform
    if (!is_valid_platform(platform_id)) {
      continue;
    }
    const uint16_t encoding_id =
        read_u16(cmap_data, table_head_offset + kEncodingIdOffset);
    const uint32_t offset =
        read_u32(cmap_data, table_head_offset + kOffsetOffset);

    if (offset > cmap_size - 2) {
      continue;  // Invalid table: not enough space to read.
    }
    const uint16_t format = read_u16(cmap_data, offset + kFormatOffset);

    if (platform_id == 0 /* Unicode */ &&
        encoding_id == 5 /* Variation Sequences */) {
      // The format must be 14 according to the otf-spec. See
      // https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#unicode-platform-platform-id--0
      // for details.
      //
      // Just ignore this table since we don't plan to support CJK ideographic
      // variations.
      continue;
    }

    uint32_t length;
    uint32_t language;

    if (format == 4) {
      constexpr size_t length_offset = 2;
      constexpr size_t language_offset = 4;
      constexpr size_t min_table_size = language_offset + 2;
      if (offset > cmap_size - min_table_size) {
        continue;  // Invalid table: not enough space to read.
      }
      length = read_u16(cmap_data, offset + length_offset);
      language = read_u16(cmap_data, offset + language_offset);
    } else if (format == 12) {
      constexpr size_t length_offset = 4;
      constexpr size_t language_offset = 8;
      constexpr size_t min_table_size = language_offset + 4;
      if (offset > cmap_size - min_table_size) {
        continue;  // Invalid table: not enough space to read.
      }
      length = read_u32(cmap_data, offset + length_offset);
      language = read_u32(cmap_data, offset + language_offset);
    } else {
      continue;
    }

    // Invalid table: table length is larger than whole cmap data
    // size.
    if (length > cmap_size - offset) {
      continue;
    }
    if (language != 0) {
      // Unsupported or invalid table: this is either a subtable for the
      // Macintosh platform (which we don't support), or an invalid subtable
      // since language field should be zero for non-Macintosh subtables.
      continue;
    }
    const uint8_t priority = get_table_priority(platform_id, encoding_id);
    if (priority < best_table_priority) {
      best_table_offset = offset;
      best_table_priority = priority;
      best_table_format = format;
    }
    if (best_table_priority == 0 /* highest priority */) {
      // Already found the highest priority table and variation sequences table.
      // No need to look at remaining tables.
      break;
    }
  }

  if (best_table_offset == kInvalidOffset) {
    return nullptr;
  }

  const uint8_t* table_data = cmap_data + best_table_offset;
  const size_t table_size = cmap_size - best_table_offset;
  std::vector<glyph_group_t> groups;

  bool success;
  if (best_table_format == 4) {
    success = get_cmap_format4(groups, table_data, table_size);
  } else {
    success = get_cmap_format12(groups, table_data, table_size);
  }

  if (success && !groups.empty()) {
    return std::make_unique<cmap_t>(std::move(groups));
  }
  return nullptr;
}

std::unique_ptr<cmap_t> get_cmap_from_typeface(sk_sp<SkTypeface> sk_typeface) {
  const uint32_t tag = make_tag("cmap");
  const size_t table_size = sk_typeface->getTableSize(tag);
  if (table_size == 0) {
    return nullptr;
  }
  void* buffer = malloc(table_size);
  if (buffer == nullptr) {
    return nullptr;
  }
  size_t actual_size = sk_typeface->getTableData(tag, 0, table_size, buffer);
  if (actual_size != table_size) {
    free(buffer);
    return nullptr;
  }
  auto cmap = get_cmap((uint8_t*)buffer, table_size);
  free(buffer);
  return cmap;
}

}  // namespace

int glyph_group_t::compare(uint32_t codepoint) const {
  if (codepoint < start_code_point) {
    return -1;
  }
  if (codepoint > end_code_point) {
    return 1;
  }
  return 0;
}

OtfCmap::OtfCmap(sk_sp<SkTypeface> sk_typeface)
    : cmap_(get_cmap_from_typeface(sk_typeface)) {}

bool OtfCmap::empty() const {
  return !cmap_ || cmap_->empty();
}

uint16_t OtfCmap::getGlyphId(uint32_t codepoint) const {
  if (!cmap_) {
    return 0;
  }
  return cmap_->get(codepoint);
}

}  // namespace txt
