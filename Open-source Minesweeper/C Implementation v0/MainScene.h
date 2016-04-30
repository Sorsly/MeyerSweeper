#pragma once

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

class MainScene
{
public:
	MainScene(void);
	~MainScene(void);


public:

	//初始化地雷场景
	void InitialScene();

	//分析场景中的未点开格的是雷的概率
	void AnalyzeScene();

	//控制模块(对该点进行点开)
	void GameControl(int iwidth,int iheight);
private:

	//布雷
	void SetMine();

	//计算相邻格数
	void CalcauteNumber(int x,int y);

	//计算按钮是否在网格中
	bool IsInMesh(int x,int y);

	//计算一个网格周围空格，标记为雷，标记不是雷的
	void neighbor(int *iempty,int *imine,int *inomine,int ix,int iy);

	void Setscore(int ix,int iy);

public:

	//棋盘中的被打开的格子数
	int i_Openedblock;

	//棋盘的高度和宽度个数
	int i_Width,i_Height;

	//雷的总数
	int i_Mine;

	//二维数组原型
	typedef vector<int> TempColumn;
	typedef vector<TempColumn>  TempData;

	//实际布雷情况(值为10表示有雷)
	TempData     mine_board;

	//分析函数中的每个格子的分值（概率*1000）
	TempData	 score_board;

	//扫雷的状态数组
	TempData	view_board;

	bool        isFailed;

	bool        isV;

	bool         bCheat;
};

