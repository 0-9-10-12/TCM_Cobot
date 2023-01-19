// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPP_Cobot_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class PASSPLZ_API ACPP_Cobot_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ACPP_Cobot_GameMode();

	// �÷��̾ �α����� �Ϸ��ϸ� ����Ǵ� �Լ�
	// ������ ���� �����ϰ� ��Ʈ�ѷ��� ���� �����ϴ� �۾��� �̷������.
	// PostInitializeComponents�Լ����� ���� ��Ʈ�ѷ��� �������� �� �� �ִ�. 
	// ���Ǵ� ��Ʈ�ѷ��� Possess, ���� PossessedBy�� �� �� �ִ�. 
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
