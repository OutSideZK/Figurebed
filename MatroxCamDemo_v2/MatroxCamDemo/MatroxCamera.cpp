#include "stdafx.h"
#include "MatroxCamera.h"
#include "MatroxCamDemoDlg.h"
#include <Thread>

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

CMatroxCamera::CMatroxCamera()
{
	m_mImageWidth = 0;
	m_mImageHeight = 0;
	/*0~3代表输入，4~7代表输出*/
	m_mapTrigSource = { {0, M_AUX_IO24}, {1, M_AUX_IO25}, {2, M_AUX_IO26}, {3, M_AUX_IO27}, {4, M_AUX_IO28}, {5, M_AUX_IO29}, {6, M_AUX_IO30}, {7, M_AUX_IO31} };
	m_mapUserBitSource = { {0, M_USER_BIT24}, {1, M_USER_BIT25}, {2, M_USER_BIT26}, {3, M_USER_BIT27}, {4, M_USER_BIT28}, {5, M_USER_BIT29}, {6, M_USER_BIT30}, {7, M_USER_BIT31} };
	IsTrigger = FALSE;

	UserHookData.Context = this;
}

CMatroxCamera::~CMatroxCamera()
{
}

bool CMatroxCamera::InitCam(int CameraID)
{	
	/*Application指的是自己开发的一个应用程序，一般应用程序同一时刻只存在一个Application对象。
	主要用它来提供一个用于控制和执行MIL应用程序的基本环境*/
	if (M_NULL == MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication))
	{
		return false;
	}

	/*System代表一个包含CPU或GPU、内存或显存和图像控制器的单元分配的一个虚拟访问对象，例如一块采集卡。
	System能够通过加上相机和显示器来采集、保存和显示。
	每个Application下可以包含多个System。*/

	if (0 == CameraID)
	{
		if (M_NULL == MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEV0, M_COMPLETE, &MilSystem))
		{
			return false;
		}
	}
	else if (1 == CameraID)
	{
		if (M_NULL == MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEV1, M_COMPLETE, &MilSystem))
		{
			return false;
		}
	}

	MsysInquire(MilSystem, M_DIGITIZER_NUM, &m_numOfDigitizer);

	//Digitizer is available
	if (m_numOfDigitizer)
	{
		m_wcFilePath = GetDCFFile();
		MdigAlloc(MilSystem, M_DEV0, m_wcFilePath, M_DEFAULT, &MilDigitizer);
	}

	/*Display对应显示器。所有的显示的操作都靠它完成*/
	/*if (M_NULL == MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay))
	{
		return false;
	}*/

	/*Digitizer对应相机。它用于相机的采集和相机属性的调整等，和相机有关的操作都是靠它来完成*/
	/*m_wcFilePath = GetDCFFile();
	if (M_NULL == MdigAlloc(MilSystem, M_DEFAULT, m_wcFilePath, M_DEFAULT, &MilDigitizer))
	{
		return false;
	}*/

	MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
	if (SystemType != M_SYSTEM_RADIENTCXP_TYPE)
	{
		CloseCam();
		return false;
	}

	//获取主窗口的句柄，用于向主窗口发送消息
	m_hWnd = AfxGetMainWnd()->m_hWnd;

	m_bIsOpen = TRUE;
	return true;
}

void CMatroxCamera::CloseCam()
{
	m_bIsOpen = FALSE;
	/*if (M_NULL != MilImage)
	{
		MbufFree(MilImage);
	}*/

	/*if (M_NULL != MilDisplay)
	{
		MdispFree(MilDisplay);
	}*/

	if (M_NULL != MilDigitizer)
	{
		MdigFree(MilDigitizer);
	}

	if (M_NULL != MilSystem)
	{		
		MsysFree(MilSystem);
	}

	if (M_NULL != MilApplication)
	{
		MappFree(MilApplication);
	}
}

void CMatroxCamera::GrabSignalImage(int iheight)
{
	if (!IsOpen())
		return;

	MdigControl(MilDigitizer, M_SOURCE_SIZE_Y, 1024 * iheight);

	/*得到采集图像的高度用于初始化CMvImage*/
	int iImageWidth = MdigInquire(MilDigitizer, M_SOURCE_SIZE_X, M_NULL);
	int iImageHeight = MdigInquire(MilDigitizer, M_SOURCE_SIZE_Y, M_NULL);

	/*申请一块内存用于保存采集到的图片，需要先确定采集宽度在申请内存*/
	LocalBufferAllocDefault(&MilSystem, M_NULL, &MilDigitizer, &MilImage);

	if (MilImage)
	{
		MbufClear(MilImage, 0);
	}

	m_mvSignalImage = CMvImage(iImageWidth, iImageHeight, MV_ITYPE_UC8);

	/*获取一张图片*/
	MdigGrab(MilDigitizer, MilImage);

	/*将得到的图片拷贝到CMvImage中*/
	MbufGet(MilImage, m_mvSignalImage.GetImageData());

	SendMessage(m_hWnd, WM_DISPLAYMESSAGE, (WPARAM)&m_mvSignalImage, NULL);
	MbufFree(MilImage);
	m_mvSignalImage.ClearImage();
}

void CMatroxCamera::GrabImageContinue()
{
	if (!IsOpen())
		return;

	int iTempSize = m_iFramesCount + 2;
	MilGrabBufferList = new MIL_ID[iTempSize];

	/*一次触发采n张图片，CMvImage的高度需要设为采集的图片的n倍*/
	mvTempImage = CMvImage(m_mImageWidth, m_mImageHeight * m_iFramesCount, MV_ITYPE_UC8);
	DesImage = CMvImage(m_mImageWidth, m_mImageHeight * m_iFramesCount, MV_ITYPE_UC8);

	for (MilGrabBufferListSize = 0; MilGrabBufferListSize < iTempSize; MilGrabBufferListSize++)
	{
		MbufAlloc2d(MilSystem,
			MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
			MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
			8 + M_UNSIGNED,
			M_IMAGE + M_GRAB + M_PROC,
			&MilGrabBufferList[MilGrabBufferListSize]);

		if (MilGrabBufferList[MilGrabBufferListSize])
		{
			MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
		}
		else
			break;
	}

	MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

	/* Free buffers to leave space for possible temporary buffers. */
	for (int n = 0; n<2 && MilGrabBufferListSize; n++)
	{
		MilGrabBufferListSize--;
		MbufFree(MilGrabBufferList[MilGrabBufferListSize]);
	}

	if (IsTrigger)
	{
		/*设置为外触发时，指定每次触发采集的帧数，M_FRAME_PER_TRIFFER(INT)用于指定每次触发采集的帧数*/
		MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START + M_FRAMES_PER_TRIGGER(m_iFramesCount), M_ASYNCHRONOUS + M_TRIGGER_FOR_FIRST_GRAB, ProcessingFunction, &UserHookData);
	}
	else
	{
		//连续采集模式时
		MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction, &UserHookData);
	}	
}

void CMatroxCamera::SoftTrigger()
{
	MdigControl(MilDigitizer, M_GRAB_TRIGGER_SOFTWARE, 1);
}

void CMatroxCamera::StopGrabImage()
{
	if (!IsOpen())
		return;

	MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

	/* Free buffers to leave space for possible temporary buffers. */
	for (int n = 0; n<MilGrabBufferListSize; n++)
	{
		MbufFree(MilGrabBufferList[n]);
	}

	delete[] MilGrabBufferList;
	MilGrabBufferList = NULL;

	UserHookData.ProcessedImageCount = 1;
}

void CMatroxCamera::callback(/*HookDataStruct *UserHook*//*MIL_INT Temp*/)
{
	/*将采集到的图片拷贝到CMvImage中，并对图片进行拼接*/
	char *chTemp;
	chTemp = (char *)(mvTempImage).GetImageData() + m_mImageWidth * m_mImageHeight * (UserHookData.ProcessedImageCount-1);

	MbufGet(UserHookData.ModifiedBufferID, chTemp);

	/*每采十张图片将拼接的图片发送出去，并置标志位为1，从新开始拼接*/
	if (m_iFramesCount == UserHookData.ProcessedImageCount)
	{
		DesImage = mvTempImage;
		UserHookData.ProcessedImageCount = 1;
		//SendMessage(m_hWnd, WM_DISPLAYMESSAGE, (WPARAM)&(DesImage), NULL);
		std::thread th(&CMatroxCamera::ImageShow, this, &DesImage);
		th.detach();
	}
	else
	{
		UserHookData.ProcessedImageCount++;
	}
}

void CMatroxCamera::ImageShow(CMvImage *image)
{
	SendMessage(m_hWnd, WM_DISPLAYMESSAGE, (WPARAM)image, NULL);
}

void CMatroxCamera::SetLinePeriod(MIL_DOUBLE LinePeriod)
{
	if (!IsOpen())
		return;

	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LinePeriod"), M_TYPE_DOUBLE, &LinePeriod);
}

void CMatroxCamera::SetExposureTime(MIL_DOUBLE Time)
{
	if (!IsOpen())
		return;

	//再设置相机的曝光时间
	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ExposureTime"), M_TYPE_DOUBLE, &Time);
}

void CMatroxCamera::GetLinePeriod(MIL_DOUBLE & LinePeriod)
{
	if (!IsOpen())
		return;

	MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("LinePeriod"), M_TYPE_DOUBLE, &LinePeriod);
}

void CMatroxCamera::GetExposureTime(MIL_DOUBLE & Time)
{
	if (!IsOpen())
		return;

	MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ExposureTime"), M_TYPE_DOUBLE, &Time);
}

void CMatroxCamera::SetPreampGain(INT Type)
{
	switch (Type)
	{
	case 1:
		MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PreampGain"), M_TYPE_STRING, MIL_TEXT("PreAmpx1"));
		break;
	case 2:
		MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PreampGain"), M_TYPE_STRING, MIL_TEXT("PreAmpx2"));
		break;
	case 4:
		MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PreampGain"), M_TYPE_STRING, MIL_TEXT("PreAmpx4"));
		break;
	default:
		break;
	}
	
}

void CMatroxCamera::GetPreampGain(MIL_INT64 & PreampGain)
{

}

void CMatroxCamera::SetGain(MIL_DOUBLE Gain)
{
	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("Gain"), M_TYPE_DOUBLE, &Gain);
}

void CMatroxCamera::GetGain(MIL_DOUBLE & Gain)
{
	MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("Gain"), M_TYPE_DOUBLE, &Gain);
}

void CMatroxCamera::SetDigitalGain(MIL_DOUBLE DigitalGain)
{
	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DigitalGain"), M_TYPE_DOUBLE, &DigitalGain);
}

void CMatroxCamera::GetDigitalGain(MIL_DOUBLE & DigitalGain)
{
	MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DigitalGain"), M_TYPE_DOUBLE, &DigitalGain);
}

void CMatroxCamera::SaveCameraParam()
{
	if (!IsOpen())
		return;

	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("UserSetSelector"), M_TYPE_STRING, MIL_TEXT("UserSet1"));
	MdigControlFeature(MilDigitizer, M_FEATURE_EXECUTE, MIL_TEXT("UserSetSave"), M_DEFAULT, M_NULL);
}

void CMatroxCamera::SetTriggerMode(int nIndex)
{
	if (!IsOpen())
		return;

	if (IsTrigger)
	{
		MdigControl(MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);
		MdigControl(MilDigitizer, M_GRAB_CONTINUOUS_END_TRIGGER, M_DISABLE);  //指定为trigger触发方式
		//MdigControl(MilDigitizer, M_GRAB_TRIGGER_ACTIVATION, M_LEVEL_HIGH);  //指定为上升沿触发
		

		//MIL_INT TriggerMode = GetTrigLine(nIndex);  //获取当前选中的触发源
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_SOURCE, M_SOFTWARE);  //指定触发源
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_STATE, M_ENABLE);   //使能外触发模式
	}
	else
	{
		MdigControl(MilDigitizer, M_GRAB_CONTINUOUS_END_TRIGGER, M_ENABLE);  //指定为trigger触发方式
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_STATE, M_DISABLE);   //使能外触发模式
	}

}

void CMatroxCamera::SetFrameCount(INT FrameCount)
{
	if (!FrameCount)
		FrameCount = 10;

	m_iFramesCount = FrameCount;
}

void CMatroxCamera::SetParam()
{
	if (!IsOpen())
		return;

	MdigControl(MilDigitizer, M_SOURCE_SIZE_Y, 1024);

	/*得到采集图像的高度用于初始化CMvImage*/
	m_mImageWidth = MdigInquire(MilDigitizer, M_SOURCE_SIZE_X, M_NULL);
	m_mImageHeight = MdigInquire(MilDigitizer, M_SOURCE_SIZE_Y, M_NULL);
}

void CMatroxCamera::SpecifyIOState()
{
	if (!IsOpen())
		return;

	//设置IO28为输出
	MdigControl(MilDigitizer, M_AUX_IO28 + M_IO_SOURCE, M_USER_BIT28);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO28, M_OUTPUT);

	//设置IO29为输出
	MdigControl(MilDigitizer, M_AUX_IO29 + M_IO_SOURCE, M_USER_BIT29);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO29, M_OUTPUT);

	//设置IO30为输出
	MdigControl(MilDigitizer, M_AUX_IO30 + M_IO_SOURCE, M_USER_BIT30);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO30, M_OUTPUT);

	//设置IO31为输出
	MdigControl(MilDigitizer, M_AUX_IO31 + M_IO_SOURCE, M_USER_BIT31);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO31, M_OUTPUT);
}


void CMatroxCamera::SetOutputState(const INT iOutput, MIL_INT iState)
{
	MIL_INT UserBit;
	UserBit = GetUserBit(iOutput);
	
	MdigControl(MilDigitizer, UserBit + M_USER_BIT_STATE, iState);
}

double CMatroxCamera::GetCameraTemperature()
{
	MIL_DOUBLE mdTemp = 0;

	MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceTemperatureSelector"), M_TYPE_STRING, MIL_TEXT("Mainboard"));
	MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceTemperature"), M_TYPE_DOUBLE, &mdTemp);

	return mdTemp;
}

MIL_INT CMatroxCamera::GetIOState(const INT iInput)
{
	MIL_INT TrigLine;
	MIL_INT IOStatus;
	TrigLine = GetTrigLine(iInput);
	
	MdigInquire(MilDigitizer, TrigLine + M_IO_STATUS, &IOStatus);
	
	return IOStatus;
}

BOOL CMatroxCamera::GetImage(BOOL Flag, CMvImage & mvImage)
{
	if (Flag)
	{
		mvImage = DesImage;
	}
	else
	{
		mvImage = m_mvSignalImage;
	}
	
	return TRUE;
}

MIL_INT CMatroxCamera::GetTrigLine(INT iTrigIndex)
{
	MIL_INT milTemp;
	std::map<int, MIL_INT>::iterator iter = m_mapTrigSource.find(iTrigIndex);

	if (iter == m_mapTrigSource.end())
		milTemp = M_AUX_IO25;
	else
		milTemp = iter->second;

	return milTemp;
}

MIL_INT CMatroxCamera::GetUserBit(INT iUserBit)
{
	MIL_INT milTemp;

	std::map<int, MIL_INT>::iterator iter = m_mapUserBitSource.find(iUserBit);

	if (iter == m_mapUserBitSource.end())
		milTemp = M_USER_BIT25;
	else
		milTemp = iter->second;

	return milTemp;
}

wchar_t* CMatroxCamera::GetDCFFile()
{
	wchar_t *FilePath;

	CString csTempName;
	csTempName = CMvBasic::GetAppPath();

	if (csTempName.IsEmpty())
		return FALSE;

	csTempName += _T("DCF1.dcf");
	//csTempName += _T("MIL_16KCL_M4T8.dcf");

	char *cname = "\0";
	USES_CONVERSION;
	cname = W2A(csTempName);

	int size;
	//wchar_t* wcname;
	size = MultiByteToWideChar(CP_ACP, 0, cname, -1, NULL, 0);
	FilePath = (wchar_t *)malloc(size * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, cname, -1, FilePath, size);

	return FilePath;
}

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void * HookDataPtr)
{
	HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;

	/*获得正在处理的内存*/
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &UserHookDataPtr->ModifiedBufferID);

	CMatroxCamera* pCam = (CMatroxCamera*)UserHookDataPtr->Context;
	pCam->callback();

	return 0;
}
