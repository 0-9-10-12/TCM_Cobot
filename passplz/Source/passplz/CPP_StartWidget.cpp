// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_StartWidget.h"
#include "CPP_Cobot_Controller.h"

bool UCPP_StartWidget::SendLoginIDPW()
{
	//����! ID, Passward->�̰� ������ ������ �α��� ����Ȯ��
	//�������� �����Ÿ� ���ϰ����� �Ѱ��ش�.
	return true;
}

//bool UCPP_StartWidget::CallEventSuccessLogin(bool loginResult)
//{
//	isSuccessLogin = loginResult;
//
//	if(isSuccessLogin)
//		Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetWidget(nullptr); //�α����� �����ϸ� ���� ������ nullptr�� �ٲ۴�.
//
//	FOutputDeviceNull pAR;
//	CallFunctionByNameWithArguments(TEXT("Success_Login"), pAR, nullptr, true);
//}

bool UCPP_StartWidget::SendSingupIDPW()
{
	//����! signupID, signupPassward->�̰� ������ ������ �α��� ����Ȯ��
	//�������� �����Ÿ� ���ϰ����� �Ѱ��ش�.
	return true;
}

//void UCPP_StartWidget::CallEventSuccessSignup(bool signupResult)
//{
//	isSuccessSignup = signupResult; //bool�� �����ϰ� Ŀ�����̺�Ʈ ȣ��
//
//	FOutputDeviceNull pAR;
//	CallFunctionByNameWithArguments(TEXT("Success_Signup"), pAR, nullptr, true);
//}

void UCPP_StartWidget::CreateRoom()
{
	CallEventSuccessAddRoom(roomName, roomMode, 0);
}

void UCPP_StartWidget::CallEventSuccessAddRoom(FString name, int mode, int id)
{
	roomName = name;
	roomMode = mode;
	roomID = id;

	FOutputDeviceNull pAR;
	CallFunctionByNameWithArguments(TEXT("add_Room"), pAR, nullptr, true);

}

