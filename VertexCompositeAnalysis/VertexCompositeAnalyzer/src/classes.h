#ifndef VertexCompositeAnalysis_VertexCompositeAnalyzer_classes_h
#define VertexCompositeAnalysis_VertexCompositeAnalyzer_classes_h

#include <vector>

// Nested vector types used in TTree branches
// These declarations ensure ROOT generates proper streamers

namespace VertexCompositeAnalyzer_dict {
  struct DictionaryTypes {
    std::vector<std::vector<float>> vvf;
    std::vector<std::vector<short>> vvs;
    std::vector<std::vector<bool>> vvb;
    std::vector<std::vector<int>> vvi;
  };
}

#endif
