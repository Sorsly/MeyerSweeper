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

	//��ʼ�����׳���
	void InitialScene();

	//���������е�δ�㿪������׵ĸ���
	void AnalyzeScene();

	//����ģ��(�Ըõ���е㿪)
	void GameControl(int iwidth,int iheight);
private:

	//����
	void SetMine();

	//�������ڸ���
	void CalcauteNumber(int x,int y);

	//���㰴ť�Ƿ���������
	bool IsInMesh(int x,int y);

	//����һ��������Χ�ո񣬱��Ϊ�ף���ǲ����׵�
	void neighbor(int *iempty,int *imine,int *inomine,int ix,int iy);

	void Setscore(int ix,int iy);

public:

	//�����еı��򿪵ĸ�����
	int i_Openedblock;

	//���̵ĸ߶ȺͿ�ȸ���
	int i_Width,i_Height;

	//�׵�����
	int i_Mine;

	//��ά����ԭ��
	typedef vector<int> TempColumn;
	typedef vector<TempColumn>  TempData;

	//ʵ�ʲ������(ֵΪ10��ʾ����)
	TempData     mine_board;

	//���������е�ÿ�����ӵķ�ֵ������*1000��
	TempData	 score_board;

	//ɨ�׵�״̬����
	TempData	view_board;

	bool        isFailed;

	bool        isV;

	bool         bCheat;
};

