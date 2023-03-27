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
	vector_d3	location;
	vector_d3	current_left;
	vector_d3	current_right;
	float		time_left;
	float		time_right;

	double		yaw;
	RingBuffer	ring_buff;
	
	std::mutex	state_lock;
	std::mutex	match_lock;

	OVER_EX		recv_over;

	// �����Ͻø� �˱� ���� ����
	int			last_move_time;

	int			tm_id;	// ���� id
	vector_d3	tm_location;
	vector_d3	tm_current_left;
	vector_d3	tm_current_right;
	float		tm_time_left;
	float		tm_time_right;

	double		tm_yaw;

	int			tm_last_mvoe_time;

	// ��Ŷ �������� ���� �κ� �̰� ���߿� �� �����ؾ� ��
	int			prev_remain;
	int			prev_packet_size;
	char		prev_packet_buff[BUF_SIZE];

public:
	void recv_packet();
	void send_packet(char* packet);
	void send_enter_packet();
	void send_left_move_packet(int client_id);
	void send_right_move_packet(int client_id);
	//void send_move_packet(int client_id);
	//void send_rotate_packet(int client_id);

public:
	SESSION();
	~SESSION();
};

