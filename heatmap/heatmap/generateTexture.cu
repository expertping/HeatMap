#ifndef GENERATE_KERNEL_H
#define GENERATE_KERNEL_H 1

#define E 2.718281828459          
#define SQRT2PI 2.5066282746

//����ɫ��洢����ӳ�䵽һ�������ϣ��Ϳ������������ʽ���д�ȡ
texture<uchar4, 2, cudaReadModeNormalizedFloat> colorTex;

//������ĳ��ԴӰ���µ�ǰ��ĸ�˹ֵ
__device__
float getGaussian(float4* sourcePoint,float trgLon,float trgLat)
{
	//�����ر�ʾ��λ�ú��ȵ�����λ�õľ���
	float spaceLon = trgLon - (*sourcePoint).z;
	float spaceLat = trgLat - (*sourcePoint).w;
	float pointSpace2 = spaceLon*spaceLon + spaceLat*spaceLat;
	
	//��˹��ʽ�ķ�ĸ
	float denominator = (*sourcePoint).y * SQRT2PI;
	//��˹��ʽ�ķ���
	
	float numerator = pow(E,(pointSpace2/(-2.0*(*sourcePoint).y*(*sourcePoint).y))); 
	
	//���Ҫ���Կ���ϵ��
	return (*sourcePoint).x*numerator/denominator;
}

//������������
__device__
float dis_PP(float x1,float y1,float x2,float y2)
{
    return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

//�����̺߳˺������˺������������ã�������һ�����ص�����ֵ

__global__ 
void generateTexKernel(float4* targetBuffer,unsigned int trgPitch,float4* sourceBuffer,float4* treeBuffer,unsigned int pointNum,unsigned int imageWidth, unsigned int imageHeight,float minLon,float maxLon,float minLat,float maxLat,int layer)
{
	//���㵱ǰ�߳��������̷߳�����������λ�ã����ھ���������������λ��
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
    
	//����̺߳ų����������С�Ͳ��ü��㣬ֱ���˳�����
	if( x < imageWidth && y < imageHeight )
    {
		//�����̺߳ż��㵽��ǰ���ص�λ��
		float4* target = (float4*)(((char*) targetBuffer) + trgPitch * y ) + x;
		
		//if( (*target).w < 0.9999999999 )
		{
			//���㵱ǰ���ر�ʾ��ʵ������
			float lon = ((float)(x)/(float)imageWidth)*(float)(maxLon - minLon)+(float)minLon;
			float lat = ((float)(y)/(float)imageHeight)*(float)(maxLat - minLat)+(float)minLat;

			float num = (*target).w;//��ǰ���ر�ʾ��ʵ��������ȶ�ֵ

			int treenum = 0;//��ǰ���ʵ��Ĳ��������±�
			for(int i=0;i<layer;i++)//�����Ĳ��������У���ǰ���ر�ʾ�ĵ�������ֻ�������ڵ�ǰ���ڵ���ĸ��ӽڵ��ʾ�ĵ���Χ�е�һ��,���жϵĴ����������Ĳ���
			{

				int maxindex = int((*(treeBuffer+treenum)).z);//��Ҫ������ȵ����ȵ������е�����±�
				int minindex = int((*(treeBuffer+treenum)).w);//��Ҫ������ȵ����ȵ������е���С�±�
			    float midlon = (*(treeBuffer+treenum)).x;//��ǰ���ڵ����ĵ���Χ�ľ�����ֵ
				float midlat = (*(treeBuffer+treenum)).y;//��ǰ���ڵ����ĵ���Χ��γ����ֵ

				if(minindex!=-1)//�ڵ�ǰ���ڵ��ʾ�ĵ���Χ�����ȵ�
				{

					for(int j=minindex;j<=maxindex;j++)//����÷�Χ�ڵ��ȵ�������ȶ�ֵ�������ۼ�
				  {
                      float dis = dis_PP(lon,lat,(*(sourceBuffer+j)).z,(*(sourceBuffer+j)).w);
                      float s = 3 * ((*(sourceBuffer+j)).y);
                     if (dis<=s)
                     {
                         num += getGaussian(sourceBuffer+j,lon,lat)*(0.9-num);
                     }
					 

				  }
				}
				//�����������жϵ�ǰ���ر�ʾ�ĵ����������ڵ�ǰ���ڵ���ĸ��ӽڵ��е���һ��
				if(lon<midlon && lat<midlat)
				{
					treenum = treenum*4+1;
				}
				else 
					if(lon>=midlon && lat<midlat)
					{
						treenum = treenum*4+2;
					}
					else
						if(lon<midlon && lat>=midlat)
						{
							treenum = treenum*4+3;
						}
						else
						{
							treenum = treenum*4+4;
						}

				
			}

			////��˳����������ȵ�Ե�ǰ���ص�Ӱ�죬��������Ӱ��ֵ����
			//for(unsigned int i=0;i<pointNum;i++)
			//{			
			//	num += getGaussian(sourceBuffer+i,lon,lat)*(0.9-num);
			//}	

		//	float _x = num>1.0?1.0:num;
			//ȥ�󶨵���ɫ�������ȡ��ɫ	
		//	float4 color = tex2D( colorTex, _x*1.111111, 0.5 );

			//�����͸������ʱֻ��˴����ȶ�ǿ�����
		//	color.w = _x;
			(*target) = make_float4(num,num,num,num);
		}
	}
}


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
							  int tlayer)
{
	colorTex.normalized = true;                      // normalized texture coordinates (element of [0:1])
    colorTex.filterMode = cudaFilterModeLinear;      // bilinear interpolation 
    colorTex.addressMode[0] = cudaAddressModeClamp;  // wrap texture coordinates
    colorTex.addressMode[1] = cudaAddressModeClamp;

    // ���Դ�����󶨵�һ������
    cudaBindTextureToArray( colorTex, reinterpret_cast<cudaArray*>(colorMap) );

	generateTexKernel<<<blocks,threads>>>(reinterpret_cast<float4*>(texBuffer),
		texPitch,
		reinterpret_cast<float4*>(heatPoints),
		reinterpret_cast<float4*>(treePoints),
		pointNum,
		imageWidth,
		imageHeight,
		minLon,
		maxLon,
		minLat,
		maxLat,
		tlayer);
		cudaThreadSynchronize();

	
	

	
	//cudaMemcpy2D(texBuffer,texPitch,tmpTexBuffer,texPitch,texPitch,imageHeight,cudaMemcpyDeviceToDevice);


}

#endif