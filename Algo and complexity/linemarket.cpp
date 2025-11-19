#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>
#include <cstdio>

using namespace std;

struct Region {
    int s;
    int f;
};

bool canPlace(int N, const vector<Region>& regions, int D) {
    if (D == 0) {
        long long total = 0;
        for (const auto& region : regions) total += (region.f - region.s + 1);
        return total >= N;
    }

    int placed = 0;
    bool hasLast = false;
    int last = 0;

    for (const auto& region : regions) {
        int pos = hasLast ? max((int)region.s, last + D) : region.s;
        if (pos > region.f) continue;

        int add = 1 + (region.f - pos) / D;
        placed += add;
        last = pos + (add - 1) * D;
        hasLast = true;

        if (placed >= N) return true;
    }
    return false;
}

void maximum_minimum_distance(int N, vector<Region>& regions) {
    sort(regions.begin(), regions.end(), [](const Region& a, const Region& b){ return a.s < b.s; });

    if (N <= 1) {
        printf("Maximum minimum distance: 0\n");
        return;
    }

    int mn = numeric_limits<int>::max();
    int mx = numeric_limits<int>::min();
    for (const auto& r : regions) { mn = min(mn, (int)r.s); mx = max(mx, (int)r.f); }

    int lo = 0, hi = mx - mn;

    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (canPlace(N, regions, mid)) lo = mid; else hi = mid - 1;
    }

    printf("%d\n", lo);
}               

int main(int argc, char* argv[]) {
    ifstream infile("lab01-2/linemarket/input19.txt");
    if (!infile) {
        cerr << "Error: Cannot open file lab01-2/linemarket/input19.txt" << endl;
        return 1;
    }
    
    int N, M;
    infile >> N >> M;

    vector<Region> regions(M);
    for (int i = 0; i < M; i++) {
        int s, f; infile >> s >> f;
        if (s > f) std::swap(s, f);
        regions[i].s = s;
        regions[i].f = f;
    }

    infile.close();
    maximum_minimum_distance(N, regions);

    return 0;
}
