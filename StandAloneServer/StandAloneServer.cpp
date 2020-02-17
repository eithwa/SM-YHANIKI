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

//
// to_string patch
//
#include <sstream>

namespace std {

    template < typename T > std::string to_string( const T& n ) {

        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class StandAloneStepManiaLanServer : StepManiaLanServer {
public:
	bool ServerStart();
	std::string ServerUpdateVerbose();
protected:
	std::string UpdateClientsVerbose();
	std::string ParseDataVerbose(PacketFunctions& Packet, const unsigned int clientNum);
};

bool StandAloneStepManiaLanServer::ServerStart() {
	return StepManiaLanServer::ServerStart();
}

std::string StandAloneStepManiaLanServer::ServerUpdateVerbose() {

	std::string log = "";

	if (!stop)
	{
		NewClientCheck();
		log += UpdateClientsVerbose();
		if (time(NULL) > statsTime)
		{
			SendStatsToClients();
			statsTime = time(NULL);
		}
	}

	return log;
}

std::string StandAloneStepManiaLanServer::UpdateClientsVerbose()
{
	std::string log = "";

	//Go through all the clients and check to see if it is being used.
	//If so then try to get a backet and parse the data.
	for (unsigned int x = 0; x < Client.size(); ++x)
		if (CheckConnection(x) && (Client[x]->GetData(Packet) >= 0))
			log += ParseDataVerbose(Packet, x);

	return log;
}

std::string StandAloneStepManiaLanServer::ParseDataVerbose(PacketFunctions& Packet, const unsigned int clientNum)
{
	int command = Packet.Read1();
	switch (command)
	{
	case NSCPing:
		// No Operation
		SendValue(NSServerOffset + NSCPingR, clientNum);
		return "";
	case NSCPingR:
		// No Operation response
		return "";
	case NSCHello:
		// Hello
		Hello(Packet, clientNum);
		return "Client " + std::to_string(clientNum) + " said hello\n";
	case NSCGSR:
		// Start Request
		Client[clientNum]->StartRequest(Packet);
		CheckReady();  //This is what ACTUALLY starts the games
		return "Client " + std::to_string(clientNum) + " pushed start\n";
	case NSCGON:
		// GameOver 
		GameOver(Packet, clientNum);
		return "Client " + std::to_string(clientNum) + " game-overed\n";
	case NSCGSU:
		// StatsUpdate
		Client[clientNum]->UpdateStats(Packet);
		if (!Client[clientNum]->lowerJudge)
			CheckLowerJudge(clientNum);
		break;
	case NSCSU:
		// Style Update
		Client[clientNum]->StyleUpdate(Packet);
		SendUserList();
		break;
	case NSCCM:
		// Chat message
		AnalizeChat(Packet, clientNum);
		return "Client " + std::to_string(clientNum) + " chatted\n";
	case NSCRSG:
		SelectSong(Packet, clientNum);
		return "Client " + std::to_string(clientNum) + " selected a song\n";
	case NSCSMS:
		ScreenNetMusicSelectStatus(Packet, clientNum);
		break;
	case NSCUPOpts:
		Client[clientNum]->Player[0].options = Packet.ReadNT();		
		Client[clientNum]->Player[1].options = Packet.ReadNT();		
		break;
	case NSCUPPer:
		Client[clientNum]->Player[0].percentage = Packet.ReadNT();
		Client[clientNum]->Player[1].percentage = Packet.ReadNT();
		break;
	case NSSSC:
		{
			CString server_ip = Packet.ReadNT();
			CString player_num = Packet.ReadNT();
			CString file_size = Packet.ReadNT();
			LOG->Info("NSSSC server_ip %s",server_ip.c_str());
			LOG->Info("NSSSC file_size %s",file_size.c_str());
			int client_index = atof( player_num.c_str() );
			
			Reply.ClearPacket();
			Reply.Write1(NSSSC + NSServerOffset);
			Reply.WriteNT(server_ip);
			Reply.WriteNT(file_size);
			// Reply.WriteNT(ListPlayers());
			SendNetPacket(client_index, Reply);

			LastSongInfo.title="NULL";
			LastSongInfo.artist="NULL";
			LastSongInfo.subtitle="NULL";//if sent file success, ask play? again
		}
		break;
	default:
		break;
	}
	return "";
}	 

int _tmain(int argc, char* argv[])
{
	FILEMAN = new RageFileManager(argv[0]);
	// FILEMAN->MountInitialFilesystems();
	LOG	= new RageLog();
	PREFSMAN = new PrefsManager;

	StandAloneStepManiaLanServer* server = new StandAloneStepManiaLanServer();

	if (!server->ServerStart())
		std::cout << "ServerStart failed!" << std::endl;

	int next = clock();
	while (true) {
		if (clock() > next)
			next += 100;
		else { Sleep(50); continue; }
		//printf("%d ", next);

		std::string log = server->ServerUpdateVerbose();
		
		if (log != "")
			printf("\n[%08d]\n%s", clock(), log.c_str());
	}

	return 0;
}

