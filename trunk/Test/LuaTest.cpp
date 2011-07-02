
/*
** $Id: lua.c,v 1.160.1.2 2007/12/28 15:32:23 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

//#if defined(__LIB)

#include "test.h"


MM_NAMESPACE_BEGIN



static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;



static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  fprintf(stderr,
  "usage: %s [options] [script [args]].\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  "  -        execute stdin and stop handling options\n"
  ,
  progname);
  fflush(stderr);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  
		if (status && !lua_isnil(L, -1)) 
		{
				const char *msg = lua_tostring(L, -1);
				if (msg == NULL) msg = "(error object is not a string)";
				l_message(progname, msg);
				lua_pop(L, 1);
		}
		return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


static void print_version (void) {
  l_message(NULL, LUA_RELEASE "  " LUA_COPYRIGHT);
}


static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}


static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  while ((status = loadline(L)) != -1) {
    if (status == 0) status = docall(L, 0, 0);
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  fputs("\n", stdout);
  fflush(stdout);
  progname = oldprogname;
}


static int handle_script (lua_State *L, char **argv, int n) {
  int status;
  const char *fname;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  fname = argv[n];
  if (strcmp(fname, "-") == 0 && strcmp(argv[n-1], "--") != 0) 
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  lua_insert(L, -(narg+1));
  if (status == 0)
    status = docall(L, narg, 0);
  else
    lua_pop(L, narg);      
  return report(L, status);
}


/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}


static int collectargs (char **argv, int *pi, int *pv, int *pe) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        notail(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        notail(argv[i]);
        *pi = 1;  /* go through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != 0)
          return 1;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename))
          return 1;  /* stop if file fails */
        break;
      }
      default: break;
    }
  }
  return 0;
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT);
  if (init == NULL) return 0;  /* status OK */
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT);
}


struct Smain {
  int argc;
  char **argv;
  int status;
};


static int pmain (lua_State *L) 
{
		struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
		char **argv = s->argv;
		int script;
		int has_i = 0, has_v = 0, has_e = 0;
		globalL = L;
		if (argv[0] && argv[0][0]) progname = argv[0];
		lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
		luaL_openlibs(L);  /* open libraries */
		lua_gc(L, LUA_GCRESTART, 0);
		s->status = handle_luainit(L);
		if (s->status != 0) return 0;
		script = collectargs(argv, &has_i, &has_v, &has_e);
		if (script < 0) {  /* invalid args? */
				print_usage();
				s->status = 1;
				return 0;
		}
		if (has_v) print_version();
		s->status = runargs(L, argv, (script > 0) ? script : s->argc);
		if (s->status != 0) return 0;
		if (script)
				s->status = handle_script(L, argv, script);
		if (s->status != 0) return 0;
		
		if (has_i)
		{
				dotty(L);
		}

		else if (script == 0 && !has_e && !has_v) 
		{
				
				if (lua_stdin_is_tty()) 
				{
						print_version();
						dotty(L);
				}
				else
				{
						dofile(L, NULL);  /* executes stdin as a file */
				}
		}
		return 0;
}












void script_mode_test(int argc, char **argv)
{
		int status;
		struct Smain s;
		/*
		lua_State *L = lua_open();
		luaopen_luasql_odbc(L);
		*/

		lua_State *L = Lua_Create(NULL);
		

		if (L == NULL) {
				l_message("%s\r\n","cannot create state: not enough memory");
				return;
		}
		s.argc = argc;
		s.argv = argv;
		status = lua_cpcall(L, &pmain, &s);
		report(L, status);

		//lua_close(L);
		Lua_Destroy(L);

		return;
}







const char *test_func = 
"function test(a) print(a) end;\r\n"
"test(type(luasql.odbc))\r\n"
"evn = luasql.odbc()\r\n"
"test(type(evn))\r\n"
;








void load_buffer_test()
{

		int stat;
		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		stat = luaL_loadstring(s, test_func);
		
		
		stat = lua_pcall(s, 0, 0,0);

		report(s, stat);
		
		Lua_Destroy(s);
}



#if(0)
static int call_test(lua_State *s)
{
		const char *str = (const char*)lua_touserdata(s, 1);
		printf("%s\r\n", str);
		luaL_error(s, "%s\r\n", "call_test failed");
		return 0;
}


void lua_call_ctest()
{
		int stat;
		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		Lua_DumpStack(s);
		
		stat = lua_cpcall(s, &call_test, "aaaaaaaaaaaaaaaaaaa");
		
		Lua_DumpStack(s);

		report(s, stat);

		Lua_Destroy(s);
}








static int l_sin (lua_State *L) 
{
		double d = luaL_checknumber(L, 1);
		d = 12345;
		lua_pushnumber(L, d);
		return 1; /* number of results */
}



const char *test_func3 = 
"function test(d,i,s,n,b,u) print(d,i,s,n,b,u); return 1,\"str\", 426, {1,2,3}; end;\r\n"
"function test2() return test; end;\r\n"
"return 3456;\r\n"
;


void load_buffer_test3()
{


		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		if(luaL_loadstring(s, test_func3) || lua_pcall(s, 0, 0, 0))
		{
				printf("cannot run string: %s\r\n",lua_tostring(s, -1));
				return;
		}

		Lua_DumpStack(s);
		
#if(0)
		if(Lua_CallFunc(s, "test", "d i s n	b		u	=	isit", (double)33.3333, 123, "aaaaaaaa", false, &stat))
		{
				printf("success\r\n");
		}else
		{
				printf("failed\r\n");
		}
#endif

		if(Lua_CallFunc(s, "test2", " = f", 23))
		{
				printf("success\r\n");
		}else
		{
				printf("failed\r\n");
		}

		
		Lua_DumpStack(s);
		printf("gettop == %d\r\n", lua_gettop(s));

		//Lua_DumpStack(s);

		Lua_Destroy(s);
}




void load_buffer_test4()
{


		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		Lua_DumpStack(s);

		if(luaL_loadstring(s, test_func3) || !Lua_CallFunc(s,NULL, " = d"))
		{
				printf("cannot run string: %s\r\n",lua_tostring(s, -1));
				return;
		}

		Lua_DumpStack(s);
		
		
		if(Lua_CallFunc(s, "test2", " = f"))
		{
				printf("success\r\n");
		}else
		{
				printf("failed\r\n");
		}
		
		
		Lua_DumpStack(s);
		printf("gettop == %d\r\n", lua_gettop(s));

		//Lua_DumpStack(s);

		Lua_Destroy(s);
}




static const char *reg_lib_func = 
//"print(type(luaR));\r\n"
//"print(type(luaR.test));\r\n"
"luaR.report(33);\r\n"
"luaR.is_continue(33);\r\n"
;

void luar_test_register_lib()
{


		lua_State *s;
		
		s = Lua_Create(NULL);
		//Lua_OpenLibs(s);
		Com_ASSERT(s != NULL);
		

		if(luaL_loadstring(s, reg_lib_func) || !Lua_CallFunc(s,NULL, " = "))
		{
				printf("cannot run string: %s\r\n",lua_tostring(s, -1));
				return;
		}
				
		Lua_Destroy(s);
}


const char *test_setglb = 
"print(type(user_data));\r\n"
"print(type(user_data[\"key\"]));\r\n"
"ctx = user_data[\"key\"];\r\n"
"print(ctx);\r\n"
;


const char *test_setglb2 = 
"testetst(\r\n"
;

const char *key = "key";
const char *user_data = "value";

void luar_test_setglobal()
{


		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		lua_newtable(s);
		lua_pushstring(s,key);
		lua_pushlightuserdata(s, (void*)user_data);
		lua_settable(s, -3);
		
		Lua_DumpStack(s);

		lua_setglobal(s, "user_data");

		Lua_DumpStack(s);

		
		if(luaL_loadstring(s, test_setglb2) || !Lua_CallFunc(s,NULL, " = "))
		{
				Lua_DumpStack(s);
				getchar();
				Com_printf("cannot run string: %s\r\n",lua_tostring(s, -1));
				return;
		}
		
				
		Lua_Destroy(s);
}



void do_file_test()
{
		const char *path = NULL;

		while(true)
		{
				lua_State *s;
				Com_ASSERT(path);

				s = Lua_Create(NULL);
				//Lua_OpenLibs(s);
				Com_ASSERT(s != NULL);


				if(luaL_dofile(s, path))
				{
						printf("cannot run file %s: %s\r\n",path, lua_tostring(s, -1));
						return;
				}

				Lua_Destroy(s);

				getchar();
		}
}

#endif



void luar_test_dostring()
{


		lua_State *s;
		
		s = Lua_Create(NULL);
		Com_ASSERT(s != NULL);

		luaL_dostring(s, "print(\"abcdef\r\n\")");
		
		Lua_Destroy(s);
}



void	Lua_Test(int argc, char **argv)
{
		Com_UNUSED(argc);
		Com_UNUSED(argv);

		//script_mode_test(argc, argv);
		//load_buffer_test();
		//lua_call_ctest();
		//load_buffer_test2();
		//load_buffer_test3();
		//load_buffer_test4();
		
		//luar_test_register_lib();

		//luar_test_setglobal();

		//do_file_test();


		luar_test_dostring();
}


MM_NAMESPACE_END


//#endif

