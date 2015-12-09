#include <osgCuda/Computation>
#include <osgCuda/Memory>
#include <osgCuda/Geometry>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgDB/WriteFile>
#include <vector>
#include "HeatMap.h"
#include "C3DHeatMap.h"
#include "Quadtree.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <fstream>
#include <math.h>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarth/MapNode>
#include <osgEarthUtil//SkyNode>

using namespace std;

extern time_t startall =0 ;
//����һ�������ض�����ķ�����Ƭ
osg::Geode* getTexturedQuad( osg::Texture2D& trgTexture )
{
	osg::Geode* geode = new osg::Geode;
	osg::Vec3 llCorner = osg::Vec3(0,0,0);
	osg::Vec3 width = osg::Vec3(2048,0,0);
	osg::Vec3 height = osg::Vec3(0,2048,0);

	////////// 
	// QUAD //
	//////////
	osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry( llCorner, width, height );
	geode->addDrawable( geom );
	geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, &trgTexture, osg::StateAttribute::ON );
	geode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
	return geode;
}


//�������˵��HeatMap���ʹ�÷�ʽ
int main()
{
	 startall = clock();
	osg::ref_ptr<osg::Group> scene = new osg::Group;
	
	//�½�HeatMap�࣬�������ĸ��ȵ㣬ͼ�ķֱ���Ϊ4096����ʾ������Ϊ ���ȣ�0-10�ȣ���γ�ȣ�0-10�ȣ�
	//Ŀǰ����һЩ�ٽ�����û���ǣ��������-180�Ⱥ�180�Ƚ�ϲ�λ�ļ���
	//int numHeatPoint = 100000;
	int numHeatPoint = 1797170;
	int numseekgroup = 0;
	int numseekdata = 3*numseekgroup;
	int times = 1000;
	int tnum = numHeatPoint / times;
	HeatPoint* h = new HeatPoint[numHeatPoint];
    HeatPoint* pxh = new HeatPoint[numHeatPoint];//�����ֲ��������ȵ�����
	int mapwidth = 2048;
	int mapheight = 2048;
	float mapminlon = -129.0;
	float mapmaxlon = -61.0;
	float mapminlat = 18.8;
	float mapmaxlat = 51.18;//�����������ݷ�Χ

	/*float mapminlon = 73.5;
	float mapmaxlon = 135.7;
	float mapminlat = 18.15;
	float mapmaxlat = 53.5;*///�й��������ݷ�Χ

    //Ϊ����ʼ��
	TreeNode* root = new TreeNode;
	root->minlon = mapminlon;
	root->maxlon = mapmaxlon;
	root->minlat = mapminlat;
	root->maxlat = mapmaxlat;
	root->num = 0;
	root->mark = 0;
	root->numpoint = 0;

	int heatpointindex = 0;//�ȵ����׷���±�
	int layer = 5;

	QuadTree* quadTree = new QuadTree(heatpointindex,layer);
	int treenum = quadTree->Calnum();//���������ܽڵ���Ŀ
	std::cout<<"������"<<layer<<"�����ܽ����Ŀ��"<<treenum<<std::endl;
	Tree* tr = new Tree[treenum];//��Ŵ��ݵ��Դ��е����ṹ
	quadTree->CreatTree(root);//�������ṹ

	fstream infile;
	infile.open("../data/dataUSEdge.txt");
	//float fvalue;
	int i=0;
	while(!infile.eof())
	{
		if(i>(numHeatPoint-1))
		{
			break;
		}
		
		infile>>h[i].lon;
		infile>>h[i].lat;
		int value;
		infile>>value;
		h[i].k = 0.0008/**(float)value*/;
		//h[i].k = 0.001;
		h[i].s = 0.08;
		i++;
	

	}//�����������ݶ�ȡ
	//while(!infile.eof())
	//{
	//	if(i>(numHeatPoint-1))
	//	{
	//		break;
	//	}
	//	infile>>h[i].lat;
	//	infile>>h[i].lon;
	//	h[i].k = 0.001;
	//	h[i].s = 0.1;
	//	i++;

	//	for(int i=0;i<(1+numseekdata);i++)
	//	{
	//		infile>>fvalue;
	//	}
	//}//�й��������ݶ�ȡ
	infile.close();//���ļ��е��ȵ����ݶ��벢������ȵ�����h��
	//for (int i=0;i<1700000;i++)
	//{
	//		cout<<h[i].lon<<" "<<h[i].lat<<endl;
	//}

    //���ȵ㰴���򻮷�
	for(int i=0;i<numHeatPoint;i++)
	{
		quadTree->Divide(h[i],root);
	}

	quadTree->PutPoint(root,pxh);//�����ֺ���ȵ��������pxh��
	quadTree->PutTree(tr,root);//�������ṹ��������
	delete(h);
	time_t endcpu = clock();
	std::cout<<"CPU������ʱ��"<<(double)(endcpu-startall)/1000.0<<std::endl;
   
    HeatMap* heatMap = new HeatMap(numHeatPoint,treenum,mapwidth,mapheight,mapminlon,mapmaxlon,mapminlat,mapmaxlat,times,tnum,layer);

	//HeatMap��̳���osgComputation�࣬������ӵ��������£�����ִ��
	scene->addChild(heatMap);


	osg::ref_ptr<osg::Texture2D> mChinamap;
	osg::Image* mapImg = osgDB::readImageFile("chinamap.png");
	if (!mapImg)
	{
		osg::notify(osg::NOTICE) << "Could not open \"chinamap.png\" image." << std::endl;
		return NULL;
	}
	mChinamap = new osg::Texture2D(mapImg);
	//mChinamap->setTextureHeight(mHeight);
	//mChinamap->setTextureWidth(mWidth);
	mChinamap->setImage(mapImg);
    osgViewer::Viewer viewer;
	osg::Node* earth = osgDB::readNodeFile("D:\\OSG\\OSGEarth\\cloudmade.earth");
	scene->addChild(earth);

	osgEarth::MapNode* mapNode = osgEarth::MapNode::findMapNode(earth);
	osgEarth::Util::SkyNode* mSky = new osgEarth::Util::SkyNode(mapNode->getMap());
	mSky->setDateTime(2012,3,9,4);
	mSky->attach(&viewer);
	scene->addChild(mSky);


	
	if (heatMap->Init())
	{
		//����HeatMap���GetTexture������ȡ����õ����ȶ�ͼ�����ɰ���ͨ����ʹ��
		//�½�һ����Ƭ�����������ɵ�����
		//scene->addChild(getTexturedQuad(*heatMap->GetTexture()));

		//���ڻ��Ƹ߳�ͼ������
		C3DHeatMap* c3dheatMap = new C3DHeatMap(mapminlon,mapmaxlon,mapminlat,mapmaxlat);
		c3dheatMap->Create(*heatMap->GetTexture());


		osg::StateSet* stateset = c3dheatMap->getOrCreateStateSet();
		//stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		//stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);





		//��ӵ�������
		//scene->addChild(getTexturedQuad(*mChinamap));
		scene->addChild(c3dheatMap);
		

	}
	
	
	
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer.setReleaseContextAtEndOfFrameHint(false);
	viewer.setSceneData(scene.get());

	
	//���Զ�����ȵ����ݸ�����ͼ
	//���ֻ��һ�����ݸı䣬���ɵ��� UpdatePoint(HeatPoint* point,int index)���е�������
	//�˺�����ÿ�ε��ý�������һ��CUDA����
	/*heatMap->UpdateAllPoint(h);*/
	heatMap->UpdateAllPoint(pxh);
	heatMap->UpdateAllTree(tr);
	//viewer.setCameraManipulator(new osgGA::TrackballManipulator);

	viewer.setCameraManipulator(new osgEarth::Util::EarthManipulator);
	viewer.setUpViewInWindow(100,100,800,600);


	while(!viewer.done())
	{			
		viewer.frame();		
	}




	//viewer.setUpViewInWindow(100,100,800,600);
	//viewer.getCamera()->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
	//viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(1114.367310,-1491.598999,1113.737549),
	//	osg::Vec3(1114.334473,-1490.685059,1113.332886),
	//	osg::Vec3(-0.026274,0.403902,0.914425));
	//
	//while(!viewer.done())
 //	{			
 //		viewer.frame();	
	//	osg::Vec3 eyepos,eyecenter,eyeup;
	//	viewer.getCamera()->getViewMatrixAsLookAt(eyepos,eyecenter,eyeup);
	//	/*printf("eyepos:%f,%f,%f eyecenter:%f,%f,%f eyeup:%f,%f,%f \n",
	//		eyepos.x(),eyepos.y(),eyepos.z(),
	//		eyecenter.x(),eyecenter.y(),eyecenter.z(),
	//		eyeup.x(),eyeup.y(),eyeup.z());*/
 //	}
}