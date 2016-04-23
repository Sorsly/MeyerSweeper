#ifndef MINECLEARANCE_H
#define MINECLEARANCE_H

#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include <QDate>
#include <QString>
#include <QObject>
#include "MainScene.h"
#include "ui_mineclearance.h"

class mineclearance : public QMainWindow
{
	Q_OBJECT

public:
	mineclearance(QWidget *parent = 0);
	~mineclearance();

private slots:

	void chtime();

private:

	void InitialUI();

	void UpDateUI();

private:
	//根据INT来换算时间
	QString GetTimeFormInt(int);

private slots:

	void Nextstep();

	void ReInitial();

	void OpenCheat();

private:
	Ui::mineclearanceClass ui;

	MainScene   *pMianScene;

	QTimer *_timer;

	//计时时间
	int i_time;
};

#endif // MINECLEARANCE_H
