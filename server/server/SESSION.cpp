#include "SESSION.h"

#include <iostream>
#include <memory>

SESSION::SESSION()
	: state(state::free)
	, sock(0)
	, id(-1)
	, location(-1.f, -1.f, -1.f)
	, yaw(0.f)
	, tm_id(-1)
	, tm_location(-1.f, -1.f, -1.f)
	, tm_yaw(0.f)
	, prev_remain(0)

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
	// std::atomic<std::shared_ptr<OVER_EX>> over;
	// std::shared_ptr<OVER_EX> over(new OVER_EX());
	// OVER_EX* over = new OVER_EX;
	// over->wsabuf.buf = reinterpret_cast<char*>(packet);

	over->wsabuf.buf = over->buffer;
	over->wsabuf.len = packet[0];
	over->mode = IO_SEND;
	ZeroMemory(&over->over, sizeof(over->over));
	memcpy(over->buffer, packet, packet[0]);
	int ret = WSASend(sock, &over->wsabuf, 1, 0, 0, &over->over, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		std::cout << "err: " << err << std::endl;
	}
}

void SESSION::send_enter_packet()
{
	sc_login_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::sc_login);
	pack.id = id;
	//pack.x = x;
	//pack.y = y;
	//pack.z = z;
	//pack.yaw = yaw;
	//pack.tm_x = tm_x;
	//pack.tm_y = tm_y;
	//pack.tm_z = tm_z;
	//pack.tm_yaw = tm_yaw;
	
	send_packet(reinterpret_cast<char*>(&pack));

	printf("%d���� enter packet�� ���½��ϴ�.\n", id);
}

void SESSION::send_left_move_packet(int client_id)
{
	sc_move_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<int>(packet_type::sc_move);
	pack.client_id = client_id;
	pack.direction = direction::left;
	if (client_id == id) {
		pack.location = location;
		pack.current = current_left;
		pack.time = time_left;
		pack.yaw = yaw;
	} else if (client_id == tm_id) {
		pack.location = tm_location;
		pack.current = tm_current_left;
		pack.time = tm_time_left;
		pack.yaw = tm_yaw;
	}

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_right_move_packet(int client_id)
{
	sc_move_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<int>(packet_type::sc_move);
	pack.client_id = client_id;
	pack.direction = direction::right;
	if (client_id == id) {
		pack.location = location;
		pack.current = current_right;
		pack.time = time_right;
		pack.yaw = yaw;
	} else if (client_id == tm_id) {
		pack.location = tm_location;
		pack.current = tm_current_right;
		pack.time = tm_time_right;
		pack.yaw = tm_yaw;
	}

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_push_maze_button_packet(direction direction)
{
	sc_button_packet pack;
	pack.size = sizeof(pack);
	if (direction::forward == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_maze_forward);
	} else if (direction::back == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_maze_back);
	} else if (direction::left == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_maze_left);
	} else if (direction::right == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_maze_right);
	}
	
	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_push_gear_button_packet(direction direction)
{
	sc_button_packet pack;
	pack.size = sizeof(pack);
	if (direction::forward == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_gear_forward);
	} else if (direction::back == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_gear_back);
	} else if (direction::left == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_gear_left);
	} else if (direction::right == direction) {
		pack.type = static_cast<char>(packet_type::sc_push_button_gear_right);
	}

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_elevator_ok_packet()
{
	sc_elevator_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::sc_elevator);

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_forklift_button(packet_type type)
{
	sc_button_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(type);

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_cobot_button(packet_type type)
{
	sc_button_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(type);

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_board_color(int color)
{
	sc_board_color_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::sc_board_color);
	pack.color = color;

	send_packet(reinterpret_cast<char*>(&pack));
}

void SESSION::send_logout_packet()
{
	sc_logout_packet pack;
	pack.size = sizeof(pack);
	pack.type = static_cast<char>(packet_type::sc_logout);

	send_packet(reinterpret_cast<char*>(&pack));
}

// 23.4.14
// 1. OVER_EX ���� �Ҵ����ִ� �κп��� �޸� ���� �ִ� ���ϴ�.
// 2. std::atomic<std::shared_ptr>�� ���ؼ� �� �����غ���
//		- C++20���� ������� �ߴ� atomic_shared_ptr���� Ȯ���غ���
//		- �ٸ��� � �κ��� �ٸ��� �˾ƺ���(����, ���, ������, ...)
//		- �����ϴٰ� Ȯ�εǸ� �����Ű��(�̰Ͷ��� ��ǳ ����,,,)
// 3. ��Ī���� �κе� �ٽ� ������� �Ѵ�.
//		- lock�� ����ϰ� �ִµ� �� ���� ����� ���� ã�ƺ���
//		- �˰��򿡵� �̻��� ������ Ȯ���غ���
// 4. ������ ����� ��Ŷ ������ �����ϱ�
//		- ������ �ִµ��ؼ� ������� �ʾ���
//		- ��Ȯ�� ������ �ľ��ϰ� ��ġ��                                    