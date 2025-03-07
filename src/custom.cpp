#include <algorithm>
#include <map>
#include "rummikub.h"

bool CheckSolution(
    RummiKub &rks,
    std::vector<Tile> original_hand // copy
);

int main() {
  RummiKub rks;
  std::vector<Tile> tiles;

  /*Tile { 12,Y } is not used in the solution*/
  /*Tile { 6,B } is not used in the solution*/
  /*Tile { 7,B } is not used in the solution*/
  /*Tile { 9,Y } is not used in the solution*/
  /*Tile { 5,B } is not used in the solution*/
  /*Tile { 10,Y } is not used in the solution*/
  /*Tile { 11,Y } is not used in the solution*/
  /*Tile { 7,R } is not used in the solution*/
  /*Tile { 7,R } is not used in the solution*/
  /*Tile { 7,G } is not used in the solution*/
  /*Tile { 7,Y } is not used in the solution*/
  /*Tile { 4,B } is not used in the solution*/
  /*Tile { 7,G } is not used in the solution*/
  /*Solved incorrectly*/

  tiles.push_back({12, Yellow});
  tiles.push_back({6, Blue});
  tiles.push_back({7, Blue});
  tiles.push_back({9, Yellow});
  tiles.push_back({5, Blue});
  tiles.push_back({10, Yellow});
  tiles.push_back({11, Yellow});
  tiles.push_back({7, Red});
  tiles.push_back({7, Red});
  tiles.push_back({7, Green});
  tiles.push_back({7, Yellow});
  tiles.push_back({4, Blue});
  tiles.push_back({7, Green});

  for (auto const &t: tiles) {
    rks.Add(t);
  }

  rks.Solve();
  if (CheckSolution(rks, tiles)) {
    std::cout << "Solved correctly\n";
  } else {
    std::cout << "Solved incorrectly\n";
  }
}

bool CheckSolution(
    RummiKub &rks,
    std::vector<Tile> original_hand // copy
) {
  std::vector<std::vector<Tile>> runs = rks.GetRuns();
  std::vector<std::vector<Tile>> groups = rks.GetGroups();

  bool correct = true;
  // check tiles in solution are legal
  for (auto const &r: runs) {
    for (Tile const &t: r) {
      auto it = std::find_if(
          original_hand.begin(), original_hand.end(), [&t](Tile const &t1) {
            return t.color == t1.color && t.denomination == t1.denomination;
          });
      if (it == original_hand.end()) {
        std::cout << "Tile " << t
                  << " in solution was not in the original hand or duplicate\n";
        correct = false;
      } else {
        original_hand.erase(it);
      }
    }
  }
  for (auto const &g: groups) {
    for (Tile const &t: g) {
      auto it = std::find_if(
          original_hand.begin(), original_hand.end(), [&t](Tile const &t1) {
            return t.color == t1.color && t.denomination == t1.denomination;
          });
      if (it == original_hand.end()) {
        std::cout << "Tile " << t
                  << " in solution was not in the original hand or duplicate\n";
        correct = false;
      } else {
        original_hand.erase(it);
      }
    }
  }

  // check all tile were used in solution
  if (original_hand.size() > 0) {
    for (Tile const &t: original_hand) {
      std::cout << "Tile " << t << " is not used in the solution\n";
      correct = false;
    }
  }

  // check groups are legal
  for (auto const &g: groups) {
    unsigned long size = g.size();
    if (size > 0) { // skip if empty
      if (size < 3 || size > 4) {
        std::cout << "Group has incorrect size\n";
        correct = false;
      } else { // size is 3 or 4
        int denomination = g[0].denomination;
        std::map<Color, int> counts;
        for (unsigned long i = 0; i < size; ++i) {
          if (g[i].denomination != denomination) {
            std::cout << "Group denominations do not match\n";
            correct = false;
          }
          ++counts[g[i].color];
        }

        // check counts of colors
        for (auto const &c: counts) {
          if (c.second > 1) {
            std::cout << "Group contains tiles of the same color\n";
            correct = false;
          }
        }
      }
    }
  }

  // check runs are legal
  for (auto const &r: runs) {
    int counts[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    Color color = r[0].color;

    for (unsigned long i = 0; i < r.size(); ++i) {
      if (r[i].color != color) {
        std::cout << "Run colors do not match\n";
        correct = false;
      }
      ++counts[r[i].denomination];
    }

    for (int const &c: counts) {
      if (c > 1) {
        std::cout << "Run contains tiles of the same denomination\n";
        correct = false;
      }
    }

    // 123--6789--- is a valid run
    if (correct) {
      int len = 0;
      for (int b: counts) {
        if (b) { // is tile
          if (len > 0) { // continued run
            ++len;
          } else { // new run started
            len = 1;
          }
        } else { // no tile
          if (len > 0) { // run ended
            if (len < 3) {
              std::cout << "Run contains less than 3 tiles \n";
              correct = false;
            }
            len = 0; // not really necessary, see "len=1" above
          } else { // no tile and no run going
          }
        }
      }
      // check the length of the last run
      if (counts[12] && len < 3) {
        correct = false;
        std::cout << "Run contains less than 3 tiles \n";
      }
    }
  }
  return correct;
}
