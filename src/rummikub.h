/**
 * @file rummikub.h
 * @author Edgar Jose Donoso Mansilla
 * @course CS280
 * @term Spring 2025
 * @assignment# 3
 */

#ifndef RUMMIKUB_H
#define RUMMIKUB_H

#include <iostream>
#include <memory>
#include <vector>

enum Color { Red, Green, Blue, Yellow };

struct Tile {
  int denomination;
  Color color;
};

std::ostream &operator<<(std::ostream &os, Tile const &t);

class RummiKub {
public:
  RummiKub(); // empty hand
  void Add(Tile const &); // add a tile to the hand

  void Solve(); // solve

  // get solution - groups
  std::vector<std::vector<Tile>> GetGroups() const;
  // get solution - runs
  std::vector<std::vector<Tile>> GetRuns() const;
  // if both vectors are empty - no solution possible

private:
  // Enables the printing of debug data
  bool debug_print{false};

  std::vector<Tile> tiles{};

  // Group: a sequence
  // 1) of 3 or 4 tiles
  // 2) same denominations
  // 3) no tiles of the same color
  std::vector<std::vector<Tile>> groups{};

  // Run (classical): a sequence
  // 1) of 3 or more tiles
  // 2) all tiles have the same color
  // 3) denomination are consecutive
  std::vector<std::vector<Tile>> runs{};

  bool validate_solution();

  struct Action {
    virtual ~Action();
    virtual bool execute(const Tile &tile) = 0;
    virtual void revert(const Tile &tile) = 0;

  protected:
    std::vector<size_t> inserts{};
  };

  // 1) add it to an existing run with the same color as tile and tile's denomination is not yet in the run
  struct AddToRun : Action {
    explicit AddToRun(std::vector<std::vector<Tile>> &runs);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &runs;
  };

  // 2) add it to an existing group with the same denomination as tile and tile's color is not yet in the group
  struct AddToGroup : Action {
    explicit AddToGroup(std::vector<std::vector<Tile>> &groups);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &groups;
  };

  // 3) create a new run
  struct CreateRun : Action {
    explicit CreateRun(std::vector<std::vector<Tile>> &runs);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &runs;
  };

  // 4) create a new group
  struct CreateGroup : Action {
    explicit CreateGroup(std::vector<std::vector<Tile>> &groups);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &groups;
  };

  bool solver_recurse(size_t current_tile, std::vector<std::unique_ptr<Action>> &actions);

  bool validate_run(std::vector<Tile> &run);

  bool validate_group(std::vector<Tile> &group);

  void print_runs() const;

  void print_groups() const;
};

#endif
