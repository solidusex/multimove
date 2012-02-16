
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

#ifndef __MULTIMOVE_COMMON_H__
#define __MULTIMOVE_COMMON_H__





#if defined(_DEBUG)
		#define DEBUG_FLAG		0x01
#endif

#define	PLAT_I386		0x01
#define	PLAT_X64		0x02
#define	PLAT_IA64		0x03


#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)

#define PLATFORM_VER	PLAT_I386

#elif defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64)

#define PLATFORM_VER	PLAT_IA64

#elif defined(__x86_64__) || defined(_M_X64)

#define PLATFORM_VER	PLAT_X64

#else

#error "Unknow Platform!"

#endif



#if !defined(_CRT_SECURE_NO_WARNINGS)
		#define _CRT_SECURE_NO_WARNINGS	1
#endif





#if defined(__cplusplus)

		#define MM_NAMESPACE_BEGIN		extern "C" { 
		#define MM_NAMESPACE_END		} 
		#define MM_INLINE				inline

		#define		MM_HAS_BOOLEAN				1

#else
		#define MM_NAMESPACE_BEGIN 
		#define MM_NAMESPACE_END
		
		#define MM_INLINE				__inline

#endif



#if defined(DEBUG_FLAG)
				#define COM_USE_CRT_ALLOCFUNC
				#define	COM_DISABLE_CRTSTDLIB
				
				#if !defined(_CRTDBG_MAP_ALLOC)
						#define _CRTDBG_MAP_ALLOC
						#include<stdlib.h>
						#include<crtdbg.h>
				#endif
#else
				#include <stdlib.h>
#endif







#if (WINVER < 0x401)
		struct _RPC_ASYNC_STATE;
#endif

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>


#include <assert.h>
#include <stdio.h>
#include <errno.h>


#pragma warning(disable : 4201)
#pragma warning(disable: 4127)
#pragma warning(disable : 4244)



MM_NAMESPACE_BEGIN





typedef __int8					int_8_t;
typedef unsigned __int8			uint_8_t;
typedef __int16					int_16_t;
typedef unsigned __int16		uint_16_t;
typedef __int32					int_32_t;
typedef unsigned __int32		uint_32_t;
typedef __int64					int_64_t;
typedef unsigned __int64		uint_64_t;


typedef uint_8_t					byte_t;


#if defined(MM_HAS_BOOLEAN)
		typedef bool				bool_t;
#else
		typedef uint_8_t			bool_t;
		#define	false				((uint_8_t)0)
		#define true				((uint_8_t)1)
#endif




/*
int_t与uint_t被定义为与处理器等长位宽
*/

#if(PLATFORM_VER == PLAT_I386)

typedef int_32_t		int_t;
typedef uint_32_t		uint_t;

#elif(PLATFORM_VER == PLAT_X64 || PLATFORM_VER == PLAT_IA64) 

typedef int_64_t		int_t;
typedef uint_64_t		uint_t;


#endif



#define			COM_KB			(1024)
#define			COM_MB			(COM_KB * 1024)
#define			COM_GB			(COM_MB * 1024)

#define			COM_BIGNUM_I64(_num)	(_num##i64)
#define			COM_BIGNUM_U64(_num)	(_num##ui64)

typedef void	(__stdcall *Com_print_func_t)(const wchar_t *msg, void *ctx);
typedef void	(__stdcall *Com_error_func_t)(int_t level, const wchar_t *msg, void *ctx);





typedef struct __io_context_tag
{
		Com_print_func_t	on_print;
		Com_error_func_t	on_error;
		void				*ctx;
}ioCtx_t;


typedef struct __ar_init_tag
{
		ioCtx_t		io_ctx;
}cmInit_t;

bool_t	Com_Init(const cmInit_t *init);
bool_t	Com_UnInit();


#if defined(DEBUG_FLAG)


#define			Com_ASSERT(_cond)		assert((_cond))
#define			Com_DebugOutput			OutputDebugStringW
#define			Com_DebugBreak			DebugBreak


#else

#if(_MSC_VER < 1300)
		#define			Com_ASSERT(_cond)		
		#define			Com_DebugOutput			
		

#else

		#define			Com_ASSERT(_cond)		__noop
		#define			Com_DebugOutput			__noop
#endif

#endif


#if(_MSC_VER < 1300)

		#define			COM_FUNC_NAME			"--"  
		
#else
		#define			COM_FUNC_NAME			__FUNCSIG__ 
#endif

#define Com_Abort		abort

void	Com_check(bool_t cond, const wchar_t *fmt, ...);

#define Com_STATIC_CHECK(_expr)	do {typedef char __static_assert_t[ (bool_t)(_expr) ]; }while(0)


void	Com_printf(const wchar_t *fmt, ...);


#define COM_ERR_FATAL			0L
#define COM_ERR_MODULE			1L
#define COM_ERR_WARNING			2L

void	Com_error(int_t level, const wchar_t *fmt,...);







#define Com_memcpy								memcpy
#define	Com_memset								memset
#define Com_memcmp								memcmp
#define Com_memmove								memmove

void*	Com_malloc(size_t nbytes);
void*	Com_calloc(size_t num, size_t size);
void*	Com_realloc(void *block, size_t nbytes);
void	Com_free(void *ptr);



#if defined(COM_USE_CRT_ALLOCFUNC)


#define Com_NEW(_type)							(_type*)malloc(sizeof(_type))
#define Com_NEW0(_type)							(_type*)calloc(1, sizeof(_type))
#define Com_NEWARR(_type, _nitems)				(_type*)malloc(sizeof(_type)* (_nitems))
#define Com_NEWARR0(_type, _nitems)				(_type*)calloc(_nitems, sizeof(_type))
#define Com_REALLOC(_type, _ptr, _new_count)	((_type*)realloc((_ptr), sizeof(_type) * (_new_count)))
#define Com_DEL(_ptr)							free((void*)(_ptr))


#else


#define Com_NEW(_type)							(_type*)Com_malloc(sizeof(_type))
#define Com_NEW0(_type)							(_type*)Com_calloc(1, sizeof(_type))
#define Com_NEWARR(_type, _nitems)				(_type*)Com_malloc(sizeof(_type)* (_nitems))
#define Com_NEWARR0(_type, _nitems)				(_type*)Com_calloc(_nitems, sizeof(_type))
#define Com_REALLOC(_type, _ptr, _new_count)	((_type*)Com_realloc((_ptr), sizeof(_type) * (_new_count)))
#define Com_DEL(_ptr)							Com_free((void*)(_ptr))

#endif



/*********************************Misc******************************************/

#define Com_UNUSED(_e)			((void)(_e))
#define Com_NELEMS(_arr)		(sizeof((_arr))/sizeof((_arr)[0]))

#define Com_MAX(_a,_b) ((_a) > (_b) ? (_a) : (_b))
#define Com_MIN(_a,_b) ((_a) < (_b) ? (_a) : (_b))
#define Com_CMP(_a,_b) ((_a) < (_b) ? -1 : ((_a) == (_b) ? 0 : 1))








int_16_t		COM_BYTEFLIP_16(int_16_t val);
uint_16_t		COM_BYTEFLIP_U16(uint_16_t val);


int_32_t		COM_BYTEFLIP_32(int_32_t val);
uint_32_t		COM_BYTEFLIP_U32(uint_32_t val);


int_64_t		COM_BYTEFLIP_64(int_64_t val);
uint_64_t		COM_BYTEFLIP_U64(uint_64_t val);



#define COM_LTON_16(_n)			COM_BYTEFLIP_16((_n))
#define COM_LTON_32(_n)			COM_BYTEFLIP_32((_n))
#define COM_LTON_64(_n)			COM_BYTEFLIP_64((_n))
#define COM_LTON_U16(_n)		COM_BYTEFLIP_U16((_n))
#define COM_LTON_U32(_n)		COM_BYTEFLIP_U32((_n))
#define COM_LTON_U64(_n)		COM_BYTEFLIP_U64((_n))

#define COM_NTOL_16(_n)			COM_BYTEFLIP_16((_n))
#define COM_NTOL_32(_n)			COM_BYTEFLIP_32((_n))
#define COM_NTOL_64(_n)			COM_BYTEFLIP_64((_n))
#define COM_NTOL_U16(_n)		COM_BYTEFLIP_U16((_n))
#define COM_NTOL_U32(_n)		COM_BYTEFLIP_U32((_n))
#define COM_NTOL_U64(_n)		COM_BYTEFLIP_U64((_n))










/*******************************String*******************************************/


#define Com_strcmp		strcmp
#define Com_strncmp		strncmp
#define Com_strchr		strchr
#define Com_strlen		strlen
#define Com_isspace		isspace
#define Com_tolower		tolower
#define Com_isdigit		isdigit
#define Com_strcpy		strcpy
#define Com_strcat		strcat

#define Com_wcscmp		wcscmp
#define Com_wcsncmp		wcsncmp
#define Com_wcslen		wcslen
#define Com_wcscpy		wcscpy
#define Com_wcschr		wcschr
#define Com_wcscat		wcscat
#define Com_wcsstr		wcsstr
#define Com_iswdigit	iswdigit
#define Com_iswspace	iswspace
#define Com_wcsncpy(_d, _s, _n)	wcsncpy((_d), (_s), (_n))

char*	Com_strdup(const char *s);
char*	Com_strndup(const char *sour, size_t len);


wchar_t*	Com_wcsdup(const wchar_t *s);
wchar_t*	Com_wcsndup(const wchar_t *sour, size_t len);


const char* Com_strtrim(const char *in, const char *trim);
const char*	Com_strtrim_space(const char *in);

const wchar_t*	Com_wcstrim_space(const wchar_t *in);
wchar_t*	Com_wcstrim_right_space(wchar_t *in);

uint_t		Com_wcshash_n(const wchar_t *str, size_t n);
uint_t		Com_wcshash(const wchar_t *str);



#if (WINVER < 0x401)
		int_t			Com_vscwprintf(const wchar_t *fmt, va_list args);
#else
		#define			Com_vscwprintf	(int_t)_vscwprintf
#endif

int_t		Com_vswprintf(wchar_t *dest, size_t count, const wchar_t *fmt, va_list args);	
int_t		Com_swprintf(wchar_t *dest, size_t count, const wchar_t *fmt, ...);

int_t		Com_vsprintf(char *dest, size_t count, const char *fmt, va_list args);
int_t		Com_sprintf(char *dest, size_t count, const char *fmt, ...);




/***********************************************StrConv************************************/

typedef enum
{
		COM_CP_ACP,
		COM_CP_UTF8,
		COM_CP_BIG5,
		COM_CP_GB2312,
		COM_CP_GB18030,
}cmCodePage_t;


size_t					Com_str_to_wcs(cmCodePage_t cp, const char *acp, size_t n, wchar_t *out, size_t out_len);
size_t					Com_wcs_to_str(cmCodePage_t cp, const wchar_t *input, size_t n, char *out, size_t out_len);
char*					Com_wcs_convto_str(cmCodePage_t cp, const wchar_t *input, size_t in_n);
wchar_t*				Com_str_convto_wcs(cmCodePage_t cp, const char *input, size_t in_n);




/***********************************************String************************************/
struct __com_string_tag;
typedef struct __com_string_tag		cmString_t;

cmString_t*		Com_CreateString();
void			Com_DestroyString(cmString_t *str);
const wchar_t*	Com_GetStrString(const cmString_t *str);
size_t			Com_GetLengthString(const cmString_t *str);
void			Com_ReserveString(cmString_t *str, size_t num);
void			Com_ClearString(cmString_t *str);
size_t			Com_AppendString(cmString_t *str, const wchar_t *sour);
void			Com_FormatString(cmString_t *str, const wchar_t *fmt, ...);
void			Com_AppendFormatString(cmString_t *str, const wchar_t *fmt, ...);
void			Com_VFormatString(cmString_t *str, const wchar_t *fmt, va_list args);
void			Com_AppendVFormatString(cmString_t *str, const wchar_t *fmt, va_list args);
void			Com_AppendCharToString(cmString_t *str, wchar_t chr);



/***********************************************Buffer***********************************/

struct commonl_buffer_tag;
typedef struct commonl_buffer_tag		cmBuffer_t;

cmBuffer_t*		Com_CreateBuffer(size_t nbytes);
void			Com_DestroyBuffer(cmBuffer_t		*buffer);

void			Com_ClearBuffer(cmBuffer_t		*buffer);

/*分配nbytes个字节以供使用*/
byte_t*			Com_AllocBuffer(cmBuffer_t *buffer, size_t	nbytes);
/*向buffer写入nbytes个字节*/
void			Com_InsertBuffer(cmBuffer_t *buffer, const byte_t *data, size_t len);

/*从buffer头擦除nbytes个字节*/
size_t			Com_EraseBuffer(cmBuffer_t *buffer, size_t nbytes);

/*返回不重新分配内存还可以写的字节数*/
size_t			Com_GetBufferCapacity(const cmBuffer_t *buffer);

/*AR_ReserveBuffer调用成功后，AR_GetBufferCapacity(buffer) >= nbytes*/
void			Com_ReserveBuffer(cmBuffer_t *buffer, size_t nbytes);

/*可读内存块*/
const byte_t*	Com_GetBufferData(const cmBuffer_t *buffer);
/*可读内存块长度*/
size_t			Com_GetBufferAvailable(const cmBuffer_t *buffer);




/***********************************************Text file************************************/
typedef enum
{
		COM_TXT_BOM_ASCII			=		0x01,
		COM_TXT_BOM_UTF16_BE		=		0x02,
		COM_TXT_BOM_UTF16_LE		=		0x04,
		COM_TXT_BOM_UTF_8			=		0x08,
		COM_TXT_BOM_UTF32_LE		=		0x10,
		COM_TXT_BOM_UTF32_BE		=		0x20
}cmTxtBom_t;


bool_t	Com_SaveBomTextFile(const wchar_t *path, cmTxtBom_t bom, const wchar_t *input);
bool_t	Com_LoadBomTextFile(const wchar_t *path, cmTxtBom_t *bom, cmString_t *out);




/*****************************************Thread*************************************************/


void			Com_Sleep(uint_64_t	millisecond);

void			Com_Yield();


/*****************************同步*******************************/


typedef int_t	cmSpinLock_t;

void			Com_InitSpinLock(cmSpinLock_t *lock);
void			Com_UnInitSpinLock(cmSpinLock_t *lock);

void			Com_LockSpinLock(cmSpinLock_t *lock);
void			Com_UnLockSpinLock(cmSpinLock_t *lock);


typedef CRITICAL_SECTION		cmMutex_t;

void			Com_InitMutex(cmMutex_t *lock);
void			Com_UnInitMutex(cmMutex_t *lock);

void			Com_LockMutex(cmMutex_t *lock);
void			Com_UnLockMutex(cmMutex_t *lock);



/*****************************Event*******************************/


typedef void	cmEvent_t;

cmEvent_t*	Com_CreateEvent(bool_t is_auto);
void		Com_DestroyEvent(cmEvent_t *event);
void		Com_SetEvent(cmEvent_t *event);
void		Com_ResetEvent(cmEvent_t *event);
void		Com_WaitEvent(cmEvent_t *event);
bool_t		Com_WaitEventTimeout(cmEvent_t *event, uint_64_t millisecond);


/*********************************Thread***************************/


typedef enum 
{
		THREAD_PREC_HIGH, 
		THREAD_PREC_NORMAL, 
		THREAD_PREC_LOW 
}cmThreadPriority_t;

typedef	void					cmThread_t;
typedef	void	(*cmThreadFunc_t)(void *data);



cmThread_t*		Com_CreateThread(cmThreadFunc_t func, void *data, size_t *thread_id);
void			Com_CloseThread(cmThread_t *thread);
void			Com_JoinThread(cmThread_t *thread);
bool_t			Com_JoinThreadTimeout(cmThread_t *thread, uint_64_t millisecond);
bool_t			Com_SetThreadPriority(cmThread_t *thread, cmThreadPriority_t priority);


/*********************************************************************************/

struct __async_data_node_tag;
typedef struct __async_data_node_tag	asyncDataNode_t;

struct async_wait_info_tag;
typedef struct async_wait_info_tag		asyncWaitInfo_t;

struct __async_wait_node_tag;
typedef struct __async_wait_node_tag	asyncWaitNode_t;

typedef struct __async_queue_tag
{
		cmSpinLock_t			mutex;
		asyncWaitNode_t			*wait_head;
		asyncWaitNode_t			*wait_tail;
		size_t					wait_cnt;

		asyncDataNode_t			*data_head;
		asyncDataNode_t			*data_tail;
		size_t					data_cnt;
}cmAsyncQueue_t;


void	Com_InitAsyncQueue(cmAsyncQueue_t *queue);
void	Com_UnInitAsyncQueue(cmAsyncQueue_t *queue);
bool_t	Com_GetFromAsyncQueueTimeOut(cmAsyncQueue_t *queue, void **pdata, uint_64_t	millisecond);
void	Com_GetFromAsyncQueue(cmAsyncQueue_t *queue, void **pdata);
void	Com_PutToAsyncQueue(cmAsyncQueue_t *queue, void *pdata);
bool_t	Com_HasIdleThreadInAsyncQueue(const cmAsyncQueue_t *queue);
bool_t	Com_AsyncQueueIsEmpty(const cmAsyncQueue_t *queue);
void	Com_ClearAsyncQueue(cmAsyncQueue_t *queue);



uint_64_t		Com_GetTime_Microseconds();
#define			Com_GetTime_Milliseconds()		(Com_GetTime_Microseconds() / 1000LL)

/*****************************************************Net********************************************************/


bool_t	Com_ipv4add(struct sockaddr_in	*addr);
int_t	Com_ipv4cmp(const struct sockaddr_in	*l, const struct sockaddr_in	*r);


bool_t Com_GetIPByHostName_V6(const wchar_t *host_name, struct sockaddr_in6 *out);
bool_t Com_GetIPByHostName_V4(const wchar_t *host_name, struct sockaddr_in *out);



/*
		在一个udp socket句柄上执行一个connect操作时，WINSOCK默认会返回WSAECONNRESET,
		所以下面参数关闭了报告WSAECONNRESET
*/

#if !defined(SIO_UDP_CONNRESET)
		#define   IOC_VENDOR   0x18000000
		#define   _WSAIOW(x,y)   (IOC_IN|(x)|(y))
		#define   SIO_UDP_CONNRESET   _WSAIOW(IOC_VENDOR,12)
#endif

#define WINSOCK_UDP_CONNRESET_BUG(sock_fd)								\
		do{																\
				BOOL bNewBehavior = FALSE;								\
				DWORD dwBytesReturned = 0;								\
																		\
						WSAIoctl(sock_fd,								\
								SIO_UDP_CONNRESET,						\
								&bNewBehavior,							\
								sizeof(bNewBehavior),					\
								NULL,									\
								0,										\
								&dwBytesReturned,						\
								NULL,									\
								NULL);									\
		}while(0)



int Com_socket_nonblocking(SOCKET fd, bool_t is_enable);

int Com_connect_timeout(SOCKET fd, const struct sockaddr* name, int namelen, const uint_64_t	*timeout_millisecond);

SOCKET Com_accpet_timeout(SOCKET fd, struct sockaddr* addr, int* addrlen, const uint_64_t	*timeout_millisecond);


int Com_sendto_timeout(SOCKET fd, const char* buf, int len, int flags, const struct sockaddr* to, int tolen, const uint_64_t	*timeout_millisecond);
int Com_recvfrom_timeout(SOCKET fd, char* buf, int len, int flags, struct sockaddr* from, int* fromlen, const uint_64_t	*timeout_millisecond);

int Com_send_timeout(SOCKET fd, const char* buf, int len, int flags, const uint_64_t *timeout_millisecond);
int Com_recv_timeout(SOCKET fd, char* buf, int len, int flags, const uint_64_t	*timeout_millisecond);


int Com_recv_all_timeout(SOCKET fd, char* buf, int len, int flags, int *bt, const uint_64_t *timeout);
int Com_send_all_timeout(SOCKET fd, const char* buf, int len, int flags, int *bt, const uint_64_t *timeout);




/**********************************************Ini Object****************************/


struct __ini_object_tag;
typedef struct __ini_object_tag iniObject_t;

#define			INI_EMPTY_SECTION_NAME	L"[[__empty_header__]]"


iniObject_t*	Ini_CreateObject();
void			Ini_DestroyObject(iniObject_t *obj);
void			Ini_ClearObject(iniObject_t *obj);

bool_t			Ini_LoadObjectFromString(iniObject_t *obj, const wchar_t *ini_data);
void			Ini_SaveObjectToString(const iniObject_t *obj, cmString_t *out);


bool_t			Ini_SectionIsExisted(const iniObject_t *obj, const wchar_t *sect);
bool_t			Ini_InsertSection(iniObject_t *obj, const wchar_t *sect, const wchar_t *comment);
bool_t			Ini_RemoveSection(iniObject_t *obj, const wchar_t *sect);

const wchar_t*	Ini_GetString(const iniObject_t *obj, const wchar_t *sect, const wchar_t *key);
bool_t			Ini_SetString(iniObject_t *obj, const wchar_t *sect, const wchar_t *key, const wchar_t *val, const wchar_t *comment);

bool_t			Ini_RemoveKey(iniObject_t *obj, const wchar_t *sect, const wchar_t *key);
bool_t			Ini_SetComment(iniObject_t *obj, const wchar_t *sect, const wchar_t *key, const wchar_t *comment);
const wchar_t*	Ini_GetComment(const iniObject_t *obj, const wchar_t *sect, const wchar_t *key);





int_64_t		Ini_GetInt(const iniObject_t *obj, const wchar_t *sect, const wchar_t *key, int_64_t default_data);
uint_64_t		Ini_GetUInt(const iniObject_t *obj, const wchar_t *sect, const wchar_t *key, uint_64_t default_data);
double			Ini_GetFloat(const iniObject_t *obj, const wchar_t *sect, const wchar_t *key, double default_data);



void			Ini_SetInt(iniObject_t *obj, const wchar_t *sect, const wchar_t *key, int_64_t val, const wchar_t *comment);
void			Ini_SetUInt(iniObject_t *obj, const wchar_t *sect, const wchar_t *key, uint_64_t val, const wchar_t *comment);
void			Ini_SetFloat(iniObject_t *obj, const wchar_t *sect, const wchar_t *key, double val, const wchar_t *comment);




MM_NAMESPACE_END


#endif