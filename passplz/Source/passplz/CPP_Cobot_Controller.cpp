// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_Cobot_Controller.h"
#include "CPP_Cobot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

#include "../../../server/server/protocol.h"

#include <chrono>

ACPP_Cobot_Controller::ACPP_Cobot_Controller()
    : x(-1.f), y(-1.f), z(-1.f), yaw(-1.f)
    , tm_x(-1.f), tm_y(-1.f), tm_z(-1.f), tm_yaw(-1.f)
    , prev_remain(0)
{
}

ACPP_Cobot_Controller::~ACPP_Cobot_Controller()
{
    //cs_logout_packet pack;
    //pack.size = sizeof(pack);
    //pack.type = static_cast<int>(type::cs_logout);

    //send(sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

    //closesocket(sock);
    //WSACleanup();
}

void ACPP_Cobot_Controller::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("�ں�����"));

    // ���� game instance �޾ƿ´�
    instance = Cast<UCPP_CobotGameInstance>(GetWorld()->GetGameInstance());

    sock = instance->GetSocketMgr()->GetSocket();

    cs_login_packet login_pack;
    login_pack.size = sizeof(login_pack);
    login_pack.type = static_cast<char>(type::cs_login);

    int ret = send(*sock, reinterpret_cast<char*>(&login_pack), sizeof(login_pack), 0);

    Player_2 = GetWorld()->SpawnActor<ACPP_Cobot>(ACPP_Cobot::StaticClass(), FVector(tm_x, tm_y, tm_z), FRotator(0.0f, tm_yaw, 0.0f));
}

void ACPP_Cobot_Controller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //RecvPacket();
    //UE_LOG(LogTemp, Warning, TEXT("Tick"));
}

void ACPP_Cobot_Controller::RecvPacket()
{
    char buff[BUF_SIZE];

    int ret = recv(*sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);
    if (ret <= 0)
    {
        GetLastError();
        std::cout << "recv() fail!" << std::endl;
        return;
    }

    if (prev_remain > 0) // ���� ���� �����ִ� �����Ͱ� �ִٸ�
    {
        strcat(prev_packet_buff, buff);
    }
    else
    {
        memcpy(prev_packet_buff, buff, ret);
    }

    int remain_data = ret + prev_remain;
    char* p = prev_packet_buff;    
    
    while (remain_data > 0)
    {
        int packet_size = p[0];
        if (packet_size <= remain_data)
        {
            ProcessPacket(p);
            p = p + packet_size;
            remain_data -= packet_size;
        } else break;
    }

    prev_remain = remain_data;

    if (remain_data > 0)
    {
        memcpy(prev_packet_buff, p, remain_data);
    }

    //ring_buff.enqueue(buff, ret);

    //int read_point = 0;
    //while (buff[read_point] <= ring_buff.diff() && !ring_buff.empty())
    //{
    //    ProcessPacket(buff);
    //    ring_buff.move_read_pos(buff[0]);
    //    read_point = buff[read_point];
    //}
}

void ACPP_Cobot_Controller::ProcessPacket(char* packet)
{
    switch (packet[1])
    {
    case static_cast<int>(type::sc_login):
    {
        sc_login_packet* pack = reinterpret_cast<sc_login_packet*>(packet);
        id = pack->id;
        x = pack->x;
        y = pack->y;
        z = pack->z;
        yaw = pack->yaw;
        tm_x = pack->tm_x;
        tm_y = pack->tm_y;
        tm_z = pack->tm_z;
        tm_yaw = pack->tm_yaw;

        UE_LOG(LogTemp, Warning, TEXT("recv login packet"));
    } break;
    case static_cast<int>(type::sc_move):
    {
        sc_move_packet* pack = reinterpret_cast<sc_move_packet*>(packet);
        if (pack->client_id != id) {
            tm_x = pack->x;
            tm_y = pack->y;
            tm_z = pack->z;
            tm_yaw = pack->yaw;

            Player_2->SetActorLocation(FVector(tm_x, tm_y, tm_z));
            Player_2->SetActorRotation(FRotator(0.f, tm_yaw, 0.f));

            UE_LOG(LogTemp, Warning, TEXT("Recv Player2 move packet!"));
            //double zz;
            //zz = Player_2->GetActorLocation().Z;
            //UE_LOG(LogTemp, Warning, TEXT("%d, %lf"), pack->client_id, zz);
            //UE_LOG(LogTemp, Warning, TEXT("recv p2 move packet"));
        } else {
            auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - pack->move_time;
            x = pack->x;
            y = pack->y;
            z = pack->z;
            yaw = pack->yaw;

            player->AddMovementInput(player_pos, new_axis_value);
            //UE_LOG(LogTemp, Warning, TEXT("recv my move packet"));
            UE_LOG(LogTemp, Warning, TEXT("Delay: %d"), delay);
        }        
    } break;
    }
}

void ACPP_Cobot_Controller::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void ACPP_Cobot_Controller::OnPossess(APawn* aPawn)
{
    Super::OnPossess(aPawn);

    UE_LOG(LogTemp, Warning, TEXT("OnPossess"));
}

void ACPP_Cobot_Controller::SetupInputComponent()
{
    Super::SetupInputComponent();

    // W, S Ű ������ Move_Forward() �Լ��� ����
    InputComponent->BindAxis(TEXT("MOVE_FORWARD"), this, &ACPP_Cobot_Controller::Move_Forward);
    InputComponent->BindAxis(TEXT("ROTATE"), this, &ACPP_Cobot_Controller::Rotate);
}

void ACPP_Cobot_Controller::Move_Forward(float NewAxisValue)
{
    new_axis_value = NewAxisValue;

    player = GetPawn();

    if (!player)
        return;

    RecvPacket();

    FRotator rotator_controller = GetControlRotation();
    FRotator rotator_forward = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, rotator_controller.Yaw);
    FVector forward_vector = UKismetMathLibrary::GetForwardVector(rotator_forward);

    // player_pos = forward_vector;
    if (forward_vector.X != 0.f) {
        player_pos.X = forward_vector.X / 0.1f;
    } else {
        player_pos.X = 0.f;
    }

    if (forward_vector.Y != 0.f) {
        player_pos.Y = forward_vector.Y / 0.1f;
    } else {
        player_pos.Y = 0.f;
    }

    if (forward_vector.Z != 0.f) {
        player_pos.Z = forward_vector.Z / 0.1f;
    } else {
        player_pos.Z = 0.f;
    }

    double player_yaw;
    if (rotator_controller.Yaw != 0.f) {
        player_yaw = rotator_controller.Yaw / 0.1f;
    } else {
        player_yaw = 0.f;
    }

    if (x != player_pos.X || y != player_pos.Y || z != player_pos.Z || yaw != player_yaw)
    {
        x = player_pos.X;
        y = player_pos.Y;
        z = player_pos.Z;
        yaw = player_yaw;

        last_move_time = std::chrono::high_resolution_clock::now();
        cs_move_packet pack;
        pack.size = sizeof(pack);
        pack.type = static_cast<char>(type::cs_move);
        pack.x = player->GetActorLocation().X;
        pack.y = player->GetActorLocation().Y;
        pack.z = player->GetActorLocation().Z;
        pack.yaw = player->GetActorRotation().Yaw;
        pack.move_time = static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());

        int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);
    }

    // ������ ������ ����
 // �������� ���⸦ ������
 // ���� �������� �ٲ���� ���� �������� ��
 /*
    ����� �ϴ� ���� �����̴� �� ��ġ �޾ƿ����� �ص� ��
    ���Ŀ� ��� �����̴� ������ ���� ���ľ� ��
 */

    //double yaw1 = player->GetActorRotation().Yaw;

    //UE_LOG(LogTemp, Warning, TEXT("Yaw: %lf"), yaw1);

    //if (x != player->GetActorLocation().X || y != player->GetActorLocation().Y || z != player->GetActorLocation().Z || yaw != player->GetActorRotation().Yaw)
    //{
    //    x = player->GetActorLocation().X;
    //    y = player->GetActorLocation().Y;
    //    z = player->GetActorLocation().Z;
    //    yaw = player->GetActorRotation().Yaw;

    //    cs_move_packet pack;
    //    pack.size = sizeof(pack);
    //    pack.type = static_cast<char>(type::cs_move);
    //    pack.x = x;
    //    pack.y = y;
    //    pack.z = z;
    //    pack.yaw = yaw;

    //    int ret = send(*sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

    //    // �̰� ��Ŷ �޴°�
    //    // ��Ŷ ���� ���� �����ؼ� ������ ��
    //    //RecvPacket();
    //}


    //RecvPacket();
}

void ACPP_Cobot_Controller::Rotate(float NewAxisValue)
{
    float delta_time = GetWorld()->GetDeltaSeconds();
    AddYawInput(delta_time * NewAxisValue * 20.0f);
}

bool ACPP_Cobot_Controller::Is_Set_IDPW(FString I, FString p)
{
    ///*����! ���⿡ ������ send������� false��� �ٷ� ���� true�� �Ʒ� �۾� �� ����*/
    ////ID = I;
    ////Passward = p;

    //wchar_t* input_id = TCHAR_TO_WCHAR(*I);
    //wchar_t* input_pw = TCHAR_TO_WCHAR(*p);

    //if (!instance->is_connect)
    //{
    //    instance->socket_mgr.ConnectServer(input_id);
    //    sock = instance->socket_mgr.socket;
    //    instance->is_connect = true;
    //}

    //// �������� ���Դٰ� �˷��ִ� ��
    //cs_login_packet login_pack;
    //login_pack.size = sizeof(login_pack);
    //login_pack.type = static_cast<char>(type::cs_login);
    ////wcscpy_s(login_pack.id, MAX_LOGIN_LEN, input_id);
    ////wcscpy_s(login_pack.passward, MAX_LOGIN_LEN, input_pw);

    //int ret = send(sock, reinterpret_cast<char*>(&login_pack), sizeof(login_pack), 0);


    //UE_LOG(LogTemp, Warning, TEXT("ID: %s, PW: %s"), input_id, input_pw);
    return true; 
}

/*
    Ŭ�� ���ƶ�
    ��ũ�� ����� �ϴ� �����ӵ� �ٲٰ� ���ÿ� ���۵Ǿ�� �ϴ°� �ٲ�� �� �� �ϴ�.
*/