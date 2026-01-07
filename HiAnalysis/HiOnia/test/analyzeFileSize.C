// ROOT macro to analyze file size of HiForest output
// Usage: root -l -b -q 'analyzeFileSize.C("HiForestOO_OniaBmesonDmeson.root")'

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

void analyzeFileSize(const char* filename = "HiForestOO_OniaBmesonDmeson.root") {
    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }
    
    Long64_t totalSize = f->GetSize();
    
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "ROOT File Size Analysis" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "File: " << filename << std::endl;
    std::cout << "Total size: " << std::fixed << std::setprecision(2) 
              << totalSize/1024.0 << " KB (" << totalSize/1024.0/1024.0 << " MB)" << std::endl;
    std::cout << std::endl;
    
    // Collect directory/tree info
    std::vector<std::tuple<Long64_t, std::string, int, std::string>> items; // size, name, nEntries, type
    
    TIter nextKey(f->GetListOfKeys());
    TKey *key;
    
    while ((key = (TKey*)nextKey())) {
        TObject *obj = key->ReadObj();
        
        if (obj->InheritsFrom("TDirectory")) {
            TDirectory *dir = (TDirectory*)obj;
            Long64_t dirSize = 0;
            int nTrees = 0;
            int totalEntries = 0;
            
            TIter nextInDir(dir->GetListOfKeys());
            TKey *keyInDir;
            
            while ((keyInDir = (TKey*)nextInDir())) {
                TObject *objInDir = keyInDir->ReadObj();
                if (objInDir->InheritsFrom("TTree")) {
                    TTree *tree = (TTree*)objInDir;
                    dirSize += tree->GetZipBytes();
                    totalEntries = tree->GetEntries();
                    nTrees++;
                } else {
                    dirSize += keyInDir->GetNbytes();
                }
            }
            items.push_back({dirSize, dir->GetName(), totalEntries, "TDirectory"});
        } 
        else if (obj->InheritsFrom("TTree")) {
            TTree *tree = (TTree*)obj;
            items.push_back({tree->GetZipBytes(), tree->GetName(), (int)tree->GetEntries(), "TTree"});
        }
    }
    
    // Sort by size (descending)
    std::sort(items.begin(), items.end(), 
              [](const auto &a, const auto &b) { return std::get<0>(a) > std::get<0>(b); });
    
    // Print summary table
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::left << std::setw(30) << "Name" 
              << std::right << std::setw(12) << "Size (KB)"
              << std::setw(10) << "Entries"
              << std::setw(10) << "% Total"
              << std::setw(8) << "Type" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    Long64_t accountedSize = 0;
    for (const auto &item : items) {
        Long64_t size = std::get<0>(item);
        const std::string &name = std::get<1>(item);
        int entries = std::get<2>(item);
        const std::string &type = std::get<3>(item);
        
        accountedSize += size;
        double pct = 100.0 * size / totalSize;
        
        std::cout << std::left << std::setw(30) << name
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << size/1024.0
                  << std::setw(10) << entries
                  << std::setw(9) << pct << "%"
                  << std::setw(8) << (type == "TDirectory" ? "Dir" : "Tree") << std::endl;
    }
    
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::left << std::setw(30) << "TOTAL (accounted)"
              << std::right << std::setw(12) << accountedSize/1024.0
              << std::setw(10) << ""
              << std::setw(9) << 100.0 * accountedSize / totalSize << "%" << std::endl;
    
    // Detailed branch analysis for largest directories
    std::cout << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "Detailed Branch Analysis (Top 5 directories)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    int count = 0;
    for (const auto &item : items) {
        if (count >= 5) break;
        
        const std::string &dirName = std::get<1>(item);
        TDirectory *dir = (TDirectory*)f->Get(dirName.c_str());
        if (!dir) continue;
        
        std::cout << std::endl << ">>> " << dirName << " <<<" << std::endl;
        
        // Find trees in this directory
        TIter nextInDir(dir->GetListOfKeys());
        TKey *keyInDir;
        
        while ((keyInDir = (TKey*)nextInDir())) {
            TObject *objInDir = keyInDir->ReadObj();
            if (!objInDir->InheritsFrom("TTree")) continue;
            
            TTree *tree = (TTree*)objInDir;
            Long64_t treeSize = tree->GetZipBytes();
            
            std::cout << "  Tree: " << tree->GetName() 
                      << " (Entries: " << tree->GetEntries() 
                      << ", Size: " << treeSize/1024.0 << " KB)" << std::endl;
            
            // Collect branch sizes
            std::vector<std::pair<Long64_t, std::string>> branches;
            TObjArray *branchList = tree->GetListOfBranches();
            
            for (int i = 0; i < branchList->GetEntries(); i++) {
                TBranch *branch = (TBranch*)branchList->At(i);
                branches.push_back({branch->GetZipBytes(), branch->GetName()});
            }
            
            // Sort branches by size
            std::sort(branches.begin(), branches.end(),
                      [](const auto &a, const auto &b) { return a.first > b.first; });
            
            // Print top 10 branches
            std::cout << "  Top branches:" << std::endl;
            int branchCount = 0;
            for (const auto &br : branches) {
                if (branchCount >= 10) {
                    std::cout << "    ... and " << branches.size() - 10 << " more branches" << std::endl;
                    break;
                }
                double brPct = treeSize > 0 ? 100.0 * br.first / treeSize : 0;
                std::cout << "    " << std::left << std::setw(35) << br.second
                          << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                          << br.first/1024.0 << " KB"
                          << std::setw(8) << brPct << "%" << std::endl;
                branchCount++;
            }
        }
        count++;
    }
    
    std::cout << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "Analysis complete." << std::endl;
    
    f->Close();
}
