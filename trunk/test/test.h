
#pragma once


#define _CRT_SECURE_NO_WARNINGS	1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmcore.h"
#pragma comment(lib, "mmcore.lib")

MM_NAMESPACE_BEGIN


void	Common_Test();
void	Hook_Test();
void	Client_Test();
void	Server_Test();


MM_NAMESPACE_END