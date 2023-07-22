// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_StartWidget.h"


//UCPP_StartWidget::UCPP_StartWidget()
//
//{
//	
//}

void UCPP_StartWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogTemp, Warning, TEXT("startwidget()"));

	instance = Cast<UCPP_CobotGameInstance>(GetWorld()->GetGameInstance());
	sock = instance->GetSocketMgr()->GetSocket();
}

bool UCPP_StartWidget::SendLoginIDPW()
{
	//����! ID, Passward->�̰� ������ ������ �α��� ����Ȯ��
	//�������� �����Ÿ� ���ϰ����� �Ѱ��ش�.
	UE_LOG(LogTemp, Warning, TEXT("SendLoginIDPW()"));

	u_long BlockingMode = 0;
	ioctlsocket(*sock, FIONBIO, &BlockingMode); // sock�� ����ŷ ���� ����

	wchar_t* id = TCHAR_TO_WCHAR(*ID);
	wchar_t* pw = TCHAR_TO_WCHAR(*Passward);

	// �������� ���Դٰ� �˷��ִ� ��
	cs_login_packet login_pack;
	login_pack.size = sizeof(login_pack);
	login_pack.type = static_cast<char>(packet_type::cs_login);
	wcscpy_s(login_pack.id, MAX_NAME, id);
	wcscpy_s(login_pack.passward, MAX_NAME, pw);

	int ret = send(*sock, reinterpret_cast<char*>(&login_pack), sizeof(login_pack), 0);

	char buff[BUF_SIZE];
	ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);

	u_long nonBlockingMode = 1;
	ioctlsocket(*sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����

	if (ret != buff[0]) {
		UE_LOG(LogTemp, Warning, TEXT("%d start widget signup recv err"), ret);
		return false;
	}

	switch (buff[1])
	{
	case static_cast<int>(packet_type::sc_login_fail): 
		UE_LOG(LogTemp, Warning, TEXT("recv login fail"));
		return false;
	case static_cast<int>(packet_type::sc_login_success): 
		UE_LOG(LogTemp, Warning, TEXT("recv login success"));
		return true;
	}

	return false;
}

//bool UCPP_StartWidget::CallEventSuccessLogin(bool loginResult)
//{
//	isSuccessLogin = loginResult;
//
//	if(isSuccessLogin)
//		Cast<ACPP_Cobot_Controller>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetWidget(nullptr); //�α����� �����ϸ� ���� ������ nullptr�� �ٲ۴�.
//
//	FOutputDeviceNull pAR;
//	CallFunctionByNameWithArguments(TEXT("Success_Login"), pAR, nullptr, true);
//}

bool UCPP_StartWidget::SendSingupIDPW()
{
	//����! signupID, signupPassward->�̰� ������ ������ �α��� ����Ȯ��
	//�������� �����Ÿ� ���ϰ����� �Ѱ��ش�.
	UE_LOG(LogTemp, Warning, TEXT("SendSignupIDPW()"));

	u_long BlockingMode = 0;
	ioctlsocket(*sock, FIONBIO, &BlockingMode); // sock�� ����ŷ ���� ����

	wchar_t* id = TCHAR_TO_WCHAR(*signupID);
	wchar_t* pw = TCHAR_TO_WCHAR(*signupPassward);

	// �������� ���Դٰ� �˷��ִ� ��
	cs_login_packet login_pack;
	login_pack.size = sizeof(login_pack);
	login_pack.type = static_cast<char>(packet_type::cs_signup);
	wcscpy_s(login_pack.id, MAX_NAME, id);
	wcscpy_s(login_pack.passward, MAX_NAME, pw);

	int ret = send(*sock, reinterpret_cast<char*>(&login_pack), sizeof(login_pack), 0);

	char buff[BUF_SIZE];
	ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);

	u_long nonBlockingMode = 1;
	ioctlsocket(*sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����

	if (ret != buff[0]) {
		UE_LOG(LogTemp, Warning, TEXT("start widget signup recv err"));
		return false;
	}

	switch (buff[1])
	{
	case static_cast<int>(packet_type::sc_signup_fail): return false;
	case static_cast<int>(packet_type::sc_signup_success): return true;
	}

	return false;
}

//void UCPP_StartWidget::CallEventSuccessSignup(bool signupResult)
//{
//	isSuccessSignup = signupResult; //bool�� �����ϰ� Ŀ�����̺�Ʈ ȣ��
//
//	FOutputDeviceNull pAR;
//	CallFunctionByNameWithArguments(TEXT("Success_Signup"), pAR, nullptr, true);
//}

void UCPP_StartWidget::CreateRoom()
{
	UE_LOG(LogTemp, Warning, TEXT("CreateAndWait()"));

	wchar_t* room_name = TCHAR_TO_WCHAR(*roomName);

	cs_create_room_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::cs_create_room);
	wcscpy_s(pack.room_name, MAX_NAME, room_name);
	pack.room_mode = roomMode;

	//������ ���̸��� �������� ��ȣ�� ������. (room_name, stageNum)
	int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
	if (ret <= 0) UE_LOG(LogTemp, Warning, TEXT("CreateRoom() send err"));

	u_long BlockingMode = 0;
	ioctlsocket(*sock, FIONBIO, &BlockingMode); // sock�� ���ŷ ���� ����

	char buff[BUF_SIZE];
	ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);

	u_long nonBlockingMode = 1;
	ioctlsocket(*sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����

	if (ret != buff[0])
		UE_LOG(LogTemp, Warning, TEXT("create room recv err"));

	switch (buff[1])
	{
	case static_cast<int>(packet_type::sc_create_room_ok):
	{
		sc_create_room_ok_packet* recv_pack = reinterpret_cast<sc_create_room_ok_packet*>(&buff);
		roomID = recv_pack->room_id;
		roomName = WCHAR_TO_TCHAR(recv_pack->room_name);
		roomMode = recv_pack->room_mode;

		FOutputDeviceNull pAR;
		CallFunctionByNameWithArguments(TEXT("wait"), pAR, nullptr, true);
	} break;
	}
}

void UCPP_StartWidget::CallEventSuccessAddRoom(FString name, int mode, int id)
{
	roomName = name;
	roomMode = mode;
	roomID = id;

	FOutputDeviceNull pAR;
	CallFunctionByNameWithArguments(TEXT("add_Room"), pAR, nullptr, true);
}

void UCPP_StartWidget::NormalModeRefresh()
{
	UE_LOG(LogTemp, Warning, TEXT("normal mode refresh()"));
	//������ ������ ��û ��û�ϴ� ��Ŷ ������
	//������ ���̸��̶� �������� ��ȣ�� ������.
	cs_show_room_list_packet send_pack;
	send_pack.size = sizeof(send_pack);
	send_pack.type = static_cast<char>(packet_type::cs_show_room_list);
	send_pack.room_mode = 0;

	int ret = send(*sock, reinterpret_cast<char*>(&send_pack), sizeof(send_pack), 0);

	//��� ���
	u_long BlockingMode = 0;
	ioctlsocket(*sock, FIONBIO, &BlockingMode); // sock�� ���ŷ ���� ����

	while (true)
	{
		char buff[BUF_SIZE];
		ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);

		if (ret != buff[0])
			UE_LOG(LogTemp, Warning, TEXT("normal mode refresh recv err"));

		switch (buff[1])
		{
		case static_cast<int>(packet_type::sc_show_room_list):
		{
			sc_show_room_list_packet* recv_pack = reinterpret_cast<sc_show_room_list_packet*>(&buff);

			roomName = WCHAR_TO_TCHAR(recv_pack->room_name);
			userName = WCHAR_TO_TCHAR(recv_pack->host_name);
			roomID = recv_pack->room_id;
			//stageNum = recv_pack->stage;

			FOutputDeviceNull pAR;
			CallFunctionByNameWithArguments(TEXT("show_Room"), pAR, nullptr, true);
		} break;
		case static_cast<int>(packet_type::sc_show_room_list_end): 
			UE_LOG(LogTemp, Warning, TEXT("show normal list end"));

			u_long nonBlockingMode = 1;
			ioctlsocket(*sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����

			return;
		}
	}

	u_long nonBlockingMode = 1;
	ioctlsocket(*sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����
}

void UCPP_StartWidget::PlayGame(int roomId)
{
	//������ �ش� ���̵� ������ �� ������ �����϶�� send�Ѵ�
	// ����� ������ �÷��̾� ��� �̰��� ������ ������ ������ �����϶�� ��Ŷ�� ������.
	// ������ ������ �÷��̾ ������ ���� ���⼭ ��� ����Ѵ�.
	//�ش� ��Ŷ�� �� �����ϸ� true�� ���� �ƴϸ� false�� ����

	if (!prev_enter) {
		cs_enter_room_packet pack;
		pack.size = sizeof(pack);
		pack.type = static_cast<char>(packet_type::cs_enter_room);
		pack.room_id = roomId;
		pack.room_mode = 0;

		int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

		if (ret != -1) {
			UE_LOG(LogTemp, Warning, TEXT("send room enter packet"));
			prev_enter = true;
		} else if (-1 == ret) {
			UE_LOG(LogTemp, Warning, TEXT("send enter room packet err"));
		} else {
			UE_LOG(LogTemp, Warning, TEXT("enter room packet err ret: %d"), ret);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("call recv()"));
	char buff[BUF_SIZE];
	int ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);
	UE_LOG(LogTemp, Warning, TEXT("ret: %d, game start packet size: %d"), ret, sizeof(sc_game_start_packet));

	if (-1 == ret) {

		FTimerHandle waitTimer;
		GetWorld()->GetTimerManager().SetTimer(waitTimer, FTimerDelegate::CreateLambda([&]()
			{
				PlayGame(roomId);
				UE_LOG(LogTemp, Warning, TEXT("Timer"));

			}), 1.f, false);

	} else if (buff[0] == ret) {
		UE_LOG(LogTemp, Warning, TEXT("recv game start packet"));
		
		sc_game_start_packet* pack = reinterpret_cast<sc_game_start_packet*>(&buff);

		// stageNum = pack->stage; // ���⿡ �����ؾ� �ϴ� ���������� ����ִ�.

		FOutputDeviceNull pAR;
		CallFunctionByNameWithArguments(TEXT("open_lavel"), pAR, nullptr, true);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("errerrerr ret: %d"), ret);
	}
}

