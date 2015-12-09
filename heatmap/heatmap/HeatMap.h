#pragma once
#include <osgCuda/Computation>
#include <osgCuda/Memory>
#include <osgCuda/Geometry>
#include <osgCuda/Texture>
#include <cuda_runtime.h>




//�ȵ���Ϣ�ṹ��
struct HeatPoint
{
	float k;//�ȶȿ���ϵ����������õ��ĸ�˹ֵ������ϴ�ϵ��
	float s;//��˹������׼�Խ���ʾ��������Խƽ��
	float lon;//�ȵ��������ȣ���λΪ�Ƕ�
	float lat;//�ȵ�����γ�ȣ���λΪ�Ƕ�
};

struct Tree;
class GenerateTexModule;

//�ȶ�ͼ��
class HeatMap:public osgCuda::Computation
{
public:
	HeatMap(unsigned int heatPointNum,//�ȵ����
		unsigned int treeNum,//���ڵ����
		unsigned int width,//��ͼ���
		unsigned int height,//��ͼ�߶�
		float minLon,//��ͼ��ʾ����С����
		float maxLon,//��ͼ��ʾ����󾭶�
		float minLat,//��ͼ��ʾ����Сγ��
		float maxLat,//��ͼ��ʾ�����γ��
		int times,
		int tnum,
		int tlayer//���Ĳ���
		);

	//����indexָʾλ�õ��ȵ�����
	void UpdatePoint(HeatPoint* point,int index);

	//����ȫ���ȵ�����
	void UpdateAllPoint(HeatPoint* pointList);

	void UpdateAllTree(Tree* pointList);
	inline bool IsValid(){return mIsValid;}

	//��ʼ����������Ҫ����Ϊ����Ա�����ĳ�ʼ��
	bool Init();

	//��ȡ����õ����ȶ�ͼ
	osg::Texture2D* GetTexture(){return mTexture.get();}
protected:
	osg::ref_ptr<osgCuda::Texture2D> mTexture;
	unsigned int mHeatPointNum;
	unsigned int mTreeNum;
	unsigned int mWidth;
	unsigned int mHeight; 
	float mMinLon;
	float mMaxLon;
	float mMinLat;
	float mMaxLat;
	int mTimes;
	int mTnum;
	int mLayer;
	bool mIsValid;

	//�ȵ����ݵı�������λ��ȫ���Դ棨�µģ�Ӧ���ǣ�
	osg::ref_ptr<osgCuda::Memory> mHeatPoint;

	//��ɫ��������������������ʱ�ڴ˴�������Ӧ����ɫֵ
	osg::ref_ptr<osgCuda::Memory> mColorMap;
    //���ṹ��������
	osg::ref_ptr<osgCuda::Memory> mTree;

	/*
	*�����Ϊ���ļ���������ڱ�����ע������ÿ�θ��±���ʱ���ø������launch����
	*��˿ɽ���Ҫ���������ָ�봫�ݸ����������cuda���㺯��д�ڸ������launch������
	*/
	osg::ref_ptr<GenerateTexModule> mTexGenerater;
	bool mNeedUpdate;
};

//���������
class GenerateTexModule : public osgCompute::Module
{
public:
	GenerateTexModule() : osgCompute::Module() 
	{
		//Ĭ��ֵ
		_minLon = 0;
		_maxLon = 10;
		_minLat = 0;
		_maxLat = 10;
		_heatPointNum = 1;
		_treeNum = 0;
		_times = 0;
		_tnum = 0;
		_tlayer = 0;
		clearLocal();
	}
	META_Object( , GenerateTexModule )

	//�������������Ǳ������ص�

	//��ʼ������Ҫ���ڷ����ڴ���Դ�ռ��
	virtual bool init();

	//һ�������ļ��㺯�����ڴ˴����ã������ע��󽫲����ֶ����øú���
	virtual void launch();

	//��ע������ͨ��addResource���ݵ����ݽ����Զ����ô˺������н���
	virtual void acceptResource( osgCompute::Resource& resource );	

	virtual void clear() { clearLocal(); osgCompute::Module::clear(); }
	
	void Update(){mNeedUpdate = true;}
	void SetUp(float minLon,float maxLon,float minLat,float maxLat,int heatPointNum,int treeNum,int times,int tnum,int tlayer)
	{
		_minLon = minLon;
		_maxLon = maxLon;
		_minLat = minLat;
		_maxLat = maxLat;
		_heatPointNum = heatPointNum;
		_treeNum = treeNum;
		_times = times;
		_tnum = tnum;
		_tlayer = tlayer;
	}
protected:
	virtual ~GenerateTexModule() { clearLocal(); }
	void clearLocal() 
	{
		_sourceBuffer = NULL;
		_targetBuffer = NULL;
		_colorMapBuffer = NULL;
		_treeBuffer = NULL;
	}

	dim3                                             _threads;
	dim3                                             _blocks;
	osg::ref_ptr<osgCompute::Memory>                 _sourceBuffer;
	osg::ref_ptr<osgCompute::Memory>                 _targetBuffer;
	osg::ref_ptr<osgCompute::Memory>                 _colorMapBuffer;
	osg::ref_ptr<osgCompute::Memory>                 _treeBuffer;
	float                                            _minLon;
	float                                            _maxLon;
	float                                            _minLat;
	float                                            _maxLat;
	unsigned int                                     _heatPointNum;
	unsigned int                                     _treeNum;
	int                                              _times;
	int                                              _tnum;
	int                                              _tlayer;
	bool                                             mNeedUpdate;
private:
	GenerateTexModule(const GenerateTexModule&, const osg::CopyOp& ) {}
	inline GenerateTexModule &operator=(const GenerateTexModule &) { return *this; }
};