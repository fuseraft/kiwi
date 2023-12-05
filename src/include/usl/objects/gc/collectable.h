#ifndef COLLECTABLE_H
#define COLLECTABLE_H

class Collectable {
  public:
    virtual void setCollectable(bool collectable) = 0;
    virtual bool isCollectable() const = 0;
};

#endif