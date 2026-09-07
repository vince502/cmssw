# EEC Analyzer: Current Status and Issues

## ✅ What is Correctly Implemented

1. **EEC Definition**: 
   - Correctly computes cos(θ*) using helicity axis (J/ψ lab direction)
   - Properly boosts PF candidates to J/ψ rest frame
   - Weight = E_i*/M_J/ψ is correct

2. **Event Treatment**:
   - Each J/ψ candidate gets its own EEC computation ✓
   - Handles multiple J/ψ per event correctly ✓

3. **Background Preparation**:
   - Separate histograms for signal and sidebands ✓
   - Tree output with classification flags ✓

4. **Prompt/Nonprompt Separation**:
   - Uses ctau (pseudo-proper decay length) ✓
   - Separate histograms if enabled ✓

## ⚠️ Issues That Need Attention

### 1. Pileup (PU) Mitigation - **CRITICAL**

**Current Implementation:**
- Uses `packedPFCandidates` directly
- Only applies PV association cuts (dz, dxy) for charged particles
- No PUPPI weights or CHS treatment
- Neutrals included without PU mitigation

**Problem:**
- `packedPFCandidates` includes pileup particles
- PV cuts alone are insufficient, especially for neutrals
- This will contaminate the EEC signal

**Recommendations:**
1. **Option A (Best)**: Use PUPPI-weighted PF candidates if available in MiniAOD
   ```python
   pfCandidateSrc = cms.InputTag("packedPFCandidates")  # Check if PUPPI weights are stored
   ```
   Then apply PUPPI weight cut in selection

2. **Option B**: Use CHS (Charged Hadron Subtraction) approach:
   - Only use charged particles with strict PV association
   - Remove neutrals or apply very strict cuts

3. **Option C (Current, Improved)**: 
   - Added `fromPV()` check for charged particles (implemented)
   - Consider removing neutrals entirely for initial analysis
   - Tighten dz/dxy cuts further

**Action Taken:**
- Added `fromPV() >= 1` requirement for charged particles
- This ensures particles are associated with primary vertex
- Still need to address neutrals

### 2. Background Subtraction - **NOT AUTOMATED**

**Current Status:**
- Produces separate histograms: `hEEC_signal` (S+B) and `hEEC_sideband` (B)
- Does NOT compute normalization factor α = B_win/B_SB
- Does NOT perform subtraction: Σ_sig = Σ_win - α × Σ_SB

**This is ACCEPTABLE** because:
- Background subtraction requires mass fits per (pT, y) bin
- Mass fits are typically done offline with RooFit/sPlot
- The histograms provide all necessary information

**What Needs to Happen Offline:**
1. Fit J/ψ mass distribution per pT/y bin
2. Extract B_win and B_SB from fits
3. Compute α = B_win / B_SB
4. Subtract: `hEEC_signal - α * hEEC_sideband`

**Recommendation:**
- Document this clearly in analysis notes
- Consider adding a helper script/function for offline subtraction

### 3. Normalization by N_J/ψ

**Current:**
- Histograms accumulate Σ(cosχ) over all events
- No per-event normalization

**This is CORRECT** because:
- Per EECPlan definition: Σ(cosχ) = (1/N_J/ψ) Σ_events Σ_particles (E_i*/M_J/ψ) δ(cosχ - cosθ_i*)
- Normalization by N_J/ψ should be done per (pT, y) bin offline
- The current implementation provides the numerator, normalization is a post-processing step

**Status:** ✓ Correct as-is

### 4. Pipeline Dependencies

**Current Pipeline Order:**
```
patMuonSequence → onia2MuMuPatGlbGlb → onia2MuMuPatGlbGlbFilter 
→ generalOttCandidatesNew → jpsiJets → ottana_new → hionia → eecAnalyzer
```

**Dependencies Check:**
- ✓ EEC analyzer needs J/ψ candidates → `onia2MuMuPatGlbGlb` (available)
- ✓ EEC analyzer needs PF candidates → `packedPFCandidates` (available from MiniAOD)
- ✓ EEC analyzer needs PV → `offlineSlimmedPrimaryVertices` (available)
- ✓ All dependencies satisfied

**Status:** ✓ Pipeline is correct

## 🔧 Recommended Improvements

### Immediate (Critical):
1. **Improve PU mitigation**:
   - Option: Remove neutrals entirely (add config flag)
   - Option: Check if PUPPI candidates available and use them
   - Tighten PV association cuts further

2. **Add validation histograms**:
   - N_PF per event vs N_PV (pileup monitoring)
   - PF candidate dz/dxy distributions
   - fromPV() flag distribution

### Short-term:
3. **Add efficiency weight placeholder**:
   - Structure to apply tracking efficiency weights (from MC)
   - Currently weights = 1.0, ready for offline correction

4. **Document background subtraction procedure**:
   - Add example ROOT macro for offline subtraction
   - Document mass fit requirements

### Long-term:
5. **Consider gen-level EEC module** (for MC):
   - Build response matrix for unfolding
   - Requires matching reco ↔ gen J/ψ

## Summary

| Aspect | Status | Action Needed |
|--------|--------|---------------|
| EEC Definition | ✅ Correct | None |
| Event Treatment | ✅ Correct | None |
| PU Mitigation | ⚠️ Weak | Improve (see above) |
| Background Subtraction | ⚠️ Manual | Document procedure |
| Pipeline | ✅ Correct | None |
| Normalization | ✅ Correct | None |

**Overall Assessment:**
The implementation is **functionally correct** but needs **improved PU mitigation** for physics-quality results. The background subtraction approach (offline) is standard practice and acceptable.
