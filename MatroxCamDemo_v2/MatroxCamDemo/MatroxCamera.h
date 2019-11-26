#pragma once

#include <Mil.h>
#include <map>
#include <afxpriv.h>
#include <vector>

#define BUFFERING_SIZE_MAX 22


/*�ɴ���ص������Ľṹ��*/
typedef struct
{
	MIL_INT ProcessedImageCount = 1;    //���ڼ�ס�ɼ�������
	INT FramesCount;					//ÿ�δ����ɼ���֡��
	CMvImage mvTempImage;				//��ʱ��ŵõ���ͼƬ
	CMvImage DesImage;				//���յõ���ͼƬ
	INT mvImageWidth;					//ͼƬ�Ŀ��
	INT mvImageHeight;				//ͼƬ�ĸ߶�
	void* Context;						//���������
	MIL_ID ModifiedBufferID;
}HookDataStruct;

class CMatroxCamera
{
public:
	CMatroxCamera();
	~CMatroxCamera();

	bool InitCam(int CameraID); //��ʼ�����
 
	void CloseCam(); //�ر����

	BOOL IsOpen() const { return m_bIsOpen; }

	void GrabSignalImage(int iheight);     //�ɼ�һ��ͼƬ
	void GrabImageContinue();   //�����ɼ�ͼƬ
	void SoftTrigger();         //����
	void StopGrabImage();       //ֹͣ�ɼ�ͼƬ���������ɼ�ʱ����

	void callback();

	void ImageShow(CMvImage *image);

	void SetLinePeriod(MIL_DOUBLE LinePeriod); //���������Ƶ
	void SetExposureTime(MIL_DOUBLE Time);  //��������ع�ʱ��
	void GetLinePeriod(MIL_DOUBLE &LinePeriod); //��ȡ�����Ƶ
	void GetExposureTime(MIL_DOUBLE &Time);  //��ȡ����ع�ʱ��
	void SetPreampGain(INT Type);  //������������棬1��x1����2��x2����4��x4��
	void GetPreampGain(MIL_INT64 &PreampGain);  //��ȡ��ǰ������������
	void SetGain(MIL_DOUBLE Gain);   //���������ģ������
	void GetGain(MIL_DOUBLE &Gain);  //��ȡ�����ģ������
	void SetDigitalGain(MIL_DOUBLE DigitalGain);  //�����������������
	void GetDigitalGain(MIL_DOUBLE &DigitalGain); //��ȡ�������������
	void SaveCameraParam();  //�����������

	void SetTriggerMode(int nIndex = 1);  //�����������ģʽ	
	void SetParam();        //���ò���
	void SetFrameCount(INT FrameCount);   //����ÿ�δ����ɼ���֡��

	void SpecifyIOState();
	MIL_INT GetIOState(const INT iInput);  //��ȡ����IO״̬

	void SetOutputState(const INT iOutput, MIL_INT iState); //�������IO״̬
	double GetCameraTemperature();  //��ȡ����¶�

	BOOL GetImage(BOOL Flag, CMvImage &mvImage);  //��ȡͼƬ��trueΪ�����ɼ���ͼƬ��falseΪ��֡�ɼ���ͼƬ


public:
	MIL_ID MilApplication;
	MIL_ID MilSystem;
	MIL_ID MilDigitizer;
	MIL_ID MilImage;
	MIL_INT SystemType = M_NULL;

	//MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
	MIL_ID *MilGrabBufferList;
	MIL_INT MilGrabBufferListSize = 0;
	HookDataStruct UserHookData;

	CMvImage m_mvSignalImage;
	INT m_mImageWidth;
	INT m_mImageHeight;
	INT m_iFramesCount;

	CMvImage mvTempImage;				//��ʱ��ŵõ���ͼƬ
	CMvImage DesImage;					//���յõ���ͼƬ

	BOOL IsTrigger;

private:
	HWND m_hWnd;

	std::map<int, MIL_INT> m_mapTrigSource;
	std::map<int, MIL_INT> m_mapUserBitSource;
	BOOL m_bIsOpen;
	MIL_INT m_numOfDigitizer;
	wchar_t* m_wcFilePath;
	
private:
	MIL_INT GetTrigLine(INT iTrigIndex);  //�������Ĵ���Դ

	MIL_INT GetUserBit(INT iUserBit);

	wchar_t* GetDCFFile();  //��������dcf�ļ�·��


};

