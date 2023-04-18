// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Maze_2S.h"

// Sets default values
ACPP_Maze_2S::ACPP_Maze_2S()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	center = CreateDefaultSubobject<UBoxComponent>(TEXT("center"));
	Maze = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Maze"));
	plan = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("plan"));
	Target = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Target"));
	forward_target_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("forward_target_collision"));
	back_target_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("back_target_collision"));
	right_target_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("right_target_collision"));
	left_target_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("left_target_collision"));

	RootComponent = center;

	Maze->SetupAttachment(RootComponent);

	plan->SetupAttachment(RootComponent);

	Target->SetupAttachment(RootComponent);
	forward_target_collision->SetupAttachment(Target);
	back_target_collision->SetupAttachment(Target);
	right_target_collision->SetupAttachment(Target);
	left_target_collision->SetupAttachment(Target);

	center->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACPP_Maze_2S::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACPP_Maze_2S::Tick(float DeltaTime) //�������� Ư�� ��Ŷ Ÿ���� ������ ����ǵ��� �ٲ۴�.
{
	Super::Tick(DeltaTime);
}

void ACPP_Maze_2S::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

//�������� ������ ����� ��Ŷ�� �޾����� ����
void ACPP_Maze_2S::target_forward()
{
	UE_LOG(LogTemp, Warning, TEXT("target_forward"));
	if (!forward_target_collision->IsOverlappingComponent(Maze))
		Target->AddLocalOffset(FVector(1.f, 0.0f, 0.0f));
	return;
}

//�������� ������ ����� ��Ŷ�� �޾����� ����
void ACPP_Maze_2S::target_back()
{
	UE_LOG(LogTemp, Warning, TEXT("target_back"));
	if (!back_target_collision->IsOverlappingComponent(Maze))
		Target->AddLocalOffset(FVector(-1.f, 0.0f, 0.0f));
	return;
}

//�������� ���������� ����� ��Ŷ�� �޾����� ����
void ACPP_Maze_2S::target_right()
{
	UE_LOG(LogTemp, Warning, TEXT("target_right"));
	if (!right_target_collision->IsOverlappingComponent(Maze))
		Target->AddLocalOffset(FVector(0.0f, 1.f, 0.0f));
	return;
}

//�������� �������� ����� ��Ŷ�� �޾����� ����
void ACPP_Maze_2S::target_left()
{
	UE_LOG(LogTemp, Warning, TEXT("target_left"));
	if (!left_target_collision->IsOverlappingComponent(Maze))
		Target->AddLocalOffset(FVector(0.0f, -1.f, 0.0f));
	return;
}
