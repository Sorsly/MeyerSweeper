#include "mineclearance.h"

mineclearance::mineclearance(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	pMianScene = new MainScene;

	InitialUI();

	connect(ui.pushButton,SIGNAL(clicked()),this,SLOT(Nextstep()));
	connect(ui.pushButton_2,SIGNAL(clicked()),this,SLOT(ReInitial()));
	connect(ui.pushButton_3,SIGNAL(clicked()),this,SLOT(OpenCheat()));
}


mineclearance::~mineclearance()
{

}

void mineclearance::InitialUI()
{
	//初始化地雷

	pMianScene->InitialScene();

	//按钮初始化
	ui.gridLayout->setSpacing(0);  
	ui.gridLayout->setMargin(0);  
	for(int i = 0; i < pMianScene->i_Width; i++)
	{
		for(int j = 0; j < pMianScene->i_Height; j++)
		{
			QPushButton *button = new QPushButton;
			button->setObjectName(QString::number(i*pMianScene->i_Height + j,10));
			button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
			button->setMaximumHeight(50);
			button->setMaximumWidth(50);
			ui.gridLayout->addWidget(button,i,j);
		}
	}

	/*ui.gridLayout_2->setSpacing(0);  
	ui.gridLayout_2->setMargin(0);  
	for(int i = 0; i < pMianScene->i_Width; i++)
	{
		for(int j = 0; j < pMianScene->i_Height; j++)
		{
			QPushButton *button = new QPushButton;
			button->setObjectName(QString::number(i*pMianScene->i_Height + j,10));
			button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
			button->setMaximumHeight(50);
			button->setMaximumWidth(50);
			ui.gridLayout->addWidget(button,i,j);
		}
	}*/

	//计时开始
	i_time = 0;
	_timer = new QTimer;

	ui.lcdNumber->setSegmentStyle(QLCDNumber::Flat);
	ui.lcdNumber->display(GetTimeFormInt(i_time));

	connect(_timer,SIGNAL(timeout()),this,SLOT(chtime()));
	_timer->start(1000);

	UpDateUI();

}

void mineclearance::UpDateUI()
{
	if (pMianScene->isFailed)
	{
		ui.label->setText("GAME OVER");
	}

	if (pMianScene->isV)
	{
		ui.label->setText("GAME SUCESS");
	}


	while(ui.gridLayout->count() != 0 )
	{
		QWidget *p = ui.gridLayout->itemAt(0)->widget();
		ui.gridLayout->removeWidget(p);
		delete p;
	}

	for (int i = 0;i < pMianScene->i_Width;i++)
	{
		for (int j = 0;j < pMianScene->i_Height;j++)
		{
			QPushButton *button = new QPushButton;
			button->setObjectName(QString::number(i*pMianScene->i_Height + j,10));
			button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
			button->setMaximumHeight(50);
			button->setMaximumWidth(50);
			
			if (pMianScene->view_board.at(i).at(j) == 11)
			{
				button->setText("");

				if (pMianScene->score_board.at(i).at(j) == 10000)
				{
					button->setText("*****");
				}
			}
			else
			{
				button->setFlat(true);

				button->setText(QString::number(pMianScene->view_board.at(i).at(j),10));
	
			}


			ui.gridLayout->addWidget(button,i,j);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/*while(ui.gridLayout_2->count() != 0 )
	{
		QWidget *p = ui.gridLayout_2->itemAt(0)->widget();
		ui.gridLayout_2->removeWidget(p);
		delete p;
	}

	for (int i = 0;i < pMianScene->i_Width;i++)
	{
		for (int j = 0;j < pMianScene->i_Height;j++)
		{
			QPushButton *button = new QPushButton;
			button->setObjectName(QString::number(i*pMianScene->i_Height + j,10));
			button->setAttribute(Qt::WA_LayoutUsesWidgetRect);
			button->setMaximumHeight(50);
			button->setMaximumWidth(50);

			
				button->setFlat(true);
				button->setText(QString::number(pMianScene->score_board.at(i).at(j),10));

			
			ui.gridLayout_2->addWidget(button,i,j);
		}
	}*/
	
}

void mineclearance::Nextstep()
{
	while((!pMianScene->isFailed)&&(!pMianScene->isV))
	{
		pMianScene->AnalyzeScene();
	}
	UpDateUI();
}

void mineclearance::chtime()
{
	i_time++;
	ui.lcdNumber->display(GetTimeFormInt(i_time));
}

QString mineclearance::GetTimeFormInt(int itime)
{
	int imuntie = itime/60.f;
	int isecond = itime%60;

	QString str_muntie = QString::number(imuntie,10);
	QString str_second = QString::number(isecond,10);
	QString str_result = str_muntie + ":" + str_second;

	return str_result;

}

void mineclearance::OpenCheat()
{
	pMianScene->bCheat = !pMianScene->bCheat;

	if (pMianScene->bCheat)
	{
		ui.pushButton_3->setText("CLOSE CHEAT");

	}else
	{
		ui.pushButton_3->setText("OPEN CHEAT");
	}
}

void mineclearance::ReInitial()
{
	ui.label->setText("");
	InitialUI();
}