#include "C3DHeatMap.h"
#include <osg/CullFace>
//������������
osg::Geometry* createRegularGrid(int samplenum)
{
	osg::ref_ptr<osg::Vec3sArray> vec = new osg::Vec3sArray;

	for(int i=0;i<samplenum-1;i++)
		for(int j=0;j<samplenum-1;j++)
		{
			osg::Vec3s vertexLeftButtom(i,j,0); //����
			osg::Vec3s vertexLeftTop(i,(j+1),0);//����
			osg::Vec3s vertexRightButtom((i+1),j,0);//����
			osg::Vec3s vertexRightTop((i+1),(j+1),0.0);//����
			//���ƹ�������
			vec->push_back(vertexLeftButtom);
			vec->push_back(vertexRightButtom);
			vec->push_back(vertexRightTop);
			vec->push_back(vertexLeftTop);
		}
		osg::Geometry* geo = new osg::Geometry;
		geo->setVertexArray( vec.get() );
		geo->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, (samplenum-1)*(samplenum-1)*4 ) );
		return geo;
}
C3DHeatMap::C3DHeatMap(float minLon,
					   float maxLon,
					   float minLat,
					   float maxLat)
{
	//��ʼ��һЩ����
	mSimpleNum=100;//�̵߳�Ĳ�����
	mSpaceLength=10;//�������
	mHeightmapID=0;
	mTexturemapID=1;
	minlon = minLon;
	maxlon = maxLon;
	minlat = minLat;
	maxlat = maxLat;
}

C3DHeatMap::~C3DHeatMap(void)
{
}

// ���������ȶ�ֵ�������ڵ�
int C3DHeatMap::Create(osg::Texture2D& trgTexture)
{
	//��ʼ��һЩ����
	mSimpleNum=2048;//�̵߳�Ĳ�����
	mSpaceLength=1;//�������


	// ��ȡ��Ⱦ״̬StateSet��
	osg::StateSet* state = getOrCreateStateSet();
	// ����һ��PolygonMode ��Ⱦ���ԡ�
	osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	osg::CullFace* cullface = new osg::CullFace(osg::CullFace::BACK);
	state->setAttributeAndModes( cullface,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	// ǿ��ʹ���߿���Ⱦ��
	//state->setAttributeAndModes( pm,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	//��������դ��
	regularGrid=(osg::Drawable*)createRegularGrid(mSimpleNum);

	//��������
	//SetHeightMap("data/ps_height_4k.png");

	//��������
	//SetTextureMap("data/ps_texture_4k.png");

	//������ɫ��
	osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
	osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
	vertexShader->loadShaderSourceFromFile( "shader/3DHeatMap.vert" );
	fragmentShader->loadShaderSourceFromFile( "shader/3DHeatMap.frag" );
	osg::Program* program = new osg::Program;
	program->addShader( vertexShader );
	program->addShader( fragmentShader);

	//��Ŀ¼����Ⱦ״̬
	osg::StateSet* stateset = getOrCreateStateSet();
	stateset->setAttributeAndModes( program, osg::StateAttribute::ON );

	osg::Geode* geode = new osg::Geode;
	geode->addDrawable(regularGrid);
	geode->getOrCreateStateSet()->setTextureAttributeAndModes( mHeightmapID, &trgTexture, osg::StateAttribute::ON );
	osg::Uniform* heightMapID = new osg::Uniform( "heightMapID", (int) mHeightmapID );
	stateset->addUniform( heightMapID );
	//������ģ
	osg::Uniform* parSimpleNum = new osg::Uniform( "parSimpleNum",int(mSimpleNum));
	stateset->addUniform( parSimpleNum );
	//�������
	osg::Uniform* parSpacing = new osg::Uniform( "parSpacing",float(mSpaceLength));
	stateset->addUniform( parSpacing );

	osg::Uniform* parMinlon = new osg::Uniform( "parMinlon",float(minlon));
	stateset->addUniform( parMinlon );

	osg::Uniform* parMaxlon = new osg::Uniform( "parMaxlon",float(maxlon));
	stateset->addUniform( parMaxlon );

	osg::Uniform* parMinlat = new osg::Uniform( "parMinlat",float(minlat));
	stateset->addUniform( parMinlat );

	osg::Uniform* parMaxlat = new osg::Uniform( "parMaxlat",float(maxlat));
	stateset->addUniform( parMaxlat );



	//��Ӳʺ�����
	osg::Image* imageTexture = osgDB::readImageFile("color.png");
	osg::Texture2D* tex = new osg::Texture2D( imageTexture );
	tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
	tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
	tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	stateset = getOrCreateStateSet();
	stateset->setTextureAttributeAndModes( mTexturemapID, tex, osg::StateAttribute::ON );
	osg::Uniform* textMapID = new osg::Uniform( "textMapID", (int) mTexturemapID );
	stateset->addUniform( textMapID );

	geode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
	addChild(geode);

	return 0;
}
