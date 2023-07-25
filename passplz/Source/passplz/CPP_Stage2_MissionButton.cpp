// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_Stage2_MissionButton.h"


// Sets default values
ACPP_Stage2_MissionButton::ACPP_Stage2_MissionButton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Center = CreateDefaultSubobject<UArrowComponent>(TEXT("Center"));

	//�̷�
	Maze_target_forward = CreateDefaultSubobject<UBoxComponent>(TEXT("Maze_target_forward"));
	Maze_target_back = CreateDefaultSubobject<UBoxComponent>(TEXT("Maze_target_back"));
	Maze_target_right = CreateDefaultSubobject<UBoxComponent>(TEXT("Maze_target_right"));
	Maze_target_left = CreateDefaultSubobject<UBoxComponent>(TEXT("Maze_target_left"));

	//���
	Gear_target_forward = CreateDefaultSubobject<UBoxComponent>(TEXT("Gear_target_forward"));
	Gear_target_back = CreateDefaultSubobject<UBoxComponent>(TEXT("Gear_target_back"));
	Gear_target_right = CreateDefaultSubobject<UBoxComponent>(TEXT("Gear_target_right"));
	Gear_target_left = CreateDefaultSubobject<UBoxComponent>(TEXT("Gear_target_left"));

	RootComponent = Center;
	Maze_target_forward->SetupAttachment(RootComponent);
	Maze_target_back->SetupAttachment(RootComponent);
	Maze_target_right->SetupAttachment(RootComponent);
	Maze_target_left->SetupAttachment(RootComponent);

	Gear_target_forward->SetupAttachment(RootComponent);
	Gear_target_back->SetupAttachment(RootComponent);
	Gear_target_right->SetupAttachment(RootComponent);
	Gear_target_left->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACPP_Stage2_MissionButton::BeginPlay()
{
	Super::BeginPlay();
	
	isMazeForward = false;
	isMazeBack = false;
	isMazeRight = false;
	isMazeLeft = false;

	
}

// Called every frame
void ACPP_Stage2_MissionButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACPP_Stage2_MissionButton::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//�浹 ���� �Լ� ����
	Maze_target_forward->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeforwardOverlap);
	Maze_target_back->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeBackOverlap);
	Maze_target_right->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeRightOverlap);
	Maze_target_left->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeLeftOverlap);

	//�浹 ���� �Լ� ����
	Maze_target_forward->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeforwardEndOverlap);
	Maze_target_back->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeBackEndOverlap);
	Maze_target_right->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeRightEndOverlap);
	Maze_target_left->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnMazeLeftEndOverlap);

	//�浹 ���� �Լ� ����
	Gear_target_forward->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearforwardOverlap);
	Gear_target_back->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearBackOverlap);
	Gear_target_right->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearRightOverlap);
	Gear_target_left->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearLeftOverlap);

	//�浹 ���� �Լ� ����
	Gear_target_forward->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearforwardEndOverlap);
	Gear_target_back->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearBackEndOverlap);
	Gear_target_right->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearRightEndOverlap);
	Gear_target_left->OnComponentEndOverlap.AddDynamic(this, &ACPP_Stage2_MissionButton::OnGearLeftEndOverlap);
}

//������ ������ Ÿ�̸� �Լ�
void ACPP_Stage2_MissionButton::SendMazeTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("SendMazeTimer"));	

	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);

	// packet_type packetType;
	//bool����� Ÿ�̸Ӹ� ���� �����Ѵ�.
	if (!isMazeForward && !isMazeBack && !isMazeRight && !isMazeLeft) {
		UE_LOG(LogTemp, Warning, TEXT("Mazereturn"));

		//��� �� false�̸� Ÿ�̸Ӹ� �����Ѵ�.
		GetWorldTimerManager().ClearTimer(mazeTimer);
		return;
	}

	//4�� �� �ϳ��� true�� �Ʒ� if������ ����ȴ�.
	if (isMazeForward && isMazeBack) {
		UE_LOG(LogTemp, Warning, TEXT("Mazereturn"));

		return;
	}
	else if (isMazeForward) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_maze_forward);
		UE_LOG(LogTemp, Warning, TEXT("isMazeForward"));
		int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isMazeBack) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_maze_back);
		UE_LOG(LogTemp, Warning, TEXT("isMazeBack"));
		int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isMazeRight && isMazeLeft) {
		UE_LOG(LogTemp, Warning, TEXT("Mazereturn"));

		return;
	}
	else if(isMazeRight){
		button_pack.type = static_cast<char>(packet_type::cs_push_button_maze_right);
		UE_LOG(LogTemp, Warning, TEXT("isMazeRight"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isMazeLeft) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_maze_left);
		UE_LOG(LogTemp, Warning, TEXT("isMazeLeft"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("elseelseelseesle errrrrrrrrr"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Mazesend"));
}

void ACPP_Stage2_MissionButton::SendGearTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("SendGearTimer"));

	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);

	// packet_type packetType;
	//bool����� Ÿ�̸Ӹ� ���� �����Ѵ�.
	if (!isGearForward && !isGearBack && !isGearRight && !isGearLeft) {
		UE_LOG(LogTemp, Warning, TEXT("Gearreturn"));

		//��� �� false�̸� Ÿ�̸Ӹ� �����Ѵ�.
		GetWorldTimerManager().ClearTimer(gearTimer);
		return;
	}

	//4�� �� �ϳ��� true�� �Ʒ� if������ ����ȴ�.
	if (isGearForward && isGearBack) {
		UE_LOG(LogTemp, Warning, TEXT("Gearreturn"));

		return;
	}
	else if (isGearForward) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_forward);
		UE_LOG(LogTemp, Warning, TEXT("isGearForward"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isGearBack) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_back);
		UE_LOG(LogTemp, Warning, TEXT("isGearBack"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isGearRight && isGearLeft) {
		UE_LOG(LogTemp, Warning, TEXT("Gearreturn"));

		return;
	}
	else if (isGearRight) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_right);
		UE_LOG(LogTemp, Warning, TEXT("isGearRight"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	else if (isGearLeft) {
		button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_left);
		UE_LOG(LogTemp, Warning, TEXT("isGearLeft"));
		send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Gearsend"));
}

//���� Ÿ�̸Ӱ� �����ϴ��� �˻��ϴ� �Լ�
void ACPP_Stage2_MissionButton::HasTimer(FTimerHandle& timer, int timerType)
{
	if (!GetWorldTimerManager().IsTimerActive(timer)) {
		UE_LOG(LogTemp, Warning, TEXT("SendMazeTimercreate"));

		if(timerType == 0)
			GetWorldTimerManager().SetTimer(timer, this, &ACPP_Stage2_MissionButton::SendMazeTimer, 0.03f, true);
		else if(timerType == 1)
			GetWorldTimerManager().SetTimer(timer, this, &ACPP_Stage2_MissionButton::SendGearTimer, 0.03f, true);
	}
}

void ACPP_Stage2_MissionButton::MazeNoCollision()
{
	Maze_target_forward->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Maze_target_back->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Maze_target_left->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Maze_target_right->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACPP_Stage2_MissionButton::GearNoCollision()
{
	Gear_target_forward->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Gear_target_back->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Gear_target_left->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Gear_target_right->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//�̷ι�ư�� ������� � ��ư�� ��Ҵ��� ������ ������======================================================
void ACPP_Stage2_MissionButton::OnMazeforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isMazeForward = true;
	HasTimer(mazeTimer, 0);
	UE_LOG(LogTemp, Warning, TEXT("OnMazeforwardOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isMazeBack = true;
	HasTimer(mazeTimer, 0);
	UE_LOG(LogTemp, Warning, TEXT("OnMazeBackOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isMazeRight = true;
	HasTimer(mazeTimer, 0);
	UE_LOG(LogTemp, Warning, TEXT("OnMazeRightOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isMazeLeft = true;
	HasTimer(mazeTimer, 0);
	UE_LOG(LogTemp, Warning, TEXT("OnMazeLeftOverlap"));
}

//�̷� �浹 �� ====================================================
void ACPP_Stage2_MissionButton::OnMazeforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isMazeForward = false;
	UE_LOG(LogTemp, Warning, TEXT("OnMazeforwardEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isMazeBack = false;
	UE_LOG(LogTemp, Warning, TEXT("OnMazeBackEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isMazeRight = false;
	UE_LOG(LogTemp, Warning, TEXT("OnMazeRightEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isMazeLeft = false;
	UE_LOG(LogTemp, Warning, TEXT("OnMazeLeftEndOverlap"));
}

//===================================================================================================================
//��� ��ư �浹 ����
void ACPP_Stage2_MissionButton::OnGearforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isGearForward = true;
	HasTimer(gearTimer, 1);
	UE_LOG(LogTemp, Warning, TEXT("OnGearforwardOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isGearBack = true;
	HasTimer(gearTimer, 1);
	UE_LOG(LogTemp, Warning, TEXT("OnGearBackOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isGearRight = true;
	HasTimer(gearTimer, 1);
	UE_LOG(LogTemp, Warning, TEXT("OnGearRightOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/game_sound/stage_1/button_click_Cue.button_click_Cue"));
	UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), GetActorRotation());

	isGearLeft = true;
	HasTimer(gearTimer, 1);
	UE_LOG(LogTemp, Warning, TEXT("OnGearLeftOverlap"));
}

//����ư �浹 ��=============================================================
void ACPP_Stage2_MissionButton::OnGearforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isGearForward = false;
	UE_LOG(LogTemp, Warning, TEXT("OnGearforwardEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isGearBack = false;
	UE_LOG(LogTemp, Warning, TEXT("OnGearBackEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isGearRight = false;
	UE_LOG(LogTemp, Warning, TEXT("OnGearRightEndOverlap"));
}

void ACPP_Stage2_MissionButton::OnGearLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isGearLeft = false;
	UE_LOG(LogTemp, Warning, TEXT("OnGearLeftEndOverlap"));
}
//===================================================================================================================

