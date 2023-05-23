// Copyright Epic Games, Inc. All Rights Reserved.


#include "COBOTGameModeBase.h"
#include "CPP_Cobot.h"
#include "CPP_Cobot_Controller.h"

ACOBOTGameModeBase::ACOBOTGameModeBase()
{
	DefaultPawnClass = ACPP_Cobot::StaticClass();
	PlayerControllerClass = ACPP_Cobot_Controller::StaticClass();
}

void ACOBOTGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("PostLogin"));
	Super::PostLogin(NewPlayer);
}
