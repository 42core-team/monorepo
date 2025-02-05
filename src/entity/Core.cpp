#include "Core.h"

Core::Core(unsigned int id, unsigned int teamId, Position pos, unsigned int hp)
    : id_(id), teamId_(teamId), pos_(pos), hp_(hp) {}

unsigned int Core::getId() const {
    return id_;
}

unsigned int Core::getTeamId() const {
    return teamId_;
}

Position Core::getPosition() const {
    return pos_;
}

unsigned int Core::getHP() const {
    return hp_;
}

bool Core::dealDamage(int dmg) {
    if (dmg < 0) return false;
    if (dmg >= (int)hp_) {
        hp_ = 0;
        return true;
    }
    hp_ -= dmg;
    return false;
}
