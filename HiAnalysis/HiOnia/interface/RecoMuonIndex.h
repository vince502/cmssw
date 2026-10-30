#ifndef HiAnalysis_HiOnia_RecoMuonIndex_h
#define HiAnalysis_HiOnia_RecoMuonIndex_h

#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>

namespace onia {

// Both insertion and lookup use the same unmodified lorentzMomentum result.
// Do not normalize/round pT, or confuse equal pT with equal muon identity.
class RecoMuonIndex {
public:
  using Key = std::tuple<double, double, double, int>;

  void clear() { indices_.clear(); }

  void insert(double pt, double eta, double phi, int charge, int index) {
    const auto key = makeKey(pt, eta, phi, charge);
    const auto result = indices_.emplace(key, index);
    if (!result.second) {
      std::ostringstream message;
      message << "AmbiguousRecoMuonKey: " << describe(key)
              << "; existing index=" << result.first->second << ", new index=" << index
              << ". Refuse to overwrite; original muon references are needed to distinguish identical keys.";
      throw std::runtime_error(message.str());
    }
  }

  int at(double pt, double eta, double phi, int charge) const {
    const auto key = makeKey(pt, eta, phi, charge);
    const auto found = indices_.find(key);
    if (found == indices_.end())
      throw std::runtime_error("MissingRecoMuonKey: " + describe(key));
    return found->second;
  }

private:
  static Key makeKey(double pt, double eta, double phi, int charge) {
    if (!std::isfinite(pt) || !std::isfinite(eta) || !std::isfinite(phi))
      throw std::invalid_argument("InvalidRecoMuonKey: pT, eta and phi must be finite");
    return std::make_tuple(pt, eta, phi, charge);
  }

  static std::string describe(const Key& key) {
    std::ostringstream message;
    message << std::setprecision(17) << "(pT=" << std::get<0>(key)
            << ", eta=" << std::get<1>(key) << ", phi=" << std::get<2>(key)
            << ", charge=" << std::get<3>(key) << ")";
    return message.str();
  }

  std::map<Key, int> indices_;
};

}  // namespace onia

#endif
