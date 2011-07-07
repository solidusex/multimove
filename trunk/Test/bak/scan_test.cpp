
//#if defined(__LIB)

#include "test.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "scan/scan.h"



NS_NAMESPACE_BEGIN

static void print_node(const scanNode_t *node)
{
		Com_printf(L"Computer Name: %ls\r\n", node->comp_name);
		Com_printf(L"Group: %ls\r\n", node->group);
		Com_printf(L"IP: %ls\r\n", node->ip);
		Com_printf(L"MAC: %ls\r\n", node->mac);
		Com_printf(L"USER: %ls\r\n", node->user);
		Com_printf(L"Active Directory Path: %ls\r\n", node->ad_path);
		Com_printf(L"\r\n");
}


void neighbor_test()
{
		scanNodeList_t	lst;
		Scan_InitNodeList(&lst);

		Scan_NetNeighbor(&lst);
		
		size_t cnt = 0;

		for(int i = 0; i < lst.cnt; ++i)
		{
				if(lst.lst[i]->is_valid_node)
				{
						print_node(lst.lst[i]);
						cnt++;
				}
		}

		Com_printf(L"find nodes count == %d\r\n", cnt);

		Scan_UnInitNodeList(&lst);
}






#if(0)

void ipscan_test_range_test()
{

		scanNodeList_t	lst;
		Scan_InitNodeList(&lst);

		Scan_IPRange(&lst, L"193.169.1.0", L"192.168.1.255", INFINITE);

		Scan_IPRange(&lst, L"192.169.1.0", L"192.168.1.255", INFINITE);


		Scan_IPRange(&lst, L"192.168.2.0", L"192.168.1.255", INFINITE);

		Scan_IPRange(&lst, L"192.168.1.10", L"192.168.1.5", INFINITE);

		Scan_IPRange(&lst, L"192.168.1.10", L"192.168.1.255", INFINITE);

		Scan_IPRange(&lst, L"192.168.0.10", L"192.168.1.0", INFINITE);

		Scan_IPRange(&lst, L"192.167.0.10", L"192.168.1.0", INFINITE);

		Scan_IPRange(&lst, L"191.167.0.10", L"192.168.1.0", INFINITE);


		Scan_UnInitNodeList(&lst);

}
#endif




void ipscan_test()
{
		scanNodeList_t	lst;
		Scan_InitNodeList(&lst);

	
		if(!Scan_IPRange(&lst, L"192.168.0.0", L"192.168.255.255",		15 * 1000))
		{
				abort();
		}
	

		if(!Scan_IPRange(&lst, L"193.168.0.0", L"193.168.255.255",		15 * 1000))
		{
				abort();
		}

		
		size_t cnt = 0;

		for(int i = 0; i < lst.cnt; ++i)
		{
				if(lst.lst[i]->is_valid_node)
				{
						print_node(lst.lst[i]);
						cnt++;
				}
		}

		Com_printf(L"find nodes count == %d\r\n", cnt);
		Scan_UnInitNodeList(&lst);
}


void	Scan_Test()
{
		//neighbor_test();
		ipscan_test();

}



NS_NAMESPACE_END

//#endif

