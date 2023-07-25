#include "ServerMain.h"

#include <MSWSock.h> // AcceptEx()
#include <vector>
#include <thread>
#include <iostream>
#include <chrono>
#include <array>
#include <mutex>
#include <memory>
#include <time.h>
#include <concurrent_priority_queue.h>
#include <sqlext.h>

#pragma comment(lib, "mswsock.lib")

#include "RingBuffer.h"
#include "SESSION.h"
#include "RoomManager.h"

std::array<SESSION, MAX_USER> clients;
std::array<RoomManager, MAX_ROOM> normal_rooms;

ServerMain::ServerMain()
	: iocp_handle(nullptr)
	, server_sock(0)
	, client_sock(0)
{
}

ServerMain::~ServerMain()
{
	DB_disconnect();
	closesocket(server_sock);
	WSACleanup();
}

bool ServerMain::init()
{
	setlocale(LC_ALL, "KOREAN");

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		GetLastError();
		return false;
	}

	iocp_handle = CreateIOCPHandle();
	if (nullptr == iocp_handle) {
		GetLastError();
		return false;
	}

	server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	sockaddr_in server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT_NUM);

	if (SOCKET_ERROR == bind(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr))) {
		GetLastError();
		return false;
	}

	if (SOCKET_ERROR == listen(server_sock, SOMAXCONN)) {
		GetLastError();
		return false;
	}

	if (!AssociateSocketWithIOCP(server_sock, 7777)) {
		GetLastError();
		return false;
	}
	
	setlocale(LC_ALL, "korean");

	// DB O
	// DB ����
	sqlret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenv); // ȯ�� �ڵ� �Ҵ�

	if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
		// ODBC ���� ȯ�� �Ӽ� ����
		sqlret = SQLSetEnvAttr(sqlenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0); 
		
		if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
			// ���� �ڵ� �Ҵ�
			sqlret = SQLAllocHandle(SQL_HANDLE_DBC, sqlenv, &sqldbc); 
			
			if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
				// login timeout 5�ʷ� ����
				SQLSetConnectAttr(sqldbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0); 
				
				// ������ ������ �ҽ�
				sqlret = SQLConnect(sqldbc, (SQLWCHAR*)L"COBOT_2023", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0); 
				
				if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO)
					sqlret = SQLAllocHandle(SQL_HANDLE_STMT, sqldbc, &sqlstmt); // ���� �ڵ� �Ҵ�
			}
		}
	}
	// --------------------

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
		worker_threads.emplace_back(std::thread(&ServerMain::worker_thread, this));
	std::thread timer_thread = std::thread{ &ServerMain::do_timer_thread,this };
	timer_thread.join();
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

void disconnect(int c_id)
{
	for (auto& pl : clients) {
		{
			std::lock_guard<std::mutex> ll(pl.sock_lock);
			if (state::ingame != pl.state) continue;
		}
		{
			std::lock_guard<std::mutex> ll(clients[pl.tm_id].sock_lock);
			if (state::ingame != clients[pl.tm_id].state) continue;
		}
		sc_logout_packet p;
		p.size = sizeof(p);
		p.type = static_cast<char>(packet_type::sc_logout);
		clients[pl.tm_id].send_logout_packet();
	}
	closesocket(clients[c_id].sock);

	std::lock_guard<std::mutex> ll(clients[c_id].sock_lock);
	clients[c_id].state = state::free;
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
		//OVER_EX* over1 = reinterpret_cast<OVER_EX*>(over);
		//std::shared_ptr<OVER_EX> over_ex = std::make_shared<OVER_EX>(over1);

		if ((0 == num_bytes) && ((IO_RECV == over_ex->mode) || (IO_SEND == over_ex->mode))) {
			
			disconnect(static_cast<int>(key));

			std::cout << "num bytes: " << num_bytes << "over_ex mode: IO_RECV" << "over_ex mode: IO_SEND" << std::endl;

			if (IO_SEND == over_ex->mode)
				delete over_ex;

				continue;
		}

		switch (over_ex->mode)
		{
		case IO_ACCEPT:
		{
			int client_id = get_client_id();

			printf("%d�� ID�� ���� Ŭ�� �����߽��ϴ�.\n", client_id);

			if (client_id != -1) {
				{
					std::lock_guard<std::mutex> lock(clients[client_id].state_lock);
					clients[client_id].state = state::alloc;
				}
				/*
					���� ���� �ʱ�ȭ
					SESSION�� ���� �߰��� ������ ���⼭ �ʱⰪ �����ֵ��� ����
				*/
				clients[client_id].id = client_id;
				clients[client_id].sock = client_sock;
				clients[client_id].tm_id = -1;

				// ������ �ٽ� �ޱ� ���ؼ�
				if (!AssociateSocketWithIOCP(client_sock, client_id)) {
					GetLastError();
					continue;
				}

				clients[client_id].recv_packet();

				client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
			}
			else {
				std::cout << "Max user!\n";
			}

			overlapped.wsabuf.len = BUF_SIZE;
			overlapped.mode = IO_ACCEPT;

			AcceptEx(server_sock, client_sock, overlapped.buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, &overlapped.over);
		} break;
		case IO_RECV:
		{
			int remain_data = num_bytes + clients[key].prev_remain;
			char* p = over_ex->buffer;

			while (remain_data > 0)
			{
				int packet_size = p[0];
				if (0 == packet_size) {
					std::cout << "packet size: 0!!!!!!!!!!!!!!!!\n";

					clients[key].prev_remain = 0;

					int tm_id = clients[key].tm_id;
					clients[key].send_cobot_button(packet_type::sc_push_button_cobot_green);
					clients[tm_id].send_cobot_button(packet_type::sc_push_button_cobot_green);

					clients[key].send_left_move_packet(key); // ������ Ŭ������ ������
					clients[tm_id].send_left_move_packet(key); // ��� Ŭ������ ������

					clients[key].send_right_move_packet(key); // ������ Ŭ������ ������
					clients[tm_id].send_right_move_packet(key); // ��� Ŭ������ ������

					clients[key].send_left_move_packet(tm_id); // ������ Ŭ������ ������
					clients[tm_id].send_left_move_packet(tm_id); // ��� Ŭ������ ������

					clients[key].send_right_move_packet(tm_id); // ������ Ŭ������ ������
					clients[tm_id].send_right_move_packet(tm_id); // ��� Ŭ������ ������

					clients[key].recv_packet();

					return;
				}
				if (packet_size <= remain_data) {
					process_packet(p, static_cast<int>(key));
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				} else break;
			}

			clients[key].prev_remain = remain_data;

			if (remain_data > 0)
				memcpy(over_ex->buffer, p, remain_data);

			clients[key].recv_packet();

			//RingBuffer* ring_buff = &clients[key].ring_buff;
			//char* p = over_ex->buffer;

			//// std::cout << "num_bytes: " << num_bytes << std::endl;

			//int ret = ring_buff->enqueue(p, num_bytes);
			//if (static_cast<int>(error::full_buffer) == ret) {
			//	std::cout << "err: ring buffer is full\n";
			//	return;
			//} else if (static_cast<int>(error::in_data_is_too_big) == ret) {
			//	std::cout << "err: in data is too big\n";
			//	return;
			//}

			//while (ring_buff->remain_data() != 0 && ring_buff->peek_front() <= ring_buff->remain_data())
			//{
			//	//std::cout << "remain data size: " << ring_buff->remain_data() << std::endl;
			//	char pack_size = ring_buff->peek_front();
			//	char dequeue_data[BUFFER_SIZE];

			//	//std::cout << "pack size: " << (int)pack_size << std::endl;

			//	ret = ring_buff->dequeue(reinterpret_cast<char*>(&dequeue_data), pack_size);
			//	//std::cout << "dequeue size: " << ret << std::endl;

			//	if (static_cast<int>(error::no_data_in_buffer) == ret) {
			//		std::cout << "err: no data in buffer\n";
			//		break;
			//	} else if (static_cast<int>(error::out_data_is_too_big) == ret) {
			//		std::cout << "ret: " << ret << ", pack size: " << pack_size << std::endl;
			//		std::cout << "p[0]: " << p[0] << std::endl;
			//		std::cout << "err: out data is too big\n";
			//		break;
			//	}

			//	process_packet(dequeue_data, static_cast<int>(key));

			//	p += pack_size;
			//}

			//if (0 < ring_buff->remain_data())
			//	memcpy(over_ex->buffer, p, ring_buff->remain_data());

			//clients[key].recv_packet();
		} break;
		case IO_SEND:
		{
			delete over_ex;
		} break;
		case MOVE_CAR:
		{
			// std::cout << "move car enter\n";
			using namespace std;

			static float direction = 0.0;
			static float acceleration = 0.0;

			if (clients[key].move_car) {
				if (clients[clients[key].tm_id].move_car) {
					direction = 0.0;
					acceleration += 0.1;

					if (acceleration >= 20.f)
						acceleration = 20.f;
				} else {
					acceleration = 0.0;

					if (key < clients[key].tm_id)
						direction = -1.5;
					else
						direction = 1.5;
				}

				// std::cout << key << " client is push? " << clients[key].move_car << ", " << clients[key].tm_id << " client is push? " << clients[clients[key].tm_id].move_car << std::endl;

				// ���⼭ ȣ��Ʈ���׸� ������ �Ѵ�.
				int host_id = normal_rooms[clients[key].room_id].get_host_id();
				std::cout << "host id: " << host_id;
				clients[host_id].send_move_car_packet(direction, acceleration);
				/*clients[key].send_move_car_packet(direction, acceleration);
				clients[clients[key].tm_id].send_move_car_packet(direction, acceleration);*/

				TIMER_EVENT event;
				event.object_id = key;
				event.event_type = event_type::move_car;
				event.execute_time = std::chrono::system_clock::now() + 1ms;

				timer_queue.push(event);
			}
		} break;
		}
	}
}

int ServerMain::get_client_id() // ������ client id ����
{
	for (int i{}; i < MAX_USER; ++i)
	{
		std::lock_guard<std::mutex> lock{ clients[i].state_lock };
		if (clients[i].state == state::free)
			return i;
	}

	return -1;
}

int ServerMain::get_normal_room_id()
{
	for (int i{}; i < MAX_ROOM; ++i) {
		if (!normal_rooms[i].is_use()) {
			normal_rooms[i].use_room();
			return i;
		}
	}
}

void ServerMain::process_packet(char* packet, int client_id)
{
	switch (packet[1])
	{
	case static_cast<int>(packet_type::cs_signup):
	{
		std::cout << "recv cs signup packet: " << client_id << " client\n";

		cs_signup_packet* pack = reinterpret_cast<cs_signup_packet*>(packet);
		
		wprintf(L"id: %s, pw: %s\n", pack->id, pack->pw);

		// DB O
		// ȸ������
		wchar_t query_str[256];
		//wsprintf(query_str, L"SELECT * FROM user_table WHERE EXISTS id='%s'", pack->id);
		//wsprintf(query_str, L"SELECT id FROM user_table WHERE id='%s'", pack->id);
		wsprintf(query_str, L"EXEC select_user_by_name '%s'", pack->id);
		sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
		if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
			// �ߺ��Ǵ� ���̵��̹Ƿ� ȸ������ ���� ��Ŷ ������
			std::cout << "sql���� ������\n";
			SQLWCHAR sz_id[MAX_NAME];
			SQLLEN cb_id = 0;

			sqlret = SQLBindCol(sqlstmt, 1, SQL_C_WCHAR, sz_id, MAX_NAME, &cb_id);

			for (int i = 0; ; ++i) { // ������ �����͸� ������.
				sqlret = SQLFetch(sqlstmt);
				if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) { // ȸ������ ����
					clients[client_id].send_signup_fail_packet();
					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
					return;
				} else { // ȸ������ ����
					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
					std::cout << "ȸ������ �������� ����\n";
					//wsprintf(query_str, L"INSERT INTO user_table(id, pw, stage) VALUES('%s', '%s', 1)", pack->id, pack->pw);
					wsprintf(query_str, L"EXEC insert_user_info '%s', '%s', 1", pack->id, pack->pw);
					sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
					if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO)
						clients[client_id].send_signup_success_packet();
					else
						show_error(sqlstmt, SQL_HANDLE_STMT, sqlret);					

					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
					return;
				}
			}

		} else {
			std::cout << "DB err\n";
		}
	} break;
	case static_cast<int>(packet_type::cs_login):
	{
		cs_login_packet* pack = reinterpret_cast<cs_login_packet*>(packet);
		printf("%d client�κ��� login packet�� �޾ҽ��ϴ�.\n", client_id);

		wcscpy_s(clients[client_id].name, MAX_NAME, pack->id);
		wprintf(L"%d client�� name: %s,\n", client_id, clients[client_id].name);

		{
			std::lock_guard<std::mutex> lock{ clients[client_id].state_lock };
			clients[client_id].state = state::ingame;
		}	

		// --- �α��� ��� X ���� ---
		// clients[client_id].send_login_success_packet();
		//if (client_id == 0) clients[client_id].tm_id = 1;
		//else if (client_id == 1) clients[client_id].tm_id = 0;
		// ------------------------
		
		// --- �α��� ��� O ���� ---
		wchar_t query_str[256];
		wsprintf(query_str, L"EXEC select_user_by_name '%s'", pack->id);
		sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
		if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
			SQLWCHAR user_id[MAX_NAME], user_pw[MAX_NAME];
			SQLINTEGER user_stage;
			SQLLEN cb_id = 0, cb_pw = 0, cb_stage;

			sqlret = SQLBindCol(sqlstmt, 1, SQL_C_WCHAR, user_id, MAX_NAME, &cb_id);
			sqlret = SQLBindCol(sqlstmt, 2, SQL_C_WCHAR, user_pw, MAX_NAME, &cb_pw);
			sqlret = SQLBindCol(sqlstmt, 3, SQL_C_LONG, &user_stage, 4, &cb_stage);

			for (int i = 0; ; ++i) { // ������ �����͸� ������.
				sqlret = SQLFetch(sqlstmt);
				if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
					char temp_pw_str[MAX_NAME];
					strcpy_s(temp_pw_str, ConvertWCtoC(user_pw));
					for (int i{}; i < strlen(temp_pw_str); ++i) {
						if (temp_pw_str[i] == ' ') {
							temp_pw_str[i] = '\0';
							break;
						}
					}

					clients[client_id].db_stage = user_stage;

					if (strcmp(ConvertWCtoC(pack->passward), temp_pw_str) == 0) {
						clients[client_id].send_login_success_packet();

						SQLCloseCursor(sqlstmt);
						SQLFreeStmt(sqlstmt, SQL_UNBIND);
						break;
					} else {
						clients[client_id].send_login_fail_packet();

						SQLCloseCursor(sqlstmt);
						SQLFreeStmt(sqlstmt, SQL_UNBIND);
						break;
					}
				} else {
					clients[client_id].send_login_fail_packet();

					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
					break;
				}
			}
		} else {
			SQLCloseCursor(sqlstmt);
			SQLFreeStmt(sqlstmt, SQL_UNBIND);
		}
		// ------------------------------

		/*bool is_matching = matching(client_id);
		if (!is_matching)
			std::cout << "�̹� �ٸ� ������ �ֽ��ϴ�." << std::endl;

		set_team_position(client_id);*/
	} break;
	case static_cast<int>(packet_type::cs_create_room):
	{
		std::cout << "cs_create_room\n";

		cs_create_room_packet* pack = reinterpret_cast<cs_create_room_packet*>(packet);

		int room_id;

		room_id = get_normal_room_id();

		std::cout << "create rood id: " << room_id << std::endl;

		clients[client_id].room_id = room_id;

		normal_rooms[room_id].set_room_name(pack->room_name);
		normal_rooms[room_id].set_host_id(client_id);
		normal_rooms[room_id].set_number_of_people(0);
		normal_rooms[room_id].set_stage(pack->stage);

		//clients[client_id].send_create_room_ok(normal_rooms[room_id].get_room_name());
	} break;
	case static_cast<int>(packet_type::cs_delete_room):
	{
		cs_delete_room_packet* pack = reinterpret_cast<cs_delete_room_packet*>(packet);

		int room_id = pack->room_id;

		normal_rooms[room_id].exit_room();

		clients[client_id].send_delete_room_packet();
	} break;
	case static_cast<int>(packet_type::cs_enter_room):
	{
		cs_enter_room_packet* pack = reinterpret_cast<cs_enter_room_packet*>(packet);

		int room_id = pack->room_id;
		if (!normal_rooms[room_id].is_use()) {
			clients[client_id].send_enter_room_fail_packet();
			std::cout << client_id << " client���� enter fail ��Ŷ�� ���½��ϴ�.\n";
			return;
		}

		int host_id = normal_rooms[room_id].get_host_id();

		std::cout << client_id << " client enter! host is " << host_id << "client!\n";

		clients[client_id].room_id = room_id;
		normal_rooms[room_id].set_number_of_people(normal_rooms[room_id].get_number_of_people() + 1);

		if (client_id != host_id) { // ȣ��Ʈ�� �ƴ� ����� ��Ŷ�� ������ ���
			normal_rooms[room_id].set_team_id(client_id);

			clients[client_id].tm_id = host_id; // ȣ��Ʈ�� �������� ����
			clients[host_id].tm_id = client_id; // ���� ���� ����� ȣ��Ʈ�� �������� ����

			std::cout << client_id << ", " << host_id << " matching!" << std::endl;
			std::cout << "room id: " << room_id << std::endl;
			std::cout << "room count: " << normal_rooms[room_id].get_number_of_people() << std::endl;
		}

		if (normal_rooms[room_id].get_number_of_people() == 2) {
			clients[client_id].send_game_start_packet(normal_rooms[pack->room_id].get_stage());
			clients[clients[client_id].tm_id].send_game_start_packet(normal_rooms[pack->room_id].get_stage());
		}
	} break;
	case static_cast<int>(packet_type::cs_show_room_list):
	{
		cs_show_room_list_packet* pack = reinterpret_cast<cs_show_room_list_packet*>(packet);

		std::cout << "show normal room list\n";
		/*for (int i{}; i < MAX_ROOM; ++i)
		   if (normal_rooms[i].is_use())
			  clients[client_id].send_show_room_list_packet(normal_rooms[i].get_room_name(), clients[normal_rooms[i].get_host_id()].name, i, normal_rooms[i].get_stage());*/

		clients[client_id].send_show_room_list_packet(normal_rooms[0].get_room_name(), clients[normal_rooms[0].get_host_id()].name, 0, normal_rooms[0].get_stage());

		std::cout << "list end\n";
		clients[client_id].send_show_room_list_end_packet();
	} break;
	case static_cast<int>(packet_type::cs_enter):
	{
		cs_enter_packet* pack = reinterpret_cast<cs_enter_packet*>(packet);
		printf("%d client�κ��� enter packet�� �޾ҽ��ϴ�.\n", client_id);

		// set_team_position(client_id);

		//clients[client_id].send_login_success_packet();
		clients[client_id].send_enter_packet();
	} break;
	case static_cast<int>(packet_type::cs_esc):
	{
		int room_id = clients[client_id].room_id;
		normal_rooms[room_id].exit_room();

		clients[clients[client_id].tm_id].send_esc_packet();
	} break;
	case static_cast<int>(packet_type::cs_move):
	{
		cs_move_packet* pack = reinterpret_cast<cs_move_packet*>(packet);
		//clients[client_id].last_move_time = pack->move_time;
		printf("%d ID�� ���� Ŭ���̾�Ʈ�� %f, %f, %f �� �̵��Ͽ����ϴ�.\n", client_id, pack->location.x, pack->location.y, pack->location.z);
		clients[client_id].location = pack->location;
		clients[clients[client_id].tm_id].tm_location = pack->location;

		clients[client_id].yaw = pack->yaw;
		clients[clients[client_id].tm_id].tm_yaw = pack->yaw;

		if (direction::left == pack->direction) {
			clients[client_id].current_left = pack->current;
			clients[client_id].time_left = pack->time;

			clients[clients[client_id].tm_id].tm_current_left = pack->current;
			clients[clients[client_id].tm_id].tm_time_left = pack->time;

			//std::cout << "client_id: " << client_id << ", tm_id: " << clients[client_id].tm_id << std::endl;

			clients[client_id].send_left_move_packet(client_id); // ������ Ŭ������ ������
			clients[clients[client_id].tm_id].send_left_move_packet(client_id); // ��� Ŭ������ ������
		}
		else if (direction::right == pack->direction) {
			clients[client_id].current_right = pack->current;
			clients[client_id].time_right;

			clients[clients[client_id].tm_id].tm_current_right = pack->current;
			clients[clients[client_id].tm_id].tm_time_right = pack->time;

            clients[client_id].send_right_move_packet(client_id); // ������ Ŭ������ ������
            clients[clients[client_id].tm_id].send_right_move_packet(client_id); // ��� Ŭ������ ������
        }
    } break;
    case static_cast<int>(packet_type::cs_logout):
    {
        {
            std::lock_guard<std::mutex> lock(clients[client_id].state_lock);
            clients[client_id].state = state::free;
        }
        {
            std::lock_guard<std::mutex> llock{ clients[client_id].match_lock };
            clients[client_id].tm_id = -1;
        }
        closesocket(clients[client_id].sock);
        std::cout << client_id << "client�� logout �Ͽ����ϴ�.\n";
    } break;
    case static_cast<int>(packet_type::cs_push_button_maze_forward):
    {
        std::cout << client_id << " client�� forward button�� ��ҽ��ϴ�." << std::endl;
        clients[client_id].send_push_maze_button_packet(direction::forward);
        clients[clients[client_id].tm_id].send_push_maze_button_packet(direction::forward);
    } break;
    case static_cast<int>(packet_type::cs_push_button_maze_back):
    {
        std::cout << client_id << " client�� back button�� ��ҽ��ϴ�." << std::endl;
        clients[client_id].send_push_maze_button_packet(direction::back);
        clients[clients[client_id].tm_id].send_push_maze_button_packet(direction::back);
    } break;
    case static_cast<int>(packet_type::cs_push_button_maze_left):
    {
        std::cout << client_id << " client�� left button�� ��ҽ��ϴ�." << std::endl;
        clients[client_id].send_push_maze_button_packet(direction::left);
        clients[clients[client_id].tm_id].send_push_maze_button_packet(direction::left);
    } break;
    case static_cast<int>(packet_type::cs_push_button_maze_right):
    {
        std::cout << client_id << " client�� right button�� ��ҽ��ϴ�." << std::endl;
        clients[client_id].send_push_maze_button_packet(direction::right);
        clients[clients[client_id].tm_id].send_push_maze_button_packet(direction::right);
    } break;
    case static_cast<int>(packet_type::cs_push_button_gear_forward):
    {
        std::cout << "gear-------------------------------------\n";
        clients[client_id].send_push_gear_button_packet(direction::forward);
        clients[clients[client_id].tm_id].send_push_gear_button_packet(direction::forward);
    } break;
    case static_cast<int>(packet_type::cs_push_button_gear_back):
    {
        std::cout << "gear-------------------------------------\n";
        clients[client_id].send_push_gear_button_packet(direction::back);
        clients[clients[client_id].tm_id].send_push_gear_button_packet(direction::back);
    } break;
    case static_cast<int>(packet_type::cs_push_button_gear_left):
    {
        std::cout << "gear-------------------------------------\n";
        clients[client_id].send_push_gear_button_packet(direction::left);
        clients[clients[client_id].tm_id].send_push_gear_button_packet(direction::left);
    } break;
    case static_cast<int>(packet_type::cs_push_button_gear_right):
    {
        std::cout << "gear-------------------------------------\n";
        clients[client_id].send_push_gear_button_packet(direction::right);
        clients[clients[client_id].tm_id].send_push_gear_button_packet(direction::right);
    } break;
	case static_cast<int>(packet_type::cs_elevator):
	{
		cs_elevator_packet* pack = reinterpret_cast<cs_elevator_packet*>(packet);
		std::cout << "elevator: " << pack->elevator_number << std::endl;
		if (clients[client_id].current_stage != 3 && 2 == pack->elevator_number) {
			int tm_id = clients[client_id].tm_id;
			++clients[client_id].current_stage;
			++clients[tm_id].current_stage;

			std::cout << client_id << " client current stage: " << clients[client_id].current_stage << ", db stage: " << clients[client_id].db_stage << std::endl;

			// DB O
			// DB�� �ִ°Ŷ� ���� �������� ��
			if (clients[client_id].db_stage < clients[client_id].current_stage) {
				// �̶� DB�� stage ���� ������Ʈ
				wchar_t query_str[256];
				wsprintf(query_str, L"EXEC update_stage '%s', %d", clients[client_id].name,  clients[client_id].current_stage);
				wprintf(L"%s\n", query_str);
				sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
				if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
					std::cout << client_id << " client stage change " << clients[client_id].db_stage << " -> " << clients[client_id].current_stage << std::endl;
					++clients[client_id].db_stage;
				} else {
					std::cout << "stage update fail\n";
				}
				SQLCloseCursor(sqlstmt);
				SQLFreeStmt(sqlstmt, SQL_UNBIND);
			}

			// DB O
			if (clients[tm_id].db_stage < clients[tm_id].current_stage) {
				// �̶� DB�� stage ���� ������Ʈ
				wchar_t query_str[256];
				wsprintf(query_str, L"EXEC update_stage '%s', %d", clients[tm_id].name, clients[tm_id].current_stage);
				wprintf(L"%s\n", query_str);
				sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
				if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
					std::cout << tm_id << " client stage change " << clients[tm_id].db_stage << " -> " << clients[tm_id].current_stage << std::endl;
					++clients[tm_id].db_stage;
				} else {
					std::cout << "stage update fail\n";
				}
				SQLCloseCursor(sqlstmt);
				SQLFreeStmt(sqlstmt, SQL_UNBIND);
			}

			clients[client_id].send_elevator_ok_packet();
			clients[clients[client_id].tm_id].send_elevator_ok_packet();
		} else if (3 == clients[client_id].current_stage && 4 == pack->elevator_number) {
			std::cout << "recv stage3 elevator packet\n";
			++clients[client_id].current_stage;

			int tm_id = clients[client_id].tm_id;
			if (clients[client_id].current_stage == clients[tm_id].current_stage) {
				// DB O
				// DB�� �ִ°Ŷ� ���� �������� ��
				if (clients[client_id].db_stage < clients[client_id].current_stage) {
					// �̶� DB�� stage ���� ������Ʈ
					wchar_t query_str[256];
					wsprintf(query_str, L"EXEC update_stage '%s', %d", clients[client_id].name, clients[client_id].current_stage);
					wprintf(L"%s\n", query_str);
					sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
					if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
						std::cout << client_id << " client stage change " << clients[client_id].db_stage << " -> " << clients[client_id].current_stage << std::endl;
						++clients[client_id].db_stage;
					} else {
						std::cout << "stage update fail\n";
					}
					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
				}

				// DB O
				if (clients[tm_id].db_stage < clients[tm_id].current_stage) {
					// �̶� DB�� stage ���� ������Ʈ
					wchar_t query_str[256];
					wsprintf(query_str, L"EXEC update_stage '%s', %d", clients[tm_id].name, clients[tm_id].current_stage);
					wprintf(L"%s\n", query_str);
					sqlret = SQLExecDirect(sqlstmt, (SQLWCHAR*)query_str, SQL_NTS);
					if (sqlret == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO) {
						std::cout << tm_id << " client stage change " << clients[tm_id].db_stage << " -> " << clients[tm_id].current_stage << std::endl;
						++clients[tm_id].db_stage;
					} else {
						std::cout << "stage update fail\n";
					}
					SQLCloseCursor(sqlstmt);
					SQLFreeStmt(sqlstmt, SQL_UNBIND);
				}

				clients[client_id].send_elevator_ok_packet();
				clients[clients[client_id].tm_id].send_elevator_ok_packet();
			}


		}
	} break;
	case static_cast<int>(packet_type::cs_push_button_Forklift_red):
	{
		std::cout << "push red button\n";
		clients[client_id].send_forklift_button(packet_type::sc_push_button_Forklift_red);
		clients[clients[client_id].tm_id].send_forklift_button(packet_type::sc_push_button_Forklift_red);
	} break;
	case static_cast<int>(packet_type::cs_push_button_Forklift_green):
	{
		std::cout << "push green button\n";
		clients[client_id].send_forklift_button(packet_type::sc_push_button_Forklift_green);
		clients[clients[client_id].tm_id].send_forklift_button(packet_type::sc_push_button_Forklift_green);
	} break;
	case static_cast<int>(packet_type::cs_push_button_Forklift_blue):
	{
		std::cout << "push blue button\n";
		clients[client_id].send_forklift_button(packet_type::sc_push_button_Forklift_blue);
		clients[clients[client_id].tm_id].send_forklift_button(packet_type::sc_push_button_Forklift_blue);
	} break;
	case static_cast<int>(packet_type::cs_push_button_Forklift_black):
	{
		std::cout << "push black button\n";
		clients[client_id].send_forklift_button(packet_type::sc_push_button_Forklift_black);
		clients[clients[client_id].tm_id].send_forklift_button(packet_type::sc_push_button_Forklift_black);
	} break;
	case static_cast<int>(packet_type::cs_push_button_cobot_red):
	{
		clients[client_id].send_cobot_button(packet_type::sc_push_button_cobot_red);
		clients[clients[client_id].tm_id].send_cobot_button(packet_type::sc_push_button_cobot_red);
	} break;
	case static_cast<int>(packet_type::cs_push_button_cobot_green):
	{
		clients[client_id].send_cobot_button(packet_type::sc_push_button_cobot_green);
		clients[clients[client_id].tm_id].send_cobot_button(packet_type::sc_push_button_cobot_green);
	} break;
	case static_cast<int>(packet_type::cs_push_button_cobot_blue):
	{
		clients[client_id].send_cobot_button(packet_type::sc_push_button_cobot_blue);
		clients[clients[client_id].tm_id].send_cobot_button(packet_type::sc_push_button_cobot_blue);
	} break;
	case static_cast<int>(packet_type::cs_push_button_cobot_black):
	{
		clients[client_id].send_cobot_button(packet_type::sc_push_button_cobot_black);
		clients[clients[client_id].tm_id].send_cobot_button(packet_type::sc_push_button_cobot_black);
	} break;
	case static_cast<int>(packet_type::cs_start_time_color):
	{
		// timer thread�� �ٲ��� ������
		std::cout << "recv cs_start_time_color" << std::endl;

		static int id = -1;

		std::mutex m;
		m.lock();
		if (-1 == id) {
			id = client_id;
		}
		m.unlock();

		if (client_id == id) {
			int color = rand() % 8;
			clients[client_id].send_board_color(color, client_id);
			clients[clients[client_id].tm_id].send_board_color(color, client_id);
		} else {
			std::cout << "packet_type::cs_start_time_color err!" << std::endl;
		}
	} break;
	case static_cast<int>(packet_type::cs_stage3_enter):
	{
		std::cout << client_id << "client enter!" << std::endl;

		// stage3�� �ٷ� ������ �� �׽�Ʈ�� �� �� �ֵ��� �ӽù�����. �����ؾ� ��.
		{
			std::lock_guard<std::mutex> lock{ clients[client_id].state_lock };
			clients[client_id].state = state::ingame;
		}

		if (client_id == 0) clients[client_id].tm_id = 1;
		else if (client_id == 1) clients[client_id].tm_id = 0;
		else std::cout << "stage3 enter matching err";

		clients[client_id].room_id = 0;
		normal_rooms[0].set_host_id(0);
		//bool is_matching = matching(client_id);
		//if (!is_matching)
		//	std::cout << "�̹� �ٸ� ������ �ֽ��ϴ�." << std::endl;

		std::cout << "complete matching\n";
		// --------------------------------------------------------------------

		clients[client_id].send_stage3_enter_packet(client_id, clients[client_id].tm_id);
	} break;
	case static_cast<int>(packet_type::cs_car_direction): // ������ �Ѹ� Ű�� ������ ������ ��
	{
		using namespace std;

		cs_car_direction_packet* pack = reinterpret_cast<cs_car_direction_packet*>(packet);

		std::cout << client_id << "push? " << pack->direction << std::endl;

		if (pack->direction) {
			clients[client_id].move_car = true;

			TIMER_EVENT timer_event;
			timer_event.event_type = event_type::move_car;
			timer_event.execute_time = std::chrono::system_clock::now() + 1ms;
			timer_event.object_id = client_id;

			timer_queue.push(timer_event);
		} else {
			clients[client_id].move_car = false;
		}
	} break;
	case static_cast<int>(packet_type::cs_car_location):
	{
		cs_car_location_packet* pack = reinterpret_cast<cs_car_location_packet*>(packet);

		int team_id = normal_rooms[clients[client_id].room_id].get_team_id();

		clients[team_id].send_car_location_packet(pack->car_location);
		// clients[client_id].send_car_location_packet(pack->car_location);
		// clients[clients[client_id].tm_id].send_car_location_packet(pack->car_location);
	} break;
	case static_cast<int>(packet_type::cs_car_rotation_yaw):
	{
		cs_car_rotation_yaw_packet* pack = reinterpret_cast<cs_car_rotation_yaw_packet*>(packet);

		clients[client_id].send_car_rotation_yaw_packet(pack->yaw);
		clients[clients[client_id].tm_id].send_car_rotation_yaw_packet(pack->yaw);
	} break;
	case static_cast<int>(packet_type::cs_cannon):
	{
		cs_cannon_packet* pack = reinterpret_cast<cs_cannon_packet*>(packet);

		clients[client_id].mouse_left_click = false;
		clients[clients[client_id].tm_id].mouse_left_click = false;

		if (1 == clients[client_id].stage3_player_number) {
			static double yaw = 0.0;
			if (0 < pack->cannon_value)
				yaw += 5.0;
			else if (0 > pack->cannon_value)
				yaw -= 5.0;
			else
				std::cout << "cannon yaw error\n";

			std::cout << "yaw: " << yaw << std::endl;
			clients[client_id].send_cannon_yaw_packet(yaw);
			clients[clients[client_id].tm_id].send_cannon_yaw_packet(yaw);
		} else if (2 == clients[client_id].stage3_player_number) {
			static double pitch = 0.0;
			if (0 < pack->cannon_value) {
				pitch += 5.0;
				if (pitch >= 20.0)
					pitch = 20.0;
			} else if (0 > pack->cannon_value) {
				pitch -= 5.0;
				if (pitch <= 0.0)
					pitch = 0.0;
			} else {
				std::cout << "cannon pitch error\n";
			}

			std::cout << "pitch: " << pack->cannon_value << std::endl;
			clients[client_id].send_cannon_pitch_packet(pitch);
			clients[clients[client_id].tm_id].send_cannon_pitch_packet(pitch);
		} else {
			std::cout << "cannon player number err\n";
		}
	} break;
	case static_cast<int>(packet_type::cs_cannon_click):
	{
		clients[client_id].mouse_left_click = true;

		if (clients[client_id].mouse_left_click && clients[clients[client_id].tm_id].mouse_left_click) {
			
			clients[client_id].send_cannon_fire_packet();
			clients[clients[client_id].tm_id].send_cannon_fire_packet();

			clients[client_id].mouse_left_click = false;
			clients[clients[client_id].tm_id].mouse_left_click = false;
		} else {
			clients[client_id].send_cannon_click_packet(clients[client_id].stage3_player_number);
			clients[clients[client_id].tm_id].send_cannon_click_packet(clients[client_id].stage3_player_number);
		}		
	} break;
	case static_cast<int>(packet_type::cs_select_bridge_widget):
	{
		cs_select_bridge_widget_packet* pack = reinterpret_cast<cs_select_bridge_widget_packet*>(packet);

		clients[clients[client_id].tm_id].send_select_bridge_widget_packet(pack->index);
	} break;
	}
}

bool ServerMain::matching(int client_id) // �Ŀ� ������ �Լ�
{
	for (int i{}; ; ++i)
	{
		if (MAX_USER == i) i = 0;
		//clients[i].state_lock.lock();
		if (state::ingame != clients[i].state || i == client_id) continue;
		//clients[i].state_lock.unlock();

		//clients[client_id].match_lock.lock();
		if (-1 == clients[client_id].tm_id) {
			//clients[i].match_lock.lock();
			if (-1 == clients[i].tm_id) {
			 	clients[client_id].tm_id = clients[i].id;
				clients[i].tm_id = client_id;
				//clients[i].match_lock.unlock();
				//clients[client_id].match_lock.unlock();

				std::cout << i << ", " << client_id << "matching!" << std::endl;

				return true;
			}
			else {
				//clients[i].match_lock.unlock();
			}
		}
		else { // �̹� ������ �ִٴ� �Ҹ�
			//clients[client_id].match_lock.unlock();

			return false;
		}
	}
}

void ServerMain::set_team_position(int client_id)
{
	clients[client_id].tm_location = clients[clients[client_id].tm_id].location;
}

void ServerMain::do_timer_thread()
{
	using namespace std;

	while (true)
	{
		TIMER_EVENT event;
		if (!timer_queue.try_pop(event)) {
			std::this_thread::sleep_for(1ms);
			continue;
		}

		if (event.execute_time <= std::chrono::system_clock::now()) {
			switch (event.event_type)
			{
			case event_type::move_car:
			{
				OVER_EX* over = new OVER_EX;
				over->mode = COMP_MODE::MOVE_CAR;
				over->object_id = event.object_id;

				PostQueuedCompletionStatus(iocp_handle, 1, event.object_id, &over->over);
			} break;
			default: continue;
			}
		} else {
			std::this_thread::sleep_for(1ms);
		}
	}
}

void ServerMain::DB_disconnect()
{
	// DB O
	SQLCancel(sqlstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, sqlstmt);
	SQLDisconnect(sqldbc);
	SQLFreeHandle(SQL_HANDLE_DBC, sqldbc);
	SQLFreeHandle(SQL_HANDLE_ENV, sqlenv);
}

void ServerMain::show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	// �ڵ��� �־��ش�. �ڵ��� Ÿ���� �־��ش�. 
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError); // wszMessage: ���⿡ ���� �޽����� �� �ִ�.
		}
	}
}

char* ServerMain::ConvertWCtoC(wchar_t* str)
{
	//��ȯ�� char* ���� ����
	char* pStr;

	//�Է¹��� wchar_t ������ ���̸� ����
	int strSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	//char* �޸� �Ҵ�
	pStr = new char[strSize];

	//�� ��ȯ 
	WideCharToMultiByte(CP_ACP, 0, str, -1, pStr, strSize, 0, 0);
	return pStr;
}

// �ؾ� �� ��
// 1. stage3���� �ڵ��� ������ �� Ű�� ����� �� ���ľ� �Ѵ�.
// -> ad�� ������ ���� �� �ߺ����� ó���Ǵ°��� Ȯ���غ��� �Ѵ�.
// 2. ���� Ŭ�󿡼��� �浹ó���� �ǵ��� �ٲ�� �Ѵ�.
// -> ȸ���� ��������� �ϴ���...
// 3. �������̼� ���� �� �� �Ǵ°� Ȯ���ؾ� �Ѵ�.
// 4. �� ����� ���� ���� ���� ������ ���� ������ �����Ѵ�. -> �� ���ľ� �Ѵ�.
// 5. ������