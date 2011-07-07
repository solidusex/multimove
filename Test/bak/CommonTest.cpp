


#include "test.h"


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common/common.h"






/****************************************************************************************************************/


static  cmMutex_t	lock;
static cmEvent_t		*event = (cmEvent_t*)INVALID_HANDLE_VALUE;
static size_t __g_cnt = 0;



void thread_func(void *data)
{
		data = data;
		Com_ASSERT(event != INVALID_HANDLE_VALUE);
		Com_WaitEvent(event);

		Com_LockMutex(&lock);

		printf("_g_cnt == %d\r\n", __g_cnt);
		__g_cnt += 1;
		
		Com_UnLockMutex(&lock);
}

void thread_test1()
{
		size_t i;
		cmThread_t *arr[1000];
		
		Com_InitMutex(&lock);
		event  = Com_CreateEvent(false);
		
		for(i = 0; i < 1000; ++i)
		{
				arr[i] = Com_CreateThread(thread_func, NULL, NULL);
		}
		
		getchar();

		Com_SetEvent(event);

		

		for(i = 0; i < 1000; ++i)
		{
				Com_CloseThread(arr[i]);
				printf("close thread nth == %d\r\n", i);
		}

		getchar();

		Com_DestroyEvent(event);
		
		Com_UnInitMutex(&lock);
		//Com_UnInitSpinLock(&lock);
}



/****************************************asyncQueue_t Test*************************************************************/



void output_thread(void *data)
{
		cmAsyncQueue_t	*queue;
		Com_ASSERT(data != NULL);
		queue = (cmAsyncQueue_t*)data;
		
		while(true)
		{
				if(Com_GetFromAsyncQueueTimeOut(queue, &data, 2000))
				{
						const char *s = (const char*)data;
						Com_ASSERT(s != NULL);
						printf("thread == %d : %s\r\n", GetCurrentThreadId(), s);
						if(strcmp(s, "quit") == 0)
						{

								Com_DEL(s);
								break;
						}else
						{
								Com_DEL(s);
						}
						
				}else
				{
						printf("thread %d timeout\r\n", GetCurrentThreadId());
						continue;
				}
		}
}




#define THREAD_CNT  20
void async_queue_test()
{
		int i;
		char buf[1024];
		cmThread_t *thread_set[THREAD_CNT];
		cmAsyncQueue_t	queue;
		Com_InitAsyncQueue(&queue);

		for(i = 0; i < THREAD_CNT; ++i)
		{
				thread_set[i] = Com_CreateThread(output_thread, (void*)&queue, NULL);
		}

		printf("ÇëÊäÈë:\r\n");
		while(true)
		{
				char *s = NULL;
				gets(buf);
				s = Com_NEWARR(char, 1024);
				strcpy(s, buf);
				Com_PutToAsyncQueue(&queue, (void*)s);
				if(strcmp(buf, "quit")== 0)break;
		}
		
		for(i = 0; i < THREAD_CNT; ++i)
		{
				char *s = Com_NEWARR(char, 1024);
				strcpy(s, "quit");
				Com_PutToAsyncQueue(&queue, (void*)s);
		}

		for(i = 0; i < THREAD_CNT; ++i)
		{
				Com_CloseThread(thread_set[i]);
				thread_set[i] = NULL;
		}

		while(!Com_AsyncQueueIsEmpty(&queue))
		{
				const char *s;
				Com_GetFromAsyncQueue(&queue, (void**)&s);
				printf("delete string == %s\r\n", s);
				Com_DEL(s);
		}

		Com_UnInitAsyncQueue(&queue);
}



void common_func_test()
{
		Com_printf(L"%ls\r\n", L"abcdefg");
		Com_check(false, L"abcdefg");
}



bool_t Com_GetIPByHostName_V6(const wchar_t *host_name, wchar_t *ip, int len);

void	net_test()
{
		
		//Com_GetIPByHostName_V6(L"Solidus-MainPC", buf, 128);
		//Com_printf(L"%ls\r\n", buf);

}



static void __message_loop_thread(void *data)
{

		MSG msg;
		while (GetMessage (&msg, NULL, 0, 0))
		{
				printf("Message ID == %d\r\n", msg.message);

				if(msg.message == 12345)
				{
						break;
				}

				TranslateMessage (&msg);
				DispatchMessage (&msg);
		};
}



static void msg_queue_test()
{
		size_t thread_id = 0;
		cmThread_t *thd = Com_CreateThread(__message_loop_thread, NULL, &thread_id);
		Com_ASSERT(thd != NULL);

		printf("press any key to post quit message\r\n");
		getchar();

		::PostThreadMessage(thread_id, 12345, NULL, NULL);


		getchar();


		Com_JoinThread(thd);
		Com_CloseThread(thd);
		thd = NULL;
		

}


void	Common_Test()
{
		//thread_test1();
		//async_queue_test();
		
		//net_test();
		//common_func_test();

		msg_queue_test();
}








