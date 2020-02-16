// StandAloneServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>

#include <time.h>

#include <vector>
using std::vector;

#include "global.h"
#include "archutils/win32/arch_setup.h"
#include "StdString.h"

//
// StepMania global classes
//
#include "RageFileManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "NetworkSyncServer.h"

int _tmain(int argc, char* argv[])
{
	FILEMAN = new RageFileManager(argv[0]);
	// FILEMAN->MountInitialFilesystems();
	LOG	= new RageLog();
	PREFSMAN = new PrefsManager;

	StepManiaLanServer* server = new StepManiaLanServer();

	if (!server->ServerStart())
		std::cout << "ServerStart failed!" << std::endl;

	int next = clock();
	while (true) {
		if (clock() > next)
			next += 100;
		else { Sleep(50); continue; }
		//printf("%d ", next);

		server->ServerUpdate();
	}

	return 0;
}

