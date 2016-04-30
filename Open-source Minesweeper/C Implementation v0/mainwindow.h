#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "gridbutton.h"
#include<QMainWindow>
#include<QSignalMapper>
#include "boardengine.h"
#include<iostream>
#include<QTimer>
#define MINE -1
#define EMPTY 0
#define FLAGGED 1
#define QUESTION 2
#define PRESSED 3
//#define NUMBOMBS 10

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    boardEngine *engine;
    QSignalMapper *regular;
    int** currentBoard;
    int num_bombs = 10;
    int nrow = 10;
    int ncol = 10;
    int cellPressed;
    bool gameover;
    bool begin;
    int gameTime;
    QTimer *timer;
    void clearSpaces(int, int, bool);
    void changeButton(gridButton*, int, int);
    int flags;



    public slots:
    void actionRightClick(QString);
    void actionRegularClick(QString);
    void flag_buttonClicked();
    void runTimer();
    void lost();
    void won();
    void reset();
    void radio_easy();
    void radio_medium();
    void radio_hard();
};

#endif // MAINWINDOW_H

