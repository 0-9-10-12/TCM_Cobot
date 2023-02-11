#include "SESSION.h"

#include <iostream>

SESSION::SESSION()
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
	OVER_EX over;
	over.wsabuf.len = packet[0];
	over.wsabuf.buf = over.buffer;
	ZeroMemory(&over.over, sizeof(over.over));
	over.mode = IO_SEND;
	memcpy(over.buffer, packet, packet[0]);
	int ret = WSASend(sock, &over.wsabuf, 1, 0, 0, &over.over, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		std::cout << err << std::endl;
	}
}
