#include "ServerMain.h"

int main()
{
	ServerMain server;

	if (server.init()) // �ʱ�ȭ�� �����ߴٸ� ���� ����
	{
		server.server_main();
	}
}