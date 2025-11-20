#include <algorithm>
#include <climits>
#include <cstdio>
#include <vector>

using namespace std;

struct Region {
  int s;
  int f;
};

bool canPlace(long long N, const vector<Region> &regions, int D) {
  if (D == 0) {
    long long total = 0;
    for (const auto &region : regions)
      total += (region.f - region.s + 1);
    return total >= N;
  }

  int placed = 0;
  bool hasLast = false;
  int last = 0;

  for (const auto &region : regions) {
    int pos = hasLast ? max((int)region.s, last + D) : region.s;
    if (pos > region.f)
      continue;

    int add = 1 + (region.f - pos) / D;
    placed += add;
    last = pos + (add - 1) * D;
    hasLast = true;

    if (placed >= N)
      return true;
  }
  return false;
}

void maximum_minimum_distance(long long N, vector<Region> &regions) {
  sort(regions.begin(), regions.end(),
       [](const Region &a, const Region &b) { return a.s < b.s; });

  if (N <= 1) {
    printf("Maximum minimum distance: 0\n");
    return;
  }

  int mn = INT_MAX;
  int mx = INT_MIN;
  for (const auto &r : regions) {
    mn = min(mn, (int)r.s);
    mx = max(mx, (int)r.f);
  }

  long long lo = 0, hi = mx - mn;
  while (lo < hi) {
    long long mid = lo + (hi - lo + 1) / 2;
    if (canPlace(N, regions, mid))
      lo = mid;
    else
      hi = mid - 1;
  }

  printf("%lld\n", lo);
}

int main(int argc, char *argv[]) {

  long long N;
  int M;
  scanf("%lld", &N);
  scanf("%d", &M);

  vector<Region> regions(M);
  for (int i = 0; i < M; i++) {
    int s, f;
    scanf("%d", &s);
    scanf("%d", &f);
    if (s > f)
      swap(s, f);
    regions[i].s = s;
    regions[i].f = f;
  }

  maximum_minimum_distance(N, regions);

  return 0;
}
