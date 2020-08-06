#include "global.h"
#include "NetworkSyncManager.h"


#include "As.hpp"
#include "ClientCmds.hpp"
#include "LanClientV2.h"
#include "NetworkSyncServer.h"
#include "LuaFunctions.h"
#include "ServerCmds.hpp"

NetworkSyncManager *NSMAN;

#if defined(WITHOUT_NETWORKING)
NetworkSyncManager::NetworkSyncManager( LoadingWindow *ld ) { useSMserver=false; }
NetworkSyncManager::~NetworkSyncManager () { }
void NetworkSyncManager::CloseConnection() { }
void NetworkSyncManager::PostStartUp(const CString& ServerIP ) { }
bool NetworkSyncManager::Connect(const CString& addy, unsigned short port) { return false; }
void NetworkSyncManager::ReportNSSOnOff(int i) { }
void NetworkSyncManager::ReportTiming(float offset, int PlayerNumber) { }
void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo) { }
void NetworkSyncManager::ReportPercentage() { }
void NetworkSyncManager::ReportSongOver() { }
void NetworkSyncManager::ReportStyle() {}
void NetworkSyncManager::StartRequest(short position) { }
void NetworkSyncManager::DisplayStartupStatus() { }
void NetworkSyncManager::Update( float fDeltaTime ) { }
bool NetworkSyncManager::ChangedScoreboard(int Column) { return false; }
void NetworkSyncManager::SendChat(const CString& message) { }
void NetworkSyncManager::SelectUserSong() { }
#else
#include "ezsockets.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "song.h"
#include "Course.h"
#include "GameState.h"
#include "StageStats.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "ProductInfo.h"
#include "ScreenMessage.h"
#include "GameManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"

const ScreenMessage	SM_AddToChat	= ScreenMessage(SM_User+4);
const ScreenMessage SM_ChangeSong	= ScreenMessage(SM_User+5);
const ScreenMessage SM_GotEval		= ScreenMessage(SM_User+6);
const ScreenMessage SM_ReloadConnectPack	        = ScreenMessage(SM_User+9);
// const ScreenMessage SM_BackFromReloadSongs			= ScreenMessage(SM_User+7);
unsigned long GetFileLength ( FILE * fileName)
{
    unsigned long pos = ftell(fileName);
    unsigned long len = 0;
    fseek ( fileName, 0L, SEEK_END );
    len = ftell ( fileName );
    fseek ( fileName, pos, SEEK_SET );
    return len;
}
NetworkSyncManager::NetworkSyncManager( LoadingWindow *ld )
{
	usingShareSongSystem=false;
	ClientNum=0;
	LANserver = NULL;	//So we know if it has been created yet
	if( GetCommandlineArgument( "runserver" ))
	{
		ld->SetText("Initilizing server...");
		LANserver = new StepManiaLanServer;
		isLanServer = true;
		GetCommandlineArgument( "runserver", LANserver->GetServerName());
	}
	else
		isLanServer = false;
	
	ld->SetText("Initilizing Client Network...");
    /*NetPlayerClient = new EzSockets;
	NetPlayerClient->blocking = false;*/
	NetPlayerClient = Yhaniki::LanClientV2::New().release();
	m_ServerVersion = 0;

   
	useSMserver = false;
	m_startupStatus = 0;	//By default, connection not tried.

	m_ActivePlayers = 0;

	StartUp();
}

NetworkSyncManager::~NetworkSyncManager ()
{
	//Close Connection to server nicely.
    if (useSMserver)
        NetPlayerClient->close();
	delete NetPlayerClient;

	if( isLanServer )
	{
		LANserver->ServerStop();
		delete LANserver;
	}
}

void NetworkSyncManager::CloseConnection()
{
	if (!useSMserver)
		return ;
	m_ServerVersion = 0;
   	useSMserver = false;
	m_startupStatus = 0;
	NetPlayerClient->close();
}

void NetworkSyncManager::PostStartUp(const CString& ServerIP)
{
	CloseConnection();
	if( ServerIP!="LISTEN" )
	{
		if( !Connect(ServerIP.c_str(), 8765) )
		{
			m_startupStatus = 2;
			LOG->Warn( "Network Sync Manager failed to connect" );
			return;
		}

	}
	else
	{
		/*if( !Listen(8765) )
		{
			m_startupStatus = 2;
			LOG->Warn( "Listen() failed" );
			return;
		}*/
	}

	useSMserver = true;

	m_startupStatus = 1;	//Connection attepmpt sucessful

	// If network play is desired and the connection works,
	// halt until we know what server version we're dealing with

	//NetPlayerClient->blocking = false;
	m_packet.ClearPacket();
	if( isLanServer )
		LANserver->ServerUpdate();
	// TODO: �O�b���o?
	{
		//m_packet.ClearPacket();

		//m_packet.Write1( NSCHello );	//Hello Packet

		//m_packet.Write1(NETPROTOCOLVERSION);

		//m_packet.WriteNT(CString(PRODUCT_NAME_VER)); 

		////Block until responce is received
		////Move mode to blocking in order to give CPU back to the 
		////system, and not wait.
		//
		//bool dontExit = true;

		////Don't block if the server is running
		//if( isLanServer )
		//	NetPlayerClient->blocking = false;
		//else
		//	NetPlayerClient->blocking = true;

		////Following packet must get through, so we block for it.
		////If we are serving we do not block for this.
		//NetPlayerClient->SendPack((char*)m_packet.Data,m_packet.Position);

		////If we are serving, do this so we properly connect
		////to the server.
		//if( isLanServer )
		//	LANserver->ServerUpdate();

		//m_packet.ClearPacket();

		//while (dontExit)
		//{
		//	m_packet.ClearPacket();
		//	if (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)<1)
		//		dontExit=false; // Also allow exit if there is a problem on the socket
		//	if (m_packet.Read1() == (NSServerOffset + NSCHello))
		//		dontExit=false;
		//	//Only allow passing on handshake. 
		//	//Otherwise scoreboard updates and such will confuse us.
		//}

		//NetPlayerClient->blocking = false;
		//m_ServerVersion = m_packet.Read1();
		//m_ServerName = m_packet.ReadNT();

		//LOG->Info("Server Version: %d %s", m_ServerVersion, m_ServerName.c_str());
	}
}


void NetworkSyncManager::StartUp()
{
	CString ServerIP;

	if( isLanServer )
		if (!LANserver->ServerStart())
		{
			//If the server happens to not start when told,
			//Print to log and release the memory where the
			//server was held.
			isLanServer = false;
			LOG->Warn("Server failed to start.");
			delete LANserver;
		}

	if( GetCommandlineArgument( "netip", &ServerIP ) )
		PostStartUp(ServerIP);
	else if( GetCommandlineArgument( "listen" ) )
		PostStartUp("LISTEN");

}


bool NetworkSyncManager::Connect(const CString& addy, unsigned short port)
{
	LOG->Info("Beginning to connect");
	if (port != 8765) 
		return false;
	//Make sure using port 8765
	//This may change in future versions
	//It is this way now for protocol's purpose.
	//If there is a new protocol developed down the road

    //NetPlayerClient->create(); // Initilize Socket
    useSMserver = NetPlayerClient->connect(addy, port);

	//m_packet.fromIp = NetPlayerClient->getIp();
    
	return useSMserver;
}


//Listen (Wait for connection in-bound)
//NOTE: Right now, StepMania cannot connect back to StepMania!

//bool NetworkSyncManager::Listen(unsigned short port)
//{
//	LOG->Info("Beginning to Listen");
//	if (port != 8765) 
//		return false;
//	//Make sure using port 8765
//	//This may change in future versions
//	//It is this way now for protocol's purpose.
//	//If there is a new protocol developed down the road
//
//
//	EzSockets * EZListener = new EzSockets;
//
//	EZListener->create();
//	NetPlayerClient->create(); // Initilize Socket
//
//	EZListener->bind(8765);
//    
//	useSMserver = EZListener->listen();
//	useSMserver = EZListener->accept( *NetPlayerClient );  //Wait for someone to connect
//
//	EZListener->close();	//Kill Listener
//	delete EZListener;
//    
//	//LOG->Info("Accept Responce: ",useSMserver);
//	useSMserver=true;
//	return useSMserver;
//}

void NetworkSyncManager::ReportNSSOnOff(int i) 
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCSMS );
	m_packet.Write1( (uint8_t) i );
	//NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position);
}

void NetworkSyncManager::ReportTiming(float offset, int PlayerNumber)
{
	m_lastOffset[PlayerNumber] = offset;
}

void NetworkSyncManager::ReportScore(int playerID, int step, int score, int combo)
{
	if (!useSMserver) //Make sure that we are using the network
		return;
	
	m_packet.ClearPacket();

	m_packet.Write1( NSCGSU );
	uint8_t ctr = (uint8_t) (playerID * 16 + step - 1);
	m_packet.Write1(ctr);

	ctr = uint8_t( g_CurStageStats.GetGrade((PlayerNumber)playerID)*16 );

	if ( g_CurStageStats.bFailedEarlier[(PlayerNumber)playerID] )
		ctr = uint8_t( 112 );	//Code for failed (failed constant seems not to work)

	m_packet.Write1(ctr);

	m_packet.Write4(score);

	m_packet.Write2((uint16_t) combo);

	m_packet.Write2((uint16_t) m_playerLife[playerID]);

	//Offset Info
	//Note: if a 0 is sent, then disregard data.
	//
	//ASSUMED: No step will be more than 16 seconds off center
	//If assumption false: read 16 seconds either direction
	int iOffset = int((m_lastOffset[playerID]+16.384)*2000.0);

	if (iOffset>65535)
		iOffset=65535;
	if (iOffset<1)
		iOffset=1;

	//Report 0 if hold, or miss (don't forget mines should report)
	if (((step<TNS_BOO)||(step>TNS_MARVELOUS))&&(step!=TNS_HIT_MINE))
		iOffset = 0;

	m_packet.Write2((uint16_t) iOffset);

	//NetPlayerClient->SendPack((char*)m_packet.Data, m_packet.Position); 

}

void NetworkSyncManager::ReportSongOver() 
{
	if (!useSMserver)	//Make sure that we are using the network
		return ;

	m_packet.ClearPacket();

	m_packet.Write1( NSCGON );

	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	return;
}

void NetworkSyncManager::ReportStyle() 
{
	if (!useSMserver)
		return;
	m_packet.ClearPacket();
	m_packet.Write1( NSCSU );
	m_packet.Write1( (int8_t) GAMESTATE->GetNumPlayersEnabled() );

	FOREACH_EnabledPlayer( pn ) 
	{
		m_packet.Write1((uint8_t) pn );
		m_packet.WriteNT(GAMESTATE->GetPlayerDisplayName(pn) );
	}

	//NetPlayerClient->SendPack( (char*)&m_packet.Data, m_packet.Position );
}

void NetworkSyncManager::StartRequest(short position) 
{
	if( !useSMserver )
		return;

	if( GAMESTATE->m_bDemonstrationOrJukebox )
		return;

	LOG->Trace("Requesting Start from Server.");

	m_packet.ClearPacket();

	m_packet.Write1( NSCGSR );

	unsigned char ctr=0;

	Steps * tSteps;
	tSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	if ((tSteps!=NULL) && (GAMESTATE->IsPlayerEnabled(PLAYER_1)))
	{
		int tmp = tSteps->GetMeter();
		if(tmp>0 && tmp%16==0)tmp = 1;
		ctr = uint8_t(ctr+tmp*16);
	}
		
	tSteps = GAMESTATE->m_pCurSteps[PLAYER_2];
	if ((tSteps!=NULL) && (GAMESTATE->IsPlayerEnabled(PLAYER_2)))
	{
		int tmp = tSteps->GetMeter();
		if(tmp>0 && tmp%16==0)tmp = 1;
		ctr = uint8_t(ctr+tmp);
	}
		
	m_packet.Write1(ctr);

	ctr=0;

	tSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	if ((tSteps!=NULL) && (GAMESTATE->IsPlayerEnabled(PLAYER_1)))
		ctr = uint8_t(ctr + (int) tSteps->GetDifficulty()*16);

	tSteps = GAMESTATE->m_pCurSteps[PLAYER_2];
	if ((tSteps!=NULL) && (GAMESTATE->IsPlayerEnabled(PLAYER_2)))
		ctr = uint8_t(ctr + (int) tSteps->GetDifficulty());

	m_packet.Write1(ctr);
	
	//Notify server if this is for sync or not.
	ctr = char(position*16);
	m_packet.Write1(ctr);

	if (GAMESTATE->m_pCurSong != NULL)
	{
		m_packet.WriteNT(GAMESTATE->m_pCurSong->m_sMainTitle);
		m_packet.WriteNT(GAMESTATE->m_pCurSong->m_sSubTitle);
		m_packet.WriteNT(GAMESTATE->m_pCurSong->m_sArtist);
	}
	else
	{
		m_packet.WriteNT("");
		m_packet.WriteNT("");
		m_packet.WriteNT("");
	}

	if (GAMESTATE->m_pCurCourse != NULL)
		m_packet.WriteNT(GAMESTATE->m_pCurCourse->GetFullDisplayTitle());
	else
		m_packet.WriteNT(CString(""));

	//Send Player (and song) Options
	m_packet.WriteNT(GAMESTATE->m_SongOptions.GetString());

	int players=0;
	FOREACH_PlayerNumber (p)
	{
		++players;
		m_packet.WriteNT(GAMESTATE->m_PlayerOptions[p].GetString());
	}
	for (int i=0; i<2-players; ++i)
		m_packet.WriteNT("");	//Write a NULL if no player

	//This needs to be reset before ScreenEvaluation could possibly be called
	for (int i=0; i<NETMAXPLAYERS; ++i)
	{
		m_EvalPlayerData[i].name=0;
		m_EvalPlayerData[i].grade=0;
		m_EvalPlayerData[i].score=0;
		m_EvalPlayerData[i].difficulty=(Difficulty)0;
		for (int j=0; j<NETNUMTAPSCORES; ++j)
			m_EvalPlayerData[i].tapScores[j] = 0;
	}

	//Block until go is recieved.
	//Switch to blocking mode (this is the only
	//way I know how to get precievably instantanious results

	bool dontExit=true;

	//Don't block if we are server.
	/*if (isLanServer)
		NetPlayerClient->blocking=false;
	else
		NetPlayerClient->blocking=true;*/
	// TODO: What does this part do?

	//The following packet HAS to get through, so we turn blocking on for it as well
	//Don't block if we are serving
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	
	LOG->Trace("Waiting for RECV");

	m_packet.ClearPacket();

	
	//while (dontExit)
	//{
	//	//Keep the server going during the loop.
	//	if (isLanServer)
	//		LANserver->ServerUpdate();

	//	m_packet.ClearPacket();
	//	if (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)<1)
	//		if (!isLanServer)
	//			dontExit=false; // Also allow exit if there is a problem on the socket
	//							// Only do if we are not the server, otherwise the sync
	//							// gets hosed up due to non blocking mode.
	//	if (m_packet.Read1() == (NSServerOffset + NSCGSR))
	//		dontExit=false;
	//	//Only allow passing on Start request. 
	//	//Otherwise scoreboard updates and such will confuse us.
	//}
	//NetPlayerClient->blocking=false;

}
	
void NetworkSyncManager::DisplayStartupStatus()
{
	CString sMessage("");

	switch (m_startupStatus)
	{
	case 0:
		//Networking wasn't attepmpted
		return;
	case 1:
		//sMessage = "Connection to " + m_ServerName + " sucessful.";
		sMessage = "Connection sucessful.";
		break;
	case 2:
		sMessage = "Connection failed.";
		break;
	}
	SCREENMAN->SystemMessage(sMessage);
}

void NetworkSyncManager::Update(float fDeltaTime)
{
	if (isLanServer)
		LANserver->ServerUpdate();

	if (useSMserver)
		ProcessInput();
}
HANDLE g_hMutex = NULL;
DWORD NetworkSyncManager::ThreadProcNSSSS(void)
{
    LOG->Info("open the share song server!!");
	// CString server_ip = m_packet.ReadNT();
	// CString player_num = m_packet.ReadNT();
	LOG->Info("player_num %d",player_num);
	// int video_file_filter = m_packet.Read1();
	CString now_SongDir;

	//============
	/*CString ip_address=this->NetPlayerClient->getIp();
	LOG->Info("IP address %s", ip_address.c_str());*/
	//==================
	// if(now_SongDir==GAMESTATE->m_pCurSong->GetSongDir())return;
	now_SongDir = GAMESTATE->m_pCurSong->GetSongDir();
	// LOG->Info("now_SongDir %s",now_SongDir.c_str());
	string tmp = now_SongDir.c_str();
	tmp = tmp.substr(0, tmp.size()-1);

	string CurrentPath;
	char buf[100];
    getcwd(buf, sizeof(buf));
	CurrentPath.assign(buf);
	
	// now_SongDir.left(now_SongDir.GetLength()-1);
	// LOG->Info("GAMESTATE->m_pCurSong.m_sSongDir %s",tmp.c_str());
	string connet_dir = CurrentPath;
		   connet_dir +="\\Songs\\connect";
	string connect_dir_cmd="mkdir ";
		connect_dir_cmd+="\"";
		connect_dir_cmd+=connet_dir;
		connect_dir_cmd+="\"";
	LOG->Info("connect_dir_cmd %s",connect_dir_cmd.c_str());
	system(connect_dir_cmd.c_str());//mkdir "C:\\StepMania\\Songs\\connect"
	string zip_name = "temp.zip";
	string init_cmd = "7za.exe d ";
		init_cmd+="\"";
		init_cmd+=connet_dir;
		init_cmd+="\\";
		init_cmd+=zip_name;
		init_cmd+="\"";
	//system("7za.exe d E:\\f5.zip");
	LOG->Info("init_cmd %s",init_cmd.c_str());
	system(init_cmd.c_str());//7za.exe d "C:\\StepMania\\Songs\\connect\\temp.zip"
	CString zip_cmd = "7za.exe a -tzip ";
	zip_cmd+="\"";
	zip_cmd+=connet_dir;
	zip_cmd+="\\";
	zip_cmd+=zip_name;
	zip_cmd+="\" ";

	zip_cmd+="\"";
	zip_cmd+=CurrentPath;
	zip_cmd+="\\";
	zip_cmd+=tmp;
	zip_cmd+="\"";
	LOG->Info("zip_cmd %s",zip_cmd.c_str());
	system(zip_cmd.c_str());//7za.exe a -tzip "C:\\StepMania\\Songs\\connect\\temp.zip" "C:\\StepMania\\Songs\\{SongDir}"
	//==========
	CString filter_cmd = "7za.exe d ";
	filter_cmd+="\"";
	filter_cmd+=connet_dir;
	filter_cmd+="\\";
	filter_cmd+=zip_name;
	filter_cmd+="\" ";
	filter_cmd+="*.avi *.mpeg *.mpg *.mp4 -r";
	if(video_file_filter)
	{
		system(filter_cmd.c_str());//7za.exe d "C:\\StepMania\\Songs\\connect\\temp.zip" *.avi *.mpeg *.mpg *.mp4 -r
	}
	//==========
	//open server and sent require to open client
	CString file_dir;
			file_dir+=connet_dir;
			file_dir+="\\";
			file_dir+=zip_name;
	
	FILE *fp = fopen(file_dir, "rb");
	int file_size = GetFileLength(fp)/1024;//(kbs)
	
	fclose(fp);
	if( PREFSMAN->m_sAdditionalSongFolders != "" && file_size<10)
	{
		system(init_cmd.c_str());//7za.exe d "C:\\StepMania\\Songs\\connect\\temp.zip"
		//now_SongDir-"Songs"
		string tmp = now_SongDir.c_str();
		tmp = tmp.substr(5, tmp.size());
		CString re_zip = "7za.exe a -tzip ";

		re_zip+="\"";
		re_zip+=connet_dir;
		re_zip+="\\";
		re_zip+=zip_name;
		re_zip+="\" ";

		re_zip+="\"";
		re_zip+=PREFSMAN->m_sAdditionalSongFolders;
		re_zip+="\\";
		re_zip+=tmp;
		re_zip+="\"";
		LOG->Info("re_zip %s",re_zip.c_str());
		system(re_zip.c_str());//7za.exe a -tzip "C:\\StepMania\\Songs\\connect\\temp.zip" "C:\\StepMania\\Songs\\{SongDir}"
		//==========
		if(video_file_filter)
		{
			system(filter_cmd.c_str());//7za.exe d "C:\\StepMania\\Songs\\connect\\temp.zip" *.avi *.mpeg *.mpg *.mp4 -r
		}
		//==========
		fp = fopen(file_dir, "rb");
		file_size = GetFileLength(fp)/1024;//(kbs)
		fclose(fp);
	}
	LOG->Info("file_size %d", file_size);

	WaitForSingleObject(g_hMutex, INFINITE);
	m_packet.ClearPacket();
	m_packet.Write1( NSSSC );
	//m_packet.WriteNT( ip_address );
	m_packet.Write1( player_num );
	m_packet.Write4( file_size );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
	// LOG->Info("player_num %d", player_num);
	ReleaseMutex(g_hMutex);
	//==========
	CString server_cmd = "winsocket_server.exe ";
			//server_cmd+=ip_address.c_str();
			server_cmd+=" \"";
			server_cmd+=connet_dir;
			server_cmd+="\\";
			server_cmd+=zip_name;
			server_cmd+="\"";
	LOG->Info("server_cmd %s",server_cmd.c_str());
	system(server_cmd.c_str());//winsocket_server.exe "{server_ip}" "C:\\StepMania\\connect\\temp.zip"
	usingShareSongSystem=false;
	ReportShareSongFinish();
    return 0L;
}
DWORD NetworkSyncManager::ThreadProcNSSSC(void)
{
	LOG->Info("open the share song client!!");
	// CString server_ip = m_packet.ReadNT();
	// CString file_size = m_packet.ReadNT();
	LOG->Info("server_ip %s",server_ip.c_str());
	LOG->Info("file_size %d", file_size);
	CString file_size_;
	file_size_.Format("%d", file_size);

	string CurrentPath;
	char buf[100];
    getcwd(buf, sizeof(buf));
	CurrentPath.assign(buf);

	string connet_dir = CurrentPath;
		connet_dir +="\\Songs\\connect";
	string connect_dir_cmd="mkdir ";
		connect_dir_cmd+="\"";
		connect_dir_cmd+=connet_dir;
		connect_dir_cmd+="\"";
	system(connect_dir_cmd.c_str());//mkdir "C:\\StepMania\\Songs\\connect"
	string zip_name = "temp.zip";
	CString client_cmd = "winsocket_client.exe ";
			client_cmd+=server_ip.c_str();
			client_cmd+=" \"";
			client_cmd+=connet_dir;
			client_cmd+="\\";
			client_cmd+=zip_name;
			client_cmd+="\" ";
			client_cmd+=file_size_.c_str();
	LOG->Info("client_cmd %s",client_cmd.c_str());
	system(client_cmd.c_str());//winsocket_client.exe {IP} "C:\\StepMania\\Songs\\connect\\temp.zip" {file_size}
	CString zip_cmd = "7za.exe x ";
			zip_cmd+=" \"";
			zip_cmd+=connet_dir;
			zip_cmd+="\\";
			zip_cmd+=zip_name;
			zip_cmd+="\" ";
			zip_cmd+="-y -aos -o";
			zip_cmd+="\"";
			zip_cmd+=connet_dir;
			zip_cmd+="\"";
	LOG->Info("zip_cmd %s",zip_cmd.c_str());
	system(zip_cmd.c_str());//7za.exe x "C:\\StepMania\\Songs\\connect\\temp.zip" -y -aos -o"C:\\StepMania\\Songs\\connect"
	usingShareSongSystem=false;
	ReportShareSongFinish();
	SCREENMAN->SendMessageToTopScreen( SM_ReloadConnectPack );
	return 0L;
}
void NetworkSyncManager::ProcessInput()
{
	//If we're disconnected, just exit
	/*if ((NetPlayerClient->state!=NetPlayerClient->skCONNECTED) || 
			NetPlayerClient->IsError())*/
	if (NetPlayerClient->IsError()) {
		SCREENMAN->SystemMessageNoAnimate("Connection to server dropped.");
		useSMserver=false;
		m_sChatText="";
		return;
	}

	//load new data into buffer
	//NetPlayerClient->update();

	m_packet.ClearPacket();

	auto cmds = NetPlayerClient->ReceiveCmds();

	//while (NetPlayerClient->ReadPack((char *)&m_packet, NETMAXBUFFERSIZE)>0)
    for (auto && servercmd : cmds) {

		using namespace Yhaniki::ServerCmd;
		using Yhaniki::As;

		if (servercmd == nullptr)
			return;

		if (auto _ = As<ServerNop>(servercmd))
			return;		

		if (const auto chatCmd = As<SomeoneChatted>(servercmd)) {
			m_sChatText += chatCmd->GetMsg() + " \n ";
			//10000 chars backlog should be more than enough
			m_sChatText = m_sChatText.Right(10000);
			SCREENMAN->SendMessageToTopScreen( SM_AddToChat );
		}

		//int command = m_packet.Read1();
		////Check to make sure command is valid from server
		//if (command < NSServerOffset)
		//{		
		//	LOG->Trace("CMD (below 128) Invalid> %d",command);
 	//		//break;
		//}

		//command = command - NSServerOffset;

		//switch (command)
		//{
		//case NSCPing: //Ping packet responce
		//	m_packet.ClearPacket();
		//	m_packet.Write1( NSCPingR );
		//	//NetPlayerClient->SendPack((char*)m_packet.Data,m_packet.Position);
		//	break;
		//case NSCPingR:	//These are in responce to when/if we send packet 0's
		//case NSCHello: //This is already taken care of by the blocking code earlier on
		//case NSCGSR: //This is taken care of by the blocking start code
		//	break;
		//case NSCGON: 
		//	{
		//		int PlayersInPack = m_packet.Read1();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			m_EvalPlayerData[i].name = m_packet.Read1();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			m_EvalPlayerData[i].score = m_packet.Read4();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			m_EvalPlayerData[i].grade = m_packet.Read1();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			m_EvalPlayerData[i].difficulty = (Difficulty) m_packet.Read1();
		//		for (int j=0; j<NETNUMTAPSCORES; ++j) 
		//			for (int i=0; i<PlayersInPack; ++i)
		//				m_EvalPlayerData[i].tapScores[j] = m_packet.Read2();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			// m_EvalPlayerData[i].percentage = m_packet.ReadNT();
		//			m_EvalPlayerData[i].playerOptions = m_packet.ReadNT();
		//		for (int i=0; i<PlayersInPack; ++i)
		//			m_EvalPlayerData[i].percentage = m_packet.ReadNT();
		//		SCREENMAN->SendMessageToTopScreen( SM_GotEval );
		//	}
		//	break;
		//case NSCGSU: //Scoreboard Update
		//	{	//Ease scope
		//		int ColumnNumber=m_packet.Read1();
		//		int NumberPlayers=m_packet.Read1();
		//		CString ColumnData;
		//		int i;
		//		switch (ColumnNumber)
		//		{
		//		case NSSB_NAMES:
		//			ColumnData = "Names\n";
		//			for (i=0; i<NumberPlayers; ++i)
		//			{
		//				unsigned int k = m_packet.Read1();
		//				if ( k < m_PlayerNames.size() )
		//					ColumnData += m_PlayerNames[k] + "\n";
		//			}
		//			break;
		//		case NSSB_COMBO:
		//			ColumnData = "Combo\n";
		//			for (i=0; i<NumberPlayers; ++i)
		//				ColumnData += ssprintf("%d\n",m_packet.Read2());
		//			break;
		//		case NSSB_GRADE:
		//			ColumnData = "Grade\n";
		//			for (i=0;i<NumberPlayers;i++)
		//				switch (m_packet.Read1())
		//				{
		//				case 0:
		//					ColumnData+="AAAA\n"; break;
		//				case 1:
		//					ColumnData+="AAA\n"; break;
		//				case 2:
		//					ColumnData+="AA\n"; break;
		//				case 3:
		//					ColumnData+="A\n"; break;
		//				case 4:
		//					ColumnData+="B\n"; break;
		//				case 5:
		//					ColumnData+="C\n"; break;
		//				case 6:
		//					ColumnData+="D\n"; break;
		//				case 7: 
		//					ColumnData+="E\n";	break;	//Is there a better way?
		//				}
		//			break;
		//		}
		//		m_Scoreboard[ColumnNumber] = ColumnData;
		//		m_scoreboardchange[ColumnNumber]=true;
		//	}
		//	break;
		//case NSCSU:	//System message from server
		//	{
		//		CString SysMSG = m_packet.ReadNT();
		//		SCREENMAN->SystemMessage( SysMSG );
		//	}
		//	break;
		//case NSCCM:	//Chat message from server
		//	{
		//		m_sChatText += m_packet.ReadNT() + " \n ";
		//		//10000 chars backlog should be more than enough
		//		m_sChatText = m_sChatText.Right(10000);
		//		SCREENMAN->SendMessageToTopScreen( SM_AddToChat );
		//	}
		//	break;
		//case NSCRSG: //Select Song/Play song
		//	{
		//		m_iSelectMode = m_packet.Read1();
		//		m_sMainTitle = m_packet.ReadNT();
		//		m_sArtist = m_packet.ReadNT();
		//		m_sSubTitle = m_packet.ReadNT();
		//		int temp_hash = m_packet.Read4();
		//		if(temp_hash!=0)
		//		{
		//			m_ihash = temp_hash;
		//		}
		//		m_sCurMainTitle=m_sMainTitle;
		//		m_sCurArtist=m_sArtist;
		//		m_sCurSubTitle=m_sSubTitle;
		//		SCREENMAN->SendMessageToTopScreen( SM_ChangeSong );
		//	}
		//	break;
		//case NSCUUL:
		//	{
		//		/*int ServerMaxPlayers=*/m_packet.Read1();
		//		int PlayersInThisPacket=m_packet.Read1();
		//		m_PlayerStatus.clear();
		//		m_PlayerNames.clear();
		//		m_ActivePlayers = 0;
		//		for (int i=0; i<PlayersInThisPacket; ++i)
		//		{
		//			int PStatus = m_packet.Read1();
		//			if ( PStatus > 0 )
		//			{
		//				m_ActivePlayers++;
		//				m_ActivePlayer.push_back( i );
		//			}
		//			m_PlayerStatus.push_back( PStatus );
		//			m_PlayerNames.push_back( m_packet.ReadNT() );	
		//		}
		//	}
		//	break;
		//case NSCSMS:
		//	{
		//		CString StyleName, GameName;
		//		GameName = m_packet.ReadNT();
		//		StyleName = m_packet.ReadNT();

		//		GAMESTATE->m_pCurGame = GAMEMAN->StringToGameType( GameName );
		//		GAMESTATE->m_pCurStyle = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_pCurGame, StyleName );

		//		SCREENMAN->SetNewScreen( "ScreenNetSelectMusic" ); //Should this be metric'd out?
		//	}
		//	break;
		//case NSSSS:
		//	{
		//		server_ip = m_packet.ReadNT();
		//		player_num = m_packet.Read1();
		//		// LOG->Info("player_num %d",player_num);
		//		int filefilterflag = m_packet.Read1();
		//		if(filefilterflag == 0)
		//		{
		//			video_file_filter = false;
		//		}else
		//		{
		//			video_file_filter = true;
		//		}

		//		DWORD ThreadID;
		//		HANDLE thread = CreateThread(NULL, 0, StaticThreadStartNSSSS, (void*) this, 0, &ThreadID);
		//		CloseHandle(thread);
		//	}
		//	break;
		//case NSSSC:
		//	{
		//		server_ip = m_packet.ReadNT();
		//		file_size = m_packet.Read4();
		//		// LOG->Info("server_ip %s",server_ip.c_str());
		//		// LOG->Info("file_size %d",file_size);
		//		DWORD ThreadID;
		//		HANDLE thread = CreateThread(NULL, 0, StaticThreadStartNSSSC, (void*) this, 0, &ThreadID);
		//		CloseHandle(thread);
		//	}
		//	break;
		//case NSCGraph:
		//	{
		//		int PlayersInPack = m_packet.Read1();
		//		int PlayerNum = m_packet.Read1();
		//		if(PlayerNum<PlayersInPack)
		//		{
		//			for(int i=0; i<100; i++)
		//			{
		//				m_EvalPlayerData[PlayerNum].Graph[i] = (float)m_packet.Read4()/10000;
		//			}
		//		}
		//	}
		//	break;
		//case NSCPC:
		//	{	
		//		m_PlayerCondition.clear();
		//		int player_number = m_packet.Read1();
		//		for(int i=0; i<player_number; i++)
		//		{
		//			m_PlayerCondition.push_back(m_packet.Read1());
		//		}
		//		ClientNum = m_packet.Read1();
		//	}
		//}
		//m_packet.ClearPacket();
	}
}

bool NetworkSyncManager::ChangedScoreboard(int Column) 
{
	if (!m_scoreboardchange[Column])
		return false;
	m_scoreboardchange[Column]=false;
	return true;
}

void NetworkSyncManager::SendChat(const CString& message) 
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCCM );
	m_packet.WriteNT( message );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
	NetPlayerClient->SendCmd(
		make_unique<Yhaniki::ClientCmd::Chat>(message));
}

void NetworkSyncManager::ReportPlayerOptions()
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCUPOpts );
	FOREACH_PlayerNumber (pn)
		m_packet.WriteNT( GAMESTATE->m_PlayerOptions[pn].GetString() );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
}
void NetworkSyncManager::ReportPercentage()
{
	m_packet.ClearPacket();
	// m_packet.Write1( NSCUPOpts );
	m_packet.Write1( NSCUPPer );
	FOREACH_PlayerNumber (pn)
	{
		m_packet.WriteNT( GAMESTATE->m_PlayerPercentage[pn] );
	}
		
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
}
void NetworkSyncManager::ReportGraph()
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCGraph );
	FOREACH_PlayerNumber (pn)
	{
		for(int i=0; i<100; i++)
		{
			m_packet.Write4( GAMESTATE->m_PlayerGraph[pn][i] );
		}
	}
		
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position); 
}
void NetworkSyncManager::SelectUserSong()
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCRSG );
	m_packet.Write1( (uint8_t) m_iSelectMode );
	m_packet.WriteNT( m_sMainTitle );
	m_packet.WriteNT( m_sArtist );
	m_packet.WriteNT( m_sSubTitle );
	m_packet.Write4( m_ihash );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}
void NetworkSyncManager::SendHasSong(bool hasSong)
{
	if(hasSong)
	{
		m_packet.ClearPacket();
		m_packet.Write1( NSCCHS );
		m_packet.Write1( 1 );
		//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
	}
}
void NetworkSyncManager::SendAskSong()
{
	m_packet.ClearPacket();
	m_packet.Write1( NSCAS );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}
void NetworkSyncManager::ReportShareSongFinish()
{
	m_packet.ClearPacket();
	m_packet.Write1( NSRSSF );
	//NetPlayerClient->SendPack((char*)&m_packet.Data, m_packet.Position);
}
//Packet functions

uint8_t PacketFunctions::Read1()
{
	if (Position>=NETMAXBUFFERSIZE)
		return 0;
	
	return Data[Position++];
}

uint16_t PacketFunctions::Read2()
{
	if (Position>=NETMAXBUFFERSIZE-1)
		return 0;

	uint16_t Temp;
	memcpy( &Temp, Data + Position,2 );
	Position+=2;		
	return ntohs(Temp);	
}

uint32_t PacketFunctions::Read4()
{
	if (Position>=NETMAXBUFFERSIZE-3)
		return 0;

	uint32_t Temp;
	memcpy( &Temp, Data + Position,4 );
	Position+=4;
	return ntohl(Temp);
}

CString PacketFunctions::ReadNT()
{
	//int Orig=Packet.Position;
	CString TempStr;
	while ((Position<NETMAXBUFFERSIZE)&& (((char*)Data)[Position]!=0))
		TempStr= TempStr + (char)Data[Position++];

	++Position;
	return TempStr;
}


void PacketFunctions::Write1(uint8_t data)
{
	if (Position>=NETMAXBUFFERSIZE)
		return;
	memcpy( &Data[Position], &data, 1 );
	++Position;
}

void PacketFunctions::Write2(uint16_t data)
{
	if (Position>=NETMAXBUFFERSIZE-1)
		return;
	data = htons(data);
	memcpy( &Data[Position], &data, 2 );
	Position+=2;
}

void PacketFunctions::Write4(uint32_t data)
{
	if (Position>=NETMAXBUFFERSIZE-3)
		return ;

	data = htonl(data);
	memcpy( &Data[Position], &data, 4 );
	Position+=4;
}

void PacketFunctions::WriteNT(const CString& data)
{
	int index=0;
	while ((Position<NETMAXBUFFERSIZE)&&(index<data.GetLength()))
		Data[Position++] = (unsigned char)(data.c_str()[index++]);
	Data[Position++] = 0;
}

void PacketFunctions::ClearPacket()
{
	memset((void*)(&Data),0, NETMAXBUFFERSIZE);
	Position = 0;
}
#endif

LuaFunction_NoArgs( IsNetConnected,			NSMAN->useSMserver )

/*
 * (c) 2003-2004 Charles Lohr, Joshua Allen
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
