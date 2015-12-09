#include <osgDB/ReadFile>
#include "HeatMap.h"
#include <iostream>
#include "Quadtree.h"

//����CUDA�˺�����һ���������ڵ��ú˺������м��㣬����ʵ��λ��cu�ļ���
extern "C"
void generateTexFromHeatPoint(const dim3& blocks,
							  const dim3& threads,
							  void* texBuffer,
							  unsigned int texPitch,
							  void* heatPoints,
							  void* treePoints,
							  void* colorMap,
							  unsigned int pointNum,
							  unsigned int imageWidth,
							  unsigned int imageHeight,
							  float minLon,
							  float maxLon,
							  float minLat,
							  float maxLat,
							  int times,
							  int tnum,
							  int tlayer);

HeatMap::HeatMap(unsigned int heatPointNum,
				 unsigned int treeNum,
				 unsigned int width,
				 unsigned int height,
				 float minLon,
				 float maxLon,
				 float minLat,
				 float maxLat,
				 int times,
				 int tnum,
				 int tlayer):osgCuda::Computation()
{	
	mHeatPointNum = heatPointNum;
	mTreeNum = treeNum;
	mWidth = width;
	mHeight = height;
	mMinLon = minLon;
	mMaxLon = maxLon;
	mMinLat = minLat;
	mMaxLat = maxLat;
	mTimes = times;
	mTnum = tnum;
	mLayer = tlayer;
	mIsValid = true;
	mNeedUpdate = true;
}

void HeatMap::UpdatePoint( HeatPoint* point,int index )
{
	if (mIsValid)
	{
		//����Ҫ���µ����ݴ��ݵ��Դ�
		memcpy((void*)((HeatPoint*)mHeatPoint->map(osgCompute::MAP_HOST_TARGET)+index),
			(void*)point,
			sizeof(HeatPoint));
	}
	mTexGenerater->Update();
}

void HeatMap::UpdateAllPoint( HeatPoint* pointList )
{
	if (IsValid())
	{
		//����Ҫ���µ����ݴ��ݵ��Դ�
		memcpy(mHeatPoint->map(osgCompute::MAP_HOST_TARGET),
			(void*)pointList,
			sizeof(HeatPoint)*mHeatPointNum);
	}
	mTexGenerater->Update();
}

void HeatMap::UpdateAllTree( Tree* pointList )
{
	if (IsValid())
	{
		//����Ҫ���µ����ݴ��ݵ��Դ�
		memcpy(mTree->map(osgCompute::MAP_HOST_TARGET),
			(void*)pointList,
			sizeof(Tree)*mTreeNum);
	}
	mTexGenerater->Update();
}


bool HeatMap::Init()
{
	//��ʼ��texture
	mTexture = new osgCuda::Texture2D;
	mTexture->setInternalFormat( GL_RGBA32F_ARB );  
	mTexture->setSourceFormat( GL_RGBA );
	mTexture->setSourceType( GL_FLOAT );
	mTexture->setTextureHeight(mHeight);
	mTexture->setTextureWidth(mWidth);

	//�����ݼ�һ����ʾ�������ڼ�����ʶ�����ݵ�����
	mTexture->addIdentifier( "TRG_BUFFER" );

	//��ʼ�������ȵ����ݵ��Դ�����
	mHeatPoint = new osgCuda::Memory();
	//���������С
	mHeatPoint->setElementSize(sizeof(HeatPoint));
	//����ÿһά���ݵĴ�С���˴�ֻ��һά����
	mHeatPoint->setDimension(0,mHeatPointNum);
	mHeatPoint->addIdentifier("SRC_ARRAY");

	//���úú���г�ʼ��
	if (!mHeatPoint->init())
	{
		mIsValid = false;
		return false;
	}
	//��ʼ���������ڵ����ݵ��Դ�����
	mTree = new osgCuda::Memory();
	//���������С
	mTree->setElementSize(sizeof(Tree));
	//����ÿһά���ݵĴ�С���˴�ֻ��һά����
	mTree->setDimension(0,mTreeNum);
	mTree->addIdentifier("TRE_ARRAY");

	//���úú���г�ʼ��
	if (!mTree->init())
	{
		mIsValid = false;
		return false;
	}

	//������ɫ�����ݿռ�
	osg::Image* colorImg = osgDB::readImageFile("color.png");
	if (!colorImg)
	{
		osg::notify(osg::NOTICE) << "Could not open \"color.png\" image." << std::endl;
		return NULL;
	}

	cudaChannelFormatDesc srcDesc;
	srcDesc.f = cudaChannelFormatKindUnsigned;
	srcDesc.x = 8;
	srcDesc.y = 8;
	srcDesc.z = 8;
	srcDesc.w = 8;

	mColorMap = new osgCuda::Memory;
	mColorMap->setElementSize( sizeof(osg::Vec4ub) );
	mColorMap->setChannelFormatDesc( srcDesc );
	mColorMap->setDimension( 0, colorImg->s() );
	mColorMap->setDimension( 1, colorImg->t() );
	mColorMap->setImage( colorImg );
	// Mark this buffer as the source array of the module
	mColorMap->addIdentifier( "COLOR_MAP" );

	//��ʼ���������
	mTexGenerater = new GenerateTexModule();
	
	if (!mTexGenerater.valid())
	{
		mIsValid = false;
		return false;
	}
	mTexGenerater->SetUp(mMinLon,mMaxLon,mMinLat,mMaxLat,mHeatPointNum,mTreeNum,mTimes,mTnum,mLayer);
	//��ʾ��ǰ�ڵ�����ӽڵ����ǰ���и���
	setComputeOrder(  osgCompute::Computation::PRERENDER_BEFORECHILDREN );
	//ע��������
	addModule(*mTexGenerater.get());
	
	//������ע����������������Դ
	addResource(*mTexture->getMemory());
	addResource(*mHeatPoint);
	addResource(*mTree);
	addResource(*mColorMap);
	//mTexture->getMemory()->init();
	mTexGenerater->init();
	return true;
}




bool GenerateTexModule::init()
{
	if (!_sourceBuffer||!_targetBuffer||!_treeBuffer)
	{
		return false;
	}

	//��ʾÿ��block���̷ֲ߳�Ϊ16*16
	_threads = dim3(16,16,1);
	unsigned int numReqBlocksWidth = 0, numReqBlocksHeight = 0;

	//����Ҫ���ɵ������С��block���̵߳ķֲ�����block�ĸ���
	//�Ա�֤ÿ��������һ���̼߳���
	if( _targetBuffer->getDimension(0) % 16 == 0) 
		numReqBlocksWidth = _targetBuffer->getDimension(0) / 16;
	else
		numReqBlocksWidth = _targetBuffer->getDimension(0) / 16 + 1;

	if( _targetBuffer->getDimension(1) % 16 == 0) 
		numReqBlocksHeight = _targetBuffer->getDimension(1) / 16;
	else
		numReqBlocksHeight = _targetBuffer->getDimension(1) / 16 + 1;

	_blocks = dim3( numReqBlocksWidth, numReqBlocksHeight, 1 );

	//��������һ�θ���ĳ�ʼ������
	return osgCompute::Module::init();
}

//��computation���ÿ�θ���ʱ���ã�����ִ��һ�μ���
void GenerateTexModule::launch()
{	
	//����ȵ�����û�и������ü���
	if (mNeedUpdate)
	{	
		extern time_t startall;
		time_t start = clock();
		std::cout<<"��ʼlaunch��"<<std::endl;
		memset((float*)_targetBuffer->map(osgCompute::MAP_HOST_TARGET),0.2,4*_targetBuffer->getDimension(0)*_targetBuffer->getDimension(1));
		//��һ�μ��㣬���Ǻ˺����Ĵ�������ʵ���ǵ����˺˺���
		generateTexFromHeatPoint(_blocks,
			_threads,
			_targetBuffer->map(),
			_targetBuffer->getPitch(),
			_sourceBuffer->map(osgCompute::MAP_DEVICE_SOURCE),
			_treeBuffer->map(osgCompute::MAP_DEVICE_SOURCE),
			_colorMapBuffer->map(osgCompute::MAP_DEVICE_ARRAY),
			_heatPointNum,
			_targetBuffer->getDimension(0),
			_targetBuffer->getDimension(1),
			_minLon,
			_maxLon,
			_minLat,
			_maxLat,
			_times,
			_tnum,
			_tlayer);

		cudaThreadSynchronize();
		time_t end = clock();
		std::cout<<"CUDA������ʱ��"<<(double)(end-start)/1000.0<<std::endl;
		std::cout<<"�ܼ�����ʱ��"<<(double)(end-startall)/1000.0<<std::endl;
		//�������󽫸��±�־��գ�ʹ�´����ݸ���֮ǰ����������
		mNeedUpdate = false;

	

	}
}

//���ܴ�computation�ഫ���������ݶ�����ΪԴ���ݺʹ��Ŀ������
void GenerateTexModule::acceptResource( osgCompute::Resource& resource )
{
	//��Ž���Ļ��棬�����ɵ�����
	if( resource.isIdentifiedBy( "TRG_BUFFER" ) )
	{
		_targetBuffer = dynamic_cast<osgCompute::Memory*>( &resource );
	}

	//�����ͼ���ȵ��λ�õ���Ϣ
	if( resource.isIdentifiedBy( "SRC_ARRAY" ) )
		_sourceBuffer = dynamic_cast<osgCompute::Memory*>( &resource );

	//������ڵ�������Ϣ
	if( resource.isIdentifiedBy( "TRE_ARRAY" ) )
		_treeBuffer = dynamic_cast<osgCompute::Memory*>( &resource );

	//��ɫ������
	if( resource.isIdentifiedBy( "COLOR_MAP" ) )
		_colorMapBuffer = dynamic_cast<osgCompute::Memory*>( &resource );
}
