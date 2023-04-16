// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "passplz.h"
#include "GameFramework/Actor.h"
#include "CPP_Maze_2S.generated.h"

UCLASS()
class PASSPLZ_API ACPP_Maze_2S : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Maze_2S();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	// �������� ���� ��Ŷ Ÿ�Կ� ���� �Ҹ��� �Լ� target�� ��ġ�� �ű��.
	void target_forward();
	void target_back();
	void target_right();
	void target_left();

private:
	UPROPERTY(VisibleAnywhere)
		UBoxComponent* center;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* Maze;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* Target;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* forward_target_collision;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* back_target_collision;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* right_target_collision;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* left_target_collision;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* plan;
}; 
