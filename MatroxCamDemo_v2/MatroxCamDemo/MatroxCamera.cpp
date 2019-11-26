#include "stdafx.h"
#include "MatroxCamera.h"
#include "MatroxCamDemoDlg.h"
#include <Thread>

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

CMatroxCamera::CMatroxCamera()
{
	m_mImageWidth = 0;
	m_mImageHeight = 0;
	/*0~3�������룬4~7�������*/
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
	/*Applicationָ�����Լ�������һ��Ӧ�ó���һ��Ӧ�ó���ͬһʱ��ֻ����һ��Application����
	��Ҫ�������ṩһ�����ڿ��ƺ�ִ��MILӦ�ó���Ļ�������*/
	if (M_NULL == MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication))
	{
		return false;
	}

	/*System����һ������CPU��GPU���ڴ���Դ��ͼ��������ĵ�Ԫ�����һ��������ʶ�������һ��ɼ�����
	System�ܹ�ͨ�������������ʾ�����ɼ����������ʾ��
	ÿ��Application�¿��԰������System��*/

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

	/*Display��Ӧ��ʾ�������е���ʾ�Ĳ������������*/
	/*if (M_NULL == MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay))
	{
		return false;
	}*/

	/*Digitizer��Ӧ���������������Ĳɼ���������Եĵ����ȣ�������йصĲ������ǿ��������*/
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

	//��ȡ�����ڵľ���������������ڷ�����Ϣ
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

	/*�õ��ɼ�ͼ��ĸ߶����ڳ�ʼ��CMvImage*/
	int iImageWidth = MdigInquire(MilDigitizer, M_SOURCE_SIZE_X, M_NULL);
	int iImageHeight = MdigInquire(MilDigitizer, M_SOURCE_SIZE_Y, M_NULL);

	/*����һ���ڴ����ڱ���ɼ�����ͼƬ����Ҫ��ȷ���ɼ�����������ڴ�*/
	LocalBufferAllocDefault(&MilSystem, M_NULL, &MilDigitizer, &MilImage);

	if (MilImage)
	{
		MbufClear(MilImage, 0);
	}

	m_mvSignalImage = CMvImage(iImageWidth, iImageHeight, MV_ITYPE_UC8);

	/*��ȡһ��ͼƬ*/
	MdigGrab(MilDigitizer, MilImage);

	/*���õ���ͼƬ������CMvImage��*/
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

	/*һ�δ�����n��ͼƬ��CMvImage�ĸ߶���Ҫ��Ϊ�ɼ���ͼƬ��n��*/
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
		/*����Ϊ�ⴥ��ʱ��ָ��ÿ�δ����ɼ���֡����M_FRAME_PER_TRIFFER(INT)����ָ��ÿ�δ����ɼ���֡��*/
		MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START + M_FRAMES_PER_TRIGGER(m_iFramesCount), M_ASYNCHRONOUS + M_TRIGGER_FOR_FIRST_GRAB, ProcessingFunction, &UserHookData);
	}
	else
	{
		//�����ɼ�ģʽʱ
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
	/*���ɼ�����ͼƬ������CMvImage�У�����ͼƬ����ƴ��*/
	char *chTemp;
	chTemp = (char *)(mvTempImage).GetImageData() + m_mImageWidth * m_mImageHeight * (UserHookData.ProcessedImageCount-1);

	MbufGet(UserHookData.ModifiedBufferID, chTemp);

	/*ÿ��ʮ��ͼƬ��ƴ�ӵ�ͼƬ���ͳ�ȥ�����ñ�־λΪ1�����¿�ʼƴ��*/
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

	//������������ع�ʱ��
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
		MdigControl(MilDigitizer, M_GRAB_CONTINUOUS_END_TRIGGER, M_DISABLE);  //ָ��Ϊtrigger������ʽ
		//MdigControl(MilDigitizer, M_GRAB_TRIGGER_ACTIVATION, M_LEVEL_HIGH);  //ָ��Ϊ�����ش���
		

		//MIL_INT TriggerMode = GetTrigLine(nIndex);  //��ȡ��ǰѡ�еĴ���Դ
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_SOURCE, M_SOFTWARE);  //ָ������Դ
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_STATE, M_ENABLE);   //ʹ���ⴥ��ģʽ
	}
	else
	{
		MdigControl(MilDigitizer, M_GRAB_CONTINUOUS_END_TRIGGER, M_ENABLE);  //ָ��Ϊtrigger������ʽ
		MdigControl(MilDigitizer, M_GRAB_TRIGGER_STATE, M_DISABLE);   //ʹ���ⴥ��ģʽ
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

	/*�õ��ɼ�ͼ��ĸ߶����ڳ�ʼ��CMvImage*/
	m_mImageWidth = MdigInquire(MilDigitizer, M_SOURCE_SIZE_X, M_NULL);
	m_mImageHeight = MdigInquire(MilDigitizer, M_SOURCE_SIZE_Y, M_NULL);
}

void CMatroxCamera::SpecifyIOState()
{
	if (!IsOpen())
		return;

	//����IO28Ϊ���
	MdigControl(MilDigitizer, M_AUX_IO28 + M_IO_SOURCE, M_USER_BIT28);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO28, M_OUTPUT);

	//����IO29Ϊ���
	MdigControl(MilDigitizer, M_AUX_IO29 + M_IO_SOURCE, M_USER_BIT29);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO29, M_OUTPUT);

	//����IO30Ϊ���
	MdigControl(MilDigitizer, M_AUX_IO30 + M_IO_SOURCE, M_USER_BIT30);
	MdigControl(MilDigitizer, M_IO_MODE + M_AUX_IO30, M_OUTPUT);

	//����IO31Ϊ���
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

	/*������ڴ�����ڴ�*/
	MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &UserHookDataPtr->ModifiedBufferID);

	CMatroxCamera* pCam = (CMatroxCamera*)UserHookDataPtr->Context;
	pCam->callback();

	return 0;
}
