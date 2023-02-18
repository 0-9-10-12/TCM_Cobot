// Fill out your copyright notice in the Description page of Project Settings.

#include "../../../server/server/protocol.h"

//#include <wchar.t>

#include "CPP_Cobot_Controller.h"
#include "CPP_Cobot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

ACPP_Cobot_Controller::ACPP_Cobot_Controller()
    : x(0.f), y(0.f), z(0.f), yaw(0.f)
    , tm_x(0.f), tm_y(0.f), tm_z(0.f), tm_yaw(0.f)
{
    UE_LOG(LogTemp, Warning, TEXT("Start connect server!"));
    //int retval;

    // PrimaryActorTick.bCanEverTick = false;

    // ���� �ʱ�ȭ

    WSADATA wsaData;
    int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);   // Winsock �ʱ�ȭ
    //if (nRet != 0) return false;

    //// ���� ����
    sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //if (Socket == INVALID_SOCKET) return false;

    // IP, Port ���� �Է�
    SOCKADDR_IN stServerAddr;
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_port = htons(7000);
    stServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // ����
    nRet = connect(sock, (sockaddr*)&stServerAddr, sizeof(sockaddr));
    //if (nRet == SOCKET_ERROR) return false;

    u_long nonBlockingMode = 1;
    ioctlsocket(sock, FIONBIO, &nonBlockingMode); // sock�� ����ŷ ���� ����

    cs_login_packet login_pack;
    login_pack.size = sizeof(login_pack);
    login_pack.type = static_cast<char>(type::cs_login);

    int ret = send(sock, reinterpret_cast<char*>(&login_pack), sizeof(login_pack), 0);

    UE_LOG(LogTemp, Warning, TEXT("Success server connect"));

}

ACPP_Cobot_Controller::~ACPP_Cobot_Controller()
{
    cs_logout_packet pack;
    pack.size = sizeof(pack);
    pack.type = static_cast<int>(type::cs_logout);

    send(sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

    closesocket(sock);
    WSACleanup();
}

void ACPP_Cobot_Controller::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("�ں�����"));

    Player_2 = GetWorld()->SpawnActor<ACPP_Cobot>(ACPP_Cobot::StaticClass(), FVector(tm_x, tm_y, tm_z), FRotator(0.0f, 0.f, 0.0f));
}

void ACPP_Cobot_Controller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RecvPacket();
    //UE_LOG(LogTemp, Warning, TEXT("Tick"));
}

void ACPP_Cobot_Controller::RecvPacket()
{
    char buff[BUF_SIZE];

    int ret = recv(sock, reinterpret_cast<char*>(&buff), BUF_SIZE, 0);
    if (ret <= 0)
    {
        GetLastError();
        std::cout << "recv() fail!" << std::endl;
        return;
    }

    ring_buff.enqueue(buff, ret);

    int read_point = 0;
    while (buff[read_point] <= ring_buff.diff() && !ring_buff.empty())
    {
        ProcessPacket(buff);
        ring_buff.move_read_pos(buff[0]);
        read_point = buff[read_point];
    }
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

            //UE_LOG(LogTemp, Warning, TEXT("recv p2 move packet"));
        } else {
            x = pack->x;
            y = pack->y;
            z = pack->z;
            yaw = pack->yaw;
            //UE_LOG(LogTemp, Warning, TEXT("recv my move packet"));
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

    APawn* player = GetPawn();

    if (!player)
        return;

    FRotator rotator_controller = GetControlRotation();
    FRotator rotator_forward = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, rotator_controller.Yaw);
    FVector forward_vector = UKismetMathLibrary::GetForwardVector(rotator_forward);

    player->AddMovementInput(forward_vector, NewAxisValue);

    // ������ ������ ����
 // �������� ���⸦ ������
 // ���� �������� �ٲ���� ���� �������� ��
 /*
    ����� �ϴ� ���� �����̴� �� ��ġ �޾ƿ����� �ص� ��
    ���Ŀ� ��� �����̴� ������ ���� ���ľ� ��
 */

    //double yaw1 = player->GetActorRotation().Yaw;

    //UE_LOG(LogTemp, Warning, TEXT("Yaw: %lf"), yaw1);

    if (x != player->GetActorLocation().X || y != player->GetActorLocation().Y || z != player->GetActorLocation().Z || yaw != player->GetActorRotation().Yaw)
    {
        x = player->GetActorLocation().X;
        y = player->GetActorLocation().Y;
        z = player->GetActorLocation().Z;
        yaw = player->GetActorRotation().Yaw;

        cs_move_packet pack;
        pack.size = sizeof(pack);
        pack.type = static_cast<char>(type::cs_move);
        pack.x = x;
        pack.y = y;
        pack.z = z;
        pack.yaw = yaw;

        int ret = send(sock, reinterpret_cast<char*>(&pack), sizeof(pack), 0);

        // �̰� ��Ŷ �޴°�
        // ��Ŷ ���� ���� �����ؼ� ������ ��
        //RecvPacket();
    }


    //RecvPacket();
}

void ACPP_Cobot_Controller::Rotate(float NewAxisValue)
{
    float delta_time = GetWorld()->GetDeltaSeconds();
    AddYawInput(delta_time * NewAxisValue * 20.0f);
}

bool ACPP_Cobot_Controller::Is_Set_IDPW(FString I, FString p)
{
    /*����! ���⿡ ������ send������� false��� �ٷ� ���� true�� �Ʒ� �۾� �� ����*/
    //ID = I;
    //Passward = p;

    //const wchar_t* input_id = TCHAR_TO_WCHAR(*I);
    //const wchar_t* input_pw = TCHAR_TO_WCHAR(*p);

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
