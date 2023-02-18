#include "ServerMain.h"

#include <MSWSock.h> // AcceptEx()
#include <vector>
#include <thread>
#include <iostream>
#include <array>
#include <mutex>

#pragma comment(lib, "mswsock.lib")

#include "RingBuffer.h"
#include "SESSION.h"

std::array<SESSION, MAX_USER> clients;

ServerMain::ServerMain()
    : iocp_handle(nullptr)
    , server_sock(0)
    , client_sock(0)
{
}

ServerMain::~ServerMain()
{
    closesocket(server_sock);
    WSACleanup();
}

bool ServerMain::init()
{
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        GetLastError();
        return false;
    }

    iocp_handle = CreateIOCPHandle();
    if (nullptr == iocp_handle)
    {
        GetLastError();
        return false;
    }

    server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    sockaddr_in server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT_NUM);

    if (SOCKET_ERROR == bind(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)))
    {
        GetLastError();
        return false;
    }

    if (SOCKET_ERROR == listen(server_sock, SOMAXCONN))
    {
        GetLastError();
        return false;
    }

    if (!AssociateSocketWithIOCP(server_sock, 7777))
    {
        GetLastError();
        return false;
    }

    std::cout << "������ Ȱ��ȭ �Ǿ����ϴ�. ������ ��ٸ��ϴ�..." << std::endl;

    return true;
}

void ServerMain::server_main() // �������� ���� ���� ���� ��
{
    client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    
    overlapped.wsabuf.len = BUF_SIZE;
    overlapped.mode = IO_ACCEPT;

    AcceptEx(server_sock, client_sock, overlapped.buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, &overlapped.over);
    
    std::vector<std::thread> worker_threads;
    int thread_num = std::thread::hardware_concurrency();
    for (int i{}; i < thread_num; ++i)
    {
       worker_threads.emplace_back(std::thread(&ServerMain::worker_thread, this));
    }

    for (auto& th : worker_threads)
        th.join();
}

HANDLE ServerMain::CreateIOCPHandle()
{
    return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}

bool ServerMain::AssociateSocketWithIOCP(SOCKET sock, ULONG_PTR key)
{
    HANDLE h = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), iocp_handle, key, 0);
    return (h == iocp_handle);
}

void ServerMain::worker_thread()
{
    DWORD num_bytes;
    ULONG_PTR key;
    WSAOVERLAPPED* over = nullptr;

    while (true)
    {
        GetQueuedCompletionStatus(iocp_handle, &num_bytes, &key, &over, INFINITE);
        OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);

        if ((0 == num_bytes) && ((IO_RECV == over_ex->mode) || (IO_SEND == over_ex->mode)))
        {
            /*
                ���� ���� �ڵ� �߰��ؾ� ��
            */

            if (IO_SEND == over_ex->mode)
            {
                delete over_ex;
            }

            continue;
        }

        switch (over_ex->mode)
        {
        case IO_ACCEPT:
        {
            int client_id = get_client_id();

            std::cout << client_id << "��° ID�� ���� Ŭ�� �����Ͽ����ϴ�." << std::endl;

            if (client_id != -1)
            {
                {
                    std::lock_guard<std::mutex> lock(clients[client_id].lock);
                    clients[client_id].state = state::alloc;
                }
				/*
	                ���� ���� �ʱ�ȭ
	                SESSION�� ���� �߰��� ������ ���⼭ �ʱⰪ �����ֵ��� ����
                */
                clients[client_id].id = client_id;
                clients[client_id].sock = client_sock;
                clients[client_id].tm_id = 1 - client_id; /* ���⵵ ��Ī �ý��� ���� �ٲ�� �� */
                clients[client_id].x = -7700.f + static_cast<double>(client_id * 300);
                clients[client_id].y = 2180.f + static_cast<double>(client_id * 300);
                clients[client_id].z = 64.741069f; // +static_cast<double>(client_id * 300);
                clients[client_id].yaw = 0.f;

                // ������ �ٽ� �ޱ� ���ؼ�
                if (!AssociateSocketWithIOCP(client_sock, client_id))
                {
                    GetLastError();
                    continue;
                }

                clients[client_id].recv_packet();

                client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

            }
            else
            {
                std::cout << "Max user!\n";
            }

            overlapped.wsabuf.len = BUF_SIZE;
            overlapped.mode = IO_ACCEPT;

            AcceptEx(server_sock, client_sock, overlapped.buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, &overlapped.over);
        } break;
        case IO_RECV:
        {
            RingBuffer* ring_buff = &clients[key].ring_buff;
            char* p = over_ex->buffer;

            ring_buff->enqueue(p, num_bytes);

			while (p[0] <= ring_buff->diff() && !ring_buff->empty())
			{
				process_packet(p, static_cast<int>(key));
				ring_buff->move_read_pos(p[0]);
				p += p[0];
			} 

            if (0 < ring_buff->diff())
            {
                memcpy(over_ex->buffer, p, num_bytes);
            }

            clients[key].recv_packet();
        } break;
        case IO_SEND:
        {
            /*
                delete ����� �ϴ°� �´µ� �� �̰Ÿ� Ű�� ������ ������ �𸣰ڴ�.

                <�ذ�>
                �Ҵ������� ���� �޸� ������ �����Ϸ��� �ؼ� ���� ��������.
                send_packet() �Լ����� new OVER_EX�� �Ͽ� ������ �ذ��Ͽ���.
                �ٵ� �� new/delete�� ���ص� �ɰ� ���� �ϴ�.
            */
            delete over_ex;
        } break;
        }
    }
}

int ServerMain::get_client_id() // ������ client id ����
{
    /*
        ���� �ӽ÷� ������ ���̴�.
        ������ lock �ɾ��ְ� �ؾ��Ѵ�.
        �׽�Ʈ�� ���� �ӽ� ����
        ���Ŀ� ���ľ� �Ѵ�. 
    */

    for (int i{}; i < MAX_USER; ++i)
    {
        std::lock_guard<std::mutex> lock{ clients[i].lock };
        if (clients[i].state == state::free)
            return i;
    }

    return -1;
}

void ServerMain::process_packet(char* packet, int client_id)
{
    switch (packet[1]) 
    {
    case static_cast<int>(type::cs_login):
    {
        cs_login_packet* pack = reinterpret_cast<cs_login_packet*>(packet);
        printf("%d client�κ��� login packet�� �޾ҽ��ϴ�.\n", client_id);
       /* wchar_t player_id[MAX_LOGIN_LEN], player_pw[MAX_LOGIN_LEN];
        wcscpy_s(player_id, MAX_LOGIN_LEN, pack->id);
        wcscpy_s(player_pw, MAX_LOGIN_LEN, pack->passward);*/
        //printf("%d client�� ID: %s, PW: %s\n", client_id, player_id, player_pw);
        {
            std::lock_guard<std::mutex> lock{ clients[client_id].lock };
            clients[client_id].state = state::ingame;
        }

        // MMORPG�� �ƴ� 2�� ���� �����̱� ������ ������ ��ٸ���.
        // �̰� �������� ��Ī �ý����� ����� �� ����� �ڵ�
        while (true)
        {
            //std::lock_guard<std::mutex> lock(clients[client_id].lock);
            //std::lock_guard<std::mutex> llock(clients[clients[client_id].tm_id].lock);
            if (state::ingame == clients[client_id].state && state::ingame == clients[clients[client_id].tm_id].state)
                break;
        }

        // ���� ��ǥ ����ϰ� �ϱ�
        clients[client_id].tm_x = clients[clients[client_id].tm_id].x;
        clients[client_id].tm_y = clients[clients[client_id].tm_id].y;
        clients[client_id].tm_z = clients[clients[client_id].tm_id].z;
        clients[client_id].tm_yaw = clients[clients[client_id].tm_id].yaw;

        // �� �� ������ �Ǿ�� �α����� ��
        clients[client_id].send_login_packet();
    } break;
    case static_cast<int>(type::cs_move):
    {
        cs_move_packet* pack = reinterpret_cast<cs_move_packet*>(packet);
        printf("%d ID�� ���� Ŭ���̾�Ʈ�� %f, %f, %f �� �̵��Ͽ����ϴ�.\n", client_id, pack->x, pack->y, pack->z);
        clients[client_id].x = pack->x;
        clients[client_id].y = pack->y;
        clients[client_id].z = pack->z;
        clients[client_id].yaw = pack->yaw;
        clients[clients[client_id].tm_id].tm_x = pack->x;
        clients[clients[client_id].tm_id].tm_y = pack->y;
        clients[clients[client_id].tm_id].tm_z = pack->z;
        clients[clients[client_id].tm_id].tm_yaw = pack->yaw;
        // ������ ���� �ٸ� Ŭ������� �����ֱ�
        // 23.2.11
        // ����� Ŭ�󿡼� �������� �� �� �� ������ ��ġ�� �����ش�.
        // ȥ�� �ϴ� ���̶�� ����� ������ ��Ƽ���ӿ����� ���ÿ� �������� ���� �߿��ϴ�.
        // �̷��� �ϸ� �ٸ� �ʿ����� ������ ���� �� �ۿ� ����.
        // ������ ���� ������ ������ �������� ���ÿ� �����ֵ��� �ؾ� �Ѵ�.
        // <���� ����>
        // Ŭ�� CPP���� �����ϸ� Ŭ�� �Ѹ� �� 2���� ������ �ǰ� �ִ�.
        // ���� ���ӵ� �ȵǾ� �ִ� Ŭ�� INGAME���� ǥ�õǴ� ������ �ִ�.
        // �̰� Ŭ���ʿ��� �����ؾ� �� �� �ϴ�.
        clients[client_id].send_move_packet(client_id); // ������ Ŭ������ ������
        clients[clients[client_id].tm_id].send_move_packet(client_id); // ��� Ŭ������ ������
    } break;
    case static_cast<int>(type::cs_logout):
    {
        std::cout << client_id << "client�� logout �Ͽ����ϴ�.\n";
        closesocket(clients[client_id].sock);
        std::lock_guard<std::mutex> lock(clients[client_id].lock);
        clients[client_id].state = state::free;
        if (clients[client_id].state == state::free)
            std::cout << client_id << "client state is free\n";
    } break;
    }
}

