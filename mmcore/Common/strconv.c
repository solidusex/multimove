#include "common.h"

MM_NAMESPACE_BEGIN



#if(0)
static size_t __utf8_to_unicode_char(const byte_t *utf8, size_t len, wchar_t *uch)
{
		size_t v,n,e;
		const byte_t *p;
		
		Com_ASSERT(utf8 != NULL && len > 0);
		
		v = (size_t)(*utf8); n = 0; e= 0; p = utf8;
		
		if(v >= 0xfc)
		{
				n = 6;/*6:<11111100>*/
				if(n > len)return 0;
				e = (p[0] & 0x01) << 30;
				e |= (p[1] & 0x3f) << 24;
				e |= (p[2] & 0x3f) << 18;
				e |= (p[3] & 0x3f) << 12;
				e |= (p[4] & 0x3f) << 6;
				e |= (p[5] & 0x3f);
		}else if(v >= 0xf8)
		{
				n = 5;/*5:<11111000>*/
				if(n > len)return 0;
				 e = (p[0] & 0x03) << 24;
				e |= (p[1] & 0x3f) << 18;
				e |= (p[2] & 0x3f) << 12;
				e |= (p[3] & 0x3f) << 6;
				e |= (p[4] & 0x3f);
		
		}else if(v >= 0xf0)
		{
				n = 4; /*4:<11110000>*/
				if(n > len)return 0;
				e = (p[0] & 0x07) << 18;
				e |= (p[1] & 0x3f) << 12;
				e |= (p[2] & 0x3f) << 6;
				e |= (p[3] & 0x3f);
		}else if(v >= 0xe0)
		{
				n = 3;/*3:<11100000>*/
				if(n > len)return 0;
				e = (p[0] & 0x0f) << 12;
				e |= (p[1] & 0x3f) << 6;
				e |= (p[2] & 0x3f);

		}else if(v >= 0xc0)
		{
				n = 2;/*3:<11000000>*/
				if(n > len)return 0;
				e = (p[0] & 0x1f) << 6;
				e |= (p[1] & 0x3f);
		}else
		{
				n = 1;
				if(n > len)return 0;
				e = p[0];
		}
		if(uch)*uch = (wchar_t)e;
		return n;
}


static size_t __unicode_to_utf8_char(wchar_t uch, byte_t *utf8)
{
		byte_t buf[10];
		byte_t *e;
		uint_32_t	uc = (uint_32_t)uch;
		
		e = (utf8 ? utf8 : buf);
    
		if(uc < 0x80)
		{
				*e++ = (byte_t)uc;
		}else if(uc < 0x800)
		{
				/*<11011111> < 000 0000 0000>*/
				*e++ = (byte_t)((uc >> 6) & 0x1f)|0xc0;
				*e++ = (byte_t)(uc & 0x3f)|0x80; 
		}else if(uc < 0x10000)
		{
				/*<11101111> <0000 0000 0000 0000>*/
				*e++ = (byte_t)(((uc >> 12) & 0x0f)|0xe0);
				*e++ = (byte_t)(((uc >> 6) & 0x3f)|0x80);
				*e++ = (byte_t)((uc & 0x3f)|0x80);
		}else if(uc < 0x200000)
		{
				/*<11110111> <0 0000 0000 0000 0000 0000>*/
				*e++ = (byte_t)(((uc >> 18) & 0x07)|0xf0);
				*e++ = (byte_t)(((uc >> 12) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 6) & 0x3f)|0x80);
				*e++ = (byte_t)((uc & 0x3f)|0x80);
		}else if(uc < 0x4000000)
		{
				/*<11111011> <00 0000 0000 0000 0000 0000 0000>*/
				*e++ = (byte_t)(((uc >> 24) & 0x03)|0xf8);
				*e++ = (byte_t)(((uc >> 18) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 12) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 6) & 0x3f)|0x80);
				*e++ = (byte_t)((uc & 0x3f)|0x80);
		}else
		{
				/*<11111101> <0000 0000 0000 0000 0000 0000 0000 0000>*/
				*e++ = (byte_t)(((uc >> 30) & 0x01)|0xfc);
				*e++ = (byte_t)(((uc >> 24) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 18) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 12) & 0x3f)|0x80);
				*e++ = (byte_t)(((uc >> 6) & 0x3f)|0x80);
				*e++ = (byte_t)((uc & 0x3f)|0x80);
		}

		return utf8 ? e - utf8 : e - buf;
}



size_t Com_wcs_to_utf8(const wchar_t *unicode, size_t n, char *out, size_t out_len)
{
		char *p;
		size_t i, need;
		Com_ASSERT(unicode != NULL);
		for(i = 0,need = 0; i < n; ++i)need += __unicode_to_utf8_char(unicode[i], NULL);

		if(out != NULL)
		{
				p = out;
				if(out_len < need)return 0;
				for(i = 0; i < n; ++i)p += __unicode_to_utf8_char(unicode[i], (byte_t*)p);
		}
		return need;
}



size_t Com_utf8_to_wcs(const char *utf8, size_t n, wchar_t *out, size_t out_len)
{
		const char *p;
		size_t need; int_t l;
		Com_ASSERT(utf8 != NULL);
		p = utf8; need = 0; l = (int_t)n;
		
		while(l > 0)
		{
				size_t nc = __utf8_to_unicode_char((const byte_t*)p, (size_t)l, NULL);
				if(nc == 0)return 0;
				need++;
				p += nc;
				l -= nc;
		}
		
		if(out != NULL)
		{
				if(out_len < need)return 0;
				
				l = n; p = utf8; need = 0;
				while(l > 0)
				{
						size_t nc = __utf8_to_unicode_char((const byte_t*)p, (size_t)l, &out[need]);
						need++;
						l -= nc;
						p += nc;
				}
		}

		return need;
}


wchar_t* Com_utf8_convto_wcs(const char *utf8)
{
		wchar_t *buf;
		size_t need;
		size_t in_len;
		
		Com_ASSERT(utf8 != NULL);

		in_len = Com_strlen(utf8);
		
		if(in_len == 0)return NULL;

		need = Com_utf8_to_wcs(utf8, in_len, NULL, 0);

		if(need == 0)return NULL;/*输入有问题*/
		
		buf = Com_NEWARR(wchar_t, need + 1); buf[need] = L'\0';
		Com_utf8_to_wcs(utf8, in_len, buf, need);
		return buf;
}

char*  Com_wcs_convto_utf8(const wchar_t *wcs)
{
		char *buf; size_t need; size_t in_len;

		in_len = Com_wcslen(wcs); if(in_len == 0)return NULL;

		need = Com_wcs_to_utf8(wcs,in_len, NULL, 0);

		if(need == 0)return NULL;/*输入有问题*/

		buf = Com_NEWARR(char, need + 1); buf[need] = '\0';

		Com_wcs_to_utf8(wcs,in_len, buf, need);
		return buf;
}





wchar_t*	Com_acp_convto_wcs(const char *input, size_t in_n)
{
		wchar_t *ret;

		Com_ASSERT(input != NULL);
		if(in_n == 0)
		{
				return Com_wcsdup(L"");
		}else
		{
				int len = MultiByteToWideChar(CP_ACP, 0, input, (int)in_n, 0, 0);
				if(len == 0)
				{
						return NULL;

				}

				ret = Com_NEWARR(wchar_t, len + 1);

				if(MultiByteToWideChar(CP_ACP, 0, input, (int)in_n, ret, len) == 0)
				{
						Com_DEL(ret);
						ret = NULL;
				}else
				{
						ret[len] = 0;
				}

				return ret;
		}
}



char*	Com_wcs_convto_acp(const wchar_t *input, size_t in_n)
{
		char *ret;
		int n;
		Com_ASSERT(input != NULL);
		n = (int)in_n;

		if(n == 0)
		{
				return Com_strdup("");
		}else
		{
				int len = WideCharToMultiByte(CP_ACP, 0, input, n, 0, 0, NULL, NULL);
				if(len == 0)
				{
						return NULL;
				}

				ret = Com_NEWARR(char, len + 1);

				if(WideCharToMultiByte(CP_ACP, 0, input, n, ret, len, NULL, NULL) == 0)
				{
						Com_DEL(ret);
						ret = NULL;
				}else
				{
						ret[len] = 0;
				}

				return ret;
		}
}

#endif





static UINT __get_codepage_for_winapi(cmCodePage_t cp)
{
		switch(cp)
		{
		default: /*CP_ACP*/
				return CP_ACP;
		case COM_CP_UTF8:
				return CP_UTF8;
		case COM_CP_GB2312:
				return 936;
		case COM_CP_GB18030:
				return 54936;
		case COM_CP_BIG5:
				return 950;
		}
}



size_t					Com_str_to_wcs(cmCodePage_t cp, const char *acp, size_t n, wchar_t *out, size_t out_len)
{
		int len;
		const UINT win_cp = __get_codepage_for_winapi(cp);
		Com_ASSERT(acp != NULL);

		len = MultiByteToWideChar(win_cp, 0, acp, (int)n, 0, 0);

		
		if(len == 0 || out_len == 0 || out == NULL)
		{
				return len;
		}

		if((int)out_len < len)
		{
				return 0;
		}

		if(MultiByteToWideChar(CP_ACP, 0, acp, (int)n, out, (int)out_len) == 0)
		{
				return 0;
		}else
		{
				return len;
		}
}


size_t					Com_wcs_to_str(cmCodePage_t cp, const wchar_t *input, size_t n, char *out, size_t out_len)
{
		int len;
		const UINT win_cp = __get_codepage_for_winapi(cp);
		Com_ASSERT(input != NULL);

		len = WideCharToMultiByte(win_cp, 0, input, (int)n, 0, 0, NULL, NULL);

		if(len == 0 || out == NULL || out_len == 0)
		{
				return len;
		}

		if((int)out_len < len)
		{
				return 0;
		}

		if(WideCharToMultiByte(CP_ACP, 0, input, (int)n, out, (int)out_len, NULL, NULL) == 0)
		{
				return 0;
		}else
		{
				return len;
		}
}


char*					Com_wcs_convto_str(cmCodePage_t cp, const wchar_t *input, size_t in_n)
{
		char *ret;
		int n;
		const UINT win_cp = __get_codepage_for_winapi(cp);
		Com_ASSERT(input != NULL);
		n = (int)in_n;

		if(n == 0)
		{
				return Com_strdup("");
		}else
		{
				int len = WideCharToMultiByte(win_cp, 0, input, n, 0, 0, NULL, NULL);
				if(len == 0)
				{
						return NULL;
				}

				ret = Com_NEWARR(char, len + 1);

				len = WideCharToMultiByte(CP_ACP, 0, input, n, ret, len, NULL, NULL);
				if(len == 0)
				{
						Com_DEL(ret);
						ret = NULL;
				}else
				{
						ret[len] = 0;
				}

				return ret;
		}
}



wchar_t*				Com_str_convto_wcs(cmCodePage_t cp, const char *input, size_t in_n)
{
		wchar_t *ret;
		const UINT win_cp = __get_codepage_for_winapi(cp);
		Com_ASSERT(input != NULL);
		if(in_n == 0)
		{
				return Com_wcsdup(L"");
		}else
		{
				int len = MultiByteToWideChar(win_cp, 0, input, (int)in_n, 0, 0);
				if(len == 0)
				{
						return NULL;

				}

				ret = Com_NEWARR(wchar_t, len + 1);

				len = MultiByteToWideChar(CP_ACP, 0, input, (int)in_n, ret, len);
				if(len == 0)
				{
						Com_DEL(ret);
						ret = NULL;
				}else
				{
						ret[len] = 0;
				}

				return ret;
		}
}





MM_NAMESPACE_END

