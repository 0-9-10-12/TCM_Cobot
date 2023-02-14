#include "SESSION.h"

#include <iostream>

SESSION::SESSION()
	: state(STATE::NOT_INGAME)
{
	//ZeroMemory(&recv_over.over, sizeof(recv_over.over));
	//recv_over.wsabuf.buf = recv_over.buffer;
	//recv_over.wsabuf.len = BUF_SIZE;
	//recv_over.mode = IO_RECV;
}

SESSION::~SESSION()
{
}

void SESSION::recv_packet()
{
	/*
		������ ���� ring_buff() �����ְ� ���� �� ���ذ� �Ȱ���.
		�� ������ ����� ����??
		���� �����ִ� ������
		
		=> ���޾Ҵ�.
		- �̰� ������ �ƴ϶� ������ �׷��� �ްڴٴ� ���̴�.
		- �̹� ���� �Ϳ��� �߰��� �ްڴٴ� ���̴�.
		- �׷��� ���� ���� ���� �� �÷����� �ڸ��� ������ �ްڴٴ� ���̴�.
	*/
	DWORD recv_flag = 0;
	ZeroMemory(&recv_over.over, sizeof(recv_over.over));
	recv_over.wsabuf.buf = recv_over.buffer + ring_buff.diff();
	recv_over.wsabuf.len = BUF_SIZE - ring_buff.diff();
	recv_over.mode = IO_RECV;
	WSARecv(sock, &recv_over.wsabuf, 1, 0, &recv_flag, &recv_over.over, 0);
}

void SESSION::send_packet(char* packet)
{
	OVER_EX* over = new OVER_EX;
	over->wsabuf.buf = reinterpret_cast<char*>(packet);
	over->wsabuf.len = packet[0];
	ZeroMemory(&over->over, sizeof(over->over));
	over->mode = IO_SEND;
	memcpy(over->buffer, packet, packet[0]);
	int ret = WSASend(sock, &over->wsabuf, 1, 0, 0, &over->over, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		std::cout << err << std::endl;
	}
}

void SESSION::send_login_packet()
{
	sc_login_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(type::sc_login);
	pack.id = id;
	pack.x = x;
	pack.y = y;
	pack.z = z;
	
	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_add_player(int client_id, double x, double y, double z)
{
	sc_add_player_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<int>(type::sc_add_player);
	//pack.id = client_id;
	pack.x = x;
	pack.y = y;
	pack.z = z;

	send_packet(reinterpret_cast<char*>(&pack));
}
