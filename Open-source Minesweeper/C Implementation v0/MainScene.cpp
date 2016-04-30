#include "MainScene.h"
#define SCOREEMPTY -2  //记分表中没有点开
#define SCORECLICKED -1 //已经点击过
#define SCORENOTMINE 0	//不是雷
#define SCOREMINE	10000	//是雷


MainScene::MainScene(void)
{

	bCheat = false;
	/*i_Openedblock = 0;
	i_Width  = 20;
	i_Height = 10;
	i_Mine   = 20;

	mine_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		mine_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			mine_board.at(i).at(j) = 0;
		}
	}

	score_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		score_board.at(i).resize(i_Height);
		view_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			score_board.at(i).at(j) = SCOREEMPTY;
		}
	}

	//初始值为11（说明没点开）
	view_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		view_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			view_board.at(i).at(j) = 11;
		}
	}*/
}


MainScene::~MainScene(void)
{
}

void MainScene::InitialScene()
{
	isV    = false;
	isFailed = false;
	i_Openedblock = 0;
	i_Width  = 16;
	i_Height = 16;
	i_Mine   = 40;

	mine_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		mine_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			mine_board.at(i).at(j) = 0;
		}
	}

	score_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		score_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			score_board.at(i).at(j) = SCOREEMPTY;
		}
	}

	//初始值为11（说明没点开）
	view_board.resize(i_Width);
	for (int i = 0;i < i_Width;i++)
	{
		view_board.at(i).resize(i_Height);
		for (int j = 0;j < i_Height;j++)
		{
			view_board.at(i).at(j) = 11;
		}
	}


	SetMine();

	//计算非雷格子的相邻雷的数目
	for (int i = 0;i < (int) mine_board.size();i++)	      //Width
	{
		for (int j = 0;j < (int)mine_board.at(i).size();j++)	//Height
		{
			if (mine_board.at(i).at(j) == 10)
			{
				CalcauteNumber(i,j);
			}
		}
	}
}


void MainScene::AnalyzeScene()
{
	//将棋盘中有雷的格子找出
	for(int i = 0;i < i_Width;i++){
		for(int j = 0;j < i_Height;j++)
		{
			//查询已经点开有数字的格子
			if (view_board.at(i).at(j) > 0 && view_board.at(i).at(j) < 9)
			{

				//查询他四周未被点开的格子的个数

				int ihasclicked = 0;

				int temp_x;
				int temp_y;
				for (int k = 0;k < 3;k++)
				{
					for (int l = 0;l < 3;l++)
					{
						temp_x = i + k - 1;
						temp_y = j + l - 1;
						//如果点在网里，且没被点开
						if (IsInMesh(temp_x,temp_y)&&view_board.at(temp_x).at(temp_y) == 11)
						{
							ihasclicked++;
						}
					}
				}

				if (ihasclicked == view_board.at(i).at(j))
				{
					for (int k = 0;k < 3;k++)
					{
						for (int l = 0;l < 3;l++)
						{
							temp_x = i + k - 1;
							temp_y = j + l - 1;
							//如果点在网里，且没被点开
							if (IsInMesh(temp_x,temp_y)&&view_board.at(temp_x).at(temp_y) == 11)
							{
								//有雷
								score_board.at(temp_x).at(temp_y) = SCOREMINE;
							
							}
						}
					}
				}
			}
		}
	}

	///重新计算数字相邻格的分值///////////////////////////////////////////////////////////////////////
	int signed_mine = 0;	//已经标记的雷数
	int empty_block = 0;	//没有点开的空格数

	for (int i = 0;i < i_Width;i++)
	{
		for (int j = 0;j < i_Height;j++)
		{
			
			Setscore(i,j);
			if (score_board.at(i).at(j) == SCOREMINE)
			{
				signed_mine ++;
			}

			if (score_board.at(i).at(j) == SCOREEMPTY)
			{
				empty_block++;
			}
		}
	}

/*	if (signed_mine == i_Mine)
	{
		isV = true;
		return;
	}

	*/

	//不与任何数字相邻的空格分值
	int iscoreempty = (i_Mine - signed_mine)/(float)(empty_block)*10000;

	//////////////////////////////////////////////////////////////////////////
	float fmin = 1e8;
	
	vector< int> temparray;

	for(int i = 0; i< i_Width;i++)
	{
		for (int j = 0;j < i_Height;j++)
		{
			//Setscore(i,j);
			if (score_board.at(i).at(j) == SCOREEMPTY)
			{
				score_board.at(i).at(j) = iscoreempty;
			}

			if (score_board.at(i).at(j) >= 0 )
			{
				if (score_board.at(i).at(j) == fmin)
				{
					temparray.push_back(i * i_Height + j);
				}

				if (score_board.at(i).at(j) < fmin)
				{
					temparray.clear();
					fmin = score_board.at(i).at(j);

				
					temparray.push_back(i * i_Height + j);
				}
			}

		}
	}
	//将空格的分值还原
	for(int i = 0; i< i_Width;i++)
	{
		for (int j = 0;j < i_Height;j++)
		{
			
			if (score_board.at(i).at(j) == iscoreempty)
			{
				score_board.at(i).at(j) = SCOREEMPTY;
			}

		}
	}

	if (i_Openedblock == i_Width*i_Height - i_Mine)
	{
		isV    = true;
		return;
	}

	//从分析分值最小的数组中随机选取下一个点击点
	srand((int)time(0));

	int inextclick = rand()%temparray.size();

	int itemp_x = temparray.at(inextclick)/i_Height;
	int itemp_y = temparray.at(inextclick)%i_Height;

	//开启作弊模式
	if (bCheat)
	{
		while(true)
		{
			itemp_x = rand()%i_Width;
			itemp_y = rand()%i_Height;
			if (view_board.at(itemp_x).at(itemp_y) == 11 && mine_board.at(itemp_x).at(itemp_y) != 10)
			{
				break;
			}

		}
	}


	GameControl(itemp_x,itemp_y);

}

void MainScene::Setscore(int ix,int iy)
{
	if (score_board.at(ix).at(iy) == SCORECLICKED || score_board.at(ix).at(iy) == SCOREMINE || score_board.at(ix).at(iy) == SCORENOTMINE)
	{
		return;
	}

	int tempx;
	int tempy;

	int ihasnumber1 = 0;

	float f_P = 1.0;
	for (int i = 0;i < 3;i++ )
	{
		for (int j = 0;j < 3;j++)
		{
			tempx = ix + i -1;
			tempy = iy + j -1;
			//周围的数字格
			if (IsInMesh(tempx,tempy)&&view_board.at(tempx).at(tempy)>0 && view_board.at(tempx).at(tempy) < 9)
			{
				ihasnumber1++;
				int iempty;
				int imine;
				int inotmine;
				neighbor(&iempty,&imine,&inotmine,tempx,tempy);
				if (view_board.at(tempx).at(tempy) == imine)
				{
					score_board.at(ix).at(iy) = SCORENOTMINE;
					return;
				}

				f_P *= (1.0 - (view_board.at(tempx).at(tempy) - imine)/(float)(iempty - imine - inotmine));

			}

		}
	}

	if (ihasnumber1 > 0)
	{
		
		score_board.at(ix).at(iy) = (1.0 - f_P)*10000;
		if (score_board.at(ix).at(iy) == 10000 || score_board.at(ix).at(iy) == 0)
		{
			int a = 1;
		}
	}
	
}


void MainScene::neighbor(int *iempty,int *imine,int *inomine,int ix,int iy)
{
	(*iempty) = 0;
	(*imine)  = 0;
	(*inomine) = 0;
	
	int tempx;
	int tempy;

	for (int i= 0;i < 3;i++ )
	{
		for (int j = 0;j < 3;j++)
		{
			tempx = ix + i - 1;
			tempy = iy + j - 1;

			if (IsInMesh(tempx,tempy) && view_board.at(tempx).at(tempy) == 11)
			{
				(*iempty)++;
			}

			if (IsInMesh(tempx,tempy) && score_board.at(tempx).at(tempy) == SCOREMINE)
			{
				(*imine)++;
			}

			if (IsInMesh(tempx,tempy) && score_board.at(tempx).at(tempy) == SCORENOTMINE)
			{
				(*inomine)++;
			}

		}
	}

}

void MainScene::GameControl(int iwidth,int iheight)
{
	//将点点开
	view_board.at(iwidth).at(iheight) = mine_board.at(iwidth).at(iheight);

	if (mine_board.at(iwidth).at(iheight) == 10)
	{
		//UI显示失败
		isFailed = true;
		return;
	}



	//如果该点是0，继续点开他的四周
	if (view_board.at(iwidth).at(iheight) == 0)
	{
		int temp_x;
		int temp_y;
		for (int i = 0;i < 3;i++)
		{
			for (int j = 0;j < 3;j++)
			{
				temp_x = iwidth + i - 1;
				temp_y = iheight + j - 1;
				//如果点在网里，且没被点开
				if (IsInMesh(temp_x,temp_y)&&view_board.at(temp_x).at(temp_y) == 11)
				{
					GameControl(temp_x,temp_y);
				}
			}
		}
	}

	score_board.at(iwidth).at(iheight) = SCORECLICKED; 
	i_Openedblock++;

}

void MainScene::SetMine()
{
	srand((int)time(0));
	for (int i = 0;i < i_Mine;i++)
	{
		int j;
		int k;
		do 
		{
			 j = rand()%i_Height;
			 k = rand()%i_Width;
		} 
		while (mine_board.at(k).at(j) == 10);

		mine_board.at(k).at(j) = 10;
	}
}

void MainScene::CalcauteNumber(int x,int y)
{
	int temp_x;
	int temp_y;

	for (int i = 0;i < 3;i++)
	{
		for (int j = 0;j < 3;j++)
		{
			temp_x = x + i - 1;
			temp_y = y + j - 1;
			if (IsInMesh(temp_x,temp_y))
			{
				if (mine_board.at(temp_x).at(temp_y) != 10)
				mine_board.at(temp_x).at(temp_y) ++;
			}
		}
	}

}

bool MainScene::IsInMesh(int x,int y)
{
	if ((x >= 0) && (x < i_Width)
		 && (y >= 0)&&(y < i_Height))
	{
		return true;
	}

	return false;
}