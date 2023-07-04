// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "passplz.h"
#include "Blueprint/UserWidget.h"
#include "CPP_StartWidget.generated.h"

/**
 *
 */
UCLASS()
class PASSPLZ_API UCPP_StartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*UFUNCTION(BlueprintCallable, Category = "print")
	void IDprint() {
		UE_LOG(LogTemp, Warning, TEXT("%s, %s"), *ID, *Passward );
	};*/
	
protected:
	UFUNCTION(BlueprintCallable)
		bool Is_Set_IDPW_Controller();

	UFUNCTION(BlueprintCallable)
		void Send_Singup_IDPW();

	void CallEventSuccess_Signup(bool signupResult);

protected:
	UPROPERTY(BlueprintReadWrite)
		bool Is_start; //�������� ���� �����ص� �Ǵ� �޴� �� �����̴�.

	UPROPERTY(BlueprintReadWrite)
		FString ID;
	UPROPERTY(BlueprintReadWrite)
		FString Passward;

	UPROPERTY(BlueprintReadWrite)
		FString signupID;
	UPROPERTY(BlueprintReadWrite)
		FString signupPassward;
	UPROPERTY(BlueprintReadWrite)
		bool isSuccessSignup;
};
