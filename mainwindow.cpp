#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    flags = 0;
    ui->setupUi(this);
    timer = new QTimer();
    ui->mineGrid->setSpacing(0);
    ui->mine_counter->setDigitCount(2);
    ui->mine_counter->display(num_bombs - flags);
    connect(ui->flag_button,SIGNAL(clicked()), this, SLOT(flag_buttonClicked()));
    connect(ui->reset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(ui->diff_easy, SIGNAL(clicked()), this, SLOT(radio_easy()));
    connect(ui->diff_medium, SIGNAL(clicked()), this, SLOT(radio_medium()));
    connect(ui->diff_hard, SIGNAL(clicked()), this, SLOT(radio_hard()));
    regular = new QSignalMapper(this);
    engine = new boardEngine(nrow, ncol, num_bombs);
    gameover = false;
    begin = false;
    gameTime = 0;
    connect(timer, SIGNAL(timeout()), this, SLOT(runTimer()));

    currentBoard = new int*[nrow];
    for (int i = 0; i < nrow; i++)
    {
        currentBoard[i] = new int[ncol];
    }

    for(int i = 0; i < nrow; i++)
    {
        for(int j = 0; j < ncol; j++)
        {
            currentBoard[i][j] = 0;
        }
    }

    for(int i = 0; i < nrow; i++)
    {
        for(int j = 0; j < ncol; j++)
        {
            gridButton *button = new gridButton("");
            button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
            button->setMaximumHeight(100);
            button->setMaximumWidth(100);
            ui->mineGrid->addWidget(button,i,j);
            QString coordinates = QString::number(i) + ","+QString::number(j);
            regular -> setMapping(button, coordinates);
            connect(button, SIGNAL(clicked()), regular, SLOT(map()));
        }
        connect(regular, SIGNAL(mapped(QString)), this, SLOT(actionRegularClick(QString)));
        connect(regular,SIGNAL(mapped(QString)), this, SLOT(actionRightClick(QString)));
    }
}

void MainWindow::runTimer()
{
    gameTime++;
    ui -> lcd_timer -> display(gameTime);

}

void MainWindow::actionRegularClick(QString coordinates)
{
    if(ui->flag_button->isFlat()) return;
    if(gameover) return;
    gridButton *buttonPushed = qobject_cast<gridButton*>(regular->mapping(coordinates));
    QStringList coor = coordinates.split(",");
    int row = coor.at(0).toInt();
    int column = coor.at(1).toInt();

    if(!begin)
    {
        begin = true;
        timer -> start(1000);
    }

    if (cellPressed == (nrow * ncol - num_bombs) && engine->getValue(row, column) != MINE)
    {
        changeButton(buttonPushed, row, column);
        won();
        return;
    }

    if(currentBoard[row][column] == FLAGGED  || currentBoard[row][column] == QUESTION)
    {
        return;
    }

    if(engine -> getValue(row,column) == 0)
    {
        clearSpaces(row,column, true);
    }

    if(currentBoard[row][column] == EMPTY)
    {
        if(engine->getValue(row,column) == MINE)
        {
            QPalette* palette1 = new QPalette();
            palette1->setColor(QPalette::ButtonText,Qt::darkRed);
            buttonPushed -> setPalette(*palette1);
            buttonPushed -> setText("B");
            buttonPushed -> setFlat(true);
            lost();
        }
        else
        {
            changeButton(buttonPushed, row, column);
        }
    }

    if(buttonPushed -> isFlat() == true)
    {
        return;
    }
}

void MainWindow::actionRightClick(QString coordinates)
{
    if(ui->flag_button->isFlat() == false) return;
    if(gameover) return;
    gridButton *buttonPushed = qobject_cast<gridButton*>(regular->mapping(coordinates));
    QStringList coor = coordinates.split(",");
    int row  = coor.at(0).toInt();
    int column = coor.at(1).toInt();

    if(!begin)
    {
        begin = true;
        timer -> start(1000);
    }
    if(buttonPushed -> isFlat() == true)
    {
        return;
    }

    if(currentBoard[row][column] == EMPTY)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::red);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("F");
        currentBoard[row][column] = FLAGGED;
        flags++;
        ui->mine_counter->display(num_bombs - flags);
    }
    else if(currentBoard[row][column] == FLAGGED)
    {
        buttonPushed -> setText("?");
        currentBoard[row][column] = QUESTION;
        flags--;
        ui->mine_counter->display(num_bombs - flags);
    }
    else if(currentBoard[row][column] == QUESTION)
    {
        buttonPushed -> setText("");
        currentBoard[row][column] = EMPTY;
    }
    else
    {
        return;
    }
}

void MainWindow::changeButton(gridButton * buttonPushed, int row, int column)
{
    if(engine->getValue(row,column) == MINE)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::darkRed);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("B");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if (engine -> getValue(row, column) == 0)
    {
        buttonPushed -> setFlat(true);
    }
    else if(engine->getValue(row,column) == 1)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::blue);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("1");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 2)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::green);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("2");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 3)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::yellow);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("3");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 4)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::cyan);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("4");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 5)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::darkBlue);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("5");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 6)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::darkMagenta);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("6");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 7)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::darkYellow);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("7");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }
    else if(engine->getValue(row,column) == 8)
    {
        QPalette* palette1 = new QPalette();
        palette1->setColor(QPalette::ButtonText,Qt::magenta);
        buttonPushed -> setPalette(*palette1);
        buttonPushed -> setText("8");
        buttonPushed -> setFlat(true);
        currentBoard[row][column] = PRESSED;
    }

}

void MainWindow::clearSpaces(int row, int column, bool nextCell)
{
    QString coordinates = QString::number(row)  + "," + QString::number(column);
    gridButton *buttonPushed = qobject_cast<gridButton*>(regular->mapping(coordinates));
    if(buttonPushed -> isFlat() == false && engine -> getValue(row, column) != MINE && nextCell == true && currentBoard[row][column] != FLAGGED)
    {
        buttonPushed -> setFlat(true);
        changeButton(buttonPushed, row, column);

        if(engine -> getValue(row,column) == 0)
        {
            nextCell = true;
        }
        else
        {
            nextCell = false;
        }

        if ((row-1) != -1 && (column -1) != -1)
        {
            clearSpaces(row-1, column-1, nextCell);
        }
        if ((row-1) != -1)
        {
            clearSpaces(row-1, column, nextCell);
        }
        if ( (row-1) != -1 && (column + 1) != ncol)
        {
            clearSpaces(row-1, column+1, nextCell);
        }
        if ( (column -1) != -1)
        {
            clearSpaces(row, column-1, nextCell);
        }
        if ( (column + 1) != ncol)
        {
            clearSpaces(row, column+1,nextCell);
        }
        if ( (row+1) != nrow && (column -1) != -1)
        {
            clearSpaces(row+1, column-1, nextCell);
        }
        if ( (row+1) != nrow)
        {
            clearSpaces(row+1, column, nextCell);
        }
        if ((row+1) != nrow && (column+1) != ncol)
        {
            clearSpaces(row+1, column+1, nextCell);
        }
    }

}


void MainWindow::flag_buttonClicked()
{
    if(ui->flag_button->isFlat() == true)
    {
        ui->flag_button->setFlat(false);
    }
    else if(ui->flag_button->isFlat() == false)
    {
        ui->flag_button->setFlat(true);
    }
}

void MainWindow::lost()
{
    timer -> stop();

    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < 10; j++)
        {
            QString coordinates = QString::number(i) + "," + QString::number(j);
            gridButton *buttonPushed   = qobject_cast<gridButton*>(regular->mapping(coordinates));
            if(!buttonPushed -> isFlat() &&  engine -> getValue(i,j) == MINE)
            {
                changeButton(buttonPushed, i,j);
            }
            else if(currentBoard[i][j] == FLAGGED && engine->getValue(i,j) != MINE)
            {
                buttonPushed ->  setText("/");
            }
        }
    }
    gameover = true;
}

void MainWindow::won()
{
    timer -> stop();
}

void MainWindow::radio_easy()
{

}

void MainWindow::radio_medium()
{

}

void MainWindow::radio_hard()
{

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reset()
{
    flags = 0;
    cellPressed = 0;
    gameTime = 0;
    begin = false;
    gameover = false;

    if ( ui->diff_easy->isChecked())
    {
        nrow = 10;
        ncol = 10;
        num_bombs = 10;
    }
    else if ( ui->diff_medium->isChecked())
    {
        nrow = 16;
        ncol = 16;
        num_bombs = 40;
    }
    else if ( ui->diff_hard->isChecked())
    {
        nrow = 16;
        ncol = 30;
        num_bombs = 99;
    }

    this ->timer->stop();
    ui->lcd_timer->display(gameTime);
    ui->mine_counter->display(num_bombs - flags);
    engine = new boardEngine(nrow, ncol, num_bombs);



    /*MainWindow* window;
    window->num_bombs = num_bombs;
    window->nrow = nrow;
    window->ncol = ncol;
    MainWindow(window);
    window->show();*/



    for(int i = 0; i < nrow; i++)
    {
        for(int j = 0; j < ncol; j++)
        {
            QString coordinates = QString::number(i) + "," + QString::number(j);
            gridButton *buttonPushed  = qobject_cast<gridButton*>(regular->mapping(coordinates));
            buttonPushed -> setFlat(false);
            buttonPushed -> setText(" ");
            currentBoard[i][j] = EMPTY;
        }
    }
}

