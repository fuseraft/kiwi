#ifndef BLOCK_H
#define BLOCK_H

class Block {
public:
    Block() = default;

    void add(int value) {
        data.push_back(value);
    }

    Token get(int index) const {
        return data.at(index);
    }

protected:
    std::vector<Token> data;
};

#endif