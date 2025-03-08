/**
 * @file rummikub.h
 * @author Edgar Jose Donoso Mansilla
 * @course CS280
 * @term Spring 2025
 * @assignment# 3
 */

// TODO: Document the code

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

  /**
   * @brief This function adds a tile to the hand.
   *
   * @param tile The tile to add.
   */
  void Add(Tile const &tile); // add a tile to the hand

  /**
   * @brief Find the play that plays all the tiles in the hand.
   */
  void Solve(); // solve

  // get solution - groups
  std::vector<std::vector<Tile>> GetGroups() const;
  // get solution - runs
  std::vector<std::vector<Tile>> GetRuns() const;
  // if both vectors are empty - no solution possible

  /**
   * @brief This prints the solution calculated
   */
  void print_solution();

private:
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

  /**
   * @brief This represents an action that can be played in the game.
   */
  struct Action {
    virtual ~Action();

    /**
     * @brief Execute the action.
     *
     * @param tile The tile to execute the action with.
     */
    virtual bool execute(const Tile &tile) = 0;

    /**
     * @brief revert the action.
     *
     * @param tile Tile to revert the action (mainly if the tile is needed for
     * printing)
     */
    virtual void revert(const Tile &tile) = 0;

  protected:
    std::vector<size_t> inserts{};
  };

  /**
   * @brief add it to an existing run with the same color as tile and tile's
   * denomination is not yet in the run
   */
  struct AddToRun : Action {
    explicit AddToRun(std::vector<std::vector<Tile>> &runs);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &runs;
  };

  /**
   * @brief  add it to an existing group with the same denomination as tile
   * and tile's color is not yet in the group
   */
  struct AddToGroup : Action {
    explicit AddToGroup(std::vector<std::vector<Tile>> &groups);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &groups;
  };

  /**
   * @brief Create a new run
   */
  struct CreateRun : Action {
    explicit CreateRun(std::vector<std::vector<Tile>> &runs);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &runs;

    bool check_for_run_bridge(
        const Tile &tile,
        const std::vector<Tile> &run_a,
        const std::vector<Tile> &run_b);
  };

  /**
   * @brief Create a new group
   */
  struct CreateGroup : Action {
    explicit CreateGroup(std::vector<std::vector<Tile>> &groups);

    bool execute(const Tile &tile) override;
    void revert(const Tile &tile) override;

  private:
    std::vector<std::vector<Tile>> &groups;
  };

  /**
   * @brief Recursive function to solve the hand.
   *
   * @param current_tile The tile to use
   * @param actions The actions to try
   * @return The success of the solve
   */
  bool solver_recurse(
      size_t current_tile, std::vector<std::unique_ptr<Action>> &actions);

  /**
   * @brief Check if a run is legal
   *
   * @param run The run to check
   * @return If the run is valid
   */
  bool validate_run(std::vector<Tile> &run);

  /**
   * @brief Check if a group is legal
   *
   * @param group The group to check
   * @return If the group is valid
   */
  bool validate_group(std::vector<Tile> &group);

  /**
   * @brief Print all runs
   */
  void print_runs() const;

  /**
   * @brief Print all groups
   */
  void print_groups() const;
};

#endif
