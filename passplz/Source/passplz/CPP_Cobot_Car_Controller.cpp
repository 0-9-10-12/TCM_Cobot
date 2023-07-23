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

	fireNum = 0;
	isFire = false;
	UIMode = false;
	carMove = false;
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
		//GetLastError();
		//UE_LOG(LogTemp, Warning, TEXT("recv() fail"));
		//std::cout << "recv() fail!" << std::endl;
		return;
	}

	if (prev_remain > 0) // ���� ���� �����ִ� �����Ͱ� �ִٸ�
	{
		strcat(prev_packet_buff, buff);
	} else {
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

			CarForward(1);
			player->ChangAim(true, true);
		}
		else {
			if (pack->direction > 0.0) {
				CarForward(50.f);

				player->ChangAim(false, true);
			}
		
			else {

				player->ChangAim(true, false);
			CarRotation(pack->direction * 5.f);
			}

		}

		//if (0.0 == pack->direction) {

		//	CarForward(pack->acceleration);
		//	player->ChangAim(true, true);
		//}
		//else {
		//	if (pack->direction > 0.0) {

		//		player->ChangAim(false, true);
		//	}

		//	else {

		//		player->ChangAim(true, false);
		//	}

		//	CarRotation(pack->direction);
		//}

	} break;
	case static_cast<int>(packet_type::sc_car_location):
	{
		sc_car_location_packet* pack = reinterpret_cast<sc_car_location_packet*>(packet);

		FVector newLocation;
		newLocation.X = pack->car_location.x;
		newLocation.Y = pack->car_location.y;
		newLocation.Z = pack->car_location.z;

		SetPlayerLocation(newLocation);
	} break;
	case static_cast<int>(packet_type::sc_car_rotation_yaw):
	{
		sc_car_rotation_yaw_packet* pack = reinterpret_cast<sc_car_rotation_yaw_packet*>(packet);

		SetPlayerYaw(pack->yaw);
	} break;
	case static_cast<int>(packet_type::sc_cannon_yaw):
	{
		sc_cannon_yaw_packet* pack = reinterpret_cast<sc_cannon_yaw_packet*>(packet);

		Cast<ACPP_Cannon>(cannon)->SetBombDropLocation(1, pack->yaw);
		isFire = true;
		FOutputDeviceNull pAR;
		Cast<ACPP_Cannon>(cannon)->CallFunctionByNameWithArguments(TEXT("Cannon_UnCheck"), pAR, nullptr, true);

	} break;
	case static_cast<int>(packet_type::sc_cannon_pitch):
	{
		sc_cannon_pitch_packet* pack = reinterpret_cast<sc_cannon_pitch_packet*>(packet);

		Cast<ACPP_Cannon>(cannon)->SetBombDropLocation(2, pack->pitch);
		isFire = true;
		FOutputDeviceNull pAR;
		Cast<ACPP_Cannon>(cannon)->CallFunctionByNameWithArguments(TEXT("Cannon_UnCheck"), pAR, nullptr, true);

	} break;
	case static_cast<int>(packet_type::sc_cannon_click):
	{
		sc_cannon_click_packet* pack = reinterpret_cast<sc_cannon_click_packet*>(packet);

		if (player_number == pack->click_id) {
			// �ڱⰡ ������
			// ���⼭�� �ƹ� �ൿ�� ���ص� �ǰ� �ƴϸ�
			// �������� �߻� ��û�� ���´ٴ� �޽����� �����ص� ��
			
		}
		else {
			// ������ ������
			// ����� �������κ��� �߻��û�� �Դٴ� �޽����� ��������� ��
			FOutputDeviceNull pAR;
			Cast<ACPP_Cannon>(cannon)->CallFunctionByNameWithArguments(TEXT("Cannon_Check"), pAR, nullptr, true);
		}
	} break;
	case static_cast<int>(packet_type::sc_cannon_fire):
	{
		FOutputDeviceNull pAR;
		Cast<ACPP_Cannon>(cannon)->CallFunctionByNameWithArguments(TEXT("Cannon_Check"), pAR, nullptr, true);
		// ���� �߻��ϴ� �Լ� ȣ��
		Cast<ACPP_Cannon>(cannon)->FireLava();
	} break;
	case static_cast<int>(packet_type::sc_esc):
	{
		sc_esc_packet* pack = reinterpret_cast<sc_esc_packet*>(packet);
		pack->stage; // ���⿡ ������ ��ư�� ���� �ش� ���������� �� ����.
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
	if (UIMode) {
		UE_LOG(LogTemp, Warning, TEXT("UIMODE"));
		return;
	}

	
	UE_LOG(LogTemp, Warning, TEXT("%f"), Value.Get<float>());

	cs_car_direction_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_car_direction);

	if (0.0 != Value.Get<float>()) {
		if (carMove)
			return;
		carMove = true;
		pack.direction = true;
	}
	else {
		carMove = false;
		pack.direction = false;
	}

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
	UE_LOG(LogTemp, Warning, TEXT("send ret: %d"), ret);

	if (mode == 0) {
		UE_LOG(LogTemp, Warning, TEXT("CarInput %f"), Value.Get<float>());
	}
}


void ACPP_Cobot_Car_Controller::CannonInput(const FInputActionValue& Value)
{
	if (mode == 0) {
		UE_LOG(LogTemp, Warning, TEXT("carMode"));
		return;
	}

	cs_cannon_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_cannon);
	pack.cannon_value = Value.Get<float>();

	send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
}

void ACPP_Cobot_Car_Controller::FireCannonInput(const FInputActionValue& Value)
{
	//if (mode == 1) {
	//	UE_LOG(LogTemp, Warning, TEXT("FireCannonInput %s"), Value.Get<bool>() ? TEXT("true") : TEXT("false"));
	//	Cast<ACPP_Cannon>(cannon)->FireLava();
	//}
	
	if (0.5f < Value.Get<float>() && fireNum != 0 && isFire) {
		isFire = false;
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

	/*AddYawInput(Value.Get<FVector2D>().Y);
	AddPitchInput(Value.Get<FVector2D>().X);*/

	player->SpringArm->AddRelativeRotation(FRotator(Value.Get<FVector2D>().Y, Value.Get<FVector2D>().X, 0.0f), true);
}

void ACPP_Cobot_Car_Controller::CarForward(float acceleration)
{
	UE_LOG(LogTemp, Warning, TEXT("CarForward"));
	UE_LOG(LogTemp, Warning, TEXT("acc: %f"), acceleration);
	FVector preLocation = player->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("preLoc: %f %f"), preLocation.X, preLocation.Y);

	FHitResult HitResult;
	player->AddActorWorldOffset(player->GetActorForwardVector() * acceleration, true, &HitResult);

	if (HitResult.IsValidBlockingHit())
	{
		FVector Normal = HitResult.Normal;
		FVector SlideDirection = FVector::VectorPlaneProject(player->GetActorForwardVector(), Normal).GetSafeNormal();
		FVector SlideMove = SlideDirection * (1.f - HitResult.Time) * 5.f;
		player->AddActorWorldOffset(SlideMove, true);
	}

	FVector newLocation = player->GetActorLocation();
	//���⼭ newLocatoin�� send
	UE_LOG(LogTemp, Warning, TEXT("newLocation: %f %f"), newLocation.X, newLocation.Y);

	cs_car_location_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_car_location);
	pack.car_location.x = newLocation.X;
	pack.car_location.y = newLocation.Y;
	pack.car_location.z = newLocation.Z;

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

	player->SetActorLocation(preLocation);
}

void ACPP_Cobot_Car_Controller::CarRotation(float rotationValue)
{
	/*FRotator control_rotation = GetControlRotation();
	control_rotation.Yaw += rotationValue;
	SetControlRotation(control_rotation);*/

	float preYaw = player->GetActorRotation().Yaw;
	preYaw += rotationValue;
	//���⼭ preYaw�� send�ϸ� ��

	cs_car_rotation_yaw_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_car_rotation_yaw);
	pack.yaw = preYaw;

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
}

void ACPP_Cobot_Car_Controller::SetPlayerLocation(FVector newLocation)
{
	player->SetActorLocation(newLocation);
	player->ChangAim(true, true);
}

void ACPP_Cobot_Car_Controller::SetPlayerYaw(float newYaw)
{
	FRotator control_rotation = GetControlRotation();
	if (control_rotation.Yaw > newYaw)
		player->ChangAim(false, true);
	else
		player->ChangAim(true, false);

	control_rotation.Yaw = newYaw;
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

void ACPP_Cobot_Car_Controller::SendEsc()
{
	cs_esc_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_esc);

	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
}





