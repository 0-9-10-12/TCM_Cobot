// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_GearKey_2S.h"
#include "CPP_Stage2_MissionButton.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

// Sets default values
ACPP_GearKey_2S::ACPP_GearKey_2S()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	clear = CreateDefaultSubobject<UBoxComponent>(TEXT("clear"));
	forward_Key_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("forward_Key_collision"));
	back_Key_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("back_Key_collision"));
	right_Key_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("right_Key_collision"));
	left_Key_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("left_Key_collision"));

	center_gear = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("center_gear"));
	child_gear1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("child_gear1"));
	child_gear2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("child_gear2"));
	child_gear3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("child_gear3"));
	Key = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Key"));
	Key_frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Key_frame"));
	blockCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("blockCube"));

	RootComponent = clear;

	center_gear->SetupAttachment(RootComponent);
	child_gear1->SetupAttachment(RootComponent);
	child_gear2->SetupAttachment(RootComponent);
	child_gear3->SetupAttachment(RootComponent);

	Key->SetupAttachment(RootComponent);
	forward_Key_collision->SetupAttachment(Key);
	back_Key_collision->SetupAttachment(Key);
	right_Key_collision->SetupAttachment(Key);
	left_Key_collision->SetupAttachment(Key);

	Key_frame->SetupAttachment(RootComponent);
	blockCube->SetupAttachment(RootComponent);

	center_gear->SetRelativeLocation(FVector(-390.f, 0.f, 0.f));
	child_gear1->SetRelativeLocation(FVector(-390.f, 230.f, -80.f));
	child_gear2->SetRelativeLocation(FVector(-180.f, 460.f, -10.f));
	child_gear3->SetRelativeLocation(FVector(-500.f, -220.f, -80.f));
	Key->SetRelativeLocation(FVector(-600.f, 0.f, 290.f));
	Key_frame->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
	forward_Key_collision->SetRelativeLocation(FVector(20.f, 0.0f, -20.0f));
	back_Key_collision->SetRelativeLocation(FVector(-20.f, 0.0f, -20.0f));
	right_Key_collision->SetRelativeLocation(FVector(0.f, 20.0f, -20.0f));
	left_Key_collision->SetRelativeLocation(FVector(0.f, -20.0f, -20.0f));

	child_gear2->SetRelativeScale3D(FVector(0.5f, 1.f, 1.0f));
	child_gear3->SetRelativeScale3D(FVector(0.5f, 1.f, 1.0f));
	Key->SetRelativeScale3D(FVector(0.25f, 0.25f, 5.0f));

	child_gear2->SetRelativeRotation(FRotator(0.0f, 0.0f, -15.0f));


	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_GEAR(TEXT("/Game/model/STAGE_2/gear_1.gear_1"));
	if (SM_GEAR.Succeeded()) {
		center_gear->SetStaticMesh(SM_GEAR.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_CHILD_GEAR(TEXT("/Game/model/STAGE_2/gear_2.gear_2"));
	if (SM_CHILD_GEAR.Succeeded()) {
		child_gear1->SetStaticMesh(SM_CHILD_GEAR.Object);
		child_gear2->SetStaticMesh(SM_CHILD_GEAR.Object);
		child_gear3->SetStaticMesh(SM_CHILD_GEAR.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_KEY(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (SM_KEY.Succeeded()) {
		Key->SetStaticMesh(SM_KEY.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_KEY_FRAME(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (SM_KEY_FRAME.Succeeded()) {
		Key_frame->SetStaticMesh(SM_KEY_FRAME.Object);
		blockCube->SetStaticMesh(SM_KEY_FRAME.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_FOOTHOLD(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (SM_FOOTHOLD.Succeeded()) {
		blockCube->SetStaticMesh(SM_FOOTHOLD.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraSystemAsset(TEXT("/Game/particles/clearFireworks/np_clearFireworks.np_clearFireworks"));
	if (NiagaraSystemAsset.Succeeded())
	{
		fireNiagaraSystemAsset = NiagaraSystemAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> fireSoundAsset(TEXT("/Game/game_sound/firework_rocket_launch.firework_rocket_launch"));
	if (fireSoundAsset.Succeeded())
	{
		fireSound = fireSoundAsset.Object;
	}
	
}

// Called when the game starts or when spawned
void ACPP_GearKey_2S::BeginPlay()
{
	Super::BeginPlay();
	blockCube->SetVisibility(false);
}

// Called every frame
void ACPP_GearKey_2S::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACPP_GearKey_2S::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	clear->OnComponentBeginOverlap.AddDynamic(this, &ACPP_GearKey_2S::OnClearOverlap);
}

void ACPP_GearKey_2S::target_forward()
{
	UE_LOG(LogTemp, Warning, TEXT("target_forward"));

	if (!forward_Key_collision->IsOverlappingComponent(center_gear)) {
		Key->AddLocalOffset(FVector(1, 0.0f, 0.0f));
	}
}

void ACPP_GearKey_2S::target_back()
{
	UE_LOG(LogTemp, Warning, TEXT("target_back"));
	if (!back_Key_collision->IsOverlappingComponent(center_gear)) {
		Key->AddLocalOffset(FVector(-1.f, 0.0f, 0.0f));
	}
}

void ACPP_GearKey_2S::target_right()
{

	UE_LOG(LogTemp, Warning, TEXT("target_right"));

	Key->AddLocalOffset(FVector(0.f, -0.001f, 0.0));

	if (!center_gear->IsOverlappingComponent(left_Key_collision)) {
		center_gear->AddLocalRotation(FRotator(0.0f, 0.0f, 1 * 0.5));
		child_gear1->AddLocalRotation(FRotator(0.0f, 0.0f, -1 * 0.5));
		child_gear2->AddLocalRotation(FRotator(0.0f, 0.0f, 1 * 0.5));
		child_gear3->AddLocalRotation(FRotator(0.0f, 0.0f, -1 * 0.5));
	}
	Key->AddLocalOffset(FVector(0.f, 0.001f, 0.0));
}

void ACPP_GearKey_2S::target_left()
{
	UE_LOG(LogTemp, Warning, TEXT("target_left"));

	Key->AddLocalOffset(FVector(0.f, 0.001f, 0.0));

	if (!center_gear->IsOverlappingComponent(right_Key_collision)) {
		center_gear->AddLocalRotation(FRotator(0.0f, 0.0f, -1 * 0.5));
		child_gear1->AddLocalRotation(FRotator(0.0f, 0.0f, 1 * 0.5));
		child_gear2->AddLocalRotation(FRotator(0.0f, 0.0f, -1 * 0.5));
		child_gear3->AddLocalRotation(FRotator(0.0f, 0.0f, 1 * 0.5));

	}
	Key->AddLocalOffset(FVector(0.f, -0.001f, 0.0));
}

void ACPP_GearKey_2S::OnClearOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	clear->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().SetTimer(bridgeTimer, this, &ACPP_GearKey_2S::BridgeTimer, 0.1f, true);


	AActor* maze_actor;
	maze_actor = UGameplayStatics::GetActorOfClass(GetWorld(), ACPP_Stage2_MissionButton::StaticClass());

	if (maze_actor)
		Cast<ACPP_Stage2_MissionButton>(maze_actor)->GearNoCollision();


	FVector spawnLocation = GetActorLocation();
	spawnLocation.X += 1000;
	spawnLocation.Y -= 100;
	spawnLocation.Z -= 2000;
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), fireNiagaraSystemAsset, spawnLocation);

	blockCube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UGameplayStatics::PlaySoundAtLocation(this, fireSound, GetActorLocation(), GetActorRotation());
}

void ACPP_GearKey_2S::BridgeTimer()
{
	//UE_LOG(LogTemp, Warning, TEXT("BridgeTimer"));
	
	bridgeTime += 0.03;
	UMaterialParameterCollection* MPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/material/function/mpc_bridge_time.mpc_bridge_time"));
	UMaterialParameterCollectionInstance* MyMPCInstance = GetWorld()->GetParameterCollectionInstance(MPC);
	MyMPCInstance->SetScalarParameterValue(FName("second time"), bridgeTime);

	if (bridgeTime > 1.0f) {
		GetWorldTimerManager().ClearTimer(bridgeTimer);
	}
}