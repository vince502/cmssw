#ifndef DataFormats_L1TrackTrigger_GTTStubPrimitive_h
#define DataFormats_L1TrackTrigger_GTTStubPrimitive_h

#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"

#include <cstdint>

namespace l1t {

  /// Serenity-decoded aggregate of the nine Apollo regional count words.
  class GTTStubPrimitive {
  public:
    static constexpr unsigned int kFormatVersion = GTTStubCountWord::kFormatVersion;
    static constexpr unsigned int kNumRegions = GTTStubCountWord::kNumRegions;
    static constexpr uint32_t kActiveRegionMask = (uint32_t{1} << kNumRegions) - 1;

    GTTStubPrimitive() = default;
    GTTStubPrimitive(uint32_t acceptedStubCount,
                     uint32_t maxRegionCount,
                     uint32_t maxRegion,
                     uint32_t activeRegionMask,
                     uint32_t trackOverflowMask,
                     bool overflow,
                     bool valid,
                     unsigned int formatVersion)
        : acceptedStubCount_(acceptedStubCount),
          maxRegionCount_(maxRegionCount),
          maxRegion_(maxRegion),
          activeRegionMask_(activeRegionMask),
          trackOverflowMask_(trackOverflowMask),
          overflow_(overflow),
          valid_(valid),
          formatVersion_(formatVersion) {}

    uint32_t acceptedStubCount() const { return acceptedStubCount_; }
    uint32_t maxRegionCount() const { return maxRegionCount_; }
    uint32_t maxRegion() const { return maxRegion_; }
    uint32_t activeRegionMask() const { return activeRegionMask_; }
    uint32_t trackOverflowMask() const { return trackOverflowMask_; }
    bool trackOverflow() const { return trackOverflowMask_ != 0; }
    bool overflow() const { return overflow_; }
    bool valid() const { return valid_; }
    unsigned int formatVersion() const { return formatVersion_; }
    bool structurallyValid() const {
      const bool maxRegionInRange = maxRegion_ < kNumRegions;
      const bool maxRegionActive = maxRegionInRange && ((activeRegionMask_ >> maxRegion_) & 1) != 0;
      const bool zeroOccupancyConsistent = acceptedStubCount_ == 0
                                               ? maxRegionCount_ == 0 && activeRegionMask_ == 0
                                               : maxRegionCount_ != 0 && activeRegionMask_ != 0 && maxRegionActive;
      const bool overflowConsistent = !overflow_ || maxRegionCount_ == GTTStubCountWord::kCountMask;
      return valid_ && formatVersion_ == kFormatVersion && maxRegionInRange &&
             acceptedStubCount_ <= kNumRegions * GTTStubCountWord::kCountMask &&
             maxRegionCount_ <= GTTStubCountWord::kCountMask && maxRegionCount_ <= acceptedStubCount_ &&
             (activeRegionMask_ & ~kActiveRegionMask) == 0 && (trackOverflowMask_ & ~kActiveRegionMask) == 0 &&
             zeroOccupancyConsistent && overflowConsistent;
    }

  private:
    uint32_t acceptedStubCount_ = 0;
    uint32_t maxRegionCount_ = 0;
    uint32_t maxRegion_ = 0;
    uint32_t activeRegionMask_ = 0;
    uint32_t trackOverflowMask_ = 0;
    bool overflow_ = false;
    bool valid_ = false;
    unsigned int formatVersion_ = 0;
  };

}  // namespace l1t

#endif
