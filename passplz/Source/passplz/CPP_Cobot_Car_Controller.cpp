// Fill out your copyright notice in the Description page of Project Settings.

//���� ��Ʈ�ѷ����� �ߴ��� ��Ŷ ó�� �ʿ�

#include "CPP_Cobot_Car_Controller.h"
#include "CPP_Cobot_Car.h"
//#include "Engine/World.h"
#include "CPP_Cannon.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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

void ACPP_Cobot_Car_Controller::CarInput(const FInputActionValue& Value)
{
	//���� : ���⼭ Ű�� �����ų� ���� ���� �����µ� �̶� ������ ��Ŷ ��������
	//�������� �� Ŭ�� Ű�� ���� bool���� ������.
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

void ACPP_Cobot_Car_Controller::CarForward()
{
	UE_LOG(LogTemp, Warning, TEXT("CarForward"));

	FRotator rotator_controller = GetControlRotation();
	FRotator rotator_forward = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, rotator_controller.Yaw);
	FVector forward_vector = UKismetMathLibrary::GetForwardVector(rotator_forward);

	player->AddMovementInput(forward_vector, 1.0);
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





