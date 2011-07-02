
//#if defined(__LIB)

#include "test.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"



NS_NAMESPACE_BEGIN


void connect_test()
{
		SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);

		struct sockaddr_in addr;

		Com_GetIPByHostName_V4(L"193.168.19.36", &addr);

		addr.sin_family = AF_INET;
		addr.sin_port = htons(8812);

		UINT64 tm = 1000 * 3;
		int ret = Com_connect_timeout(fd, (const struct sockaddr*)&addr, sizeof(addr), &tm);


		const char *buf = new char[40 * COM_MB];
		int n = 40 * COM_MB;

		while(true)
		{
				uint_64_t timeout = 15 * 1000;
				ret = Com_send_timeout(fd, buf, n, 0, NULL);


				int err = WSAGetLastError();
		}
		
}

void	Net_Test()
{

		connect_test();
}



NS_NAMESPACE_END

//#endif

