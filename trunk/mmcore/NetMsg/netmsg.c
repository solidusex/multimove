#include "netmsg.h"

MM_NAMESPACE_BEGIN




bool_t	keepalive_to_buffer(const nmMsg_t	*msg, cmBuffer_t		*out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type));
		package_type = NM_MSG_KEEPALIVE;
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		return true;
}

static bool_t	handshake_to_buffer(const nmMsg_t	*msg, cmBuffer_t		*out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;
		uint_8_t		pos;
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(pos));
		package_type = NM_MSG_HANDSHAKE;
		pos = (uint_8_t)msg->handshake.srv_pos;
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		Com_InsertBuffer(out, (const byte_t*)&pos, sizeof(pos));
		
		return true;

}

static bool_t	handshake_reply_to_buffer(const nmMsg_t	*msg, cmBuffer_t		*out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type));
		package_type = NM_MSG_HANDSHAKE_REPLY;
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		
		return true;
}

static bool_t	enter_to_buffer(const nmMsg_t *msg, cmBuffer_t *out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;


		uint_16_t		src_x_fullscreen;
		uint_16_t		src_y_fullscreen;
		int_16_t		x;
		int_16_t		y;
		
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(src_x_fullscreen) + sizeof(src_y_fullscreen) + sizeof(x) + sizeof(y));
		package_type = NM_MSG_ENTER;
		
		src_x_fullscreen = COM_LTON_U16(msg->enter.src_x_fullscreen);
		src_y_fullscreen = COM_LTON_U16(msg->enter.src_y_fullscreen);
		x = COM_LTON_16(msg->enter.x);
		y = COM_LTON_16(msg->enter.y);
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		

		Com_InsertBuffer(out, (const byte_t*)&src_x_fullscreen, sizeof(src_x_fullscreen));
		Com_InsertBuffer(out, (const byte_t*)&src_y_fullscreen, sizeof(src_y_fullscreen));
		Com_InsertBuffer(out, (const byte_t*)&x, sizeof(x));
		Com_InsertBuffer(out, (const byte_t*)&y, sizeof(y));

		return true;
}

static bool_t	leave_to_buffer(const nmMsg_t *msg, cmBuffer_t *out)
{
		
		uint_16_t		package_len;
		uint_8_t		package_type;

		uint_16_t		src_x_fullscreen;
		uint_16_t		src_y_fullscreen;
		int_16_t		x;
		int_16_t		y;
		
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(src_x_fullscreen) + sizeof(src_y_fullscreen) + sizeof(x) + sizeof(y));
		package_type = NM_MSG_LEAVE;
		
		src_x_fullscreen = COM_LTON_U16(msg->leave.src_x_fullscreen);
		src_y_fullscreen = COM_LTON_U16(msg->leave.src_y_fullscreen);
		x = COM_LTON_16(msg->leave.x);
		y = COM_LTON_16(msg->leave.y);
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		

		Com_InsertBuffer(out, (const byte_t*)&src_x_fullscreen, sizeof(src_x_fullscreen));
		Com_InsertBuffer(out, (const byte_t*)&src_y_fullscreen, sizeof(src_y_fullscreen));
		Com_InsertBuffer(out, (const byte_t*)&x, sizeof(x));
		Com_InsertBuffer(out, (const byte_t*)&y, sizeof(y));

		return true;
}

static bool_t	mouse_to_buffer(const nmMsg_t *msg, cmBuffer_t *out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;

		uint_16_t		msg_code;
		int_16_t		x;
		int_16_t		y;
		int_16_t		data;
		
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(msg_code) + sizeof(x) + sizeof(y) + sizeof(data));
		package_type = NM_MSG_MOUSE;
		
		msg_code = COM_LTON_U16(msg->mouse.msg);
		x = COM_LTON_16(msg->mouse.x);
		y = COM_LTON_16(msg->mouse.y);
		data = COM_LTON_16(msg->mouse.data);
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		

		Com_InsertBuffer(out, (const byte_t*)&msg_code, sizeof(msg_code));
		Com_InsertBuffer(out, (const byte_t*)&x, sizeof(x));
		Com_InsertBuffer(out, (const byte_t*)&y, sizeof(y));
		Com_InsertBuffer(out, (const byte_t*)&data, sizeof(data));


		return true;
}

static bool_t	keybd_to_buffer(const nmMsg_t *msg, cmBuffer_t *out)
{
		uint_16_t		package_len;
		uint_8_t		package_type;

		byte_t			vk;
		byte_t			scan;
		bool_t			is_keydown;
		
		Com_ASSERT(msg != NULL && out != NULL);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(vk) + sizeof(scan) + sizeof(is_keydown));
		package_type = NM_MSG_KEYBOARD;
		
		vk = msg->keyboard.vk;
		scan = msg->keyboard.scan;
		is_keydown = msg->keyboard.is_keydown;
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));
		

		Com_InsertBuffer(out, (const byte_t*)&vk, sizeof(vk));
		Com_InsertBuffer(out, (const byte_t*)&scan, sizeof(scan));
		Com_InsertBuffer(out, (const byte_t*)&is_keydown, sizeof(is_keydown));
		
		return true;
}


static bool_t	clipdata_to_buffer(const nmMsg_t *msg, cmBuffer_t *out)
{

		uint_16_t		package_len;
		uint_8_t		package_type;
		
		uint_8_t		clipdata_type;
		Com_ASSERT(msg && msg->t == NM_MSG_CLIPDATA && out != NULL);
		Com_ASSERT(msg->clip_data.data != NULL && msg->clip_data.length > 0);

		package_len = COM_LTON_U16(sizeof(package_type) + sizeof(clipdata_type) + msg->clip_data.length);
		package_type = NM_MSG_CLIPDATA;
		clipdata_type = msg->clip_data.data_type;
		
		
		Com_InsertBuffer(out, (const byte_t*)&package_len, sizeof(package_len));
		Com_InsertBuffer(out, (const byte_t*)&package_type, sizeof(package_type));

		Com_InsertBuffer(out, (const byte_t*)&clipdata_type, sizeof(clipdata_type));
		Com_InsertBuffer(out, (const byte_t*)msg->clip_data.data, msg->clip_data.length);
		return true;
}



bool_t	NM_MsgToBuffer(const nmMsg_t	*msg, cmBuffer_t		*out)
{
		Com_ASSERT(msg != NULL && out != NULL);

		switch(msg->t)
		{
		case NM_MSG_KEEPALIVE:
				return keepalive_to_buffer(msg, out);
		case NM_MSG_HANDSHAKE:
				return handshake_to_buffer(msg, out);
				break;
		case NM_MSG_HANDSHAKE_REPLY:
				return handshake_reply_to_buffer(msg, out);
				break;
		case NM_MSG_ENTER:
				return enter_to_buffer(msg, out);
				break;
		case NM_MSG_LEAVE:
				return leave_to_buffer(msg, out);
				break;
		case NM_MSG_MOUSE:
				return mouse_to_buffer(msg, out);
				break;
		case NM_MSG_KEYBOARD:
				return keybd_to_buffer(msg, out);
				break;
		case NM_MSG_CLIPDATA:
				return clipdata_to_buffer(msg, out);
		default:
				Com_error(COM_ERR_FATAL, L"Invalid msg type : %d\r\n", msg->t);
				return false; /*disable warning*/
				break;
		}
}



static bool_t parse_keepalive(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		Com_ASSERT(data != NULL && msg != NULL);

		if(len != 0)
		{
				return false;
		}
		
		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_KEEPALIVE;
		return true;
}





static bool_t parse_handshake(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		uint_8_t		pos;
		Com_ASSERT(data != NULL && msg != NULL);

		if(len != 1)
		{
				return false;
		}

		Com_memset(msg, 0, sizeof(*msg));

		Com_memcpy(&pos, data, sizeof(pos));
		data += 1;

		msg->t = NM_MSG_HANDSHAKE;
		msg->handshake.srv_pos = (nmPosition_t)pos;

		if(msg->handshake.srv_pos >= NM_POS_MAX)
		{
				return false;
		}

		return true;
}

static bool_t parse_handshake_reply(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		Com_ASSERT(data != NULL && msg != NULL);

		if(len != 0)
		{
				return false;
		}
		
		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_HANDSHAKE_REPLY;
		return true;
}

static bool_t parse_enter(const byte_t *data, size_t len, nmMsg_t	*msg)
{

		uint_16_t		src_x_fullscreen;
		uint_16_t		src_y_fullscreen;
		int_16_t		x;
		int_16_t		y;

		Com_ASSERT(data != NULL && msg != NULL);
		
		if(len != sizeof(src_x_fullscreen) + sizeof(src_y_fullscreen) + sizeof(x) + sizeof(y))
		{
				return false;
		}

		Com_memcpy(&src_x_fullscreen, data, sizeof(src_x_fullscreen));
		data += 2;

		Com_memcpy(&src_y_fullscreen, data, sizeof(src_y_fullscreen));
		data += 2;

		Com_memcpy(&x, data, sizeof(x));
		data += 2;

		Com_memcpy(&y, data, sizeof(y));
		data += 2;


		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_ENTER;
		msg->enter.src_x_fullscreen = COM_NTOL_U16(src_x_fullscreen);
		msg->enter.src_y_fullscreen = COM_NTOL_U16(src_y_fullscreen);
		msg->enter.x = COM_NTOL_16(x);
		msg->enter.y = COM_NTOL_16(y);

		return true;
}


static bool_t parse_leave(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		uint_16_t		src_x_fullscreen;
		uint_16_t		src_y_fullscreen;
		int_16_t		x;
		int_16_t		y;

		Com_ASSERT(data != NULL && msg != NULL);
		
		if(len != sizeof(src_x_fullscreen) + sizeof(src_y_fullscreen) + sizeof(x) + sizeof(y))
		{
				return false;
		}

		Com_memcpy(&src_x_fullscreen, data, sizeof(src_x_fullscreen));
		data += 2;

		Com_memcpy(&src_y_fullscreen, data, sizeof(src_y_fullscreen));
		data += 2;

		Com_memcpy(&x, data, sizeof(x));
		data += 2;

		Com_memcpy(&y, data, sizeof(y));
		data += 2;


		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_LEAVE;
		msg->leave.src_x_fullscreen = COM_NTOL_U16(src_x_fullscreen);
		msg->leave.src_y_fullscreen = COM_NTOL_U16(src_y_fullscreen);
		msg->leave.x = COM_NTOL_16(x);
		msg->leave.y = COM_NTOL_16(y);


		return true;
}



static bool_t parse_mouse(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		uint_16_t		msg_code;
		int_16_t		x;
		int_16_t		y;
		int_16_t		mouse_data;

		Com_ASSERT(data != NULL && msg != NULL);

		if(len != sizeof(msg_code) + sizeof(x) + sizeof(y) + sizeof(mouse_data))
		{
				return false;
		}


		Com_memcpy(&msg_code, data, sizeof(msg_code));
		data += 2;

		Com_memcpy(&x, data, sizeof(x));
		data += 2;

		Com_memcpy(&y, data, sizeof(y));
		data += 2;

		Com_memcpy(&mouse_data, data, sizeof(mouse_data));
		data += 2;


		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_MOUSE;
		msg->mouse.msg = COM_NTOL_U16(msg_code);
		msg->mouse.x = COM_NTOL_16(x);
		msg->mouse.y = COM_NTOL_16(y);
		msg->mouse.data = COM_NTOL_16(mouse_data);


		return true;
}


static bool_t parse_keybd(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		byte_t			vk;
		byte_t			scan;
		bool_t			is_keydown;

		Com_ASSERT(data != NULL && msg != NULL);

		if(len != sizeof(vk) + sizeof(scan) + sizeof(is_keydown))
		{
				return false;
		}

		vk = *data;
		data += 1;

		scan = *data;
		data += 1;

		is_keydown = *data != 0 ? true : false; /*Suppress Warning*/
		data += 1;

		Com_memset(msg, 0, sizeof(*msg));
		msg->t = NM_MSG_KEYBOARD;
		
		msg->keyboard.vk = vk;
		msg->keyboard.scan = scan;
		msg->keyboard.is_keydown = is_keydown;

		return true;
}


static bool_t parse_clipdata(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		uint_8_t clip_data_type;
		const byte_t *content;
		size_t length;
		Com_ASSERT(data != NULL &&  msg != NULL);
		
		if(len < 2) /*至少有一个类型和一个数据*/
		{
				return false;
		}

		clip_data_type = (uint_8_t)*data;
		data++;
		len--;
		switch(clip_data_type)
		{
		case NM_CLIP_TEXT:
				break;
		default:
				return false;
				break;
		}

		content = data;
		length = len;

		Com_memset(msg, 0, sizeof(*msg));
		
		msg->t = NM_MSG_CLIPDATA;
		msg->clip_data.data_type = (nmClipDataType_t)clip_data_type;
		msg->clip_data.data = content;
		msg->clip_data.length = length;
		
		return true;

}


bool_t	NM_ParseFromBuffer(const byte_t *data, size_t len, nmMsg_t	*msg)
{
		uint_8_t		package_type;
		const byte_t	*p;
		Com_ASSERT(data != NULL && msg != NULL);

		if(len < 1)
		{
				return false;
		}
		
		p = data;
		
		package_type = (uint_8_t)*p;
		p += 1;
		
		

		switch(package_type)
		{
		case NM_MSG_KEEPALIVE:
				return parse_keepalive(p, len - 1, msg);
		case NM_MSG_HANDSHAKE:
				return parse_handshake(p, len - 1, msg);
				break;
		case NM_MSG_HANDSHAKE_REPLY:
				return parse_handshake_reply(p, len - 1, msg);
				break;
		case NM_MSG_ENTER:
				return parse_enter(p, len - 1, msg);
				break;
		case NM_MSG_LEAVE:
				return parse_leave(p, len - 1, msg);
				break;
		case NM_MSG_MOUSE:
				return parse_mouse(p, len - 1, msg);
				break;
		case NM_MSG_KEYBOARD:
				return parse_keybd(p, len - 1, msg);
				break;
		case NM_MSG_CLIPDATA:
				return parse_clipdata(p, len - 1, msg);
		default:
				Com_error(COM_ERR_WARNING, L"Receive invalid msg type : %d\r\n", package_type);
				return false; /*disable warning*/
				break;
		}
}

MM_NAMESPACE_END