// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_StartWidget.h"
#include "CPP_Cobot_Controller.h"

void UCPP_StartWidget::SendLoginIDPW()
{

	Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetWidget(Cast<UUserWidget>(this));
	//Send�ʿ� id�� passward
	CallEventSuccessLogin(true);
}

void UCPP_StartWidget::CallEventSuccessLogin(bool loginResult)
{
	isSuccessLogin = loginResult;

	if(isSuccessLogin)
		Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetWidget(nullptr); //�α����� �����ϸ� ���� ������ nullptr�� �ٲ۴�.

	FOutputDeviceNull pAR;
	CallFunctionByNameWithArguments(TEXT("Success_Login"), pAR, nullptr, true);
}

void UCPP_StartWidget::SendSingupIDPW()
{
	Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetWidget(Cast<UUserWidget>(this));
		//ignupID, signupPassward �� �ΰ� ������ ������ �ִ� ���̵� ���� Ȯ���ϴ� �۾� �ʿ� ���ٸ� ���� ����
	//�ڽ��� �����͵� ���� �Ѱ�����
	// send���ְ� CallEventSuccessSignup ���⿡ ���� ���� bool������ �����ֱ�
	CallEventSuccessSignup(true);
}

void UCPP_StartWidget::CallEventSuccessSignup(bool signupResult)
{
	isSuccessSignup = signupResult; //bool�� �����ϰ� Ŀ�����̺�Ʈ ȣ��

	FOutputDeviceNull pAR;
	CallFunctionByNameWithArguments(TEXT("Success_Signup"), pAR, nullptr, true);
}

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

