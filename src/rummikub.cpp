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

// NOTE: The proffessor recommends making it work first and then optimizing

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
 * @brief Function to help find the index in a vector<T> by using function F. This is to reduce the amount of searches
 * that are boilerplate enough.
 *
 * @param vec Reference to vector to look inside of.
 * @param function to call for looking inside the vector. Requires signature bool(const T&).
 * @return Pair which means (success, index)
 */
template<typename T, typename F>
std::pair<bool, size_t> find_index_by(const std::vector<T> &vec, F function) {
  for (size_t i = 0; i < vec.size(); i++) {
    if (function(vec[i])) {
      return std::make_pair(true, i);
    }
  }

  return std::make_pair(false, 0);
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

RummiKub::RummiKub() {
  // TODO: Make sure that this is optimized for the right behavior
  // Reserving so that there are less memory allocations while doing operations.
  runs.reserve(4);
  groups.reserve(4);
};

void RummiKub::Add(Tile const &tile) { tiles.push_back(tile); }

void RummiKub::Solve() {
  // Setting up the actions (with a level of indirection so that the vtable is used)
  std::vector<std::unique_ptr<Action>> actions;
  actions.push_back(std::unique_ptr<AddToRun>(new AddToRun(runs)));
  actions.push_back(std::unique_ptr<AddToGroup>(new AddToGroup(groups)));
  actions.push_back(std::unique_ptr<CreateRun>(new CreateRun(runs)));
  actions.push_back(std::unique_ptr<CreateGroup>(new CreateGroup(groups)));

  // Calling the recursive function
  solver_recurse(0, actions);

  tiles.clear();
}

std::vector<std::vector<Tile>> RummiKub::GetGroups() const { return groups; }

std::vector<std::vector<Tile>> RummiKub::GetRuns() const { return runs; }

bool RummiKub::validate_solution() {
  for (std::vector<Tile> &run: runs) {
    if (validate_run(run)) {
      return false;
    }
  }

  for (std::vector<Tile> &group: groups) {
    if (validate_group(group)) {
      return false;
    }
  }

  return true;
}

bool RummiKub::solver_recurse(size_t current_tile, std::vector<std::unique_ptr<Action>> &actions) {
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

RummiKub::AddToRun::AddToRun(std::vector<std::vector<Tile>> &runs) : runs(runs) {}

bool RummiKub::AddToRun::execute(const Tile &tile) {
  std::pair<bool, size_t> color_index = find_index_by(
      runs,
      [tile](const std::vector<Tile> &vec) -> bool { //
        return vec.size() > 0 && (vec[0].color == tile.color);
      });

  if (not color_index.first) {
    return false;
  }

  std::vector<Tile> &run = runs[color_index.second];
  bool is_sequence{false};

  for (Tile &current_tile: run) {
    if (tile.denomination == current_tile.denomination) {
      return false;
    }

    if (((tile.denomination + 1) == current_tile.denomination) ||
        (tile.denomination == (current_tile.denomination + 1))) {
      is_sequence = true;
    }
  }

  if (is_sequence) {
    run.push_back(tile);
    inserts.push_back(color_index.second);
    return true;
  }

  return false;
}

void RummiKub::AddToRun::revert(const Tile &) {
  runs[inserts.back()].pop_back();
  inserts.pop_back();
}

RummiKub::AddToGroup::AddToGroup(std::vector<std::vector<Tile>> &groups) : groups(groups) {}

bool RummiKub::AddToGroup::execute(const Tile &tile) {
  std::pair<bool, size_t> denom_index = find_index_by(
      groups,
      [tile](const std::vector<Tile> &vec) -> bool { //
        return vec.size() > 0 && vec[0].denomination == tile.denomination;
      });

  if (not denom_index.first) {
    return false;
  }

  std::vector<Tile> &group = groups[denom_index.second];
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
  groups[inserts.back()].pop_back();
  inserts.pop_back();
}

RummiKub::CreateRun::CreateRun(std::vector<std::vector<Tile>> &runs) : runs(runs) {}

bool RummiKub::CreateRun::execute(const Tile &tile) {
  runs.emplace_back();
  runs.back().push_back(tile);
  return true;
}

void RummiKub::CreateRun::revert(const Tile &) { runs.pop_back(); }

RummiKub::CreateGroup::CreateGroup(std::vector<std::vector<Tile>> &groups) : groups(groups) {}

bool RummiKub::CreateGroup::execute(const Tile &tile) {
  groups.emplace_back();
  groups.back().push_back(tile);
  return true;
}

void RummiKub::CreateGroup::revert(const Tile &) { groups.pop_back(); }

bool RummiKub::validate_run(std::vector<Tile> &run) {
  if (run.size() < 3) {
    return false;
  }

  std::vector<Tile> sorted_run = run;
  std::sort(sorted_run.begin(), sorted_run.end(), [](Tile &current, Tile &next) -> bool {
    return current.denomination < next.denomination;
  });

  for (size_t i = 0; i + 1 < sorted_run.size(); i++) {
    if (sorted_run[i].color != sorted_run[i + 1].color) {
      std::cout << "Run is not of one color" << std::endl;
      print_vector(run);
      return false;
    }

    if (sorted_run[i].denomination + 1 != (sorted_run[i + 1].denomination)) {
      std::cout << "Run is not consecutive" << std::endl;
      print_vector(sorted_run);
      return false;
    }
  }

  return true;
}

bool RummiKub::validate_group(std::vector<Tile> &group) {
  if (group.size() > 4 || group.size() < 3) {
    return false;
  }

  for (size_t i = 0; i + 1 < group.size(); i++) {
    if (group[i].color == group[i + 1].color) {
      std::cout << "Group has repeat color" << std::endl;
      print_vector(group);
      return false;
    }

    if (group[i].denomination != group[i + 1].denomination) {
      std::cout << "Group has different denominations" << std::endl;
      print_vector(group);
      return false;
    }
  }

  return true;
}

void RummiKub::print_runs() const {
  std::cout << "print_runs" << std::endl;

  for (const std::vector<Tile> &run: runs) {
    std::cout << "Run" << std::endl;

    print_vector(run);
  }

  std::cout << std::endl;
}

void RummiKub::print_groups() const {
  std::cout << "print_groups" << std::endl;

  for (const std::vector<Tile> &group: groups) {
    std::cout << "Group" << std::endl;

    print_vector(group);
  }

  std::cout << std::endl;
}
