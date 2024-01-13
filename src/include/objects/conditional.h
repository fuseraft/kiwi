#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include <vector>
#include "../parsing/tokens.h"
#include "block.h"

class IfStatement : public Block {
private:
    Block expression;
    Block code;
};

class ElsifStatement : public Block {
private:
    Block expression;
    Block code;
};

class ElseStatement : public Block {
private:
    Block code;
};

class Condtional {
private:
    IfStatement ifStatement;
    std::vector<ElsifStatement> elsifStatements;
    ElseStatement elseStatement;
};

#endif