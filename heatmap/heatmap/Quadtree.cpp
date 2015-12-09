#include "Quadtree.h"
#include <fstream>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <vector>

//struct HeatPoint;

QuadTree::QuadTree(
				   int index,
				   int layer
				   )
{
	mIndex = index;
	mLayer = layer;
	mTreeNum = 0;
	mNum = 0;

}


void QuadTree::CreatTree(TreeNode* rot)      //�����Ĳ���
	{
		
		if((rot->num)>=mNum)      //��������С�㹻С��С���趨���ٽ�ֵ�����ڼ������֣������ݹ�
		{
			/*float kuadulon = rot->maxlon - rot->minlon;
			float kuadulat = rot->maxlat - rot->minlat;
			std::cout<<"lon���:"<<kuadulon<<"lat��ȣ�"<<kuadulat<<std::endl;*/
			rot->mark = 1;  //Ҷ�ӽڵ���
			rot->firch = NULL;
			rot->secch = NULL;
			rot->thrch = NULL;
			rot->fouch = NULL;
		}
		else  //Ŀǰ�����Կ��Լ���������ȥ
		{
			TreeNode* one = new TreeNode;  //�����ĸ��ӽڵ�
			TreeNode* two = new TreeNode;
			TreeNode* three = new TreeNode;
			TreeNode* four = new TreeNode;
			float midlon = (rot->maxlon + rot->minlon)/2.0;
			float midlat = (rot->maxlat + rot->minlat)/2.0;


			one->minlon = rot->minlon;  //�����ĸ��ӽڵ������Χ
			one->maxlon = midlon;
			one->minlat = rot->minlat;
			one->maxlat = midlat;
			one->mark = 0;
			one->numpoint = 0;

			two->minlon = midlon;
			two->maxlon = rot->maxlon;
			two->minlat = rot->minlat;
			two->maxlat = midlat;
			two->mark = 0;
			two->numpoint = 0;

			three->minlon = rot->minlon;
			three->maxlon = midlon;
			three->minlat = midlat;
			three->maxlat = rot->maxlat;
			three->mark = 0;
			three->numpoint = 0;

			four->minlon = midlon;
			four->maxlon = rot->maxlon;
			four->minlat = midlat;
			four->maxlat = rot->maxlat;
			four->mark = 0;
			four->numpoint = 0;

			one->num = 4*(rot->num)+1;//��ĳһ��Ҷ�ӽ�������Ϊn�����ĸ��ӽ����ŷֱ�Ϊ4n+1��4n+2��4n+3��4n+4
			two->num = 4*(rot->num)+2; 
			three->num = 4*(rot->num)+3;
			four->num = 4*(rot->num)+4;

			rot->firch = one;
			rot->secch = two;
			rot->thrch = three;
			rot->fouch = four;

			CreatTree(one);
			CreatTree(two);
			CreatTree(three);
			CreatTree(four);
		}

	}

void QuadTree::Divide(HeatPoint j,TreeNode* rot)//�ȵ����鰴���򻮷�
	{
		float s = j.s *3;//�ȵ�Ӱ�췶Χ�����ȵ�λ��ΪԲ�ģ�3sΪ�뾶����Բ���ȵ��Բ�η�Χ�����������ȶ�ֵ����Ϊ0�����Ժ��Բ������㣬ֻ������ȵ��Բ������������ȶ�ֵ
		if (rot->mark == 1)// �����Ҷ�ӽڵ�
		{
			rot->h.push_back(j);//���ȵ�ѹ�뵱ǰ����vector��
			rot->numpoint ++;//��ǰ�����ȵ��������1
		}

		else  //�������Ҷ�ӽ��,�ж��ȵ�Ӱ�췶Χ���ڵ�ǰ�����ĸ��ӽڵ㷶Χ��
			if ((j.lat+s) <= rot->firch->maxlat && (j.lat-s) >= rot->firch->minlat && (j.lon+s) <= rot->firch->maxlon && (j.lon-s) >= rot->firch->minlon)
			{
				Divide(j,rot->firch);//�����ݹ����
			}
			else
				if ((j.lat+s) <= rot->secch->maxlat && (j.lat-s) >= rot->secch->minlat && (j.lon+s) <= rot->secch->maxlon && (j.lon-s) >= rot->secch->minlon)
				{
					Divide(j,rot->secch);
				} 
				else
					if ((j.lat+s) <= rot->thrch->maxlat && (j.lat-s) >= rot->thrch->minlat && (j.lon+s) <= rot->thrch->maxlon && (j.lon-s) >= rot->thrch->minlon)
					{
						Divide(j,rot->thrch);
					}
					else
						if ((j.lat+s) <= rot->fouch->maxlat && (j.lat-s) >= rot->fouch->minlat && (j.lon+s) <= rot->fouch->maxlon && (j.lon-s) >= rot->fouch->minlon)
						{
							Divide(j,rot->fouch);
						}
						else //�ȵ�Ӱ�췶Χ�����ڵ�ǰ�����ĸ��ӽڵ��е��κ�һ����Χ��
						{
							rot->h.push_back(j);//�ȵ�ѹ�뵱ǰ����vector��
							rot->numpoint ++; //��ǰ�����ȵ��������1
						}

	}

void QuadTree::PutPoint (TreeNode* rot,HeatPoint* pt)//�������򻮷�֮����ȵ������������
{
	int numpoint = rot->numpoint;
	if (numpoint==0)  //�����ǰ���������������û���ȵ�
	{
		rot->minindex = -1; //���ȵ㷶Χ�±���Ϊ-1
		rot->maxindex = -1;

	} 
	else  //�������ʾ�����������ȵ�
	{
		rot->minindex = mIndex;  //�ý���ʾ��Χ����С�ȵ������±�
		rot->maxindex = mIndex+numpoint-1; //�ýڵ��ʾ��Χ������ȵ������±�
		for (int i=0;i<numpoint;i++) //vector�е�ÿ���ȵ㸴�Ƶ�����pt�У�ͬʱ����ֵ��������
		{
			pt[mIndex] = rot->h.at(i);
			mIndex++;
		}
	}

	if (rot->mark == 1)
	{
		return;
	} 
	else  //�����ǰ��㲻��Ҷ�ӽ��
	{
		PutPoint(rot->firch,pt); //�ݹ����
		PutPoint(rot->secch,pt);
		PutPoint(rot->thrch,pt);
		PutPoint(rot->fouch,pt);

	}
}

void QuadTree::PutTree(Tree* t,TreeNode* rot)//��������GPU�е����ṹ
{
	int i = rot->num;//��������ż�Ϊ�������±�
	int mark = rot->mark;//�����ж��Ƿ�ΪҶ�ӽ��
	float minlon = rot->minlon;// ���㾭��γ���м�ֵ
	float maxlon = rot->maxlon;
	t[i].midlon = (minlon+maxlon)/2.0;
	float minlat = rot->minlat;
	float maxlat = rot->maxlat;
	t[i].midlat = (minlat+maxlat)/2.0;

	t[i].minindex = float(rot->minindex);//�ȵ������±괫��
	t[i].maxindex = float(rot->maxindex);
	if (mark==1)
	{
		return;
	} 
	else//�������Ҷ�ӽ��
	{
		PutTree(t,rot->firch);//�ݹ�
		PutTree(t,rot->secch);
		PutTree(t,rot->thrch);
		PutTree(t,rot->fouch);
	}
}

//int QuadTree::Callayer()//�������Ĳ���
//{
//	int mapsize = mWid * mLeng;
//	int di = 2;
//	double zmapsize = double(mapsize);
//	double zdi = double(di);
//	double mici ;
//	mici = floor(log(zmapsize)/log(zdi)+0.5);
//	int zmici = int(mici);
//	mLayer = (zmici-6)/2+1;
//	return(mLayer);
//}

int QuadTree::Calnum()//�������Ľڵ����
{
	double b = pow(4.0,mLayer);
	int c = int(b);
    mTreeNum = (c-1)/3;

	double a = pow(4.0,mLayer-1);
	int d = int(a);
	mNum = (d-1)/3;
	std::cout<<"Ҷ�ӽڵ���ʼ�±�Ϊ"<<mNum<<std::endl;
	return(mTreeNum);
}

