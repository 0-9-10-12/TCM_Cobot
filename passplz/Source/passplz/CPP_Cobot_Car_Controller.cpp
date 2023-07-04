// Fill out your copyright notice in the Description page of Project Settings.

//���� ��Ʈ�ѷ����� �ߴ��� ��Ŷ ó�� �ʿ�

#define _CRT_SECURE_NO_WARNINGS

#include "CPP_Cobot_Car_Controller.h"
#include "CPP_Cobot_Car.h"
//#include "Engine/World.h"
#include "CPP_Stage3Cobot.h"
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
	char recv_buff[BUF_SIZE];

	int recv_ret = recv(*sock, reinterpret_cast<char*>(&recv_buff), BUF_SIZE, 0);
	if (recv_ret <= 0)
	{
		//GetLastError();
		//std::cout << "recv() fail!" << std::endl;
		return;
	}

	int ring_ret = ring_buff.enqueue(recv_buff, recv_ret);
	if (static_cast<int>(error::full_buffer) == ring_ret) {
		//std::cout << "err: ring buffer is full\n";
		return;
	} else if (static_cast<int>(error::in_data_is_too_big) == ring_ret) {
		//std::cout << "err: in data is too big\n";
		return;
	}

	int buffer_start = 0;
	while (ring_buff.remain_data() > 0)
	{
		char pack_size = recv_buff[buffer_start];
		if (pack_size <= ring_buff.remain_data()) {
			char dequeue_data[BUFFER_SIZE];

			ring_ret = ring_buff.dequeue(reinterpret_cast<char*>(&dequeue_data), pack_size);
			if (static_cast<int>(error::no_data_in_buffer) == ring_ret)
				break;
			else if (static_cast<int>(error::out_data_is_too_big) == ring_ret)
				break;

			ProcessPacket(dequeue_data);

			buffer_start += pack_size;
		} else break;
	}
}

void ACPP_Cobot_Car_Controller::ProcessPacket(char* packet)
{
	switch (packet[1])
	{
	case static_cast<int>(packet_type::sc_stage3_enter):
	{
		sc_stage3_enter_packet* pack = reinterpret_cast<sc_stage3_enter_packet*>(packet);
		player_number = pack->player_number;
	} break;
	case static_cast<int>(packet_type::sc_car_direction):
	{
		//UE_LOG(LogTemp, Warning, TEXT("recv sc_car_direction"));

		sc_car_direction_packet* pack = reinterpret_cast<sc_car_direction_packet*>(packet);

		UE_LOG(LogTemp, Warning, TEXT("direction: %lf"), pack->direction);

		if (0.0 == pack->direction) {

			//CarForward(�������� �޴� ���� �ʿ�);
			player->ChangAim(true, true);
			;
		}
		else {
			if (pack->direction > 0.0) {

				player->ChangAim(false, true);
			}
		
			else
				player->ChangAim(true, false);

			CarRotation(pack->direction);
		}

	} break;
	case static_cast<int>(packet_type::sc_cannon_yaw):
	{
		sc_cannon_yaw_packet* pack = reinterpret_cast<sc_cannon_yaw_packet*>(packet);

		Cast<ACPP_Cannon>(cannon)->SetBombDropLocation(1, pack->yaw);
	} break;
	case static_cast<int>(packet_type::sc_cannon_pitch):
	{
		sc_cannon_pitch_packet* pack = reinterpret_cast<sc_cannon_pitch_packet*>(packet);

		Cast<ACPP_Cannon>(cannon)->SetBombDropLocation(2, pack->pitch);
	} break;
	case static_cast<int>(packet_type::sc_cannon_click):
	{
		sc_cannon_click_packet* pack = reinterpret_cast<sc_cannon_click_packet*>(packet);

		if (player_number == pack->click_id) {
			// �ڱⰡ ������
			Cast<ACPP_Cannon>(cannon)->FireLava();
		}
		else {
			// ������ ������
			Cast<ACPP_Cannon>(cannon)->FireLava();
		}
	} break;
	case static_cast<int>(packet_type::sc_cannon_fire):
	{
		// ���� �߻��ϴ� �Լ� ȣ��
		Cast<ACPP_Cannon>(cannon)->FireLava();
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

	cs_cannon_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_cannon);
	pack.cannon_value = Value.Get<float>();

	send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
}

void ACPP_Cobot_Car_Controller::FireCannonInput(const FInputActionValue& Value)
{
	if (mode == 1) {
		UE_LOG(LogTemp, Warning, TEXT("FireCannonInput %s"), Value.Get<bool>() ? TEXT("true") : TEXT("false"));
		Cast<ACPP_Cannon>(cannon)->FireLava();
	}

	if (1.f == Value.Get<float>()) {
		cs_cannon_click_packet pack;
		pack.size = sizeof(pack);
		pack.type = static_cast<char>(packet_type::cs_cannon_click);

		send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("FireCannonInput"));
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
	player->AddActorWorldOffset(player->GetActorForwardVector() * acceleration);
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





