// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "COBOT.h"
#include "GameFramework/Character.h"

#include "CPP_Cobot.generated.h"

UCLASS()
class COBOT_API ACPP_Cobot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACPP_Cobot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void AnimatiomChange(int mode);
	void SetCollision();
	FVector GetColor() { return color; }
	void SetColor(FVector newColor) { color = newColor; }
	void RecvColor(int newcolor);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		UCameraComponent* Camera;


	TSubclassOf<class UAnimInstance> AnimInstanceClass;

public:
	UPROPERTY(BlueprintReadWrite)
		UBoxComponent* Foot_left_Zone;
	UPROPERTY(BlueprintReadWrite)
		UBoxComponent* Foot_right_Zone;
	UPROPERTY(BlueprintReadWrite)
		float Time_left;
	UPROPERTY(BlueprintReadWrite)
		float Time_right;
	UPROPERTY(BlueprintReadWrite)
		FVector Target_left;
	UPROPERTY(BlueprintReadWrite)
		FVector Target_right;
	UPROPERTY(BlueprintReadWrite)
		FVector Current_left;
	UPROPERTY(BlueprintReadWrite)
		FVector Current_right;
	UPROPERTY(BlueprintReadWrite)
		FVector Start_left;
	UPROPERTY(BlueprintReadWrite)
		FVector Start_right;
	UPROPERTY(BlueprintReadWrite)
		bool IsUnion_Jump_anim;
	UPROPERTY(BlueprintReadWrite)
		bool IsFast_run;
	UPROPERTY(BlueprintReadWrite)
		FVector color;

};