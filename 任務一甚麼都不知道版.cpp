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

//--------------------------------------------------//
void PrintMenu();
bool GetFileNum(std::string &fileNo);
void Sort(std::vector<Edge>& buffer, int size);
void ExternalSort(std::string fileNo);
void InnerSort(std::string inputFile, std::vector<std::string>& runFiles);
void mergeTwoFiles(std::string file1, std::string file2, std::string outFile);
void MergeSort2Way(std::vector<std::string>& runFiles, std::string OutputFile);
//--------------------------------------------------//

int main() {
    while (true) {
        PrintMenu();
        std::cout << "\nInput the file name: [0]Quit\n";
        std::string fileNo;
        while (!GetFileNum(fileNo)) {
            if (fileNo == "0") return 0;
            std::cout << "\nInput the file name: [0]Quit\n";
        }
        ExternalSort(fileNo);
    }
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

//----------------------------------------------------------------------//
void Sort(std::vector<Edge>& buffer, int size) {
    std::sort(buffer.begin() , buffer.begin() + size, [](Edge& a, Edge& b) { return a.weight > b.weight;});
}

void InnerSort(std::string inputFile, std::vector<std::string>& runFiles) {
    std::ifstream fin(inputFile, std::ios::binary);

    std::vector<Edge> buffer(300);
    int runCount = 0;

    while (fin) {
        fin.read(reinterpret_cast<char*>(buffer.data()), 300 * sizeof(Edge));//讀300比Edge到buffer裡
        int realsize = fin.gcount() / sizeof(Edge);//計算實際讀到的筆數.gcount()可以知道
        if (realsize == 0) break;
        Sort(buffer, realsize);

        std::string runFileName = "run_0_" + std::to_string(runCount) + ".bin";
        std::ofstream fout(runFileName, std::ios::binary);
        fout.write(reinterpret_cast<const char*>(buffer.data()), realsize * sizeof(Edge));
        fout.close();

        runFiles.push_back(runFileName);
        runCount++;
    }

    fin.close();
}

void mergeTwoFiles(std::string file1, std::string file2, std::string outFile) {
    std::ifstream fin1(file1, std::ios::binary);
    std::ifstream fin2(file2, std::ios::binary);
    std::ofstream fout(outFile, std::ios::binary);

    Edge read1, read2;
    bool hasRead1 = false , hasRead2 = false;
    fin1.read((char*)&read1, sizeof(Edge));
    if (fin1) hasRead1 = true;
    fin2.read((char*)&read2, sizeof(Edge));
    if (fin2) hasRead2 = true;

    while (hasRead1 && hasRead2) {//兩個資料都還有
        if (read1.weight >= read2.weight) {
            fout.write(reinterpret_cast<const char*>(&read1), sizeof(Edge));
            fin1.read((char*)&read1, sizeof(Edge));
            if (!fin1) hasRead1 = false;
        } else {
            fout.write(reinterpret_cast<const char*>(&read2), sizeof(Edge));
            fin2.read((char*)&read2, sizeof(Edge));
            if (!fin2) hasRead2 = false;
        }
    }

    //剩下的
    while (hasRead1) {
        fout.write(reinterpret_cast<const char*>(&read1), sizeof(Edge));
        fin1.read((char*)&read1, sizeof(Edge));
        if (!fin1) hasRead1 = false;
    }
    while (hasRead2) {
        fout.write(reinterpret_cast<const char*>(&read2), sizeof(Edge));
        fin2.read((char*)&read2, sizeof(Edge));
        if (!fin2) hasRead2 = false;
    }

    fin1.close();
    fin2.close();
    fout.close();
}

void MergeSort2Way(std::vector<std::string>& runFiles, std::string OutputFile) {
    int num = 1;
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

void ExternalSort(std::string fileNo) {
    std::string inputFile = "pairs" + fileNo + ".bin";
    std::string outputFile = "order" + fileNo + ".bin";

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
