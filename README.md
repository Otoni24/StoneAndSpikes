# Stone & Spikes

**A turn-based tactical game developed in C++20 using raylib.**

`Stone & Spikes` is a small grid-based combat game focused on action-point management, positioning, and environmental damage. Control your knights, fight enemy skeletons, and use pushes to turn spike tiles into an advantage.

I developed the project to explore gameplay architecture and tactical AI in standalone C++. The core systems include procedural board generation, shared combat rules, turn management, and a planner that simulates action sequences before choosing how to spend an enemy's turn.

## Objective

**Defeat every enemy skeleton while keeping at least one knight alive.**

Each unit takes an individual turn and spends action points to move, attack, or push an adjacent opponent. Walls constrain movement, while spikes deal damage when a unit enters their tile. A push can create an opening, change an opponent's position, or deal damage through the environment.

## Features

* **Turn-Based Combat** gives each unit its own turn and action-point budget. Surviving units cycle through the turn order.
* **Environmental Interactions** let both knights and skeletons push opponents onto spike tiles.
* **Procedural Boards** place walls and spikes while checking that ordinary floor tiles remain connected through a breadth-first search.
* **Weighted Spawning** favors greater separation between opposing units during initial placement.
* **Tactical Enemy Planning** compares simulated movement, attack, and push sequences within the active skeleton's remaining AP.
* **Mouse-Driven UI** shows available moves, opens contextual attack and push buttons, and displays health, action points, and the turn order.
* **Gameplay Feedback** includes attack effects, death animations, and an indicator for the active unit.
* **Seeded Generation** records the battle seed so an initial layout can be reproduced for debugging with the same build.

## Tactical AI

At the start of an enemy turn, the planner copies the units into a planning state and explores legal action sequences using breadth-first search. Each branch applies an action to its own copy through the same combat rules used by the actual battle.

The planner evaluates states reached throughout the turn, allowing the skeleton to end its turn early when further actions would worsen the result. States are compared in priority order:

1. Victory, with all knights eliminated.
2. Number of knights eliminated during the simulated turn.
3. Damage dealt to knights minus damage received by the active skeleton.
4. Distance to a position from which a living knight can be attacked.

A separate BFS on the board measures that distance while accounting for walls and occupied tiles. The combination lets a skeleton plan movement around obstacles, select a target, or reposition before pushing an opponent onto spikes.

`EnemyAI` executes the selected plan one action at a time, with a short delay between actions to make the turn readable. Planning covers the active skeleton's current turn. Opponent responses and later turns are outside the search horizon.

## Gameplay Architecture

* **`Board`** stores terrain and handles procedural generation, grid queries, and connectivity checks.
* **`Unit`** stores a character's position, faction, health, attack power, and action points.
* **`CombatRules`** validates and applies movement, attacks, and pushes to the supplied units. Both live gameplay and simulated branches use these functions.
* **`Battle`** owns the board and units, manages turn progression and victory conditions, and produces gameplay events for presentation.
* **`EnemyPlanner`** generates successor states and chooses the highest-scoring action sequence.
* **`EnemyAI`** manages the execution and timing of the current enemy plan.
* **`BattleHud`** handles rendering, mouse interactions, contextual actions, and the turn-order display.
* **`AnimationPlayer`** updates and draws gameplay effect animations.

## Controls

| Input | Action |
| --- | --- |
| Click a highlighted movement tile | Move the active knight |
| Click an interactable enemy | Open the action menu |
| Click Attack or Push in the action menu | Attempt the selected action |
| Click End Turn | Finish the active knight's turn |

## Art Assets

Character sprites, interface artwork, and visual effect assets were created with AI-assisted image generation and prepared for the game's runtime dimensions. Gameplay, planning, UI integration, and rendering systems are implemented in C++.

## Main Technologies

C++20, raylib, CMake, CPack, MinGW, CLion, and Git.
