// Fill out your copyright notice in the Description page of Project Settings.

//���� ��Ʈ�ѷ����� �ߴ��� ��Ŷ ó�� �ʿ�

#define _CRT_SECURE_NO_WARNINGS

#include "CPP_Cobot_Car_Controller.h"
#include "CPP_Cobot_Car.h"
//#include "Engine/World.h"
#include "CPP_Cannon.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "../../../server/server/protocol.h"

ACPP_Cobot_Car_Controller::ACPP_Cobot_Car_Controller()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>DEFAULT_CONTEXT
	(TEXT("/Game/K_Test/input/stage3IMCTest.stage3IMCTest"));
	if (DEFAULT_CONTEXT.Succeeded())
		DefaultContext = DEFAULT_CONTEXT.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_MOVE
	(TEXT("/Game/K_Test/input/Move.Move"));
	if (IA_MOVE.Succeeded())
		Move = IA_MOVE.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_MOUSEWHEEL
	(TEXT("/Game/K_Test/input/MouseWheel.MouseWheel"));
	if (IA_MOUSEWHEEL.Succeeded())
		MouseWheel = IA_MOUSEWHEEL.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_MOUSELEFT
	(TEXT("/Game/K_Test/input/MouseLeft.MouseLeft"));
	if (IA_MOUSELEFT.Succeeded())
		MouseLeft = IA_MOUSELEFT.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_ROTATE
	(TEXT("/Game/K_Test/input/rotate.rotate"));
	if (IA_ROTATE.Succeeded())
		Rotate = IA_ROTATE.Object;
}

ACPP_Cobot_Car_Controller::~ACPP_Cobot_Car_Controller()
{
}

void ACPP_Cobot_Car_Controller::BeginPlay()
{
	Super::BeginPlay();

	instance = Cast<UCPP_CobotGameInstance>(GetWorld()->GetGameInstance());
	sock = instance->GetSocketMgr()->GetSocket();

	cs_stage3_enter_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_stage3_enter);

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("car controller enter!"));

	if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		SubSystem->AddMappingContext(DefaultContext, 0);

	player = Cast<ACPP_Cobot_Car>(GetPawn());
	if (!player)
		return;

	cannon = UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Cannon::StaticClass());
	if(cannon)
		UE_LOG(LogTemp, Warning, TEXT("cannon OK"));

}

void ACPP_Cobot_Car_Controller::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACPP_Cobot_Car_Controller::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	UE_LOG(LogTemp, Warning, TEXT("OnPossess"));
}

void ACPP_Cobot_Car_Controller::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		//�ڵ��� �����̱�
		EnhancedInputComponent->BindAction(Move, ETriggerEvent::Triggered, this, &ACPP_Cobot_Car_Controller::CarInput);
		//������ ����
		EnhancedInputComponent->BindAction(MouseWheel, ETriggerEvent::Triggered, this, &ACPP_Cobot_Car_Controller::CannonInput);
		//�߻�
		EnhancedInputComponent->BindAction(MouseLeft, ETriggerEvent::Triggered, this, &ACPP_Cobot_Car_Controller::FireCannonInput);
		//���� ��ȭ
		EnhancedInputComponent->BindAction(Rotate, ETriggerEvent::Triggered, this, &ACPP_Cobot_Car_Controller::RotateInput);
	}
}

void ACPP_Cobot_Car_Controller::RecvPacket()
{
	char buff[BUF_SIZE];

	int ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);
	if (ret <= 0)
	{
		GetLastError();
		std::cout << "recv() fail!" << std::endl;
		return;
	}

	if (prev_remain > 0) // ���� ���� �����ִ� �����Ͱ� �ִٸ�
	{
		strcat(prev_packet_buff, buff);
	} else
	{
		memcpy(prev_packet_buff, buff, ret);
	}

	int remain_data = ret + prev_remain;
	char* p = prev_packet_buff;

	while (remain_data > 0)
	{
		int packet_size = p[0];
		if (packet_size <= remain_data)
		{
			ProcessPacket(p);
			p = p + packet_size;
			remain_data -= packet_size;
		} else break;
	}

	prev_remain = remain_data;

	if (remain_data > 0)
	{
		memcpy(prev_packet_buff, p, remain_data);
	}
}

void ACPP_Cobot_Car_Controller::ProcessPacket(char* packet)
{
    switch (packet[1])
    {
	case static_cast<int>(packet_type::sc_car_direction):
	{
		//UE_LOG(LogTemp, Warning, TEXT("recv sc_car_direction"));

		sc_car_direction_packet* pack = reinterpret_cast<sc_car_direction_packet*>(packet);
		// �ϴ��� 0�� ����
		// �´� 5ms���� -0.1
		// ��� 5ms���� +0.1

		// Ŭ��: ���� �ڵ��� ������ �Լ� ȣ��
		//CarForward();
		UE_LOG(LogTemp, Warning, TEXT("direction: %lf"), pack->direction);

		// �ٵ� �̰� �ʹ� �Ҷ� ���� �ε巴�� �ȵǳ�?
		if (0.0 == pack->direction)
			//CarForward(�������� �޴� ���� �ʿ�);
			;
		else 
			CarRotation(pack->direction);

	} break;
    }
}

void ACPP_Cobot_Car_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RecvPacket();
}

void ACPP_Cobot_Car_Controller::CarInput(const FInputActionValue& Value)
{
	//���� : ���⼭ Ű�� �����ų� ���� ���� �����µ� �̶� ������ ��Ŷ ��������
	//�������� �� Ŭ�� Ű�� ���� bool���� ������.
	//�������� ���Ӱ� ���õ� ����� �ʿ��մϴ�. -> ���� Ŀ���� �� �׸��� �������� ���� �۾����� �� �ʿ� �� ���� �Ѱ��ּ���
	UE_LOG(LogTemp, Warning, TEXT("%f"), Value.Get<float>());

	cs_car_direction_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_car_direction);

	if (0.0 != Value.Get<float>())
		pack.direction = true;
	else
		pack.direction = false;

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
	if (mode == 0) {
		UE_LOG(LogTemp, Warning, TEXT("CarInput %f"), Value.Get<float>());
	}
}


void ACPP_Cobot_Car_Controller::CannonInput(const FInputActionValue& Value)
{
	if (mode == 1) {
		UE_LOG(LogTemp, Warning, TEXT("CannonInput %f"), Value.Get<float>());
	}

	//���� : 1�� Ŭ��� �������� �յڷ�, 2��Ŭ��� �翷���� �����̱� �ؾ���
	//������ ��Ŷ ������ Value.Get<float>() �� �� ���������� �� ���� ���콺 �� ������ ���⿡ ���� ����, ��� ���ε� �̰ɷ� ������ ��ġ ����ؼ� �����ּ�
	//������ �̸� ���� ������ ȸ������ ������ FRotator�� �ʿ� yaw�� 1��Ŭ��, pitch�� 2��Ŭ�� ������ ���� ���ؼ� ���� ������ ȸ�� ���� �ٽ� Ŭ�󿡰� ���������
	//cpp_cannon�� �ִ� SetBombDropLocation�Լ� ȣ���ϸ� ��

	//�������� �̷������� �Ի��ؼ� �ؿ� �ڵ� ��򰡿� �߰��ؼ� ȣ���ϸ� ��
	//FRotator rota;
	//rota.Roll = �������� �Ѱ��ذ�;
	//rota.Yaw = �������� �Ѱ��ذ�;
	//rota.Pitch = �������� �Ѱ��ذ�;
	//Cast<ACPP_Cannon>(cannon)->SetBombDropLocation(rota);
	// �̸� ���� Rotator ����ü �ϳ� �����ؼ� �ϴ°�...
	// struct Rotator{
	// float Roll
	// ...
	// ...
	// } �̰� //�ڵ� ¥�� ���� �ʿ���� �ּ� ������ �����ּ���
}

void ACPP_Cobot_Car_Controller::FireCannonInput(const FInputActionValue& Value)
{
	if (mode == 1) {
		UE_LOG(LogTemp, Warning, TEXT("FireCannonInput %s"), Value.Get<bool>() ? TEXT("true") : TEXT("false"));
		Cast<ACPP_Cannon>(cannon)->FireLava();
	}
}

void ACPP_Cobot_Car_Controller::RotateInput(const FInputActionValue& Value)
{
	//��Ʈ�ѷ��� �ƴ� �÷��̾ �ִ� ���������� ȸ���ؾ���
	//��Ʈ�ѷ� ȸ���� �������� �����Ѵ�.
	player->SpringArm->AddRelativeRotation(FRotator(Value.Get<FVector2D>().Y, Value.Get<FVector2D>().X, 0.0f));
}

void ACPP_Cobot_Car_Controller::CarForward(float acceleration)
{
	UE_LOG(LogTemp, Warning, TEXT("CarForward"));
	player->AddActorWorldOffset(player->GetActorForwardVector()* acceleration);
}

void ACPP_Cobot_Car_Controller::CarRotation(float rotationValue)
{
	FRotator control_rotation = GetControlRotation();
	control_rotation.Yaw += rotationValue;
	SetControlRotation(control_rotation);
}

void ACPP_Cobot_Car_Controller::ChangeMode(int Mode)
{
	UE_LOG(LogTemp, Warning, TEXT("ChangeMode"));

	switch (Mode)
	{
		//�Ϲ� ���
		//ī�޶� ���� �����͵� �ʿ���
	case 0:
		mode = 0;
		break;
		//���� ���
	case 1:
		mode = 1;
		break;

	default:
		break;
	}
}





