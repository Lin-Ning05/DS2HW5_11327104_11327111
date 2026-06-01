//11327104 林采寧 11327111 林方晴

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

struct Edge {
    char putID[10];
    char getID[10];
    float weight;
};

struct Index {
    float weight;
    int offset; // 先用int不確定會不會爆
};

class ExternalSorter {
  private:
    std::string fileNum;

    void Sort(std::vector<Edge>& buffer, int size);
    void InnerSort(std::string inputFile, std::vector<std::string>& runFiles);
    void mergeTwoFiles(std::string file1, std::string file2, std::string outFile);
    void MergeSort2Way(std::vector<std::string>& runFiles, std::string OutputFile);

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
        std::cout << "\nInput the file name: [0]Quit\n";
        std::string fileNo;
        while (!GetFileNum(fileNo)) {
            if (fileNo == "0") return 0;
            std::cout << "\nInput the file name: [0]Quit\n";
        }
        ExternalSorter externalSorter;
        externalSorter.SetFileNum(fileNo);
        externalSorter.ExternalSort();
        PrimaryIndex primaryIndex;
        primaryIndex.EstablishPrimaryIndex(fileNo);

        std::cout << "\n[0]Quit or [Any other key]continue?\n";
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
    std::cout << "##################################" << std::endl;
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
        std::cout << inputFile << " does not exist!!!";
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

void ExternalSorter::mergeTwoFiles(std::string file1, std::string file2, std::string outFile) {
    std::ifstream fin1(file1, std::ios::binary);
    std::ifstream fin2(file2, std::ios::binary);
    std::ofstream fout(outFile, std::ios::binary);

    std::vector<Edge> buffer1(100);
    std::vector<Edge> buffer2(100);
    std::vector<Edge> outBuffer;

    int size1 = 0, size2 = 0;
    int pos1 = 0, pos2 = 0;

    // file1 file2都先取一些資料出來
    fin1.read((char*)buffer1.data(), 100 * sizeof(Edge));
    size1 = fin1.gcount() / sizeof(Edge);
    fin2.read((char*)buffer2.data(), 100 * sizeof(Edge));
    size2 = fin2.gcount() / sizeof(Edge);

    while (size1 > 0 && size2 > 0) { //兩個資料都還有
        if (buffer1[pos1].weight >= buffer2[pos2].weight) {
            outBuffer.push_back(buffer1[pos1]);
            pos1++;
        } else {
            outBuffer.push_back(buffer2[pos2]);
            pos2++;
        }

        if (pos1 == size1) {//讀完了就讀新的
            fin1.read((char*)buffer1.data(), 100 * sizeof(Edge));
            size1 = fin1.gcount() / sizeof(Edge);
            if (size1 == 0) {//沒有1了就把剩下的2寫完
                fout.write((char*)outBuffer.data(), outBuffer.size() * sizeof(Edge));
                outBuffer.clear();
                while (pos2 < size2) {
                    fout.write((char*)&buffer2[pos2], sizeof(Edge));
                    pos2++;
                }

                while (true) {
                    fin2.read((char*)buffer2.data(), 100 * sizeof(Edge));
                    size2 = fin2.gcount() / sizeof(Edge);
                    if (size2 == 0) break;
                    fout.write((char*)buffer2.data(), size2 * sizeof(Edge));
                }
                break;
            }
            pos1 = 0;
        }
        if (pos2 == size2) {
            fin2.read((char*)buffer2.data(), 100 * sizeof(Edge));
            size2 = fin2.gcount() / sizeof(Edge);
            if (size2 == 0) {
                fout.write((char*)outBuffer.data(), outBuffer.size() * sizeof(Edge));
                outBuffer.clear();
                while (pos1 < size1) {
                    fout.write((char*)&buffer1[pos1], sizeof(Edge));
                    pos1++;
                }

                while (true) {
                    fin1.read((char*)buffer1.data(), 100 * sizeof(Edge));
                    size1 = fin1.gcount() / sizeof(Edge);
                    if (size1 == 0) break;
                    fout.write((char*)buffer1.data(), size1 * sizeof(Edge));
                }
                break;
            }
            pos2 = 0;
        }
        // buffer滿了再一次寫出
        if (outBuffer.size() == 100) {
            fout.write((char*)outBuffer.data(), 100 * sizeof(Edge));
            outBuffer.clear();
        }
    }

    if (!outBuffer.empty()) {
        fout.write((char*)outBuffer.data(), outBuffer.size() * sizeof(Edge));
    }

    fin1.close();
    fin2.close();
    fout.close();
}

void ExternalSorter::MergeSort2Way(std::vector<std::string>& runFiles, std::string OutputFile) {
    int num = 1;
    // runFiles 不只一個 run,繼續兩兩合併
    while (runFiles.size() > 1) {
        std::cout << "Now there are " << runFiles.size() << " runs." << std::endl;
        std::vector<std::string> nextRuns;//存下一圈的

        for (int i = 0; i < runFiles.size(); i+=2) {
            if (i + 1 < runFiles.size()) {// 有兩個檔案可以合併
                std::string outName = "run_" + std::to_string(num) + "_" + std::to_string(i / 2) + ".bin";
                mergeTwoFiles(runFiles[i], runFiles[i + 1], outName);
                nextRuns.push_back(outName);
                std::remove(runFiles[i].c_str());
                std::remove(runFiles[i + 1].c_str());
            } else {//剩一個，直接下一圈做
                std::string outName = "run_" + std::to_string(num) + "_" + std::to_string(i / 2) + ".bin";
                std::rename(runFiles[i].c_str(), outName.c_str());
                nextRuns.push_back(outName);
            }
        }
        runFiles = nextRuns;
        num++;
    }

    std::cout << "Now there are " << runFiles.size() << " runs." << std::endl;
    std::remove(OutputFile.c_str());//不知道為甚麼不把它刪掉我會留下一個垃圾
    std::rename(runFiles[0].c_str(), OutputFile.c_str());//最後那個Run就是結果
}

void ExternalSorter::ExternalSort() {
    std::string inputFile = "pairs" + fileNum + ".bin";
    std::string outputFile = "order" + fileNum + ".bin";

    auto internalStart = std::chrono::high_resolution_clock::now();
    std::vector<std::string> runFiles;
    InnerSort(inputFile, runFiles);//初始化最開始用內部排序
    auto internalEnd = std::chrono::high_resolution_clock::now();


    std::cout << "\nThe internal sort is completed. Check the initial sorted runs!" << std::endl;

    auto externalStart = std::chrono::high_resolution_clock::now();
    MergeSort2Way(runFiles, outputFile);
    auto externalEnd = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> internal_ms = internalEnd - internalStart;
    std::chrono::duration<double, std::milli> external_ms = externalEnd - externalStart;
    double total_ms = internal_ms.count() + external_ms.count();

    std::cout << "\nThe execution time ..." << std::endl;
    std::cout << "Internal Sort = " << internal_ms.count() << " ms" << std::endl;
    std::cout << "External Sort = " << external_ms.count() << " ms" << std::endl;
    std::cout << "Total Execution Time = " << total_ms << " ms" << std::endl;
}

//-----------------------------PrimaryIndex-----------------------------------//

void PrimaryIndex::PrintPrimaryIndex() {
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
    std::cout << "Mission 2: Build the primary index\n";
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n";
    std::cout << "<Primary index>: (key, offset)\n";
    for (int i = 0; i < primaryIndex.size(); i++) {
        std::cout << "[" << i + 1 << "] (" <<  primaryIndex[i].weight << ", " << primaryIndex[i].offset << ")\n";
    }
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
