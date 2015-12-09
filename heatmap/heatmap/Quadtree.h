#pragma once
#include <vector>
#include "HeatMap.h"
using std::vector;


//CPU�ϵ��Ĳ����ṹ
struct TreeNode
{
	float minlon; //�õ���Χ����С����
	float maxlon; //�õ���Χ����󾭶�
	float minlat; //�õ���Χ����Сγ��
	float maxlat; //�õ���Χ�����γ��
	TreeNode *firch; //�ýڵ���ĸ��ӽڵ�
	TreeNode *secch;
	TreeNode *thrch;
	TreeNode *fouch;
	int mark;    //��ǣ�0��ʾ�м�ڵ㣬1��ʾҶ�ӽڵ�
	int num;     //��ʾ�ڵ����
	int maxindex;//�÷�Χ���ȵ�����±�
	int minindex;//�÷�Χ���ȵ���С�±�
	int numpoint;//�÷�Χ���ȵ����
	vector<HeatPoint> h;//�����ȵ������

};

//���ݵ�GPU�ϵ��Ĳ����ṹ
struct Tree
{

	float midlon;//�ýڵ����ĵ���Χ�ľ�����ֵ
	float midlat;//�ýڵ����ĵ���Χ��γ����ֵ
	float maxindex;//���ڸýڵ����ĵ���Χ�ڵ��ȵ���������±�
	float minindex;//���ڸýڵ����ĵ���Χ�ڵ��ȵ�������С�±�
};

class QuadTree
{
public:
	QuadTree(
		int index, //�����±�����
		int layer
		);
	void CreatTree(TreeNode* rot);//������

	void Divide(HeatPoint j,TreeNode* rot);//�ȵ����鰴���򻮷�

	void PutPoint (TreeNode* rot,HeatPoint* pt);//�������򻮷�֮����ȵ����������

	void PutTree(Tree* t,TreeNode* rot);//��������GPU�е����ṹ

	//int Callayer();//�������Ĳ���

	int Calnum();//�������Ľڵ��ܸ���

protected:
	int mIndex;
	int mLayer;//�Ĳ����Ĳ���
	int mTreeNum;//�Ĳ����ڵ����
	int mNum;//���һ��Ҷ�ӽڵ���ʼ�±�

};