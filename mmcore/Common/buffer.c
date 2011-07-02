
#include "common.h"


MM_NAMESPACE_BEGIN







/*-------------------------------------------------------------------------*/
struct commonl_buffer_tag
{
		byte_t	*first;
		byte_t	*last;
		byte_t	*read_cur;
		byte_t	*write_cur;
};





#if defined(DEBUG_FLAG)

static bool_t	__buffer_is_valid(const cmBuffer_t *pbuf)
{
		if(pbuf == NULL)return false;

		if(pbuf->read_cur < pbuf->first || pbuf->read_cur > pbuf->last || pbuf->write_cur < pbuf->first || pbuf->write_cur > pbuf->last)return false;

		if(pbuf->first > pbuf->last)return false;

		return true;
}

#else

#define __buffer_is_valid(_pbuf)		


#endif

static  void		__increase_capability(cmBuffer_t *pbuf, size_t inc_len)
{
		size_t cur_len = 0;
		size_t data_len = 0;
		byte_t *new_buf = NULL;
		byte_t *old_buf = NULL;
		Com_ASSERT(__buffer_is_valid(pbuf));
		
		if(inc_len == 0)return;

		cur_len = pbuf->last - pbuf->first;
		data_len = pbuf->write_cur - pbuf->read_cur;

		new_buf = Com_NEWARR(byte_t, inc_len + cur_len);

		Com_ASSERT(new_buf != NULL);

		if(data_len > 0)
		{
				Com_memcpy(new_buf, pbuf->read_cur, data_len);
		}

		old_buf = pbuf->first;
		pbuf->first = new_buf;
		pbuf->last = pbuf->first + inc_len + cur_len;
		pbuf->read_cur = pbuf->first;
		pbuf->write_cur = pbuf->read_cur + data_len;
		Com_DEL(old_buf);
}


static   bool_t __move_internal(cmBuffer_t *pbuf, size_t len)
{
		size_t data_len = 0;	/* r***w read_cur 至write_cur之间的可读数据成都*/
		size_t vacancy_len = 0;	/* ***r read_cur之前的空白空间  */
		size_t free_len = 0;	/* ***r  w*** first 至read_cur和 write_cur 至 last的空闲空间数量之和*/
		Com_ASSERT(__buffer_is_valid(pbuf) && len > 0);

		data_len = pbuf->write_cur - pbuf->read_cur;
		vacancy_len = pbuf->read_cur - pbuf->first;
		free_len = (pbuf->last - pbuf->write_cur) + vacancy_len;

		if(free_len < len) return false;
		
		if(data_len > 0)
		{
				Com_memmove(pbuf->first, pbuf->read_cur, data_len);
		}

		pbuf->read_cur = pbuf->first;
		pbuf->write_cur = pbuf->first + data_len;

		Com_ASSERT((size_t)(pbuf->last - pbuf->write_cur) >= len);
		return true;

}




#define INIT_BUF_LEN	1024

cmBuffer_t*		Com_CreateBuffer(size_t nbytes)
{
		cmBuffer_t		*buf;
		
		buf = Com_NEW0(cmBuffer_t);
		if(nbytes > 0)
		{
				nbytes = Com_MAX(nbytes, INIT_BUF_LEN);
				__increase_capability(buf, nbytes);
		}

		return buf;
}

void			Com_DestroyBuffer(cmBuffer_t		*buffer)
{
		Com_ASSERT(__buffer_is_valid(buffer));
		Com_ClearBuffer(buffer);
		Com_DEL(buffer);

}

void			Com_ClearBuffer(cmBuffer_t		*buffer)
{
		Com_ASSERT(__buffer_is_valid(buffer));
		if(buffer->first)
		{
				Com_DEL(buffer->first);
		}
		Com_memset(buffer, 0, sizeof(*buffer));
}

void			Com_ReserveBuffer(cmBuffer_t *pbuf, size_t nbytes)
{
		size_t inc_len = 0;
		size_t capa_len = 0;
		size_t cur_buf_len = 0;
		Com_ASSERT(__buffer_is_valid(pbuf));
		
		capa_len = Com_GetBufferCapacity(pbuf);
		cur_buf_len = pbuf->last - pbuf->first;
		if(capa_len < nbytes)
		{
				inc_len = (size_t)Com_MAX((nbytes - capa_len),  cur_buf_len * 2 - cur_buf_len);
				__increase_capability(pbuf, inc_len);
		}
}

byte_t*			Com_AllocBuffer(cmBuffer_t *buffer, size_t	nbytes)
{
		size_t write_able = 0;
		byte_t *res = NULL;
		Com_ASSERT(__buffer_is_valid(buffer));

		if(nbytes == 0) return NULL;

		write_able = buffer->last - buffer->write_cur;
		if(write_able < nbytes && !__move_internal(buffer, nbytes))
		{
				Com_ReserveBuffer(buffer, nbytes);
		}
		
		Com_ASSERT((size_t)(buffer->last- buffer->write_cur) >= nbytes);

		res = buffer->write_cur;
		buffer->write_cur += nbytes;
		return res;
}


void			Com_InsertBuffer(cmBuffer_t *buffer, const byte_t *data, size_t len)
{
		byte_t *ptr = NULL;
		Com_ASSERT(__buffer_is_valid(buffer));
		if(len == 0)return;
		ptr = Com_AllocBuffer(buffer, len);

		Com_ASSERT(ptr != NULL);
		memcpy(ptr, data, len);
}

size_t			Com_EraseBuffer(cmBuffer_t *pbuf, size_t nbytes)
{
		size_t rm_data_len = 0;
		Com_ASSERT(__buffer_is_valid(pbuf));
		if(nbytes == 0)return 0;

		rm_data_len = Com_MIN(Com_GetBufferAvailable(pbuf), nbytes);
		
		pbuf->read_cur += rm_data_len;

		Com_ASSERT(pbuf->read_cur <= pbuf->write_cur);

		if(pbuf->read_cur == pbuf->write_cur)
		{
				pbuf->read_cur = pbuf->first;
				pbuf->write_cur = pbuf->read_cur;
		}
		return rm_data_len;
}


size_t			Com_GetBufferCapacity(const cmBuffer_t *buffer)
{
		Com_ASSERT(__buffer_is_valid(buffer));
		return buffer->last - buffer->first - Com_GetBufferAvailable(buffer);
}


const byte_t*	Com_GetBufferData(const cmBuffer_t *pbuf)
{
		Com_ASSERT(__buffer_is_valid(pbuf));
		return Com_GetBufferAvailable(pbuf) > 0 ? pbuf->read_cur : NULL;
}

size_t			Com_GetBufferAvailable(const cmBuffer_t *buffer)
{
		Com_ASSERT(__buffer_is_valid(buffer));
		return (buffer->write_cur - buffer->read_cur);
}








MM_NAMESPACE_END


