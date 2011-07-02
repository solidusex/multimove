
//#if defined(__LIB)


#include "test.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



NS_NAMESPACE_BEGIN


const wchar_t *path[] = 
{
		L"黑名单		\\		a		\\b		\\		a		b\\",
		L"黑名单	\\a\\b\\	a		b\\",
		L"		常规		\\		a		\\b		\\		a		b\\",
		L"		未知组	x		\\		a		\\b		\\		a		b\\",
		L"		常规		\\		a		\\b		\\		a		b\\xxx\\\\\\\\"
};


void path_table_test()
{
		storeGroupMap_t	group_map;
		storeGroupRec_t	*rec;
		Store_InitGroupMap(&group_map);
		
		for(size_t i = 0; i < sizeof(path)/sizeof(path[0]); ++i)
		{
				rec = Store_GetGroupMapRecord(&group_map, path[i]);
		}


		Store_UnInitGroupMap(&group_map);
}



static void print_node(const groupClientNode_t *node)
{
		Com_printf(L"Computer Name: %ls\r\n", node->comp_name);
		Com_printf(L"IP: %ls\r\n", node->ip);
		Com_printf(L"MAC: %ls\r\n", node->mac);
		Com_printf(L"USER: %ls\r\n", node->user);
		Com_printf(L"\r\n");
}

void path_table_test2()
{
		scanNodeList_t	lst;
		storeGroupMap_t	group_map;
		storeGroupRec_t	*rec;
		
		Store_InitGroupMap(&group_map);
		Scan_InitNodeList(&lst);



		Scan_NetNeighbor(&lst);
		
		if(!Scan_IPRange(&lst, L"192.168.0.0", L"192.168.255.255",		20 * 1000))
		{
				abort();
		}
	

		if(!Scan_IPRange(&lst, L"193.168.0.0", L"193.168.255.255",		20 * 1000))
		{
				abort();
		}
		
		
		for(size_t i = 0; i < lst.cnt; ++i)
		{
				wchar_t tmp[1024] = {0};
				if(!lst.lst[i]->is_valid_node)continue;

				Com_wcscat(tmp, path[rand() % (sizeof(path)/sizeof(path[0]))]);
				Com_wcscat(tmp,L"\\");
				if(Com_wcslen(lst.lst[i]->group) > 0)
				{
						Com_wcscat(tmp, lst.lst[i]->group);
						Com_wcscat(tmp,L"\\");
				}
				rec = 	Store_GetGroupMapRecord(&group_map, tmp);

				Store_InsertToGroupRecord(rec, lst.lst[i]);


		}

		Scan_UnInitNodeList(&lst);

		for(size_t i = 0; i < GROUP_MAP_BUCKET; ++i)
		{
				if(group_map.bucket[i] != NULL)
				{
						const storeGroupRec_t *r = group_map.bucket[i];
						while(r)
						{
								Com_printf(L"%ls\r\n", r->path);
								for(size_t k = 0; k < r->cnt; ++k)
								{
										print_node(&r->nodes[k]);
								}
								
								Com_printf(L"%ls\r\n", L"-------------------------------");
								r = r->next;
						}
				}
		}

		Store_UnInitGroupMap(&group_map);
}


void	Store_Test()
{
		
	
		//path_table_test();
		path_table_test2();
}



NS_NAMESPACE_END

//#endif

