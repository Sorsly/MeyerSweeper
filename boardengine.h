#ifndef BOARDENGINE_H
#define BOARDENGINE_H
#define MINE -1
#include<QtGlobal>

class boardEngine
{
public:
    boardEngine(int, int, int);
    bool ismine(int, int);
    int getValue(int, int);
    int *bomblist;

private:
    void CreateBoard();
    int** board;
    int nrow;
    int ncol;
    int nbombs;
};

#endif // BOARDENGINE_H
