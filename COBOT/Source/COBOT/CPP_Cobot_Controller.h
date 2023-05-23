// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

///////#include "../../../server/server/RingBuffer.h"

///////#include "CPP_CobotGameInstance.h"

////////#include "Windows/AllowWindowsPlatformTypes.h"
////////#include "Windows/prewindowsapi.h"
////////
////////#include <WinSock2.h>
////////#pragma comment(lib, "WS2_32.lib")
////////
////////#include "Windows/PostWindowsApi.h"
///////#include "CPP_Elevator.h"

#include "COBOT.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "CPP_Cobot_Controller.generated.h"

/**
 *
 */
UCLASS()
class COBOT_API ACPP_Cobot_Controller : public APlayerController
{
	GENERATED_BODY()

		ACPP_Cobot_Controller();
	~ACPP_Cobot_Controller();

private:
	///////SOCKET* sock;
	///////vector_d3 location;
	///////int	id;
	///////RingBuffer ring_buff;
	// TMap<int, AActor*> actorList;

	////////vector_d3 tm_location;
	///////vector_d3 tm_current;
	///////float tm_time_left;
	///////float tm_time_right;
	///////
	///////TArray<AActor*> maze_actor;
	///////TArray<AActor*> gear_actor;
	///////TArray<AActor*> forklift_actor;
	///////TArray<AActor*> board_actor;
	///////
	///////bool is_maze_button_push_forward;
	///////bool is_maze_button_push_back;
	///////bool is_maze_button_push_left;
	///////bool is_maze_button_push_right;

protected:
	virtual void BeginPlay() override;

private:
	virtual void Tick(float DeltaTime) override;
	////////void RecvPacket();
	////////void ProcessPacket(char* packet);
	////////void SendEnterPacket();
	////////void SendMovePacket(direction direction);

	///////UCPP_CobotGameInstance* instance;

	// 패킷 재조립을 위한 부분 나중에 무조건 수정
	////////////int		prev_remain;
	////////////int		prev_packet_size;
	////////////char	prev_packet_buff[10000];

	////////std::chrono::high_resolution_clock::time_point last_move_time;

public:
	virtual void PostInitializeComponents() override;
	virtual void OnPossess(APawn* aPawn) override;
protected:
	virtual void SetupInputComponent() override;

private:
	//void Left_Right(float NewAxisValue);
	//void Turn(float NewAxisValue);
	//void LookUp(float NewAxisValue);
	//void Union_Pressed();
	//void Union_Released();
	//void Run_Pressed();
	//void Run_Released();

public: //장애물과 서버 데이터 통신을 위해 필요한 API
	////////////SOCKET* GetSocket() { return sock; };

private:
	class ACPP_Cobot* player;
	FVector player_pos;
	float new_axis_value;

	/////////////class ACPP_Cobot* Player_2; //상대 플레이어 정보


public:
	/////////////bool Is_Set_IDPW(FString I, FString p);

protected:
	/////////////UPROPERTY(BlueprintReadWrite)
	/////////////	FString ID; //아이디
	/////////////UPROPERTY(BlueprintReadWrite)
	/////////////	FString Passward; //비밀번호

private:
	int previous_input;
	int current_input;
	float rotate_min;
	float rotate_max;

private:
	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputMappingContext* imcCobot;

	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* iaRotate;
	UPROPERTY(VisibleAnywhere, Category = Input)
		class UInputAction* iaMove;

protected:
	void Rotate(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
};