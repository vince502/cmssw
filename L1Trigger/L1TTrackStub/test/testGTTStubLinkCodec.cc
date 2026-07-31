#include "L1Trigger/L1TTrackStub/interface/GTTStubLinkCodec.h"
#include "L1Trigger/DemonstratorTools/interface/codecs/tracks.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {
  [[gnu::noinline]] uint64_t extractWithoutInlining(const tt::Stream& stream, unsigned int trackSlot) {
    return l1t::GTTStubLinkCodec::extract(stream, trackSlot);
  }
}  // namespace

TEST_CASE("GTTStubLinkCodec uses an odd 96-bit track slot without changing its neighbour", "[GTTStubLinkCodec]") {
  constexpr unsigned int slot = 103;
  constexpr uint64_t word = 0x0123456789abcdefULL;
  constexpr uint64_t originalSharedFrame = 0xfedcba9876543210ULL;
  constexpr uint64_t originalLastFrame = 0x8000000011223344ULL;

  tt::Stream stream(l1t::GTTStubLinkCodec::kFramesPerLink);
  stream[154] = tt::Frame(originalSharedFrame);
  stream[155] = tt::Frame(originalLastFrame);

  REQUIRE(l1t::GTTStubLinkCodec::slotOccupied(stream, slot));
  REQUIRE_FALSE(l1t::GTTStubLinkCodec::carrierUpperBitsZero(stream, slot));
  l1t::GTTStubLinkCodec::insert(stream, slot, word);

  REQUIRE_FALSE(l1t::GTTStubLinkCodec::slotOccupied(stream, slot));
  REQUIRE(l1t::GTTStubLinkCodec::carrierUpperBitsZero(stream, slot));
  REQUIRE(l1t::GTTStubLinkCodec::extract(stream, slot) == word);
  REQUIRE(stream[154].to_ullong() == 0x89abcdef76543210ULL);
  REQUIRE(stream[155].to_ullong() == 0x0000000001234567ULL);
}

TEST_CASE("GTTStubLinkCodec uses an even 96-bit track slot without changing its neighbour", "[GTTStubLinkCodec]") {
  constexpr unsigned int slot = 102;
  constexpr uint64_t word = 0x0123456789abcdefULL;
  constexpr uint64_t originalSharedFrame = 0xaabbccdd80000000ULL;

  tt::Stream stream(l1t::GTTStubLinkCodec::kFramesPerLink);
  stream[154] = tt::Frame(originalSharedFrame);

  REQUIRE(l1t::GTTStubLinkCodec::slotOccupied(stream, slot));
  REQUIRE_FALSE(l1t::GTTStubLinkCodec::carrierUpperBitsZero(stream, slot));
  l1t::GTTStubLinkCodec::insert(stream, slot, word);

  REQUIRE_FALSE(l1t::GTTStubLinkCodec::slotOccupied(stream, slot));
  REQUIRE(l1t::GTTStubLinkCodec::carrierUpperBitsZero(stream, slot));
  REQUIRE(l1t::GTTStubLinkCodec::extract(stream, slot) == word);
  REQUIRE(stream[153].to_ullong() == word);
  REQUIRE(stream[154].to_ullong() == 0xaabbccdd00000000ULL);
}

TEST_CASE("GTTStubLinkCodec rejects an incomplete packet or invalid slot", "[GTTStubLinkCodec]") {
  tt::Stream complete(l1t::GTTStubLinkCodec::kFramesPerLink);
  tt::Stream shortPacket(l1t::GTTStubLinkCodec::kFramesPerLink - 1);

  REQUIRE_THROWS_AS(extractWithoutInlining(shortPacket, l1t::GTTStubLinkCodec::kDefaultMetadataTrackSlot),
                    std::out_of_range);
  REQUIRE_THROWS_AS(extractWithoutInlining(complete, l1t::GTTStubLinkCodec::kTracksPerLink), std::out_of_range);
}

TEST_CASE("GTTStubLinkCodec validates every link in a complete packet", "[GTTStubLinkCodec]") {
  constexpr unsigned int noncarrierLink = 1;
  tt::Streams streams(l1t::GTTStubLinkCodec::kNumLinks, tt::Stream(l1t::GTTStubLinkCodec::kFramesPerLink));

  REQUIRE(l1t::GTTStubLinkCodec::packetStructurallyValid(streams));

  streams.at(noncarrierLink).pop_back();
  REQUIRE_FALSE(l1t::GTTStubLinkCodec::packetStructurallyValid(streams));

  streams.at(noncarrierLink).emplace_back(0);
  streams.pop_back();
  REQUIRE_FALSE(l1t::GTTStubLinkCodec::packetStructurallyValid(streams));

  streams.emplace_back(l1t::GTTStubLinkCodec::kFramesPerLink);
  streams.emplace_back(l1t::GTTStubLinkCodec::kFramesPerLink);
  REQUIRE_FALSE(l1t::GTTStubLinkCodec::packetStructurallyValid(streams));
}

TEST_CASE("GTT stub metadata is invalid padding to the official track decoder", "[GTTStubLinkCodec]") {
  constexpr uint64_t metadata = 0x000000000060000fULL;
  tt::Stream stream(l1t::GTTStubLinkCodec::kFramesPerLink);
  l1t::GTTStubLinkCodec::insert(stream, l1t::GTTStubLinkCodec::kDefaultMetadataTrackSlot, metadata);

  std::vector<ap_uint<64>> frames;
  frames.reserve(stream.size());
  for (const auto& frame : stream)
    frames.emplace_back(frame.to_ullong());

  const auto decodedTracks = l1t::demo::codecs::decodeTracks(frames);
  REQUIRE(decodedTracks.size() == l1t::GTTStubLinkCodec::kTracksPerLink);
  REQUIRE_FALSE(decodedTracks.at(l1t::GTTStubLinkCodec::kDefaultMetadataTrackSlot).getValid());
}
