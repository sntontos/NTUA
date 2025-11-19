#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>
#include <cstdio>

int main(){
    
    std::ifstream inputfile("lab01-2/maxintvsum/input19.txt");
    if (!inputfile) {
        std::cerr << "Error: Cannot open input file" << std::endl;
        return 1;
    }

    int N, Q;
    inputfile >> N >> Q;

    std::vector<long long> S(N + 1, 0);
    for (int i = 1; i <= N; i++) {
        int value;
        inputfile >> value;
        S[i] = S[i-1] + value;
    }

    int k = floor(std::log2(N + 1));

    std::vector<int> log2Table(N + 2);
    log2Table[1] = 0;
    for (int i = 2; i <= N + 1; i++) {
        log2Table[i] = log2Table[i/2] + 1;
    }

    std::vector<std::vector<long long>> minTable(N + 1, std::vector<long long>(k + 1));    
    for (int i = 0; i <= N; i++) {
        minTable[i][0] = S[i];
    }
    
    for (int j = 1; j <= k; j++) {
        for (int i = 0; i + (1 << j) <= N + 1; i++) {
            minTable[i][j] = std::min(minTable[i][j-1], minTable[i + (1 << (j-1))][j-1]);
        }
    }
    int t, a, b, L, R;
    long long result;
    for(int q = 0; q < Q; q++) {
        inputfile >> t >> a >> b;
        L = std::max(0, t - b);
        R = t - a;
        int j = log2Table[R - L + 1];
        long long minVal = std::min(minTable[L][j], minTable[R - (1 << j) + 1][j]);
        result = S[t] - minVal;
        std::cout << result << std::endl;
    }
    inputfile.close();
    
    return 0;
}