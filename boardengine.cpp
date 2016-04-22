#include "boardengine.h"
#include <QtGlobal>
#include <QTime>

boardEngine::boardEngine(int num_row, int num_col, int num_bombs)
{
    nrow = num_row;
    ncol = num_col;
    nbombs = num_bombs;
    board = new int*[nrow];

    //Allocate memory for board

    for (int i = 0; i < nrow; i++)
    {
        board[i] = new int[ncol];
    }

    //Initialize board

    for(int i = 0; i < nrow; i++)
    {
        for(int j = 0; j < ncol; j++)
        {
            board[i][j] = 0;
        }
    }

    bomblist = new int[num_bombs * 2];
    CreateBoard();
}

void boardEngine::CreateBoard()
{
    int bomb_number = nbombs;
    int row,column;
    int k = 0;
    while(bomb_number != 0)
    {
        //seed qrand() with current system time
        qsrand(QTime::currentTime().msec());
        row = qrand() % nrow;
        column = qrand() % ncol;
        if(!ismine(row,column))
        {
            board[row][column] = MINE;

            if((row - 1) != -1 && (column - 1) != -1 && !ismine(row - 1,column - 1))
            {
                board[row - 1][column - 1]++;
            }
            if((row - 1) != -1 && !ismine(row - 1, column))
            {
                board[row - 1][column]++;
            }
            if((row - 1) != -1 && (column + 1) != ncol && !ismine(row - 1, column + 1))
            {
                board[row - 1][column + 1]++;
            }
            if((column - 1) != -1 && !ismine(row, column -1))
            {
                board[row][column - 1]++;
            }
            if((column +1) != ncol && !ismine(row, column + 1))
            {
                board[row][column + 1]++;
            }
            if((row + 1) != nrow && (column - 1) != -1 && !ismine(row + 1, column -1))
            {
                board[row + 1][column - 1]++;
            }
            if((row + 1) != nrow && !ismine(row + 1, column))
            {
                board[row + 1][column]++;
            }
            if((row + 1) != nrow && (column + 1) != ncol && !ismine(row + 1, column + 1))
            {
                board[row + 1][column + 1]++;
            }
            bomblist[k++] = row;
            bomblist[k++] = column;
            bomb_number--;
        }
    }
}

bool boardEngine:: ismine(int row, int column)
{
    bool output;

    if(row < 0 || row >= nrow)
    {
        qFatal("incorrect input");
    }
    if(column < 0 || column >= ncol)
    {
        qFatal("incorrect input");
    }
    if(board[row][column] == MINE)
    {
        output = true;
    }
    else
    {
        output = false;
    }
    return output;
}

int boardEngine:: getValue(int row, int column)
{
    if(row < 0 || row >= nrow)
    {
        qFatal("incorrect input");
    }
    if(column < 0 || column >= ncol)
    {
        qFatal("incorrect input");
    }
    return board[row][column];
}
