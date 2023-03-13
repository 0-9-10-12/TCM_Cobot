#pragma once
#include "OBJECT.h"
#include "pch.h"

#include <mutex>

#include "RingBuffer.h"

enum class state { free, alloc, ingame };

class SESSION : public OBJECT
{
public:
	state		state;
	SOCKET		sock;
	int			id;				// ���� id
	wchar_t		name[MAX_LOGIN_LEN];
	double		x, y, z;
	double		yaw;
	RingBuffer	ring_buff;
	
	std::mutex	state_lock;
	std::mutex	match_lock;

	OVER_EX		recv_over;

	// �����Ͻø� �˱� ���� ����
	int			last_move_time;

	int			tm_id;	// ���� id
	double		tm_x, tm_y, tm_z;
	double		tm_yaw;

	int			tm_last_mvoe_time;

	// ��Ŷ �������� ���� �κ� �̰� ���߿� �� �����ؾ� ��
	int			prev_remain;
	int			prev_packet_size;
	char		prev_packet_buff[BUF_SIZE];

public:
	void recv_packet();
	void send_packet(char* packet);
	void send_login_packet();
	void send_move_packet(int client_id);
	void send_rotate_packet(int client_id);

public:
	SESSION();
	~SESSION();
};

