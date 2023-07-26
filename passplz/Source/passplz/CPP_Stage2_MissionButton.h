// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../server/server/protocol.h"
#include "CPP_Cobot_Controller.h"

#include "passplz.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "CPP_Stage2_MissionButton.generated.h"

UCLASS()
class PASSPLZ_API ACPP_Stage2_MissionButton : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Stage2_MissionButton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;


private:
	// SOCKET* sock;

private:
	UPROPERTY(VisibleAnywhere)
		UArrowComponent* Center;

	//�̷� �浹
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Maze_target_forward;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Maze_target_back;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Maze_target_right;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Maze_target_left;

	//��� �浹
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Gear_target_forward;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Gear_target_back;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Gear_target_right;
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* Gear_target_left;

	FTimerHandle mazeTimer;
	bool isMazeForward;
	bool isMazeBack;
	bool isMazeRight;
	bool isMazeLeft;

	FTimerHandle gearTimer;
	bool isGearForward;
	bool isGearBack;
	bool isGearRight;
	bool isGearLeft;

	USoundBase* buttonSound;

private:

	//�̷� �浹 ���� �̺�Ʈ==========================================================================
	UFUNCTION()
		void OnMazeforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnMazeBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnMazeRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnMazeLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//�̷� �浹 �� �̺�Ʈ
	UFUNCTION()
		void OnMazeforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnMazeBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnMazeRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnMazeLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//=============================================================================================
	//��� �浹 ���� �̺�Ʈ
	UFUNCTION()
		void OnGearforwardOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnGearBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnGearRightOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnGearLeftOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//��� �浹 �� �̺�Ʈ
	UFUNCTION()
		void OnGearforwardEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnGearBackEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnGearRightEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnGearLeftEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//=============================================================================================

public:
	void SendMazeTimer();
	void SendGearTimer();
	void HasTimer(FTimerHandle& timer, int timerType);
	void MazeNoCollision();
	void GearNoCollision();

};
