/**
 * @file rummikub.cpp
 * @author Edgar Jose Donoso Mansilla
 * @course CS280
 * @term Spring 2025
 * @assignment# 3
 */

#include "rummikub.h"
#include <algorithm>
#include <iosfwd>
#include <iostream>
#include <ostream>

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
 * @param filter to call for comparing found indices. Requires signature
 * bool(const T&, const T&).
 * @return Pair which means (success, index)
 */
template<typename T, typename F, typename C>
std::pair<bool, size_t>
find_index_qualified(const std::vector<T> &vec, F function, C filter) {
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

#if SORT_HAND
  std::sort(
      tiles.begin(), tiles.end(), [](const Tile &a, const Tile &b) -> bool {

        if (a.denomination == b.denomination) {
          return (a.color < b.color);
        }

        return (a.denomination < b.denomination);
      });
  print_vector(tiles);
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

  print_solution();

  dbg("\nSolver terminated\n");
}

std::vector<std::vector<Tile>> RummiKub::GetGroups() const { return groups; }

std::vector<std::vector<Tile>> RummiKub::GetRuns() const { return runs; }

void RummiKub::print_solution() {
  print_runs();
  print_groups();
}

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
  std::pair<bool, size_t> color_index = find_index_qualified(
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
  std::pair<bool, size_t> denom_index = find_index_qualified(
      groups,
      [tile](const std::vector<Tile> &group) -> bool {
        // Check if the tile is the right denomination
        if (group.empty() || (group.at(0).denomination != tile.denomination) ||
            group.size() == 4) {
          return false;
        };

        // Check if the color is already in the run
        for (const Tile &current_tile: group) {
          if (tile.color == current_tile.color) {
            return false;
          }
        }

        return true;
      },
      [](const std::vector<Tile> &max,
         const std::vector<Tile> &current) -> bool {
        // making sure that it is the smallest possible group to add to
        return current.size() < max.size();
      });

  if (denom_index.first) {
    groups.at(denom_index.second).push_back(tile);
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
  runs.emplace_back();
  runs.back().push_back(tile);
  return true;
}

void RummiKub::CreateRun::revert(const Tile &) { runs.pop_back(); }

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

  std::vector<Tile> sorted_run{run};
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
