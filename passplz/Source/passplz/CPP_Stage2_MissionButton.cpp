// Fill out your copyright notice in the Description page of Project Settings.

#include "../../../server/server/protocol.h"
#include "CPP_Cobot_Controller.h"
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
	
	isForward = false;
	isBack = false;
	isRight = false;
	isLeft = false;
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
}

//������ ������ Ÿ�̸� �Լ�
void ACPP_Stage2_MissionButton::SendTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("SendTimer"));	

	packet_type packetType;
	//bool����� Ÿ�̸Ӹ� ���� �����Ѵ�.
	if (!isForward && !isBack && !isRight && !isLeft) {
		UE_LOG(LogTemp, Warning, TEXT("return"));

		//��� �� false�̸� Ÿ�̸Ӹ� �����Ѵ�.
		GetWorldTimerManager().ClearTimer(Timer);
		return;
	}

	//4�� �� �ϳ��� true�� �Ʒ� if������ ����ȴ�.
	if (isForward && isBack) {
		UE_LOG(LogTemp, Warning, TEXT("return"));

		return;
	}
	else if (isForward) {
		packetType = packet_type::cs_push_button_maze_forward;
		UE_LOG(LogTemp, Warning, TEXT("isForward"));
	}
	else if (isBack) {
		packetType = packet_type::cs_push_button_maze_back;
		UE_LOG(LogTemp, Warning, TEXT("isBack"));

	}
	else if (isRight && isLeft) {
		UE_LOG(LogTemp, Warning, TEXT("return"));

		return;
	}
	else if(isRight){
		packetType = packet_type::cs_push_button_maze_right;
		UE_LOG(LogTemp, Warning, TEXT("isRight"));

	}
	else if (isLeft) {
		packetType = packet_type::cs_push_button_maze_left;
		UE_LOG(LogTemp, Warning, TEXT("isLeft"));

	}
	UE_LOG(LogTemp, Warning, TEXT("send"));

	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packetType);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);
}

//�̷ι�ư�� ������� � ��ư�� ��Ҵ��� ������ ������======================================================
void ACPP_Stage2_MissionButton::OnMazeforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isForward = true;

	//Ÿ�̸Ӱ� ������ ���� Ÿ�̸� �Լ����� �����Ѵ�. (bool������)
	if (!GetWorldTimerManager().IsTimerActive(Timer)) {
		UE_LOG(LogTemp, Warning, TEXT("timercreate"));

		GetWorldTimerManager().SetTimer(Timer, this, &ACPP_Stage2_MissionButton::SendTimer, 0.03f, true);
	}
	UE_LOG(LogTemp, Warning, TEXT("OnMazeforwardOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isBack = true;

	if (!GetWorldTimerManager().IsTimerActive(Timer)) {
		GetWorldTimerManager().SetTimer(Timer, this, &ACPP_Stage2_MissionButton::SendTimer, 0.03f, true);
	}
	UE_LOG(LogTemp, Warning, TEXT("OnMazeBackOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isRight = true;

	if (!GetWorldTimerManager().IsTimerActive(Timer)) {
		GetWorldTimerManager().SetTimer(Timer, this, &ACPP_Stage2_MissionButton::SendTimer, 0.03f, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("OnMazeRightOverlap"));
}

void ACPP_Stage2_MissionButton::OnMazeLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	isLeft = true;

	if (!GetWorldTimerManager().IsTimerActive(Timer)) {
		GetWorldTimerManager().SetTimer(Timer, this, &ACPP_Stage2_MissionButton::SendTimer, 0.03f, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("OnMazeLeftOverlap"));
}

//�̷ι�ư�� �浹�� �����ٸ� ��ŶŸ���� ���� �浹�� ������ Ȯ���ϰ� ���ٸ� 
void ACPP_Stage2_MissionButton::OnMazeforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isForward = false;

	UE_LOG(LogTemp, Warning, TEXT("OnMazeforwardEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnMazeBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isBack = false;

	UE_LOG(LogTemp, Warning, TEXT("OnMazeBackEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnMazeRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isRight = false;

	UE_LOG(LogTemp, Warning, TEXT("OnMazeRightEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnMazeLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	isLeft = false;

	UE_LOG(LogTemp, Warning, TEXT("OnMazeLeftEndOverlap"));
}
//===================================================================================================================
//��� ��ư �浹 ����
void ACPP_Stage2_MissionButton::OnGearforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_forward);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearforwardOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_back);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearBackOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_right);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearRightOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_push_button_gear_left);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearLeftOverlap"));
}
//����ư �浹 ��
void ACPP_Stage2_MissionButton::OnGearforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_end_button_gear_forward);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearforwardEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_end_button_gear_back);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearBackEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_end_button_gear_right);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearRightEndOverlap"));
}
void ACPP_Stage2_MissionButton::OnGearLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SOCKET* sock = Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->GetSocket();

	cs_button_packet button_pack;
	button_pack.size = sizeof(button_pack);
	button_pack.type = static_cast<char>(packet_type::cs_end_button_gear_left);

	int ret = send(*sock, reinterpret_cast<char*>(&button_pack), sizeof(button_pack), 0);

	UE_LOG(LogTemp, Warning, TEXT("OnGearLeftEndOverlap"));
}
//===================================================================================================================

