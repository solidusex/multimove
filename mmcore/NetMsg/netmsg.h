#ifndef __MMCORE_NEGMSG_H__
#define __MMCORE_NEGMSG_H__

#include "../Common/common.h"

MM_NAMESPACE_BEGIN



/*在server端，如果位置为NM_POS_LEFT， 则证明此台计算机在Client的左侧，否则反之*/
typedef enum
{
		NM_POS_LEFT = 0x00,
		NM_POS_RIGHT,
		
		NM_POS_MAX
}nmPosition_t;


#define NM_KEEPALIVE_TIMEOUT			1000 * 1000		/*1000秒钟没有任何socket上的IO操作则认为该客户端已不存在*/
#define NM_TIMER_TICK					1  * 1000		/*2秒检查一次*/


#define NM_BLOCKING_TIMEOUT				20				/*所有阻塞等待最长为20毫秒*/




#define NM_PACKAGE_HEADER_LENGTH			2

typedef enum
{
		NM_MSG_KEEPALIVE = 0x00,		/* Client -> Server && Server -> Client */
		NM_MSG_HANDSHAKE,				/*Client -> Server*/
		NM_MSG_HANDSHAKE_REPLY,			/*Server -> Client*/

		NM_MSG_ENTER,					/*Client -> Server*/
		NM_MSG_LEAVE,					/*Server -> Client*/

		NM_MSG_MOUSE,					/*Client -> Server*/
		NM_MSG_KEYBOARD					/*Client -> Server*/
}nmMsgType_t;



typedef struct __handshake_msg_tag
{
		nmPosition_t	srv_pos;
}nmHandShakeMsg_t;


typedef struct __enter_msg_tag
{
		uint_32_t		src_x_fullscreen;
		uint_32_t		src_y_fullscreen;
		int_32_t		x;
		int_32_t		y;
}nmEnterMsg_t;


typedef struct __leave_msg_tag
{
		uint_32_t		src_x_fullscreen;
		uint_32_t		src_y_fullscreen;
		int_32_t		x;
		int_32_t		y;
}nmLeaveMsg_t;


typedef struct __mouse_msg_tag
{
		uint_32_t	msg;
		int_32_t	x;
		int_32_t	y;
		int_32_t	data;		/*滚轮偏移*/
}nmMouseMsg_t;


typedef struct __keyboard_msg_tag
{
		byte_t			vk;
		byte_t			scan;
		bool_t			is_keydown;
}nmKeyboardMsg_t;



typedef struct __netmsg_tag
{
		nmMsgType_t		t;
		union{
				nmHandShakeMsg_t		handshake;
				nmEnterMsg_t			enter;
				nmLeaveMsg_t			leave;
				nmMouseMsg_t			mouse;
				nmKeyboardMsg_t			keyboard;
		};
}nmMsg_t;



bool_t	NM_MsgToBuffer(const nmMsg_t	*msg, cmBuffer_t		*out);
bool_t	NM_ParseFromBuffer(const byte_t *data, size_t len, nmMsg_t	*msg);








MM_NAMESPACE_END


#endif
