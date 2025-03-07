/**
 * @file rummikub.cpp
 * @author Edgar Jose Donoso Mansilla
 * @course CS280
 * @term Spring 2025
 * @assignment# 3
 */

#include "rummikub.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

// NOTE: The proffessor recommends making it work first and then optimizing.

#define DEBUG 0

// NOTE: You are allowed to sort the hand. Nature is healing.
#define SORT_HAND 1

#if DEBUG
template<typename T, typename... Args>
void dbg(T &&x, Args &&...args) {
  std::cout << x;
  dbg(args...);
}

template<typename T>
void dbg(T &&x) {
  std::cout << x;
}

/**
 * @brief Helper function to print a vector of T
 *
 * @param vec Reference to vector to print.
 */
template<typename T>
void print_vector(const std::vector<T> &vec) {
  for (const T &t: vec) {
    std::cout << t << std::endl;
  }
}
#else
  #define dbg(...)
  #define print_vector(...)                                                    \
    do {                                                                       \
      std::ignore = std::make_tuple(__VA_ARGS__);                              \
    } while (false)
#endif

std::ostream &operator<<(std::ostream &os, Tile const &t) {
  os << "{ " << t.denomination << ",";
  switch (t.color) {
    case Red: os << "R"; break;
    case Green: os << "G"; break;
    case Blue: os << "B"; break;
    case Yellow: os << "Y"; break;
  }
  os << " }";
  return os;
}

/**
 * @brief Function to help find the index in a vector<T> by using function F.
 * This is to reduce the amount of searches that are boilerplate enough.
 *
 * @param vec Reference to vector to look inside of.
 * @param function to call for looking inside the vector. Requires signature
 * bool(const T&).
 * @return Pair which means (success, index)
 */
template<typename T, typename F>
std::pair<bool, size_t> find_index_by(const std::vector<T> &vec, F function) {
  for (size_t i = 0; i < vec.size(); i++) {
    if (function(vec.at(i))) {
      return std::make_pair(true, i);
    }
  }

  return std::make_pair(false, 0);
}

/**
 * @brief Function to help find the index in a vector<T> by using function F.
 * This is to reduce the amount of searches that are boilerplate enough.
 *
 * @param vec Reference to vector to look inside of.
 * @param function to call for looking inside the vector. Requires signature
 * bool(const T&).
 * @param filter to call for comparing found indices. Requires signature
 * bool(const T&, const T&).
 * @return Pair which means (success, index)
 */
template<typename T, typename F, typename C>
std::pair<bool, size_t>
find_index_filter(const std::vector<T> &vec, F function, C filter) {
  std::pair<bool, size_t> output{false, 0};

  for (size_t i = 0; i < vec.size(); i++) {
    // if the index is valid for this function and the filter determines its
    // more
    if (function(vec.at(i))) {
      if (!output.first || filter(vec.at(output.second), vec.at(i))) {
        output = std::make_pair(true, i);
      }
    }
  }

  return output;
}

RummiKub::RummiKub() {};

void RummiKub::Add(Tile const &tile) { tiles.push_back(tile); }

void RummiKub::Solve() {
  dbg("Solver Started\n\n");

// BUG: If the hand is sorted, even the edge case in custom doesn't fail. This
// must mean something is weird with the behaviour of the edge case that
// causes the runs to be generated weirdly. Error probably related to
// backtracking.
#if SORT_HAND
  std::sort(
      tiles.begin(), tiles.end(), [](const Tile &a, const Tile &b) -> bool {
        return a.denomination < b.denomination;
      });
#endif

  // Setting up the actions (with a level of indirection so that the vtable
  // is used)
  std::vector<std::unique_ptr<Action>> actions;
  actions.push_back(std::unique_ptr<AddToRun>(new AddToRun(runs)));
  actions.push_back(std::unique_ptr<AddToGroup>(new AddToGroup(groups)));
  actions.push_back(std::unique_ptr<CreateRun>(new CreateRun(runs)));
  actions.push_back(std::unique_ptr<CreateGroup>(new CreateGroup(groups)));

  // Calling the recursive function
  solver_recurse(0, actions);
  tiles.clear();

  dbg("\nSolver terminated\n");
}

std::vector<std::vector<Tile>> RummiKub::GetGroups() const { return groups; }

std::vector<std::vector<Tile>> RummiKub::GetRuns() const { return runs; }

bool RummiKub::validate_solution() {
  dbg("Validating Solution start\n");

  for (std::vector<Tile> &run: runs) {
    if (!validate_run(run)) {
      dbg("Validating solution end: failed\n\n");
      return false;
    }
  }

  for (std::vector<Tile> &group: groups) {
    if (!validate_group(group)) {
      dbg("Validating solution end: failed\n\n");
      return false;
    }
  }

  dbg("Validating solution end: success\n\n");
  return true;
}

bool RummiKub::solver_recurse(
    size_t current_tile, std::vector<std::unique_ptr<Action>> &actions) {
  print_runs();
  if (current_tile == tiles.size()) {
    return validate_solution();
  }

  // Checking all possible actions with the tile
  for (std::unique_ptr<Action> &action: actions) {
    bool success = action->execute(tiles.at(current_tile));
    if (!success) continue;

    // If the action could be performed, recurse
    bool recursive_success = solver_recurse(current_tile + 1, actions);
    if (recursive_success) {
      return true;
    }

    // Backtracking
    action->revert(tiles.at(current_tile));
  }

  return false;
}

RummiKub::Action::~Action() = default;

RummiKub::AddToRun::AddToRun(std::vector<std::vector<Tile>> &runs) :
    runs(runs) {}

bool RummiKub::AddToRun::execute(const Tile &tile) {
  std::pair<bool, size_t> color_index = find_index_filter(
      runs,
      [tile](const std::vector<Tile> &run) -> bool {
        // Check if the tile is the right color
        if (run.empty() || (run.at(0).color != tile.color)) {
          return false;
        };

        // Check if the tile is already in the run and if it is in sequence to
        // another
        bool is_sequence{false};
        for (const Tile &current_tile: run) {
          if (tile.denomination == current_tile.denomination) {
            return false;
          }

          if (((tile.denomination + 1) == current_tile.denomination) ||
              (tile.denomination == (current_tile.denomination + 1))) {
            is_sequence = true;
          }
        }

        return is_sequence;
      },
      [](const std::vector<Tile> &max,
         const std::vector<Tile> &current) -> bool {
        // making sure that it is the smallest possible run to add to
        return current.size() < max.size();
      });

  if (not color_index.first) {
    return false;
  }

  std::vector<Tile> &run = runs.at(color_index.second);
  run.push_back(tile);
  inserts.push_back(color_index.second);
  return true;
}

void RummiKub::AddToRun::revert(const Tile &) {
  runs.at(inserts.back()).pop_back();
  inserts.pop_back();
}

RummiKub::AddToGroup::AddToGroup(std::vector<std::vector<Tile>> &groups) :
    groups(groups) {}

bool RummiKub::AddToGroup::execute(const Tile &tile) {
  std::pair<bool, size_t> denom_index = find_index_by(
      groups,
      [tile](const std::vector<Tile> &vec) -> bool { //
        return vec.size() > 0 && vec.at(0).denomination == tile.denomination;
      });

  if (not denom_index.first) {
    return false;
  }

  std::vector<Tile> &group = groups.at(denom_index.second);
  if (group.size() + 1 > 4) {
    return false;
  }

  std::pair<bool, size_t> index_with_denom = find_index_by(
      group,
      [tile](const Tile &current_tile) -> bool { //
        return tile.color == current_tile.color;
      });

  if (!index_with_denom.first) {
    group.push_back(tile);
    inserts.push_back(denom_index.second);
    return true;
  }

  return false;
}

void RummiKub::AddToGroup::revert(const Tile &) {
  groups.at(inserts.back()).pop_back();
  inserts.pop_back();
}

RummiKub::CreateRun::CreateRun(std::vector<std::vector<Tile>> &runs) :
    runs(runs) {}

bool RummiKub::CreateRun::execute(const Tile &tile) {
  // TODO: Make this method actually see if it can join other 2 runs to create a
  // new one. This way, even if the order is weird it will pick up that there
  // are solutions.

  /*for (size_t i = 0; i + 1 < runs.size(); i++) {*/
  /*  // Check only for not yet valid runs*/
  /*  if (!runs.at(i).empty() && runs.at(i).size() >= 3 &&*/
  /*      runs.at(i).at(0).color == tile.color) {*/
  /*    continue;*/
  /*  }*/
  /*  for (size_t j = i; j < runs.size(); j++) {*/
  /*    // Check only for not yet valid runs*/
  /*    if (!runs.at(j).empty() && runs.at(j).size() >= 3 &&*/
  /*        runs.at(j).at(0).color == tile.color) {*/
  /*      continue;*/
  /*    }*/
  /**/
  /*    // Compare the two runs to see if there is a way to merge them into a*/
  /*    // single larger one (undo is going to suck to do, but it is for the next*/
  /*    // test so we'll deal with it when we get there)*/
  /*    bool test = check_for_run_bridge(tile, runs.at(i), runs.at(j));*/
  /**/
  /*    if (!test) {*/
  /*      continue;*/
  /*    }*/
  /**/
  /*    // HACK: Erina left off here.*/
  /*    // TODO: Merge the bridges here and record data required to the undo*/
  /**/
  /*    return true;*/
  /*  }*/
  /*}*/

  runs.emplace_back();
  runs.back().push_back(tile);
  return true;
}

void RummiKub::CreateRun::revert(const Tile &) { runs.pop_back(); }

bool RummiKub::CreateRun::check_for_run_bridge(
    const Tile &tile,
    const std::vector<Tile> &run_a,
    const std::vector<Tile> &run_b) {
  for (size_t i = 0; i < run_a.size(); i++) {
    for (size_t j = 0; j < run_b.size(); j++) {
      int denom_a = run_a.at(i).denomination;
      int denom_b = run_a.at(j).denomination;

      // Checking if there is a diff of 2 meaning a single tile
      if (std::abs(denom_a - denom_b) != 2) {
        continue;
      }

      // If there is, then if the tile is between both it must mean that it
      // is a bridge
      bool a_then_b =
          tile.denomination > denom_a && tile.denomination < denom_b;
      bool b_then_a =
          tile.denomination > denom_b && tile.denomination < denom_a;
      if (a_then_b || b_then_a) {
        return true;
      }
    }
  }

  return false;
}

RummiKub::CreateGroup::CreateGroup(std::vector<std::vector<Tile>> &groups) :
    groups(groups) {}

bool RummiKub::CreateGroup::execute(const Tile &tile) {
  groups.emplace_back();
  groups.back().push_back(tile);
  return true;
}

void RummiKub::CreateGroup::revert(const Tile &) { groups.pop_back(); }

bool RummiKub::validate_run(std::vector<Tile> &run) {
  if (run.size() < 3) {
    dbg("Run is less than 3 tiles\n");
    print_vector(run);
    return false;
  }

  std::vector<Tile> sorted_run = run;
  std::sort(
      sorted_run.begin(),
      sorted_run.end(),
      [](Tile &current, Tile &next) -> bool {
        return current.denomination < next.denomination;
      });

  for (size_t i = 0; i + 1 < sorted_run.size(); i++) {
    if (sorted_run[i].color != sorted_run[i + 1].color) {
      dbg("Run is not of one color\n");
      print_vector(run);
      return false;
    }

    if (sorted_run[i].denomination + 1 != (sorted_run[i + 1].denomination)) {
      dbg("Run is not consecutive\n");
      print_vector(sorted_run);
      return false;
    }
  }

  return true;
}

bool RummiKub::validate_group(std::vector<Tile> &group) {
  if (group.size() > 4 || group.size() < 3) {
    dbg("Group is not within range [3, 4]\n");
    print_vector(group);
    return false;
  }

  for (size_t i = 0; i + 1 < group.size(); i++) {
    if (group[i].color == group[i + 1].color) {
      dbg("Group has repeat color\n");
      print_vector(group);
      return false;
    }

    if (group[i].denomination != group[i + 1].denomination) {
      dbg("Group has different denominations\n");
      print_vector(group);
      return false;
    }
  }

  return true;
}

void RummiKub::print_runs() const {
  dbg("print_runs\n");

  for (const std::vector<Tile> &run: runs) {
    dbg("Run\n");
    print_vector(run);
  }

  dbg("\n");
}

void RummiKub::print_groups() const {
  dbg("print_groups\n");

  for (const std::vector<Tile> &group: groups) {
    dbg("Group\n");

    print_vector(group);
  }

  dbg("\n");
}
