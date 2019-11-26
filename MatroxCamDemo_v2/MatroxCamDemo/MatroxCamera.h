#pragma once

#include <Mil.h>
#include <map>
#include <afxpriv.h>
#include <vector>

#define BUFFERING_SIZE_MAX 22


/*可传入回调函数的结构体*/
typedef struct
{
	MIL_INT ProcessedImageCount = 1;    //用于记住采集的张数
	INT FramesCount;					//每次触发采集的帧数
	CMvImage mvTempImage;				//临时存放得到的图片
	CMvImage DesImage;				//最终得到的图片
	INT mvImageWidth;					//图片的宽度
	INT mvImageHeight;				//图片的高度
	void* Context;						//保存相机类
	MIL_ID ModifiedBufferID;
}HookDataStruct;

class CMatroxCamera
{
public:
	CMatroxCamera();
	~CMatroxCamera();

	bool InitCam(int CameraID); //初始化相机
 
	void CloseCam(); //关闭相机

	BOOL IsOpen() const { return m_bIsOpen; }

	void GrabSignalImage(int iheight);     //采集一张图片
	void GrabImageContinue();   //连续采集图片
	void SoftTrigger();         //软触发
	void StopGrabImage();       //停止采集图片，当连续采集时调用

	void callback();

	void ImageShow(CMvImage *image);

	void SetLinePeriod(MIL_DOUBLE LinePeriod); //设置相机行频
	void SetExposureTime(MIL_DOUBLE Time);  //设置相机曝光时间
	void GetLinePeriod(MIL_DOUBLE &LinePeriod); //获取相机行频
	void GetExposureTime(MIL_DOUBLE &Time);  //获取相机曝光时间
	void SetPreampGain(INT Type);  //设置相机的增益，1：x1倍，2：x2倍，4：x4倍
	void GetPreampGain(MIL_INT64 &PreampGain);  //获取当前相机的增益参数
	void SetGain(MIL_DOUBLE Gain);   //设置相机的模拟增益
	void GetGain(MIL_DOUBLE &Gain);  //获取相机的模拟增益
	void SetDigitalGain(MIL_DOUBLE DigitalGain);  //设置相机的数字增益
	void GetDigitalGain(MIL_DOUBLE &DigitalGain); //获取相机的数字增益
	void SaveCameraParam();  //保存相机配置

	void SetTriggerMode(int nIndex = 1);  //设置相机触发模式	
	void SetParam();        //设置参数
	void SetFrameCount(INT FrameCount);   //设置每次触发采集的帧数

	void SpecifyIOState();
	MIL_INT GetIOState(const INT iInput);  //获取输入IO状态

	void SetOutputState(const INT iOutput, MIL_INT iState); //设置输出IO状态
	double GetCameraTemperature();  //获取相机温度

	BOOL GetImage(BOOL Flag, CMvImage &mvImage);  //获取图片，true为连续采集的图片，false为单帧采集的图片


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

	CMvImage mvTempImage;				//临时存放得到的图片
	CMvImage DesImage;					//最终得到的图片

	BOOL IsTrigger;

private:
	HWND m_hWnd;

	std::map<int, MIL_INT> m_mapTrigSource;
	std::map<int, MIL_INT> m_mapUserBitSource;
	BOOL m_bIsOpen;
	MIL_INT m_numOfDigitizer;
	wchar_t* m_wcFilePath;
	
private:
	MIL_INT GetTrigLine(INT iTrigIndex);  //获得相机的触发源

	MIL_INT GetUserBit(INT iUserBit);

	wchar_t* GetDCFFile();  //获得相机的dcf文件路径


};

