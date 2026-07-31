#ifndef L1Trigger_L1TTrackStub_GTTStubLinkCodec_h
#define L1Trigger_L1TTrackStub_GTTStubLinkCodec_h

#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace l1t {

  /**
   * Insert a 64-bit metadata word into one reserved 96-bit GTT track slot.
   *
   * The standard Track Finder link codec places two 96-bit track words in
   * three 64-bit frames. The metadata occupies bits [63:0] of the selected
   * slot, while bits [95:64] are forced to zero so a standard track decoder
   * sees an invalid padding track.
   */
  class GTTStubLinkCodec {
  public:
    static constexpr unsigned int kNumLinks = 18;
    static constexpr unsigned int kTracksPerLink = 104;
    static constexpr unsigned int kFramesPerLink = 156;
    static constexpr unsigned int kDefaultMetadataTrackSlot = kTracksPerLink - 1;

    static bool packetStructurallyValid(const tt::Streams& streams) {
      return streams.size() == kNumLinks && std::all_of(streams.begin(), streams.end(), [](const auto& stream) {
               return stream.size() == kFramesPerLink;
             });
    }

    static bool slotOccupied(const tt::Stream& stream, unsigned int trackSlot) {
      validate(stream, trackSlot);
      const unsigned int firstFrame = frameOffset(trackSlot);
      if (trackSlot % 2 == 0)
        return stream.at(firstFrame + 1).test(31);
      return stream.at(firstFrame + 2).test(63);
    }

    static bool carrierUpperBitsZero(const tt::Stream& stream, unsigned int trackSlot) {
      validate(stream, trackSlot);
      const unsigned int firstFrame = frameOffset(trackSlot);
      if (trackSlot % 2 == 0)
        return (stream.at(firstFrame + 1).to_ullong() & 0xffffffffULL) == 0;
      return (stream.at(firstFrame + 2).to_ullong() >> 32) == 0;
    }

    static void insert(tt::Stream& stream, unsigned int trackSlot, uint64_t word) {
      validate(stream, trackSlot);
      const unsigned int firstFrame = frameOffset(trackSlot);
      if (trackSlot % 2 == 0) {
        stream.at(firstFrame) = tt::Frame(word);
        const uint64_t sharedFrame = stream.at(firstFrame + 1).to_ullong() & 0xffffffff00000000ULL;
        stream.at(firstFrame + 1) = tt::Frame(sharedFrame);
      } else {
        const uint64_t sharedFrame = stream.at(firstFrame + 1).to_ullong() & 0x00000000ffffffffULL;
        stream.at(firstFrame + 1) = tt::Frame(sharedFrame | ((word & 0xffffffffULL) << 32));
        stream.at(firstFrame + 2) = tt::Frame(word >> 32);
      }
    }

    static uint64_t extract(const tt::Stream& stream, unsigned int trackSlot) {
      validate(stream, trackSlot);
      const unsigned int firstFrame = frameOffset(trackSlot);
      if (trackSlot % 2 == 0)
        return stream.at(firstFrame).to_ullong();
      const uint64_t low = stream.at(firstFrame + 1).to_ullong() >> 32;
      const uint64_t high = stream.at(firstFrame + 2).to_ullong() & 0xffffffffULL;
      return low | (high << 32);
    }

  private:
    static void validate(const tt::Stream& stream, unsigned int trackSlot) {
      if (trackSlot >= kTracksPerLink)
        throw std::out_of_range("GTT stub metadata track slot is outside the 104-slot packet");
      if (stream.size() != kFramesPerLink)
        throw std::out_of_range("GTT stub metadata codec requires exactly 156 frames");
    }

    static constexpr unsigned int frameOffset(unsigned int trackSlot) { return 3 * (trackSlot / 2); }
  };

}  // namespace l1t

#endif
