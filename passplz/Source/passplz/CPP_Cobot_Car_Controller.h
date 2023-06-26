// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CPP_CobotGameInstance.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"

#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "passplz.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CPP_Cobot_Car_Controller.generated.h"

/**
 *
 */
UCLASS()
class PASSPLZ_API ACPP_Cobot_Car_Controller : public APlayerController
{
	GENERATED_BODY()
	ACPP_Cobot_Car_Controller();
	~ACPP_Cobot_Car_Controller();


protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void SetupInputComponent() override;

private:
	UCPP_CobotGameInstance* instance;

	SOCKET* sock;

	// ��Ŷ �������� ���� �κ� ���߿� ������ ����
	int		prev_remain;
	int		prev_packet_size;
	char	prev_packet_buff[10000];

	void RecvPacket();
	void ProcessPacket(char* packet);

private:
	class ACPP_Cobot_Car* player;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputMappingContext* DefaultContext;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* Move1;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* Move2;

public:

	//a�� d�� ����
	void CarInput(const FInputActionValue& Value);
	//���� : ������ ����� �Լ� �� �Լ� ȣ���ϸ� ��
	void CarForward();
	//���� : ȸ�� �϶�� �Լ� �� �Լ� ȣ���ϸ� ��
	void CarRotation(float rotationValue);
};
