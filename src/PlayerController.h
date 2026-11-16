#pragma once

class Battle;
class BoardView;

class PlayerController {
public:
    PlayerController() = default;

    void Update(Battle& battle, BoardView& boardView);
private:

};
