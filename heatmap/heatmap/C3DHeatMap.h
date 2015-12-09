#pragma once
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/TextureCubeMap>
#include <osg/Drawable>
#include <osg/Geode>
#include <osgDB/ReadFile>
typedef unsigned int        UINT;
class C3DHeatMap :
	public osg::Group
{
	UINT mHeightmapID;//�������
	UINT mTexturemapID;//�ʺ��
	int mSimpleNum;//�̵߳�Ĳ�����
	float mSpaceLength;//�������
	osg::Drawable* regularGrid;
	float minlon;
	float maxlon;
	float minlat;
	float maxlat;
public:
	C3DHeatMap(
		float minLon,//��ͼ��ʾ����С����
		float maxLon,//��ͼ��ʾ����󾭶�
		float minLat,//��ͼ��ʾ����Сγ��
		float maxLat//��ͼ��ʾ�����γ��
		);
	~C3DHeatMap(void);
	// ���������ȶ�ֵ�������ڵ�
	int Create(osg::Texture2D& trgTexture);
};
