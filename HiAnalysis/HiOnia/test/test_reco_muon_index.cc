#include "HiAnalysis/HiOnia/interface/RecoMuonIndex.h"
#include <cassert>
#include <iostream>
#include <limits>

long legacy_key(float pt) {
  float value = std::fabs(pt);
  while (value > 0.1)
    value /= 10;
  return static_cast<long>(10000000 * value);
}

template <typename Exception, typename Function>
void expect_exception(Function function, const char* message) {
  bool caught = false;
  try {
    function();
  } catch (const Exception& error) {
    caught = std::string(error.what()).find(message) != std::string::npos;
  }
  assert(caught);
}

int main() {
  onia::RecoMuonIndex index;
  // Actual opposite-charge equal-pT collision observed in run 375064.
  index.insert(2.337890625, 2.0561540126800537, -0.12842606008052826, 1, 0);
  index.insert(2.337890625, 2.014587879180908, 0.11835881322622299, -1, 1);
  assert(index.at(2.337890625, 2.0561540126800537, -0.12842606008052826, 1) == 0);
  assert(index.at(2.337890625, 2.014587879180908, 0.11835881322622299, -1) == 1);
  // Each of the four fields independently separates keys.
  index.insert(2.5, 1, 0.2, 1, 2);
  index.insert(25, 1, 0.2, 1, 3);
  index.insert(2.5, 1.1, 0.2, 1, 4);
  index.insert(2.5, 1, 0.3, 1, 5);
  index.insert(2.5, 1, 0.2, -1, 6);
  index.insert(std::nextafter(2.5, 3.0), 1, 0.2, 1, 7);
  assert(legacy_key(2.5) == legacy_key(25));
  assert(index.at(2.5, 1, 0.2, 1) == 2);
  assert(index.at(25, 1, 0.2, 1) == 3);
  assert(index.at(2.5, 1.1, 0.2, 1) == 4);
  assert(index.at(2.5, 1, 0.3, 1) == 5);
  assert(index.at(2.5, 1, 0.2, -1) == 6);
  assert(index.at(std::nextafter(2.5, 3.0), 1, 0.2, 1) == 7);
  expect_exception<std::runtime_error>([&] { index.insert(2.5, 1, 0.2, 1, 8); }, "AmbiguousRecoMuonKey");
  assert(index.at(2.5, 1, 0.2, 1) == 2);  // no overwrite after failure
  expect_exception<std::runtime_error>([&] { index.at(3, 1, 0.2, 1); }, "MissingRecoMuonKey");
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  expect_exception<std::invalid_argument>([&] { index.insert(nan, 1, 0.2, 1, 8); }, "InvalidRecoMuonKey");
  expect_exception<std::invalid_argument>([&] { index.insert(2.5, inf, 0.2, 1, 8); }, "InvalidRecoMuonKey");
  expect_exception<std::invalid_argument>([&] { index.at(2.5, 1, nan, 1); }, "InvalidRecoMuonKey");
  index.clear();
  expect_exception<std::runtime_error>([&] { index.at(2.5, 1, 0.2, 1); }, "MissingRecoMuonKey");
  index.insert(2.5, 1, 0.2, 1, 0);  // a new event may reuse the key
  assert(index.at(2.5, 1, 0.2, 1) == 0);
  std::cout << "PASS: four-field separation, observed collision, decade collision, full precision, duplicate/missing/nonfinite failures, no overwrite, event reset\n";
}
