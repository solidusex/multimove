
#include "common.h"

/******************************************************TextFile**************************************************************/

MM_NAMESPACE_BEGIN



/***********************************************************Misc**********************************************************/


static FILE*	__open_file(const wchar_t *path, const wchar_t *mode)
{
		Com_ASSERT(path != NULL && mode != NULL);
		return _wfopen(path, mode);
}









/***************************************Read File**********************************************************/

static bool_t	__dectect_encoding(FILE *file, cmTxtBom_t *bom)
{
		byte_t b[2];

		Com_ASSERT(file != NULL && bom != NULL);

		if(fread((void*)b, 1, 2, file) != 2)
		{
				return false;
		}else
		{
				if(b[0] == 0xFF && b[1] == 0xFE)
				{
						byte_t tmp[2] = {0xcc,0xcc};
						fread((void*)tmp, 1, 2, file);

						if(tmp[0] == 0x00 && tmp[1] == 0x00)
						{
								*bom = COM_TXT_BOM_UTF32_LE;
						}else
						{
								*bom = COM_TXT_BOM_UTF16_LE;
								fseek(file, 2, 0);
						}
				}else if(b[0] == 0xFE && b[1] == 0xFF)
				{
						*bom = COM_TXT_BOM_UTF16_BE;
				}else if(b[0] == 0x00 && b[1] == 0x00)
				{
						byte_t tmp[2] = {0xcc,0xcc};
						fread((void*)tmp, 1, 2, file);

						if(tmp[0] == 0xFE && tmp[1] == 0xFF)
						{
								*bom = COM_TXT_BOM_UTF32_BE;
						}else
						{
								fseek(file, 0, 0);
								*bom = COM_TXT_BOM_ASCII;
						}
				}else if(b[0] == 0xEF && b[1] == 0xBB)
				{
						byte_t	tmp = 0;
						fread((void*)&tmp, 1, 1, file);

						if( tmp == 0xBF)
						{
								*bom = COM_TXT_BOM_UTF_8;
						}else
						{
								*bom = COM_TXT_BOM_ASCII;
								fseek(file, 0,0);
						}
				}
				else
				{
						*bom = COM_TXT_BOM_ASCII;
						fseek(file, 0,0);
				}
		}

		return true;
}





typedef enum
{
		TXT_READ_OK = 0x00,
		TXT_READ_INVALID,
		TXT_READ_EOF
}txtReadStatus_t;

static txtReadStatus_t		__read_wchar(FILE *file, cmTxtBom_t enc, wchar_t *out)
{

		uint_32_t e;


		Com_ASSERT(file != NULL);

		e = 0;

		switch(enc)
		{
		case COM_TXT_BOM_UTF_8:
		{
				byte_t		b;
				byte_t		buf[5];
				size_t		rn;
				uint_32_t	v;

				b = 0;
				rn = 0;
				v = 0;

				rn = fread((void*)&b, 1, 1, file);

				if(rn != 1)
				{
						return feof(file) ? TXT_READ_EOF : TXT_READ_INVALID;
				}

				v = (uint_32_t)b;

				if(v >= 0xfc)
				{
						/*6:<11111100>*/
						/*¶ÁÊ£ÏÂµÄ×Ö½Ú*/
						rn = fread((void*)buf, 1, 5, file);
						if(rn != 5)
						{
								return TXT_READ_INVALID;
						}

						e = (v & 0x01) << 30;
						e |= (buf[0] & 0x3f) << 24;
						e |= (buf[1] & 0x3f) << 18;
						e |= (buf[2] & 0x3f) << 12;
						e |= (buf[3] & 0x3f) << 6;
						e |= (buf[4] & 0x3f);
				}else if(v >= 0xf8)
				{
						/*5:<11111000>*/
						rn = fread((void*)buf, 1,4, file);
						if(rn != 4)
						{
								return TXT_READ_INVALID;
						}

						e = (v & 0x03) << 24;
						e |= (buf[0] & 0x3f) << 18;
						e |= (buf[1] & 0x3f) << 12;
						e |= (buf[2] & 0x3f) << 6;
						e |= (buf[3] & 0x3f);

				}else if(v >= 0xf0)
				{
						/*4:<11110000>*/
						rn = fread((void*)buf, 1, 3, file);
						if(rn != 3)
						{
								return TXT_READ_INVALID;
						}
						e = (v & 0x07) << 18;
						e |= (buf[0] & 0x3f) << 12;
						e |= (buf[1] & 0x3f) << 6;
						e |= (buf[2] & 0x3f);

				}else if(v >= 0xe0)
				{
						/*3:<11100000>*/
						rn = fread((void*)buf, 1, 2, file);
						if(rn != 2)
						{
								return TXT_READ_INVALID;
						}

						e = (v & 0x0f) << 12;
						e |= (buf[0] & 0x3f) << 6;
						e |= (buf[1] & 0x3f);

				}else if(v >= 0xc0)
				{
						/*3:<11000000>*/
						rn = fread((void*)buf, 1,1,file);
						if(rn != 1)
						{
								return TXT_READ_INVALID;
						}
						e = (v & 0x1f) << 6;
						e |= (buf[0] & 0x3f);
				}else
				{
						e = (wchar_t)v;
				}
		}
				break;
		case COM_TXT_BOM_UTF16_BE:
		case COM_TXT_BOM_UTF16_LE:
		{
				byte_t buf[2];
				size_t	rn;
				rn = fread((void*)buf, 1, 2, file);
				if(rn != 2)
				{
						return feof(file) ? TXT_READ_EOF : TXT_READ_INVALID;
				}

				if(enc == COM_TXT_BOM_UTF16_BE)
				{
						e = ((uint_32_t)buf[0]) << 8 | (uint_32_t)buf[1];
				}else
				{
						e = ((uint_32_t)buf[1]) << 8 | (uint_32_t)buf[0];
				}
		}
				break;
		case COM_TXT_BOM_UTF32_BE:
		case COM_TXT_BOM_UTF32_LE:
		{
				byte_t buf[4];
				size_t	rn;
				rn = fread((void*)buf, 1, 4, file);
				if(rn != 4)
				{
						return feof(file) ? TXT_READ_EOF : TXT_READ_INVALID;
				}

				if(enc == COM_TXT_BOM_UTF32_BE)
				{
						e = ((uint_32_t)buf[0]) << 24 | (uint_32_t)buf[1] << 16 | (uint_32_t)buf[2] << 8 | (uint_32_t)buf[3];
				}else
				{
						e = (uint_32_t)buf[0] | (uint_32_t)buf[1] << 8 | (uint_32_t)buf[2] << 16 | ((uint_32_t)buf[3]) << 24;
				}
		}
				break;
		case COM_TXT_BOM_ASCII:
		default:
				return TXT_READ_INVALID;
				break;
		}

		if(out)*out = (wchar_t)e;
		return TXT_READ_OK;
}



bool_t	Com_LoadBomTextFile(const wchar_t *path, cmTxtBom_t *bom, cmString_t *out)
{

		FILE	*file = NULL;
		cmTxtBom_t	enc;
		wchar_t c;
		bool_t	is_ok;
		txtReadStatus_t	status;
		Com_ASSERT(path != NULL && out != NULL);

		is_ok = true;
		Com_ClearString(out);

/***************************************************/
		file = __open_file(path, L"rb");
/***************************************************/

		if(!file)
		{
				is_ok = false;
				goto FAILED_POINT;
		}


		if(!__dectect_encoding(file, &enc))
		{
				goto FAILED_POINT;
		}

		if(enc == COM_TXT_BOM_ASCII)
		{
				byte_t	tmp[1024];
				size_t	rn;
				cmBuffer_t *ascii_buf = Com_CreateBuffer(1024);

				do{
						rn = fread((void*)tmp, 1, 1024, file);
						if(rn > 0)
						{
								Com_InsertBuffer(ascii_buf, tmp, rn);
						}
				}while(!feof(file) && !ferror(file));

				if(ferror(file))
				{
						status = TXT_READ_INVALID;
				}else
				{
						tmp[0] = '\0';
						Com_InsertBuffer(ascii_buf, tmp, 1);
						status = TXT_READ_EOF;
				}

				if(status != TXT_READ_INVALID)
				{
						wchar_t *str;

						str = Com_str_convto_wcs(COM_CP_ACP, (const char*)Com_GetBufferData(ascii_buf), Com_GetBufferAvailable(ascii_buf));

						if(!str)
						{
								status = TXT_READ_INVALID;
						}

						if(out && str)
						{
								Com_AppendString(out, str);
						}

						if(str)
						{
								Com_DEL(str);
								str = NULL;
						}

						if(ascii_buf)
						{
								Com_DestroyBuffer(ascii_buf);
						}
						ascii_buf = NULL;
				}
		}else
		{
				do{

						status = __read_wchar(file, enc, &c);

						if(status == TXT_READ_OK && out)
						{
								Com_AppendCharToString(out, c);
						}
				}while(status == TXT_READ_OK);
		}

		if(status == TXT_READ_INVALID)
		{
				is_ok = false;
				goto FAILED_POINT;
		}

		if(bom)
		{
				*bom = enc;
		}

FAILED_POINT:
		if(file)
		{
				fclose(file);
				file = NULL;
		}

		return is_ok;
}



/***************************************Write File**********************************************************/


static bool_t __write_bom(FILE *file, cmTxtBom_t bom)
{
		byte_t buf[4];
		size_t wn;
		bool_t is_ok;
		Com_ASSERT(file != NULL);
		is_ok = true;

		switch(bom)
		{
		case COM_TXT_BOM_UTF_8:
		{
				wn = 3;
				buf[0] = 0xEF;
				buf[1] = 0xBB;
				buf[2] = 0xBF;
		}
				break;
		case COM_TXT_BOM_UTF16_BE:
		{
				wn = 2;
				buf[0] = 0xFE;
				buf[1] = 0xFF;
		}
				break;
		case COM_TXT_BOM_UTF16_LE:
		{
				wn = 2;
				buf[0] = 0xFF;
				buf[1] = 0xFE;
		}
				break;
		case COM_TXT_BOM_UTF32_BE:
		{
				wn = 4;
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0xFE;
				buf[3] = 0xFF;
		}
				break;
		case COM_TXT_BOM_UTF32_LE:
		{
				wn = 4;
				buf[0] = 0xFF;
				buf[1] = 0xFE;
				buf[2] = 0x00;
				buf[3] = 0x00;
		}
				break;
		case COM_TXT_BOM_ASCII:
		default:
				wn = 0;
				is_ok = false;
				break;
		}

		if(wn > 0)
		{
				if(fwrite((void*)buf, 1, wn, file) != wn)
				{
						is_ok = false;
				}
		}

		return is_ok;
}






static bool_t __write_wchar(FILE *file, cmTxtBom_t bom, wchar_t c)
{
		bool_t is_ok;
		Com_ASSERT(file != NULL);

		is_ok = true;

		switch(bom)
		{
		case COM_TXT_BOM_UTF_8:
		{
				byte_t utf8[10];
				size_t n;
				byte_t *e;
				uint_32_t	uc = (uint_32_t)c;

				e = utf8;

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

				n =  e - utf8;

				if(fwrite((void*)utf8, 1, n, file) != n)
				{
						is_ok = false;
				}
		}
				break;
		case COM_TXT_BOM_UTF16_BE:
		case COM_TXT_BOM_UTF16_LE:
		{
				byte_t buf[2];
				uint_16_t	uc = (uint_16_t)c;


				if(bom == COM_TXT_BOM_UTF16_BE)
				{
						buf[0] = (byte_t)(uc >> 8);
						buf[1] = (byte_t)(uc & 0x00FF);

				}else
				{
						buf[1] = (byte_t)(uc >> 8);
						buf[0] = (byte_t)(uc & 0x00FF);
				}

				if(fwrite((void*)&buf, 1,2,file) != 2)
				{
						is_ok = false;
				}
		}
				break;
		case COM_TXT_BOM_UTF32_BE:
		case COM_TXT_BOM_UTF32_LE:
		{
				byte_t buf[4];
				uint_32_t	uc = (uint_32_t)c;


				if(bom == COM_TXT_BOM_UTF32_BE)
				{
						buf[0] = (byte_t)(uc >> 24);
						buf[1] = (byte_t)(uc >> 16);
						buf[2] = (byte_t)(uc >> 8);
						buf[3] = (byte_t)(uc & 0x000000FF);
				}else
				{
						buf[3] = (byte_t)(uc >> 24);
						buf[2] = (byte_t)(uc >> 16);
						buf[1] = (byte_t)(uc >> 8);
						buf[0] = (byte_t)(uc & 0x000000FF);
				}

				if(fwrite((void*)&buf, 1,4,file) != 4)
				{
						is_ok = false;
				}

		}
				break;
		case COM_TXT_BOM_ASCII:
		default:
				return TXT_READ_INVALID;
				break;
		}

		return is_ok;
}



bool_t	Com_SaveBomTextFile(const wchar_t *path, cmTxtBom_t bom, const wchar_t *input)
{
		FILE	*file;
		bool_t	is_ok;
		const wchar_t  *p;

		Com_ASSERT(path != NULL && input != NULL);



		is_ok = true;
		file = NULL;

		file = __open_file(path, L"wb");

		if(!file)
		{
				is_ok = false;
				goto FAILED_POINT;
		}

		if(bom == COM_TXT_BOM_ASCII)
		{
				size_t n;
				char *s = Com_wcs_convto_str(COM_CP_ACP, input, Com_wcslen(input));
				n = strlen(s);

				if(!s)
				{
						is_ok = false;
						goto CLEAR_LOCAL;
				}

				if(n == 0)
				{
						goto CLEAR_LOCAL;
				}else
				{
						if(fwrite((void*)s, 1, n, file) != n)
						{
								is_ok = false;
								goto CLEAR_LOCAL;
						}
				}
CLEAR_LOCAL:
				if(s)
				{
						Com_DEL(s);
						s = NULL;
				}
		}else
		{

				if(!__write_bom(file, bom))
				{
						is_ok = false;
						goto FAILED_POINT;
				}
				p = input;

				while(*p)
				{

						if(!__write_wchar(file, bom, *p))
						{
								is_ok = false;
								goto FAILED_POINT;
						}
						++p;
				}
		}


FAILED_POINT:
		if(file)
		{
				fclose(file);
				file = NULL;
		}
		return is_ok;
}





MM_NAMESPACE_END


