#ifndef DataFormats_L1TrackTrigger_GTTStubCountWord_h
#define DataFormats_L1TrackTrigger_GTTStubCountWord_h

#include <algorithm>
#include <cstdint>

namespace l1t {

  /**
   * Provisional 64-bit Apollo-to-Serenity count word for one processing region.
   *
   * Bit allocation:
   *   [15:0]  accepted DTC stub-frame count in this region
   *   [19:16] processing-region index
   *   [20]    count overflow
   *   [21]    valid
   *   [25:22] format version
   *   [26]    reserved metadata slot displaced a valid track
   *   [63:27] reserved, required to be zero
   *
   * The layout is intentionally isolated in this class. It is an emulation
   * contract and must not be treated as approved firmware protocol.
   */
  class GTTStubCountWord {
  public:
    using word_type = uint64_t;

    static constexpr unsigned int kFormatVersion = 1;
    static constexpr unsigned int kNumRegions = 9;
    static constexpr word_type kCountMask = (word_type{1} << 16) - 1;
    static constexpr word_type kRegionMask = (word_type{1} << 4) - 1;

    GTTStubCountWord() = default;
    explicit GTTStubCountWord(word_type word) : word_(word) {}

    static GTTStubCountWord pack(uint32_t count, uint32_t region, bool valid = true, bool trackSlotOverflow = false) {
      word_type word = std::min<word_type>(count, kCountMask);
      word |= (static_cast<word_type>(region) & kRegionMask) << 16;
      word |= static_cast<word_type>(count > kCountMask) << 20;
      word |= static_cast<word_type>(valid && region < kNumRegions) << 21;
      word |= static_cast<word_type>(kFormatVersion) << 22;
      word |= static_cast<word_type>(trackSlotOverflow) << 26;
      return GTTStubCountWord(word);
    }

    word_type word() const { return word_; }
    uint32_t count() const { return word_ & kCountMask; }
    uint32_t region() const { return (word_ >> 16) & kRegionMask; }
    bool overflow() const { return (word_ >> 20) & 1; }
    bool valid() const { return (word_ >> 21) & 1; }
    unsigned int formatVersion() const { return (word_ >> 22) & 0xf; }
    bool trackSlotOverflow() const { return (word_ >> 26) & 1; }
    bool reservedBitsZero() const { return (word_ >> 27) == 0; }
    bool structurallyValid() const {
      return valid() && formatVersion() == kFormatVersion && region() < kNumRegions && reservedBitsZero();
    }

  private:
    word_type word_ = 0;
  };

}  // namespace l1t

#endif
