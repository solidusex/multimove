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

#include "common.h"

MM_NAMESPACE_BEGIN





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

		if(MultiByteToWideChar(win_cp, 0, acp, (int)n, out, (int)out_len) == 0)
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

		if(WideCharToMultiByte(win_cp, 0, input, (int)n, out, (int)out_len, NULL, NULL) == 0)
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

				len = WideCharToMultiByte(win_cp, 0, input, n, ret, len, NULL, NULL);
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

				len = MultiByteToWideChar(win_cp, 0, input, (int)in_n, ret, len);
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

