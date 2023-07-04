// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_StartWidget.h"
#include "CPP_Cobot_Controller.h"

bool UCPP_StartWidget::Is_Set_IDPW_Controller()
{

	return Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->Is_Set_IDPW(ID, Passward);
	/*����! ��Ʈ�ѷ����� ������ send�� ������ ��Ʈ�ѷ����� bool���� �����ְ� ��Ʈ�ѷ��� �� �Լ��� bool���� �����*/
}

void UCPP_StartWidget::Send_Singup_IDPW()
{
		//ignupID, signupPassward �� �ΰ� ������ ������ �ִ� ���̵� ���� Ȯ���ϴ� �۾� �ʿ� ���ٸ� ���� ����
	//�ڽ��� �����͵� ���� �Ѱ�����
	CallEventSuccess_Signup(true);
}

void UCPP_StartWidget::CallEventSuccess_Signup(bool signupResult)
{
	isSuccessSignup = signupResult;

	FOutputDeviceNull pAR;
	CallFunctionByNameWithArguments(TEXT("Success_Signup"), pAR, nullptr, true);
}

