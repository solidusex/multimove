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

#ifndef __MMCORE_NEGMSG_H__
#define __MMCORE_NEGMSG_H__

#include "../Common/common.h"

MM_NAMESPACE_BEGIN



/*在server端，如果位置为NM_POS_LEFT， 则证明此台计算机在Client的左侧，否则反之*/
typedef enum
{
		NM_POS_LEFT = 0x00,
		NM_POS_RIGHT,
		NM_POS_UP,
		NM_POS_DOWN,
		NM_POS_MAX
}nmPosition_t;


#define NM_KEEPALIVE_TIMEOUT			100 * 1000		/*10秒钟没有任何socket上的IO操作则认为该客户端已不存在*/
#define NM_TIMER_TICK					1  * 1000		/*2秒检查一次*/


#define NM_BLOCKING_TIMEOUT				20				/*所有阻塞等待最长为20毫秒*/




#define NM_PACKAGE_HEADER_LENGTH			4

typedef enum
{
		NM_MSG_KEEPALIVE = 0x00,		/* Client -> Server && Server -> Client */
		NM_MSG_HANDSHAKE,				/*Client -> Server*/
		NM_MSG_HANDSHAKE_REPLY,			/*Server -> Client*/

		NM_MSG_ENTER,					/*Client -> Server*/
		NM_MSG_LEAVE,					/*Server -> Client*/

		NM_MSG_MOUSE,					/*Client -> Server*/
		NM_MSG_KEYBOARD, 				/*Client -> Server*/
		NM_MSG_CLIPDATA					/*Client -> Server && Server -> Client */
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




typedef enum
{
		NM_CLIP_TEXT	/*UTF8编码文本*/
}nmClipDataType_t;

typedef struct __clipboard_data_tag
{
		nmClipDataType_t		data_type;
		const byte_t			*data;
		size_t					length;
}nmClipboardData_t;


typedef struct __netmsg_tag
{
		nmMsgType_t		t;
		union{
				nmHandShakeMsg_t		handshake;
				nmEnterMsg_t			enter;
				nmLeaveMsg_t			leave;
				nmMouseMsg_t			mouse;
				nmKeyboardMsg_t			keyboard;
				nmClipboardData_t		clip_data;
		};
}nmMsg_t;



bool_t	NM_MsgToBuffer(const nmMsg_t	*msg, cmBuffer_t		*out);
bool_t	NM_ParseFromBuffer(const byte_t *data, size_t len, nmMsg_t	*msg);




typedef enum
{
		NM_RECV_WAIT_HEADER,
		NM_RECV_WAIT_PACKAGE
}nmRecvState_t;




MM_NAMESPACE_END


#endif
