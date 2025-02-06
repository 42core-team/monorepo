#ifndef ENTITY_H
#define ENTITY_H

struct Position {
    unsigned int x;
    unsigned int y;
};

class Entity {
public:
    virtual unsigned int getId() const = 0;
    virtual unsigned int getTeamId() const = 0;
    virtual Position getPosition() const = 0;
    virtual unsigned int getHP() const = 0;
    // Returns true if the entity is “destroyed.”
    virtual bool dealDamage(int dmg) = 0;
    virtual ~Entity() {}
};

#endif // ENTITY_H
