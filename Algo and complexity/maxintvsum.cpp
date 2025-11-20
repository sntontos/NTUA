#include <algorithm>
#include <cmath>
#include <vector>

using namespace std;

int main() {

  int N, Q;
  scanf("%d", &N);
  scanf("%d", &Q);

  vector<long long> S(N + 1, 0);
  for (int i = 1; i <= N; i++) {
    int value;
    scanf("%d", &value);
    S[i] = S[i - 1] + value;
  }

  int k = floor(log2(N + 1));

  vector<int> log2Table(N + 2);
  log2Table[1] = 0;
  for (int i = 2; i <= N + 1; i++) {
    log2Table[i] = log2Table[i / 2] + 1;
  }

  vector<vector<long long>> minTable(N + 1, vector<long long>(k + 1));
  for (int i = 0; i <= N; i++) {
    minTable[i][0] = S[i];
  }

  for (int j = 1; j <= k; j++) {
    for (int i = 0; i + (1 << j) <= N + 1; i++) {
      minTable[i][j] =
          min(minTable[i][j - 1], minTable[i + (1 << (j - 1))][j - 1]);
    }
  }
  int t, a, b, L, R;
  long long result;
  for (int q = 0; q < Q; q++) {
    scanf("%d", &t);
    scanf("%d", &a);
    scanf("%d", &b);
    L = max(0, t - b);
    R = t - a;
    int j = log2Table[R - L + 1];
    long long minVal = min(minTable[L][j], minTable[R - (1 << j) + 1][j]);
    result = S[t] - minVal;
    printf("%lld\n", result);
  }

  return 0;
}