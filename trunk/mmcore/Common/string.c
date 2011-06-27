#include "common.h"

MM_NAMESPACE_BEGIN




struct __com_string_tag
{
		wchar_t			*str;
		size_t			count;
		size_t			cap;
};

cmString_t*	Com_CreateString()
{
		cmString_t *str;
		str = Com_NEW0(cmString_t);
		
		Com_ReserveString(str, 8);
		return str;
}



void	Com_DestroyString(cmString_t *str)
{
		Com_ASSERT(str != NULL);
		if(str->str)
		{
				Com_DEL(str->str);
		}
		Com_DEL(str);
}



const wchar_t*	Com_GetStrString(const cmString_t *str)
{
		Com_ASSERT(str != NULL && str->str != NULL);
		return str->str;
}

size_t	Com_GetLengthString(const cmString_t *str)
{
		Com_ASSERT(str != NULL && str->str != NULL);
		return str->count;
}

void			Com_ReserveString(cmString_t *str, size_t num)
{
		
		size_t i;
		
		Com_ASSERT(str != NULL);
		num += 1;

		if(str->cap - str->count < num)
		{
				str->cap = (str->count + num)*2;
				str->str = Com_REALLOC(wchar_t, str->str, str->cap);
				for(i = str->count; i < str->cap; ++i)
				{
						str->str[i] = L'\0';
				}
		}
}

void			Com_ClearString(cmString_t *str)
{
		Com_ASSERT(str != NULL);
		str->str[0] = L'\0';
		str->count = 0;
}

size_t	Com_AppendString(cmString_t *str, const wchar_t *sour)
{
		size_t len;
		size_t i;
		Com_ASSERT(str != NULL && sour != NULL);
		len = Com_wcslen(sour);
		Com_ReserveString(str,len);
		
		for(i = 0; i < len; ++i)
		{
				str->str[str->count++] = sour[i];
		}
		str->str[str->count] = L'\0';
		return len;
}



void	Com_FormatString(cmString_t *str, const wchar_t *fmt, ...)
{
		va_list args;
		Com_ASSERT(str != NULL && fmt != NULL);

		va_start(args, fmt);
		Com_VFormatString(str, fmt, args);
		va_end(args);
}




void	Com_AppendFormatString(cmString_t *str, const wchar_t *fmt, ...)
{
		va_list args;
		Com_ASSERT(str != NULL && fmt != NULL);
		
		va_start(args, fmt);
		Com_AppendVFormatString(str, fmt, args);
		va_end(args);
}



void			Com_VFormatString(cmString_t *str, const wchar_t *fmt, va_list args)
{
		int_t len;
		Com_ASSERT(str != NULL && fmt != NULL && args != NULL);
		Com_ClearString(str);
		
		len = Com_vscwprintf(fmt, args);
		Com_ASSERT(len >= 0);
		Com_ReserveString(str, (size_t)len + 1);
		
		str->str[0] = L'\0';
		len = Com_vswprintf(str->str, str->cap-1, fmt, args) + 1;
		
		Com_ASSERT(len >= 0);

		
		str->str[len] = L'\0';
		str->count = Com_wcslen(str->str);
}

void			Com_AppendVFormatString(cmString_t *str, const wchar_t *fmt, va_list args)
{
		int_t len;
		wchar_t *buf;
		
		Com_ASSERT(str != NULL && fmt != NULL && args != NULL);
		
		len = Com_vscwprintf(fmt, args);
		Com_ASSERT(len >= 0);

		buf = Com_NEWARR0(wchar_t, len + 1);
		len = Com_vswprintf(buf, len + 1, fmt, args);
		
		Com_ASSERT(len >= 0);

		buf[len] = L'\0';
		Com_AppendString(str, buf);
		Com_DEL(buf);
}


void			Com_AppendCharToString(cmString_t *str, wchar_t chr)
{
		Com_ReserveString(str, 1);
		str->str[str->count++] = chr;
}


MM_NAMESPACE_END
