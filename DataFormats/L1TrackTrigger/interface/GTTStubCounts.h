#ifndef DataFormats_L1TrackTrigger_GTTStubCounts_h
#define DataFormats_L1TrackTrigger_GTTStubCounts_h

#include <cstdint>
#include <utility>
#include <vector>

namespace l1t {

  /**
   * Diagnostic stub occupancies at the DTC-to-Track-Finder boundary.
   *
   * The accepted count includes copies routed to overlapping Track Finder
   * regions. It is therefore the number of valid input frames seen by the
   * Track Finder, not the number of unique physical TTStub objects.
   */
  class GTTStubCounts {
  public:
    GTTStubCounts() = default;

    GTTStubCounts(uint32_t raw,
                  uint32_t accepted,
                  uint32_t lost,
                  std::vector<uint32_t> acceptedByRegion,
                  std::vector<uint32_t> lostByRegion,
                  std::vector<uint32_t> acceptedByLink,
                  std::vector<uint32_t> lostByLink)
        : raw_(raw),
          accepted_(accepted),
          lost_(lost),
          acceptedByRegion_(std::move(acceptedByRegion)),
          lostByRegion_(std::move(lostByRegion)),
          acceptedByLink_(std::move(acceptedByLink)),
          lostByLink_(std::move(lostByLink)) {}

    uint32_t raw() const { return raw_; }
    uint32_t accepted() const { return accepted_; }
    uint32_t lost() const { return lost_; }

    const std::vector<uint32_t>& acceptedByRegion() const { return acceptedByRegion_; }
    const std::vector<uint32_t>& lostByRegion() const { return lostByRegion_; }
    const std::vector<uint32_t>& acceptedByLink() const { return acceptedByLink_; }
    const std::vector<uint32_t>& lostByLink() const { return lostByLink_; }

  private:
    uint32_t raw_ = 0;
    uint32_t accepted_ = 0;
    uint32_t lost_ = 0;
    std::vector<uint32_t> acceptedByRegion_;
    std::vector<uint32_t> lostByRegion_;
    std::vector<uint32_t> acceptedByLink_;
    std::vector<uint32_t> lostByLink_;
  };

}  // namespace l1t

#endif
