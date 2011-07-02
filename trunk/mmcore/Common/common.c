#include "common.h"

MM_NAMESPACE_BEGIN



void	__stdcall __print_func(const wchar_t *msg, void *ctx)
{
		Com_UNUSED(ctx);
		Com_UNUSED(msg);
		Com_ASSERT(msg != NULL);
		
		Com_DebugOutput(msg);
}

void	__stdcall __error_func(int_t level, const wchar_t *msg, void *ctx)
{
		Com_UNUSED(ctx);
		Com_UNUSED(msg);
		Com_ASSERT(msg != NULL);
		
		Com_DebugOutput(msg);
		if(level == COM_ERR_FATAL)
		{
				Com_Abort();
		}
}


static cmInit_t	__g_init = 
{
		{__print_func, __error_func, NULL}
};




/*****************************************************************************/

/*
init winsock 
*/
static bool_t __net_init()
{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD( 2, 2 );

		err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 ) 
		{
				return false;
		}else
		{
				return true;
		}
}


static bool_t	__net_uninit()
{
		/*
		MSND:

		An application must call the WSACleanup function for every successful time the WSAStartup function is called. 
		This means, for example, that if an application calls WSAStartup three times, it must call WSACleanup three times. 
		The first two calls to WSACleanup do nothing except decrement an internal counter; the final WSACleanup call for the 
		task does all necessary resource deallocation for the task.
		*/
		return WSACleanup() == 0;
}

/*****************************************************************************/
bool_t	Com_Init(const cmInit_t *init)
{

		if(!__net_init())
		{
				return false;
		}

		if(init)
		{
				__g_init = *init;
		}

		Com_printf(L"On Com_Init\r\n");
		return true;
}

bool_t	Com_UnInit()
{
		Com_printf(L"On Com_UnInit\r\n");

		__net_uninit();
		return true;
}




void	Com_printf(const wchar_t *fmt, ...)
{
		wchar_t *buf;
		int_t len;
		va_list arg_ptr;
		
		if(__g_init.io_ctx.on_print != NULL)
		{
				va_start(arg_ptr, fmt);
				len = Com_vscwprintf(fmt, arg_ptr);
				va_end(arg_ptr);

				if(len <= 0)return;
				
				buf = Com_NEWARR0(wchar_t, len + 1);
				
				va_start(arg_ptr, fmt);
				if(Com_vswprintf(buf, len + 1, fmt, arg_ptr) <= 0)
				{
						buf[0] = L'\0';
						len = 0;
				}
				va_end(arg_ptr);
		
				__g_init.io_ctx.on_print(buf, __g_init.io_ctx.ctx);
				
				Com_DEL(buf);
		}
}

void	Com_error(int_t level, const wchar_t *msg,...)
{

		wchar_t *buf;
		int_t len;
		va_list arg_ptr;
		
		if(__g_init.io_ctx.on_error != NULL)
		{
				
				va_start(arg_ptr, msg);
				len = Com_vscwprintf(msg, arg_ptr);
				va_end(arg_ptr);

				if(len <= 0)
				{
						len = 0;
				}
				
				buf = Com_NEWARR0(wchar_t, len + 1);
				
				va_start(arg_ptr, msg);
				if(Com_vswprintf(buf, len + 1, msg, arg_ptr) <= 0)
				{
						len = 0;
						buf[0] = L'\0';
				}
				va_end(arg_ptr);
		
				__g_init.io_ctx.on_error(level, buf, __g_init.io_ctx.ctx);
				
				Com_DEL(buf);
		}
		
		if(level == COM_ERR_FATAL)
		{
				Com_Abort();
		}
}

void	Com_check(bool_t cond, const wchar_t *fmt, ...)
{
		wchar_t *buf;
		int_t len;
		va_list arg_ptr;
		Com_ASSERT(fmt != NULL);
		
		if(!cond)
		{
				va_start(arg_ptr, fmt);
				len = Com_vscwprintf(fmt, arg_ptr);
				va_end(arg_ptr);
				
				if(len < 0)
				{
						len = 0;
				}

				buf = Com_NEWARR0(wchar_t, len + 1);

				va_start(arg_ptr, fmt);
				len = Com_vswprintf(buf, len + 1, fmt, arg_ptr);
				va_end(arg_ptr);

				if(len <= 0)
				{
						buf[0] = L'\0';
						len = 0;
				}

				Com_error(COM_ERR_FATAL, buf );
				Com_DEL(buf);
		}
}


/******************************************String*************************************/


uint_t	Com_wcshash(const wchar_t *str)
{
		uint_t	ret;
		size_t	i;
		Com_ASSERT(str != NULL);

		ret = 0;
		for(i = 0; str[i]; ++i)
		{
				ret ^= (str[i] << (i & 0x0F));
		}
		return ret;
}


uint_t		Com_wcshash_n(const wchar_t *str, size_t n)
{
		uint_t	ret;
		size_t	i;
		if(n == 0)return 0;
		Com_ASSERT(str != NULL);

		ret = 0;
		for(i = 0; i < n && str[i]; ++i)
		{
				ret ^= (str[i] << (i & 0x0F));
		}
		return ret;
}




const char* Com_strtrim(const char *in, const char *trim)
{
		Com_ASSERT(in != NULL && trim != NULL);

		while(*in != '\0' && Com_strchr(trim, *in) != NULL)in++;
		return in;
}


const char*	Com_strtrim_space(const char *in)
{
		Com_ASSERT(in != NULL);
		while(*in != '\0' && Com_isspace(*in))in++;
		return in;
}



char* Com_strndup(const char *sour, size_t len)
{
		char *result;
		size_t i;
		Com_ASSERT(sour != NULL);
		
		result = Com_NEWARR(char, len + 1);
		for(i = 0; i < len; ++i)result[i] = sour[i];
		result[len] = '\0';
		return result;
}


char* Com_strdup(const char *s)
{
		Com_ASSERT(s != NULL);
		return Com_strndup(s, Com_strlen(s));
}

wchar_t*	Com_wcsdup(const wchar_t *s)
{
	Com_ASSERT(s != NULL);
	return Com_wcsndup(s, Com_wcslen(s));
}
wchar_t*	Com_wcsndup(const wchar_t *sour, size_t len)
{
	wchar_t *result;
	size_t i;
	Com_ASSERT(sour != NULL);
	result = Com_NEWARR(wchar_t, len + 1);
	for(i = 0; i < len; ++i)result[i] = sour[i];
	result[len] = L'\0';
	return result;
}






int_t Com_wchartodigit(wchar_t ch)
{

		#define DIGIT_RANGE_TEST(zero)  \
		if (ch < zero)					\
				return -1;              \
		if (ch < zero + 10)				\
		{								\
				return ch - zero;       \
		}


		DIGIT_RANGE_TEST(0x0030)			/* 0030;DIGIT ZERO*/
		if (ch < 0xFF10)					/* FF10;FULLWIDTH DIGIT ZERO*/
		{
				DIGIT_RANGE_TEST(0x0660)    /* 0660;ARABIC-INDIC DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x06F0)    /* 06F0;EXTENDED ARABIC-INDIC DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0966)    /* 0966;DEVANAGARI DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x09E6)    /* 09E6;BENGALI DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0A66)    /* 0A66;GURMUKHI DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0AE6)    /* 0AE6;GUJARATI DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0B66)    /* 0B66;ORIYA DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0C66)    /* 0C66;TELUGU DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0CE6)    /* 0CE6;KANNADA DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0D66)    /* 0D66;MALAYALAM DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0E50)    /* 0E50;THAI DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0ED0)    /* 0ED0;LAO DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x0F20)    /* 0F20;TIBETAN DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x1040)    /* 1040;MYANMAR DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x17E0)    /* 17E0;KHMER DIGIT ZERO*/
				DIGIT_RANGE_TEST(0x1810)    /* 1810;MONGOLIAN DIGIT ZERO*/

				return -1;
		}

		if (ch < 0xFF10 + 10)/* FF10;FULLWIDTH DIGIT ZERO*/
		{
				return ch - 0xFF10;
		}
		return -1;

		#undef DIGIT_RANGE_TEST
}



#if (WINVER < 0x401)

#define __MODIFIER_ANSI			0x10000
#define __MODIFIER_UNICODE		0x20000
#define	__MODIFIER_INT64		0x40000

int_t Com_vscwprintf(const wchar_t *fmt, va_list args)
{
		int_t res;
		va_list save;
		Com_ASSERT(fmt != NULL && args != NULL);
		res = 0;
		Com_memcpy(&save, &args, sizeof(va_list));

		while(*fmt)
		{
				int_t len = 0, width = 0, prec = 0, modifier = 0;
				if(*fmt != L'%' || *(++fmt) == L'%')
				{
						fmt++;
						res++;
						continue;
				}

				/*处理%后面的*/


				while(*fmt)
				{
						if(*fmt == L'#')
						{
								res += 2; /*0x*/
						}else if(*fmt == L'*')
						{
								width = va_arg(args, int);
						}else if(*fmt == L'-' || *fmt == L'+' || *fmt == L'0' || *fmt == L' ')
						{

						}else
						{
								break;
						}
						fmt++;
				}

				if(width == 0)
				{
						while(*fmt && Com_iswdigit(*fmt))
						{
								width *= 10;
								width += Com_wchartodigit(*fmt);
								fmt++;
						}
				}

				if(*fmt == L'.')
				{
						/*width.prec*/
						fmt++;
						if(*fmt == L'*')
						{
								prec = va_arg(args, int_32_t);
								fmt++;
						}else
						{
								prec = 0;
								while(*fmt && Com_iswdigit(*fmt))
								{
										prec *= 10;
										prec += Com_wchartodigit(*fmt);
										fmt++;
								}
						}

				}

				modifier = 0;

				if(*fmt == L'I' && *(fmt + 1) == L'6' && *(fmt + 2) == L'4')/*VC,BCB等*/
				{
						/*例如%I64d*/
						fmt += 3;
						modifier |= __MODIFIER_INT64;
				}else if(*fmt == L'I' && *(fmt + 1) == L'3' && *(fmt + 2) == L'2')/*VC,BCB等*/
				{
						/*例如%I32d*/
						fmt += 3;
				}else if(*fmt == L'I')
				{
						/*例如%Id*/
						fmt += 1;

						#if(PLATFORM_VER != PLAT_I386)
								modifier |= __MODIFIER_INT64;
						#endif

				}else if(*fmt == L'l' && *(fmt + 1) == L'l')/*VC gcc一族编译器*/
				{
						/*例如%lld*/
						fmt += 2;
						modifier |= __MODIFIER_INT64;
				}else
				{
						switch (*fmt)
						{
								/*强制ANSI*/
						case L'h':
								modifier |= __MODIFIER_ANSI;
								fmt++;
								break;
						case L'l':
								modifier |= __MODIFIER_UNICODE;
								fmt++;
								break;
								/*无用*/
						case L'F':
						case L'N':
						case L'L':
								fmt++;
								break;
						}
				}

				switch (*fmt | modifier)
				{
				case L'c':
				case L'C':
						len = 2;
						va_arg(args, wint_t);
						break;
				case L'c' | __MODIFIER_ANSI:
				case L'C' | __MODIFIER_ANSI:

						len = 2;
						va_arg(args, char);
						break;
				case L'c' | __MODIFIER_UNICODE:
				case L'C' | __MODIFIER_UNICODE:
						len = 2;
						va_arg(args, wchar_t);
						break;
				case L's':
				{
						const wchar_t *str = va_arg(args, const wchar_t*);
						if(str == NULL)
						{
								len = 6;/*(null)*/
						}else
						{
								len =(int_t)Com_wcslen(str);
								len = Com_MAX(len, 1);
						}
						break;
				}
				case L'S':
				{
						const char *str = va_arg(args, const char*);
						if(str == NULL)
						{
								len = 6;/*(null)*/
						}else
						{
								len = (int_t)Com_strlen(str);
								len = Com_MAX(len,1);
						}
						break;
				}
				case L's' | __MODIFIER_ANSI:
				case L'S' | __MODIFIER_ANSI:
				{
						const char *str = va_arg(args, const char*);
						if(str == NULL)
						{
								len = 6;/*(null)*/
						}else
						{
								len = (int_t)Com_strlen(str);
								len = Com_MAX(len,1);
						}
						break;
				}
				case L's' | __MODIFIER_UNICODE:
				case L'S' | __MODIFIER_UNICODE:
				{
						const wchar_t *str = va_arg(args, const wchar_t*);
						if(str == NULL)
						{
								len = 6;/*(null)*/
						}else
						{
								len = (int_t)Com_wcslen(str);
								len = Com_MAX(len, 1);
						}
						break;

				}
				}

				if(len != 0)
				{
						len = Com_MAX(len, width);
						if(prec != 0)len = Com_MIN(len, prec);
				}else
				{
						switch (*fmt)
						{
						case L'd':
						case L'i':
						case L'u':
						case L'x':
						case L'X':
						case L'o':
						{
								if(modifier & __MODIFIER_INT64)
								{
										va_arg(args, int_64_t);
								}else
								{
										va_arg(args, int_32_t);
								}
								len = 32;
								len = Com_MAX(len, width + prec);
								break;
						}
						case L'e':
						case L'E':
						case L'g':
						case L'G':
						{
								va_arg(args, double);
								len = 128;
								len = Com_MAX(len, width + prec);
								break;
						}
						case L'f':
						{
								int_t cclen;
								wchar_t *buf;
								double f= va_arg(args, double);

								/*
										312 == wcslen(L"-1+(0{309})")
										double 最大精度为.0{309},这里精度默认设置为6
								*/

								cclen = Com_MAX(width, 312 + prec + 6);/*取最大值*/

								buf = Com_NEWARR0(wchar_t, cclen);
								Com_swprintf(buf, cclen, L"%*.*f", width, prec + 6, f);
								len = Com_wcslen(buf);
								Com_DEL(buf);
								break;
						}
						case L'p':
						{
								va_arg(args, void*);
								len = Com_MAX(32, width + prec);
								break;
						}
						case L'n':
						{
								va_arg(args, int*);
								break;
						}
						default:
						{
								Com_ASSERT(false);
								return -1;
						}

						}
				}
				fmt++;
				res += len;
		}
		va_end(save);
		return res;
}
#undef	__MODIFIER_ANSI
#undef	__MODIFIER_UNICODE
#undef	__MODIFIER_INT64


#endif


int_t			Com_swprintf(wchar_t *dest, size_t count, const wchar_t *fmt, ...)
{
		int_t len = -1;
		va_list	arg_ptr;
		Com_ASSERT(fmt != NULL);

		va_start(arg_ptr, fmt);
		len = Com_vswprintf(dest, count, fmt, arg_ptr);
		va_end(arg_ptr);
		
		if(len <= 0)
		{
				dest[0] = L'\0';
				len = 0;
		}
		return len;
}







int_t			Com_vswprintf(wchar_t *dest, size_t count, const wchar_t *fmt, va_list args)
{
		int_t res;
		va_list save;
		Com_ASSERT(dest != NULL && fmt != NULL && args != NULL);
		res = 0;
		Com_memcpy(&save, &args, sizeof(va_list));

		res = _vsnwprintf(dest, count, fmt, args);
		
		va_end(save);

		if(res <= 0)
		{
				dest[0] = L'\0';
				res = 0;
		}
		return res;
}



int_t			Com_vsprintf(char *dest, size_t count, const char *fmt, va_list args)
{
		int_t res;
		va_list save;
		Com_ASSERT(dest != NULL && fmt != NULL && args != NULL);
		res = 0;
		Com_memcpy(&save, &args, sizeof(va_list));

		res = _vsnprintf(dest, count, fmt, args);
		
		va_end(save);

		if(res <= 0)
		{
				dest[0] = '\0';
				res = 0;
		}
		return res;
}

int_t		Com_sprintf(char *dest, size_t count, const char *fmt, ...)
{		
		int_t len = -1;
		va_list	arg_ptr;
		Com_ASSERT(fmt != NULL);

		va_start(arg_ptr, fmt);
		len = Com_vsprintf(dest, count, fmt, arg_ptr);
		va_end(arg_ptr);
		
		if(len <= 0)
		{
				dest[0] = '\0';
				len = 0;
		}
		return len;
}




const wchar_t*	Com_wcstrim_space(const wchar_t *in)
{
		Com_ASSERT(in != NULL);
		while(*in != L'\0' && Com_iswspace(*in))in++;
		return in;
}

wchar_t*	Com_wcstrim_right_space(wchar_t *in)
{
		wchar_t *p = NULL, *plast = NULL;
		Com_ASSERT(in != NULL);
		p = in;
		while(*p != L'\0')
		{
				if(Com_iswspace(*p) == 0)
				{
						plast = NULL;
				}else if(plast == NULL)
				{
						plast = p;
				}
				++p;
		}

		if(plast)*plast = L'\0';
		return in;
}



/******************************************Memory**********************************/

/*
我不认为在这种类型的软件中，内存不足会成为问题。
*/


void*	Com_malloc(size_t nbytes)
{
		void *ptr;
		Com_ASSERT(nbytes > 0);
		while((ptr = malloc(nbytes)) == NULL)Com_Yield();

		return ptr;
}


void*	Com_calloc(size_t num, size_t size)
{
		void *ptr;
		

		Com_ASSERT(num > 0 && size > 0);
		while((ptr = calloc(num,size)) == NULL)Com_Yield();
		return ptr;
}		


void*	Com_realloc(void *block, size_t nbytes)
{
		void *ptr;

		Com_ASSERT(nbytes > 0);

		
		while((ptr = realloc(block, nbytes)) == NULL)Com_Yield();
		return ptr;
}


void	Com_free(void *ptr)
{
		Com_ASSERT(ptr != NULL);
		if(ptr)free(ptr);
}


/************************************************memory**********************************************/



MM_NAMESPACE_END

