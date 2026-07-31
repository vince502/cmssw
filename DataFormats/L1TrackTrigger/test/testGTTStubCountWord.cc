#include "DataFormats/L1TrackTrigger/interface/GTTStubCountWord.h"
#include "DataFormats/L1TrackTrigger/interface/GTTStubPrimitive.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("GTTStubCountWord packs one nominal regional count", "[GTTStubCountWord]") {
  constexpr uint32_t count = 12345;
  constexpr uint32_t region = 4;

  const auto word = l1t::GTTStubCountWord::pack(count, region);

  REQUIRE(word.count() == count);
  REQUIRE(word.region() == region);
  REQUIRE_FALSE(word.overflow());
  REQUIRE_FALSE(word.trackSlotOverflow());
  REQUIRE(word.structurallyValid());
  REQUIRE(word.word() == 0x0000000000643039);
}

TEST_CASE("GTTStubCountWord saturates an overflowing regional count", "[GTTStubCountWord]") {
  const auto word = l1t::GTTStubCountWord::pack(70000, 8, true, true);

  REQUIRE(word.count() == l1t::GTTStubCountWord::kCountMask);
  REQUIRE(word.overflow());
  REQUIRE(word.trackSlotOverflow());
  REQUIRE(word.structurallyValid());
}

TEST_CASE("GTTStubCountWord distinguishes the count boundary", "[GTTStubCountWord]") {
  const auto atLimit = l1t::GTTStubCountWord::pack(65535, 0);
  const auto aboveLimit = l1t::GTTStubCountWord::pack(65536, 0);

  REQUIRE(atLimit.count() == 65535);
  REQUIRE_FALSE(atLimit.overflow());
  REQUIRE(aboveLimit.count() == 65535);
  REQUIRE(aboveLimit.overflow());
}

TEST_CASE("GTTStubCountWord rejects malformed transport words", "[GTTStubCountWord]") {
  const auto nominal = l1t::GTTStubCountWord::pack(10, 1);

  REQUIRE_FALSE(l1t::GTTStubCountWord(nominal.word() & ~(uint64_t{1} << 21)).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubCountWord(nominal.word() | (uint64_t{1} << 27)).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubCountWord(nominal.word() ^ (uint64_t{1} << 22)).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubCountWord::pack(10, 9).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubCountWord::pack(10, 16).structurallyValid());
}

TEST_CASE("GTTStubPrimitive accepts consistent decoded aggregates", "[GTTStubPrimitive]") {
  const l1t::GTTStubPrimitive occupied(
      229, 59, 4, l1t::GTTStubPrimitive::kActiveRegionMask, 0, false, true, l1t::GTTStubPrimitive::kFormatVersion);
  const l1t::GTTStubPrimitive empty(0, 0, 0, 0, 0, false, true, l1t::GTTStubPrimitive::kFormatVersion);

  REQUIRE(occupied.structurallyValid());
  REQUIRE(empty.structurallyValid());
}

TEST_CASE("GTTStubPrimitive rejects inconsistent decoded aggregates", "[GTTStubPrimitive]") {
  constexpr uint32_t regionFour = uint32_t{1} << 4;

  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 4, regionFour, 0, false, false, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 4, regionFour, 0, false, true, 2).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 9, regionFour, 0, false, true, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 11, 4, regionFour, 0, false, true, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 4, 0, 0, false, true, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(0, 0, 0, 1, 0, false, true, 1).structurallyValid());
  REQUIRE_FALSE(
      l1t::GTTStubPrimitive(10, 10, 4, regionFour | (uint32_t{1} << 9), 0, false, true, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 4, regionFour, uint32_t{1} << 9, false, true, 1).structurallyValid());
  REQUIRE_FALSE(l1t::GTTStubPrimitive(10, 10, 4, regionFour, 0, true, true, 1).structurallyValid());

  const l1t::GTTStubPrimitive saturated(
      65535, 65535, 4, regionFour, 0, true, true, l1t::GTTStubPrimitive::kFormatVersion);
  REQUIRE(saturated.structurallyValid());
}
