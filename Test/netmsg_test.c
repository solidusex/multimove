
#define OEMRESOURCE




#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"


MM_NAMESPACE_BEGIN

		
//bool_t	NM_MsgToBuffer(const nmMsg_t	*msg, cmBuffer_t		*out);
//bool_t	NM_ParseFromBuffer(const byte_t *data, size_t len, nmMsg_t	*msg);


void msg_test_handshake_msg()
{
		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_HANDSHAKE;
		s.handshake.srv_pos = NM_POS_LEFT;
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 4);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}

void msg_test_handshake_reply_msg()
{

		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_HANDSHAKE_REPLY;
		
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 2);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}





void msg_test_enter_msg()
{

		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_ENTER;
		s.enter.src_x_fullscreen = 1024;
		s.enter.src_y_fullscreen = 768;
		s.enter.x = 33;
		s.enter.y = -33;
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 18);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}


void msg_test_leave_msg()
{

		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_LEAVE;
		s.enter.src_x_fullscreen = 1024;
		s.enter.src_y_fullscreen = 768;
		s.enter.x = 33;
		s.enter.y = -33;
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 18);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}



void msg_test_mouse_msg()
{

		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_MOUSE;
		s.mouse.x = -33;
		s.mouse.y = -22;
		s.mouse.msg = 1234;
		s.mouse.data = -123;
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 18);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}



void msg_test_keyboard_msg()
{

		nmMsg_t	s,d;
		cmBuffer_t *buf;
		buf = Com_CreateBuffer(1024);

		Com_memset(&s,0,sizeof(s));
		Com_memset(&d,0,sizeof(d));

		s.t = NM_MSG_KEYBOARD;
		s.keyboard.is_keydown = true;
		s.keyboard.vk = 125;
		s.keyboard.scan = 67;
		
		if(!NM_MsgToBuffer(&s, buf))
		{
				Com_ASSERT(false);
		}

		uint_16_t package_len;
		Com_memcpy(&package_len, Com_GetBufferData(buf), sizeof(package_len));
		package_len = COM_NTOL_U16(package_len);

		Com_ASSERT(package_len == 5);
		Com_EraseBuffer(buf, sizeof(package_len));

		if(!NM_ParseFromBuffer(Com_GetBufferData(buf), Com_GetBufferAvailable(buf), &d))
		{
				Com_ASSERT(false);
		}
		Com_ASSERT(Com_memcmp(&s, &d, sizeof(s)) == 0);

		Com_DestroyBuffer(buf);
		buf = NULL;
}






void NetMsg_Test()
{
		
		msg_test_handshake_msg();

		msg_test_handshake_reply_msg();

		msg_test_enter_msg();

		msg_test_leave_msg();

		msg_test_mouse_msg();

		msg_test_keyboard_msg();

}


MM_NAMESPACE_END