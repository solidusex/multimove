
#include <process.h>
#include "common.h"










#if(PLATFORM_VER == PLAT_I386)
		
		#if (WINVER < 0x401)
				#define COMP_EXCH(_dest, _exch, _comp_val)	InterlockedCompareExchange((PVOID*)(_dest), (PVOID)(_exch), (PVOID)(_comp_val))
		#else
				#define COMP_EXCH(_dest, _exch, _comp_val)	InterlockedCompareExchange((volatile LONG*)(_dest), (LONG )(_exch), (LONG)(_comp_val))
		#endif


#elif(PLATFORM_VER == PLAT_X64 || PLATFORM_VER == PLAT_IA64)
		
				#define COMP_EXCH(_dest, _exch, _comp_val)	InterlockedCompareExchange64((volatile LONGLONG*)(_dest), (LONGLONG )(_exch), (LONGLONG)(_comp_val))
#else
				#error "unrecognized platform!"

#endif



MM_NAMESPACE_BEGIN

void			Com_Sleep(uint_64_t	millisecond)
{
		Sleep((DWORD)millisecond);
}

void			Com_Yield()
{
		Com_Sleep(0);
}


/****************************************************************************SpinLock***********************************************/

#define LOCK_STATE			1
#define UNLOCK_STATE		0


void			Com_InitSpinLock(cmSpinLock_t *lock)
{
		Com_ASSERT(lock != NULL);
		*lock = UNLOCK_STATE;
}


void			Com_UnInitSpinLock(cmSpinLock_t *lock)
{
		Com_ASSERT(lock != NULL && *lock == UNLOCK_STATE);
		*lock = UNLOCK_STATE;
}

void			Com_LockSpinLock(cmSpinLock_t *lock)
{

		size_t count;
		Com_ASSERT(lock != NULL);
		count = 0;

		while(COMP_EXCH(lock, LOCK_STATE, UNLOCK_STATE) != UNLOCK_STATE)
		{
				if(++count > 10000)
				{
						Com_Sleep(0);
						count = 0;
				}
		}
}


void			Com_UnLockSpinLock(cmSpinLock_t *lock)
{
		Com_ASSERT(lock != NULL && *lock == LOCK_STATE);

		COMP_EXCH(lock, UNLOCK_STATE, LOCK_STATE);
}


/************************************************************************************************/


void			Com_InitMutex(cmMutex_t *lock)
{
		Com_ASSERT(lock != NULL);
		InitializeCriticalSection(lock);
}

void			Com_UnInitMutex(cmMutex_t *lock)
{
		Com_ASSERT(lock != NULL);
		DeleteCriticalSection(lock);
}

void			Com_LockMutex(cmMutex_t *lock)
{
		Com_ASSERT(lock != NULL);
		EnterCriticalSection(lock);
}

void			Com_UnLockMutex(cmMutex_t *lock)
{
		Com_ASSERT(lock != NULL);
		LeaveCriticalSection(lock);
}


/**************************************************Event*****************************************/



cmEvent_t*	Com_CreateEvent(bool_t is_auto)
{
		cmEvent_t *event;
		event = CreateEvent(NULL, is_auto ? FALSE : TRUE, FALSE, NULL);
		Com_check((bool_t)(event != INVALID_HANDLE_VALUE), L"%hs\r\n", "CreateEvent failed in Com_CreateEvent!");
		return event;
}

void		Com_DestroyEvent(cmEvent_t *event)
{
		Com_ASSERT(event != NULL);
		CloseHandle(event);
}

void	Com_SetEvent(cmEvent_t *event)
{
		Com_ASSERT(event != NULL);
		SetEvent(event);
}



void	Com_ResetEvent(cmEvent_t *event)
{
		Com_ASSERT(event != NULL);
		ResetEvent(event);
}



void	Com_WaitEvent(cmEvent_t *event)
{
		DWORD stat;
		Com_ASSERT(event != NULL);
		stat = WaitForSingleObject(event, INFINITE);
		Com_check((bool_t)(stat == WAIT_OBJECT_0), L"%hs\r\n", "WaitForSingleObject failed in Com_WaitEvent!");
}

 

bool_t	Com_WaitEventTimeout(cmEvent_t *event, uint_64_t millisecond)
{
		DWORD stat;
		
		Com_ASSERT(event != NULL);
		
		stat = WaitForSingleObject(event, (DWORD)millisecond);

		if(stat == WAIT_OBJECT_0)
		{
				return true;
		}else if(stat == WAIT_TIMEOUT)
		{
				return false;
		}else
		{
				Com_check(false, L"%hs\r\n", "WaitForSingleObject failed in Com_WaitEvent!");
				return false;/*避免warning*/
		}
}

/*************************************************Thread*******************************************/



typedef struct __local_thread_info_tag
{
		cmThreadFunc_t	func;
		void			*data;
}thread_info_t;


unsigned int	__stdcall __sys_thread(void *data)
{
		thread_info_t	*info;
		info = (thread_info_t*)data;
		Com_ASSERT(info != NULL && info->func != NULL);
		info->func(info->data);
		Com_DEL(info);
		return 0;
}


cmThread_t*	Com_CreateThread(cmThreadFunc_t func, void *data, size_t *thread_id)
{
		thread_info_t	*info;
		cmThread_t		*thd;
		unsigned int	thd_id;
		Com_ASSERT(func != NULL);
		info = Com_NEW(thread_info_t);
		info->func = func;
		info->data = data;

		thd_id = 0;
		thd = (HANDLE)_beginthreadex(0, 0, __sys_thread, (void*)info, 0, &thd_id);

		if(thd != INVALID_HANDLE_VALUE && thread_id)
		{
				*thread_id = (size_t)thd_id;
		}
		return thd == INVALID_HANDLE_VALUE ? NULL : thd;
}


void			Com_CloseThread(cmThread_t *thread)
{
		Com_ASSERT(thread != NULL);
		Com_JoinThread(thread);
		CloseHandle(thread);
}


void	Com_JoinThread(cmThread_t *thread)
{
		DWORD stat;
		Com_ASSERT(thread != NULL);
		stat = WaitForSingleObject(thread, INFINITE);
		Com_check((bool_t)(stat == WAIT_OBJECT_0), L"%ls\r\n", L"WaitForSingleObject failed in Com_JoinThread!");
}


bool_t	Com_JoinThreadTimeout(cmThread_t *thread, uint_64_t millisecond)
{

		DWORD stat;
		Com_ASSERT(thread != NULL);
		stat = WaitForSingleObject(thread, (DWORD)millisecond);
		
		if(stat == WAIT_OBJECT_0)
		{
				return true;
		}else if(stat == WAIT_TIMEOUT)
		{
				return false;
		}else
		{
				Com_check(false, L"%ls\r\n", L"WaitForSingleObject failed in Com_JoinThreadTimeout");
				return false;/*避免warning*/
		}
}

bool_t			Com_SetThreadPriority(cmThread_t *thread, cmThreadPriority_t priority)
{
		int p;
		Com_ASSERT(thread != NULL);
		
		switch(priority)
		{
		case THREAD_PREC_HIGH:
				p = THREAD_PRIORITY_HIGHEST;
		case THREAD_PREC_LOW:
				p = THREAD_PRIORITY_IDLE;	
		default:
				p = THREAD_PRIORITY_NORMAL;
		}

		return SetThreadPriority(thread, p) != 0 ? true : false;
}







/******************************************AsyncQueue_t****************************/

struct __async_data_node_tag
{
		void							*data;
		struct __async_data_node_tag	*next;
};

struct async_wait_info_tag
{
		cmEvent_t						*event;
		void							*data;
};

struct __async_wait_node_tag 
{
		asyncWaitInfo_t					*info;
		struct __async_wait_node_tag	*next;
};


/**************aux****************************/
static void __init_data_queue(cmAsyncQueue_t *queue)
{
		Com_ASSERT(queue != NULL);
		queue->data_head = queue->data_tail = NULL;
		queue->data_cnt = 0;
}

static void* __pop_data(cmAsyncQueue_t *queue)
{
		void *ret;
		asyncDataNode_t *node;
		Com_ASSERT(queue && queue->data_cnt > 0 && queue->data_head != NULL && queue->data_tail != NULL);

		node = queue->data_head; 
		queue->data_head = queue->data_head->next;
		queue->data_cnt--;

		if(queue->data_head == NULL)
		{
				queue->data_head = queue->data_tail = NULL;
				Com_ASSERT(queue->data_cnt == 0);
		}

		ret = node->data;
		Com_DEL(node);
		return ret;
}

static void		__push_data(cmAsyncQueue_t *queue, void *data)
{
		asyncDataNode_t *node;
		Com_ASSERT(queue != NULL);
		node = Com_NEW(asyncDataNode_t);
		node->data = data;
		node->next = NULL;
		if(queue->data_cnt == 0)
		{
				Com_ASSERT(queue->data_head == NULL && queue->data_tail == NULL);
				queue->data_head = queue->data_tail = node;
				
		}else
		{
				Com_ASSERT(queue->data_head != NULL && queue->data_tail != NULL);
				queue->data_tail->next = node;
				queue->data_tail = node;
		}
		queue->data_cnt++;
}

static void __uninit_data_queue(cmAsyncQueue_t *queue)
{
		while(queue->data_cnt)
		{
				__pop_data(queue);
		}
}







static void __init_wait_queue(cmAsyncQueue_t *queue)
{
		Com_ASSERT(queue != NULL);
		queue->wait_head = queue->wait_tail = NULL;
		queue->wait_cnt = 0;
}


static asyncWaitInfo_t* __pop_wait(cmAsyncQueue_t *queue)
{
		
		asyncWaitNode_t *node;
		asyncWaitInfo_t *ret;
		Com_ASSERT(queue && queue->wait_cnt > 0 && queue->wait_head != NULL && queue->wait_tail != NULL);
		node = queue->wait_head; 
		queue->wait_head = queue->wait_head->next;
		queue->wait_cnt--;

		if(queue->wait_head == NULL)
		{
				queue->wait_head = queue->wait_tail = NULL;
				Com_ASSERT(queue->wait_cnt == 0);
		}
		ret = node->info;
		Com_DEL(node);
		return ret;
}



static void		__push_wait(cmAsyncQueue_t *queue, asyncWaitInfo_t *info)
{
		asyncWaitNode_t *node;
		Com_ASSERT(queue != NULL);
		Com_ASSERT(info != NULL && info->event != NULL);
		node = Com_NEW(asyncWaitNode_t);
		node->info = info;
		node->next = NULL;
		node->info->data = NULL;

		if(queue->wait_cnt == 0)
		{
				Com_ASSERT(queue->wait_head == NULL && queue->wait_tail == NULL);
				queue->wait_head = queue->wait_tail = node;
		}else
		{
				Com_ASSERT(queue->wait_head != NULL && queue->wait_tail != NULL);
				queue->wait_tail->next = node;
				queue->wait_tail = node;
		}

		queue->wait_cnt++;
}


static void __uninit_wait_queue(cmAsyncQueue_t *queue)
{
		while(queue->wait_cnt)
		{
				asyncWaitInfo_t *info;
				Com_ASSERT(false);/*不可能执行到此*/
				info = __pop_wait(queue);
				Com_ASSERT(info != NULL);
		}
}



static bool_t __remove_wait_node(cmAsyncQueue_t *queue, asyncWaitInfo_t *info)
{
		asyncWaitNode_t *curr, *prev;
		Com_ASSERT(queue != NULL && info != NULL);

		prev = NULL;
		for(curr = queue->wait_head; curr != NULL && curr->info != info; curr = curr->next)
		{
				prev = curr;
		}

		if(curr == NULL)
		{
				return false;
		}else
		{
				Com_ASSERT(curr->info == info && curr->info->event == info->event);
				
				if(prev == NULL)
				{
						Com_ASSERT(curr == queue->wait_head);
						queue->wait_head = queue->wait_head->next;
						queue->wait_cnt--;
						
						if(queue->wait_head == NULL)
						{
								Com_ASSERT(queue->wait_cnt == 0);
								queue->wait_head = queue->wait_tail = NULL;
						}
				}else
				{
						prev->next = curr->next;
						queue->wait_cnt--;

						if(curr == queue->wait_tail)
						{
								queue->wait_tail = prev;
						}
				}
				Com_DEL(curr);

				return true;
		}
}



/**************aux***************************end*/


void	Com_InitAsyncQueue(cmAsyncQueue_t *queue)
{
		Com_ASSERT(queue != NULL);
		memset(queue, 0, sizeof(*queue));
		__init_data_queue(queue);
		__init_wait_queue(queue);
		Com_InitSpinLock(&queue->mutex);
}


void	Com_UnInitAsyncQueue(cmAsyncQueue_t *queue)
{
		Com_ASSERT(queue != NULL);
		
		__uninit_wait_queue(queue);
		__uninit_data_queue(queue);

		Com_UnInitSpinLock(&queue->mutex);
}


bool_t	Com_GetFromAsyncQueueTimeOut(cmAsyncQueue_t *queue, void **pdata, uint_64_t	millisecond)
{
		bool_t res;
		Com_ASSERT(queue != NULL && pdata != NULL);
		Com_LockSpinLock(&queue->mutex);

		if(queue->data_cnt > 0)
		{
				*pdata = __pop_data(queue);
				Com_UnLockSpinLock(&queue->mutex);
				res = true;
		}else
		{
				cmEvent_t		*event = Com_CreateEvent(false);
				asyncWaitInfo_t	info;
				info.event = event;
				info.data = NULL;

				__push_wait(queue, &info);
				Com_UnLockSpinLock(&queue->mutex);
				
				if(Com_WaitEventTimeout(event, millisecond))
				{
						*pdata = info.data;
						res = true;
				}else
				{
						Com_LockSpinLock(&queue->mutex);
						/*
						如果__remove_wait_node不成功，则证明在在本函数Com_WaitEventTimeout失败之后，
						Com_LockSpinLock(&queue->mutex);之前，有Com_PutToAsyncQueue获得锁且将info.data
						赋值
						*/
						if(__remove_wait_node(queue, &info))
						{
								res = false;
						}else
						{
								res = true;
								*pdata = info.data;
						}
						Com_UnLockSpinLock(&queue->mutex);
				}
				Com_DestroyEvent(event);
		}

		return res;
}

void	Com_GetFromAsyncQueue(cmAsyncQueue_t *queue, void **pdata)
{

		Com_ASSERT(queue != NULL && pdata != NULL);
		Com_LockSpinLock(&queue->mutex);

		if(queue->data_cnt > 0)
		{
				*pdata = __pop_data(queue);
				Com_UnLockSpinLock(&queue->mutex);
		}else
		{
				cmEvent_t		*event = Com_CreateEvent(false);
				asyncWaitInfo_t	info;
				info.event = event;
				info.data = NULL;

				__push_wait(queue, &info);
				Com_UnLockSpinLock(&queue->mutex);
				Com_WaitEvent(event);
				*pdata = info.data;
				Com_DestroyEvent(event);
		}

}

void	Com_PutToAsyncQueue(cmAsyncQueue_t *queue, void *data)
{
		Com_ASSERT(queue != NULL);
		Com_LockSpinLock(&queue->mutex);
		
		if(queue->wait_cnt > 0)
		{
				asyncWaitInfo_t *info = __pop_wait(queue);
				Com_ASSERT(info != NULL && info->event != NULL);
				info->data = data;
				Com_SetEvent(info->event);
		}else
		{
				__push_data(queue, data);
		}
		
		Com_UnLockSpinLock(&queue->mutex);
}


bool_t	Com_HasIdleThreadInAsyncQueue(const cmAsyncQueue_t *queue)
{
		bool_t ret;
		cmAsyncQueue_t *que;
		Com_ASSERT(queue != NULL);
		que = (cmAsyncQueue_t*)queue;
		Com_LockSpinLock(&que->mutex);

		ret = que->wait_cnt == 0 ? false : true;

		Com_UnLockSpinLock(&que->mutex);
		return ret;

}

bool_t	Com_AsyncQueueIsEmpty(const cmAsyncQueue_t *queue)
{
		bool_t ret;
		cmAsyncQueue_t *que;
		Com_ASSERT(queue != NULL);
		que = (cmAsyncQueue_t*)queue;
		Com_LockSpinLock(&que->mutex);

		ret = que->data_cnt == 0 ? true : false;

		Com_UnLockSpinLock(&que->mutex);
		return ret;
}

void	Com_ClearAsyncQueue(cmAsyncQueue_t *queue)
{
		Com_ASSERT(queue != NULL);
		Com_LockSpinLock(&queue->mutex);
		
		while(queue->data_cnt)
		{
				__pop_data(queue);
		}
		
		Com_UnLockSpinLock(&queue->mutex);
}



/******************************************Time****************************************/


static uint_64_t __get_time_microseconds()
{
		FILETIME ft;
		ULARGE_INTEGER epoch; /* UNIX epoch (1970-01-01 00:00:00) expressed in Windows NT FILETIME*/
		ULARGE_INTEGER ts;
		
		GetSystemTimeAsFileTime(&ft);

		/*我不知道这做了什么，也不想知道*/
		epoch.LowPart  = 0xD53E8000;
		epoch.HighPart = 0x019DB1DE;
		ts.LowPart  = ft.dwLowDateTime;
		ts.HighPart = ft.dwHighDateTime;
		ts.QuadPart -= epoch.QuadPart;
		return ts.QuadPart/10;
}


uint_64_t		Com_GetTime_Microseconds()
{
		return __get_time_microseconds();
}





MM_NAMESPACE_END



