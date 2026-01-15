#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <tuple>
#include <set>

using namespace std;

void solve() {
  int n, m;
  cin >> n >> m;
  vector<string> grid(n, "");
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
        char c;
        cin >> c;
        grid[i] += c;
    }
  }

  map<tuple<int, int, char>, bool> visited;
  queue<tuple<int, int, char>> q;

  set<pair<int, int>> ladder;
  set<pair<int, int>> dest;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (grid[i][j] == 'I') {
        ladder.insert({i, j});
        grid[i][j] = 'E';
      }
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (grid[i][j] == 'L') {
        dest.insert({i, j});
        grid[i][j] = 'E';
      }
    }
  }

  if (ladder.size() < 2 || dest.size() < 2) {
    cout << "Impossible" << endl;
    return;
  }

  auto it = *ladder.begin();
  auto ladder_it = ladder.begin();
  ++ladder_it;
  auto it2 = *ladder_it;
  int difx = it2.first - it.first;
  int dify = it2.second - it.second;

  char dir = difx == 0 ? 'H' : 'V';

  auto itd = *dest.begin();
  auto dest_it = dest.begin();
  ++dest_it;
  auto itd2 = *dest_it;
  int difxd = itd2.first - itd.first;
  int difyd = itd2.second - itd.second;

  int len1 = ladder.size();
  int len2 = dest.size();

  if (len1 != len2) {
    cout << "Impossible" << endl;
    return;
  }

  char dird = difxd == 0 ? 'H' : 'V';

  auto destination = tuple{itd.first, itd.second, dird};

  q.push({it.first, it.second, dir});
  visited[{it.first, it.second, dir}] = true;

  int cnt = 0;

  while (!q.empty()) {
    int sz = q.size();

    for (int i = 0; i < sz; i++) {
      auto [x, y, d] = q.front();
      q.pop();

      if (make_tuple(x, y, d) == destination) {
        cout << cnt << endl;
        return;
      }

      if (d == 'H') {
        // move left
        if (y - 1 >= 0 && grid[x][y - 1] != 'B' && !visited[{x, y - 1, 'H'}]) {
          visited[{x, y - 1, 'H'}] = true;
          q.push({x, y - 1, 'H'});
        }

        // move right
        if (y + len1 < m) {
          bool canMove = true;
          for (int k = 0; k < len1; k++) {
            if (grid[x][y + len1 - 1 + 1] == 'B') {
              canMove = false;
              break;
            }
          }
          if (canMove && !visited[{x, y + 1, 'H'}]) {
            visited[{x, y + 1, 'H'}] = true;
            q.push({x, y + 1, 'H'});
          }
        }

        if (x + len1 - 1 < n && y + len1 - 1 < m) {
          bool canRotate = true;
          for (int k = 0; k < len1 && canRotate; k++) {
            for (int l = 0; l < len1 && canRotate; l++) {
              if (x + k >= n || y + l >= m || grid[x + k][y + l] == 'B') {
                canRotate = false;
              }
            }
          }

          if (canRotate && !visited[{x, y, 'V'}]) {
            visited[{x, y, 'V'}] = true;
            q.push({x, y, 'V'});
          }

          if (canRotate && !visited[{x, y + len1 - 1, 'V'}]) {
            visited[{x, y + len1 - 1, 'V'}] = true;
            q.push({x, y + len1 - 1, 'V'});
          }
        }

        if (x - len1 + 1 >= 0 && y + len1 - 1 < m) {
          bool canRotate = true;
          for (int k = 0; k < len1 && canRotate; k++) {
            for (int l = 0; l < len1 && canRotate; l++) {
              if (x - len1 + 1 + k < 0 || x - len1 + 1 + k >= n || 
                  y + l < 0 || y + l >= m || 
                  grid[x - len1 + 1 + k][y + l] == 'B') {
                canRotate = false;
              }
            }
          }

          if (canRotate && !visited[{x - len1 + 1, y, 'V'}]) {
            visited[{x - len1 + 1, y, 'V'}] = true;
            q.push({x - len1 + 1, y, 'V'});
          }

          if (canRotate && !visited[{x - len1 + 1, y + len1 - 1, 'V'}]) {
            visited[{x - len1 + 1, y + len1 - 1, 'V'}] = true;
            q.push({x - len1 + 1, y + len1 - 1, 'V'});
          }
        }

        if (x - 1 >= 0) {
          bool pos = true;
          for (int l = 0; l < len1; l++) {
            if (grid[x - 1][y + l] == 'B') {
              pos = false;
              break;
            }
          }

          if (pos && !visited[{x - 1, y, 'H'}]) {
            visited[{x - 1, y, 'H'}] = true;
            q.push({x - 1, y, 'H'});
          }
        }

        if (x + 1 < n) {
          bool pos = true;
          for (int l = 0; l < len1; l++) {
            if (grid[x + 1][y + l] == 'B') {
              pos = false;
              break;
            }
          }

          if (pos && !visited[{x + 1, y, 'H'}]) {
            visited[{x + 1, y, 'H'}] = true;
            q.push({x + 1, y, 'H'});
          }
        }
      }

      if (d == 'V') {
        // move up
        if (x - 1 >= 0 && grid[x - 1][y] != 'B' && !visited[{x - 1, y, 'V'}]) {
          visited[{x - 1, y, 'V'}] = true;
          q.push({x - 1, y, 'V'});
        }

        // move down
        if (x + len1 < n && grid[x + len1][y] != 'B' &&
            !visited[{x + 1, y, 'V'}]) {
          visited[{x + 1, y, 'V'}] = true;
          q.push({x + 1, y, 'V'});
        }

        if (y + len1 - 1 < m) {
          bool canRotate = true;
          for (int k = 0; k < len1; k++) {
            for (int l = 0; l < len1; l++) {
              if (grid[x + k][y + l] == 'B') {
                canRotate = false;
                break;
              }
            }
          }

          if (canRotate && !visited[{x, y, 'H'}]) {
            visited[{x, y, 'H'}] = true;
            q.push({x, y, 'H'});
          }

          if (canRotate && !visited[{x + len1 - 1, y, 'H'}]) {
            visited[{x + len1 - 1, y, 'H'}] = true;
            q.push({x + len1 - 1, y, 'H'});
          }
        }

        if (y + 1 >= len1) {
          bool canRotate = true;
          for (int k = 0; k < len1; k++) {
            for (int l = 0; l < len1; l++) {
              if (grid[x + k][y - len1 + 1 + l] == 'B') {
                canRotate = false;
                break;
              }
            }
          }

          if (canRotate && !visited[{x, y - len1 + 1, 'H'}]) {
            visited[{x, y - len1 + 1, 'H'}] = true;
            q.push({x, y - len1 + 1, 'H'});
          }

          if (canRotate && !visited[{x + len1 - 1, y - len1 + 1, 'H'}]) {
            visited[{x + len1 - 1, y - len1 + 1, 'H'}] = true;
            q.push({x + len1 - 1, y - len1 + 1, 'H'});
          }
        }

        if (y - 1 >= 0) {
          bool pos = true;
          for (int k = 0; k < len1; k++) {
            if (grid[x + k][y - 1] == 'B') {
              pos = false;
              break;
            }
          }

          if (pos && !visited[{x, y - 1, 'V'}]) {
            visited[{x, y - 1, 'V'}] = true;
            q.push({x, y - 1, 'V'});
          }
        }

        if (y + 1 < m) {
          bool pos = true;
          for (int k = 0; k < len1; k++) {
            if (grid[x + k][y + 1] == 'B') {
              pos = false;
              break;
            }
          }

          if (pos && !visited[{x, y + 1, 'V'}]) {
            visited[{x, y + 1, 'V'}] = true;
            q.push({x, y + 1, 'V'});
          }
        }
      }
    }
    cnt++;
  }

  cout << "Impossible" << endl;
}

int main() {

  int tt;
  tt = 1;
  // cin >> tt;
  while (tt--) {
    solve();
  }
}