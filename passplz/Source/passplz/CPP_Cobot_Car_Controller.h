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

	TArray<AActor*> cannon_actor;

	// ��Ŷ �������� ���� �κ� ���߿� ������ ����
	int		prev_remain;
	int		prev_packet_size;
	char	prev_packet_buff[10000];

	int		player_number;

	void RecvPacket();
	void ProcessPacket(char* packet);

private:
	virtual void Tick(float DeltaTime) override;

	class ACPP_Cobot_Car* player;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputMappingContext* DefaultContext;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* Move;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* MouseWheel;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* MouseLeft;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* Rotate;

	AActor* cannon;

	int mode;

public:

	//a�� d�� ����
	void CarInput(const FInputActionValue& Value);

	//1Ŭ��� ������ ��ǥ������ �յڷ�, 2Ŭ��� ��ǥ������ �翷���� �����Ѵ�.(�������� ���)
	//������ ���� ������ �Ǵ��ؼ� ���� ��ǥ������ �����ش�.
	void CannonInput(const FInputActionValue& Value);

	//Ŭ����ư �߻��ϰڴٴ� ��ȣ�� ������ ��
	//���� Ŭ�� ��� Ŭ����ư�� �����ٸ�(bool������ �����Ѵ�) �߻��϶�� ��Ŷ�� ���������
	//�� (0,0,0)��ġ���� 1��Ŭ�� Ŭ���� �ߴµ�, 1��Ŭ�� 2��Ŭ�� ���콺���� �������� ���� ��ġ�� �ٲ۴ٸ� bool���� �Ѵ� false�� �ٲ۴�.
	void FireCannonInput(const FInputActionValue& Value);

	void RotateInput(const FInputActionValue& Value);

	//���� : ������ ����� �Լ� �� �Լ� ȣ���ϸ� ��
	void CarForward(float acceleration);
	//���� : ȸ�� �϶�� �Լ� �� �Լ� ȣ���ϸ� ��
	void CarRotation(float rotationValue);
	
	void ChangeMode(int Mode);


};
