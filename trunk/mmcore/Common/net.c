/*
Copyright (C) 2011 by Solidus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/




#include "common.h"




MM_NAMESPACE_BEGIN




#define _IP1(_addr)		((_addr)->sin_addr.s_net)
#define _IP2(_addr)		((_addr)->sin_addr.s_host)

#define _IP3(_addr)		((_addr)->sin_addr.s_lh)
#define _IP4(_addr)		((_addr)->sin_addr.s_impno)


int_t	Com_ipv4cmp(const struct sockaddr_in	*l, const struct sockaddr_in	*r)
{
		int_t cmp = 0;
		Com_ASSERT(l && r);
		
		cmp = _IP1(l) - _IP1(r);
		if(cmp != 0)return cmp;

		cmp = _IP2(l) - _IP2(r);
		if(cmp != 0)return cmp;

		cmp = _IP3(l) - _IP3(r);
		if(cmp != 0)return cmp;

		cmp = _IP4(l) - _IP4(r);
		if(cmp != 0)return cmp;

		return 0;

}

bool_t	Com_ipv4add(struct sockaddr_in	*addr)
{
		Com_ASSERT(addr != NULL);

		if(_IP4(addr) < 255)
		{
				_IP4(addr)++;
				return true;
		}else
		{
				_IP4(addr) = 0;
		}


		if(_IP3(addr) < 255)
		{
				_IP3(addr)++;
				return true;
		}else
		{
				_IP3(addr) = 0;
		}


		if(_IP2(addr) < 255)
		{
				_IP2(addr)++;
				return true;
		}else
		{
				_IP2(addr) = 0;
		}


		if(_IP1(addr) < 255)
		{
				_IP1(addr)++;
				return true;
		}else
		{
				_IP1(addr) = 0;
		}
		return false;
}



#undef _IP1
#undef _IP2
#undef _IP3
#undef _IP4







bool_t Com_GetIPByHostName_V4(const wchar_t *host_name, struct sockaddr_in *out)
{
		ADDRINFOW		*res;
		ADDRINFOW		hints;
		static const wchar_t *serv = L"0";
		const ADDRINFOW *curr;
		int err;
		Com_ASSERT(host_name != NULL && out != NULL);

		memset(&hints, 0, sizeof(ADDRINFOW));
		hints.ai_family = AF_INET;
		hints.ai_protocol = AF_UNSPEC;
		hints.ai_flags = AI_CANONNAME;
		
		err = GetAddrInfoW(host_name, serv, &hints, &res);
		if(err != 0 || res == NULL)
		{
				return false;
		}

		curr = res;
		while(curr)
		{
				struct sockaddr_in *addr;
				err = GetNameInfoW((struct sockaddr*)curr->ai_addr, sizeof(struct	 sockaddr_in),  NULL, 0, NULL, 0, NI_NUMERICHOST);
				addr = (struct sockaddr_in *)(curr->ai_addr);
				if(err == 0)
				{
						Com_memcpy(out, addr, sizeof(*out));
						break;
				}else
				{
						curr = curr->ai_next;
				}
		}
		FreeAddrInfoW(res);
		return curr != NULL;
}


bool_t Com_GetIPByHostName_V6(const wchar_t *host_name, struct sockaddr_in6 *out)
{
		ADDRINFOW		*res;
		ADDRINFOW		hints;
		static const wchar_t *serv = L"0";
		const ADDRINFOW *curr;
		int err;
		Com_ASSERT(host_name != NULL && out != NULL);

		memset(&hints, 0, sizeof(ADDRINFOW));
		hints.ai_family = AF_INET6;
		hints.ai_protocol = AF_UNSPEC;
		hints.ai_flags = AI_CANONNAME;
		
		err = GetAddrInfoW(host_name, serv, &hints, &res);
		if(err != 0 || res == NULL)
		{
				return false;
		}

		curr = res;
		while(curr)
		{
				struct sockaddr_in6 *addr_v6;
				err = GetNameInfoW((struct sockaddr*)curr->ai_addr, sizeof(struct	 sockaddr_in6),  NULL, 0, NULL, 0, NI_NUMERICHOST);
				addr_v6 = (struct sockaddr_in6 *)(curr->ai_addr);
				

				if(err == 0)
				{
						Com_memcpy(out, addr_v6, sizeof(*out));
						break;
				}else
				{
						curr = curr->ai_next;
				}
		}
		FreeAddrInfoW(res);
		return curr != NULL;
}


int Com_socket_nonblocking(SOCKET fd, bool_t is_enable)
{
		unsigned long enable = is_enable ? 1 : 0;
		return ioctlsocket(fd, FIONBIO, &enable);
}


int Com_connect_timeout(SOCKET fd, const struct sockaddr* name, int namelen, const uint_64_t	*timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set	wd_set, exp_set;


		int ret;
		Com_ASSERT(name != NULL && namelen > 0);

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}

		ret = Com_socket_nonblocking(fd, true);
		
		if(ret != 0)
		{
				return ret;
		}

		ret = connect(fd, name, namelen);
		
		if(ret == 0)
		{
				goto RESET_POINT;
		}

		if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
				return ret;
		}

		Sleep(35);//bug

		FD_ZERO(&wd_set);
		FD_ZERO(&exp_set);

		FD_SET(fd, &wd_set);
		FD_SET(fd, &exp_set);

		ret = select(0, NULL, &wd_set, &exp_set, ptv);

		if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				return SOCKET_ERROR;
		}

		if(FD_ISSET(fd, &wd_set) && !FD_ISSET(&fd, &exp_set))
		{
				ret = 0;
		}else
		{
				ret = SOCKET_ERROR;
		}

RESET_POINT:
		ret = Com_socket_nonblocking(fd, false);

		return ret;
}



SOCKET Com_accpet_timeout(SOCKET fd, struct sockaddr* addr, int* addrlen, const uint_64_t	*timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set 	rd_set, exp_set;
		int ret;
		SOCKET cli_sockfd;

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}

		ret = Com_socket_nonblocking(fd, true);

		if(ret != 0)
		{
				return INVALID_SOCKET;
		}
		
		FD_ZERO(&rd_set);
		FD_ZERO(&exp_set);

		FD_SET(fd, &rd_set);
		
		ret = select(0, &rd_set, NULL, NULL, ptv);

		if(ret <  0)
		{
				return INVALID_SOCKET;
		}else if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				return INVALID_SOCKET;
		}else
		{
				cli_sockfd = accept(fd, addr, addrlen);
				return cli_sockfd;
		}
}



int Com_sendto_timeout(SOCKET fd, const char* buf, int len, int flags, const struct sockaddr* to, int tolen, const uint_64_t	*timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set	wd_set;
		int ret;
		Com_ASSERT(buf != NULL && len > 0);

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}


		FD_ZERO(&wd_set);
		FD_SET(fd, &wd_set);
		
		ret = select(0, NULL, &wd_set, NULL, ptv);

		if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				ret = -1;
		}else if(ret == -1)
		{
				return ret;
		}else
		{
				ret = sendto(fd, buf, len, flags, to, tolen);
		}

		return ret;

}



int Com_recvfrom_timeout(SOCKET fd, char* buf, int len, int flags, struct sockaddr* from, int* fromlen, const uint_64_t	*timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set	rd_set;
		int ret;
		Com_ASSERT(buf != NULL && len > 0);

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}

		FD_ZERO(&rd_set);
		FD_SET(fd, &rd_set);

		ret = select(0, &rd_set, NULL, NULL, ptv);

		if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				ret = -1;
		}else if(ret == -1)
		{
				return ret;
		}else
		{
				ret = recvfrom(fd, buf, len, flags, from, fromlen);
		}

		return ret;
}



int Com_send_timeout(SOCKET fd, const char* buf, int len, int flags, const uint_64_t *timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set	wd_set;
		int ret;
		Com_ASSERT(buf != NULL && len > 0);

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}

		FD_ZERO(&wd_set);
		FD_SET(fd, &wd_set);

		ret = select(0, NULL, &wd_set, NULL, ptv);

		if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				ret = -1;
		}else if(ret == -1)
		{
				return ret;
		}else
		{

				ret = send(fd, buf, len, flags);
		}

		return ret;
}


int Com_recv_timeout(SOCKET fd, char* buf, int len, int flags, const uint_64_t	*timeout_millisecond)
{
		struct timeval tv;
		struct timeval *ptv;
		struct fd_set	rd_set;
		int ret;
		Com_ASSERT(buf != NULL && len > 0);

		ptv = timeout_millisecond == NULL ? NULL : &tv;
		
		if(ptv)
		{
				ptv->tv_sec = (long)(*timeout_millisecond / 1000);
				ptv->tv_usec = (long)((*timeout_millisecond % 1000) * 1000);
		}

		FD_ZERO(&rd_set);
		FD_SET(fd, &rd_set);

		ret = select(0, &rd_set, NULL, NULL, ptv);

		if(ret == 0)
		{
				WSASetLastError(WSAETIMEDOUT);
				ret = -1;
		}else if(ret == -1)
		{
				return ret;
		}else
		{
				ret = recv(fd, buf, len, flags);
		}

		return ret;
}


int Com_send_all_timeout(SOCKET fd, const char* buf, int len, int flags, int *bt, const uint_64_t *timeout)
{
		uint_64_t time_left;
		int temp;
		int *bytes_transferred = (bt == NULL ? &temp : bt);
		*bytes_transferred = 0;

		time_left = timeout == NULL ? 0 : *timeout;
		

		for (*bytes_transferred = 0; *bytes_transferred < len; )
		{
				int n;
				uint_64_t beg, end;
				beg = Com_GetTime_Milliseconds();

				n = Com_send_timeout(fd, buf + *bytes_transferred, len - *bytes_transferred, flags, timeout == NULL ? NULL : &time_left);

				end = Com_GetTime_Milliseconds();

				if (n == 0 || n == -1)
				{
						return n;
				}else
				{
						*bytes_transferred += n;
				}

				assert(*bytes_transferred <= len);

				if(*bytes_transferred == len)
				{
						continue;
				}

				if(timeout != 0)
				{
						const uint_64_t elapsed = end - beg;
						
						if(elapsed < time_left)
						{
								time_left -= elapsed;
						}else
						{
								WSASetLastError(WSAETIMEDOUT);
								return -1;
						}
				}
		}
		
		return *bytes_transferred;
}



int Com_recv_all_timeout(SOCKET fd, char* buf, int len, int flags, int *bt, const uint_64_t *timeout)
{
		uint_64_t time_left;
		int temp;
		int *bytes_transferred = (bt == NULL ? &temp : bt);
		*bytes_transferred = 0;

		time_left = timeout == NULL ? 0 : *timeout;
		

		for (*bytes_transferred = 0; *bytes_transferred < len; )
		{
				int n;
				uint_64_t beg, end;
				beg = Com_GetTime_Milliseconds();

				n = Com_recv_timeout(fd, buf + *bytes_transferred, len - *bytes_transferred, flags, timeout == NULL ? NULL : &time_left);

				end = Com_GetTime_Milliseconds();


				if (n == 0 || n == -1)
				{
						return n;
				}else
				{
						*bytes_transferred += n;
				}
				
				assert(*bytes_transferred <= len);

				if(*bytes_transferred == len)
				{
						continue;
				}

				if(timeout != 0)
				{
						uint_64_t elapsed = end - beg;
						
						if(elapsed < time_left)
						{
								time_left -= elapsed;
						}else
						{
								WSASetLastError(WSAETIMEDOUT);
								return -1;
						}
				}
		}
		
		return *bytes_transferred;
}


MM_NAMESPACE_END

