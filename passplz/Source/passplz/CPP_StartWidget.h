// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CPP_Cobot_Controller.h"

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

		//UCPP_StartWidget();

protected:
	virtual void NativeOnInitialized();

private:
	SOCKET* sock;
	UCPP_CobotGameInstance* instance;

public:
	/*UFUNCTION(BlueprintCallable, Category = "print")
	void IDprint() {
		UE_LOG(LogTemp, Warning, TEXT("%s, %s"), *ID, *Passward );
	};*/
	
	UFUNCTION(BlueprintCallable)
		bool SendLoginIDPW();
	//void CallEventSuccessLogin(bool loginResult);

	UFUNCTION(BlueprintCallable)
		bool SendSingupIDPW();
	//void CallEventSuccessSignup(bool signupResult);

	//�游���
	UFUNCTION(BlueprintCallable)
		void CreateRoom();
	void CallEventSuccessAddRoom(FString name, int mode, int id);

	//���ΰ�ħ
	UFUNCTION(BlueprintCallable)
		void NormalModeRefresh();

	UFUNCTION(BlueprintCallable)
		bool IsPLayGame(int roomId);

protected:
	UPROPERTY(BlueprintReadWrite)
		bool Is_start; //�������� ���� �����ص� �Ǵ� �޴� �� �����̴�.

	//�α��� ����
	UPROPERTY(BlueprintReadWrite)
		FString ID;
	UPROPERTY(BlueprintReadWrite)
		FString Passward;
	UPROPERTY(BlueprintReadWrite)
		bool isSuccessLogin;

	//ȸ������ ����
	UPROPERTY(BlueprintReadWrite)
		FString signupID;
	UPROPERTY(BlueprintReadWrite)
		FString signupPassward;
	UPROPERTY(BlueprintReadWrite)
		bool isSuccessSignup;

	//�� ����
	UPROPERTY(BlueprintReadWrite)
		FString roomName;
	UPROPERTY(BlueprintReadWrite)
		int stageNum;
	UPROPERTY(BlueprintReadWrite)
		FString userName;
	UPROPERTY(BlueprintReadWrite)
		int roomMode;
	UPROPERTY(BlueprintReadWrite)
		int roomID;

	//���� ��
	UPROPERTY(BlueprintReadWrite)
		int roomCount;
	UPROPERTY(BlueprintReadWrite)
		bool isRoomAdd;
};
