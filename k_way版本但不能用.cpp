//11327104 林采寧 11327111 林方晴

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <queue>
struct Edge {
    char putID[10];
    char getID[10];
    float weight;
};

struct Index {
    float weight;
    int offset; // 先用int不確定會不會爆
};

struct Node {
    Edge edge;
    int runFrom;
};

struct Compare {
    bool operator()(Node a, Node b) {
        return a.edge.weight < b.edge.weight;
    }
};

class ExternalSorter {
  private:
    std::string fileNum;

    void Sort(std::vector<Edge>& buffer, int size);
    void InnerSort(std::string inputFile, std::vector<std::string>& runFiles);
    void mergeFourFiles(std::vector<std::string>, std::string outFile);
    void MergeSort4Way(std::vector<std::string>& runFiles, std::string OutputFile);

  public:
    void ExternalSort();
    void SetFileNum(std::string fileNo) {fileNum = fileNo;}
};

class PrimaryIndex {
  private:
    std::vector<Index> primaryIndex;

    void PrintPrimaryIndex();

  public:
    void EstablishPrimaryIndex (std::string fileNo);
};

//--------------------------------------------------//
void PrintMenu();
bool GetFileNum(std::string &fileNo);
std::string RemoveSpace(std::string target);
//--------------------------------------------------//

int main() {
    std::string temp;
    do {
        PrintMenu();
        std::cout << "Input the file name: [0]Quit\n";
        std::string fileNo;
        while (!GetFileNum(fileNo)) {
            if (fileNo == "0") break;
            std::cout << "Input the file name: [0]Quit\n";
        }
        if (fileNo == "0") {
            std::cout << "\n[0]Quit or [Any other key]continue?\n";
            std::cin >> temp;
            if (temp != "0") std::cout << std::endl;
            continue;
        }
        ExternalSorter externalSorter;
        externalSorter.SetFileNum(fileNo);
        externalSorter.ExternalSort();
        PrimaryIndex primaryIndex;
        primaryIndex.EstablishPrimaryIndex(fileNo);

        std::cout << "[0]Quit or [Any other key]continue?\n\n";
        std::cin >> temp;
    } while(temp != "0");
    return 0;
}


//----------------------------------------------------------------------//
void PrintMenu() {
    std::cout << "* Data Structures and Algorithms *" << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << "* 1. External merge sort on file *" << std::endl;
    std::cout << "* 2: Construct the primary index *" << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << "*** The buffer size is 300" << std::endl;
    std::cout << "##################################" << std::endl;
    std::cout << "Mission 1: External merge sort " << std::endl;
    std::cout << "##################################\n" << std::endl;
}

std::string RemoveSpace(std::string target) {
    std::string to_return;
    for (int i = 0; i < target.size() ;i++) {
        if (target[i] != ' ' && target[i] != '\t' && target[i] != '\n' && target[i] != '\r') {
            to_return = to_return + target[i];
        }
    }
    return to_return;
}

bool GetFileNum(std::string &fileNo) {
    std::cin >> fileNo;
    fileNo = RemoveSpace(fileNo);
    std::string inputFile = "pairs" + fileNo + ".bin";
    std::ifstream fin(inputFile, std::ios::binary);
    if (fileNo == "0") return false;
    if (!fin.is_open()) {
        std::cout << "\n" << inputFile << " does not exist!!!\n\n";
        return false;
    }
    return true;
}

//----------------------------ExternalSorter----------------------------------//
void ExternalSorter::Sort(std::vector<Edge>& buffer, int size) {
    std::sort(buffer.begin() , buffer.begin() + size, [](Edge& a,Edge& b) { return a.weight > b.weight;});
}

void ExternalSorter::InnerSort(std::string inputFile, std::vector<std::string>& runFiles) {
    std::ifstream fin(inputFile, std::ios::binary);

    std::vector<Edge> buffer(300);
    int runCount = 0;

    while (fin) {
        fin.read(reinterpret_cast<char*>(buffer.data()), 300 * sizeof(Edge));//讀300筆Edge到buffer裡
        int realsize = fin.gcount() / sizeof(Edge);//計算實際讀到的筆數.gcount()可以知道
        if (realsize == 0) break;
        Sort(buffer, realsize);

        std::string runFileName = "run_0_" + std::to_string(runCount) + ".bin";
        std::ofstream fout(runFileName, std::ios::binary);
        fout.write((char*)buffer.data() , realsize * sizeof(Edge));
        fout.close();

        runFiles.push_back(runFileName);
        runCount++;
    }

    fin.close();
}

void ExternalSorter:: mergeFourFiles(std::vector<std::string> chooseToMerge, std::string outFile) {
    //std::ifstream fin1(file1, std::ios::binary);
    //std::ifstream fin2(file2, std::ios::binary);
    // 開全部要合併的檔
    std::vector<std::ifstream> fin(chooseToMerge.size());
    for (int i = 0; i < chooseToMerge.size(); i++) {
        fin[i].open(chooseToMerge[i], std::ios::binary);
    }
    std::ofstream fout(outFile, std::ios::binary);// 輸出檔

    std::vector<std::vector<Edge> > buffers(chooseToMerge.size(), std::vector<Edge>(100)); // 每個檔有自己的buffer
    std::vector<Edge> outBuffer;

    // int size1 = 0, size2 = 0;
    std::vector<int> size(chooseToMerge.size());
    // int pos1 = 0, pos2 = 0;
    std::vector<int> pos(chooseToMerge.size());


    std::priority_queue< Node, std::vector<Node>, Compare> heap;

    /*file1 file2都先取一些資料出來
    fin1.read((char*)buffer1.data(), 100 * sizeof(Edge));
    size1 = fin1.gcount() / sizeof(Edge);
    fin2.read((char*)buffer2.data(), 100 * sizeof(Edge));
    size2 = fin2.gcount() / sizeof(Edge);*/
    for (int i = 0; i < chooseToMerge.size(); i++) {
        fin[i].read( (char*)buffers[i].data(), 100 * sizeof(Edge));
        size[i] = fin[i].gcount() / sizeof(Edge);
        pos[i] = 0;


        if (size[i] > 0) {
            Node node;
            // 第一筆資料放入 Heap
            node.edge = buffers[i][0];
            // 記錄來自哪個檔 因為後續要從這個檔的buffer補東西進heap
            node.runFrom = i;
            heap.push(node);
            pos[i] = 1;
        }
    }

    while (!heap.empty()) { //兩個資料都還有
        Node maxData = heap.top(); // priory_queue會自己找最大(根據自己寫的compare)
        heap.pop();

        // 放入輸出 buffer
        outBuffer.push_back(maxData.edge);

        // buffer滿了再一次寫出
        if (outBuffer.size() == 100) {
            fout.write((char*)outBuffer.data(), 100 * sizeof(Edge));
            outBuffer.clear();
        }

        int fromFile = maxData.runFrom;

        // 補東西進heap
        if (pos[fromFile] < size[fromFile]) { // 目前拿出資料的那個檔的buffer還有東西

            Node next;

            // 拿下一筆
            next.edge = buffers[fromFile][pos[fromFile]];

            next.runFrom = fromFile;
            // 放進heap
            heap.push(next);
            pos[fromFile]++;
        } 
        else { // buffer沒東西了 再從檔案讀一點出來
            fin[fromFile].read((char*)buffers[fromFile].data(), 100 * sizeof(Edge));
            size[fromFile] = fin[fromFile].gcount() / sizeof(Edge);
            pos[fromFile] = 0;
            if (size[fromFile] > 0) {

                Node next;
                // 第一筆資料放入
                next.edge = buffers[fromFile][0];
                next.runFrom = fromFile;
                heap.push(next);
                pos[fromFile] = 1;
            }
        }
    }
    // outbuffer剩餘資料寫完
    if (!outBuffer.empty()) {

        fout.write(
            (char*)outBuffer.data(),
            outBuffer.size() * sizeof(Edge)
        );
    }

    // 關檔案
    for (int i = 0; i < chooseToMerge.size(); i++) {
        fin[i].close();
    }
    fout.close();

}

void ExternalSorter::MergeSort4Way(std::vector<std::string>& runFiles, std::string OutputFile) {
    int num = 1;
    // runFiles 不只一個 run,繼續兩兩合併
    while (runFiles.size() > 1) {
        std::cout << "Now there are " << runFiles.size() << " runs.\n" << std::endl;
        std::vector<std::string> nextRuns;//存下一圈的
        std::vector<std::string> chooseToMerge; // 當下要合併的檔(可能有2 3 4)
        for (int i = 0; i < runFiles.size(); i+=4) {
            for (int j = i; j < i + 4; j++) { // 最多一次放4個檔
                chooseToMerge.push_back(runFiles[j]);
            }
            std::string outName = "run_" + std::to_string(num) + "_" + std::to_string(i / 4) + ".bin";
                
            //剩一個，直接改檔名
            if (chooseToMerge.size() == 1) {
                std::rename(runFiles[i].c_str(), outName.c_str());
                nextRuns.push_back(outName);
            }else {
                mergeFourFiles(chooseToMerge, outName);

                for (int a = 0; a < chooseToMerge.size(); a++) { // 用過的檔刪掉
                    std::remove(chooseToMerge[a].c_str());
                }
                nextRuns.push_back(outName);
            }
        }
        runFiles = nextRuns;
        num++;
    }

    std::cout << "Now there are " << runFiles.size() << " runs.\n" << std::endl;
    std::remove(OutputFile.c_str());
    std::rename(runFiles[0].c_str(), OutputFile.c_str());//最後那個Run就是結果
}

void ExternalSorter::ExternalSort() {
    std::string inputFile = "pairs" + fileNum + ".bin";
    std::string outputFile = "order" + fileNum + ".bin";

    auto internalStart = std::chrono::high_resolution_clock::now();
    std::vector<std::string> runFiles;
    InnerSort(inputFile, runFiles);//初始化最開始用內部排序
    auto internalEnd = std::chrono::high_resolution_clock::now();


    std::cout << "\nThe internal sort is completed. Check the initial sorted runs! \n" << std::endl;

    auto externalStart = std::chrono::high_resolution_clock::now();
    MergeSort4Way(runFiles, outputFile);
    auto externalEnd = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> internal_ms = internalEnd - internalStart;
    std::chrono::duration<double, std::milli> external_ms = externalEnd - externalStart;
    double total_ms = internal_ms.count() + external_ms.count();

    std::cout << "The execution time ..." << std::endl;
    std::cout << "Internal Sort = " << internal_ms.count() << " ms" << std::endl;
    std::cout << "External Sort = " << external_ms.count() << " ms" << std::endl;
    std::cout << "Total Execution Time = " << total_ms << " ms\n" << std::endl;
}

//-----------------------------PrimaryIndex-----------------------------------//

void PrimaryIndex::PrintPrimaryIndex() {
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
    std::cout << "Mission 2: Build the primary index \n";
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n";
    std::cout << "<Primary index>: (key, offset)\n";
    for (int i = 0; i < primaryIndex.size(); i++) {
        std::cout << "[" << i + 1 << "] (" <<  primaryIndex[i].weight << ", " << primaryIndex[i].offset << ")\n";
    }
    std::cout << std::endl;
}

void PrimaryIndex::EstablishPrimaryIndex (std::string fileNo) {
    std::string orderFile = "order" + fileNo + ".bin";
    Edge edge;
    float preWeight = -1;
    int offset = 0;
    std::ifstream fin(orderFile, std::ios::binary);
    while (fin.read((char*)&edge, sizeof(edge))) { // 每次讀一筆
        if (edge.weight != preWeight) { // 是新的weight了
            Index index;
            index.weight = edge.weight;
            index.offset = offset;
            primaryIndex.push_back(index);
            preWeight = edge.weight;
        }
        offset++;// 繼續下一筆
    }
    fin.close();
    PrintPrimaryIndex();
}
