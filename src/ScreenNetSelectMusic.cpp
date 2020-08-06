#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectMusic.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Style.h"
#include "Steps.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "Actor.h"
#include "AnnouncerManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "StepsUtil.h"
#include "RageUtil.h"
#include "RageLog.h"

#include "SongUtil.h"
#include "InputMapper.h"
//==========
#include "NoteField.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctime>

using namespace std;
//===========

#define CHATINPUT_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","ChatInputBoxWidth")
#define CHATINPUT_HEIGHT			THEME->GetMetricF("ScreenNetSelectMusic","ChatInputBoxHeight")
#define CHATINPUT_COLOR				THEME->GetMetricC("ScreenNetSelectMusic","ChatInputBoxColor")
#define CHATOUTPUT_WIDTH			THEME->GetMetricF("ScreenNetSelectMusic","ChatOutputBoxWidth")
#define CHATOUTPUT_HEIGHT			THEME->GetMetricF("ScreenNetSelectMusic","ChatOutputBoxHeight")
#define CHATOUTPUT_COLOR			THEME->GetMetricC("ScreenNetSelectMusic","ChatOutputBoxColor")
#define SHOW_CHAT_LINES				THEME->GetMetricI("ScreenNetSelectMusic","ChatOutputLines")


#define GROUPSBG_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","GroupsBGWidth")
#define GROUPSBG_HEIGHT				THEME->GetMetricF("ScreenNetSelectMusic","GroupsBGHeight")
#define GROUPSBG_COLOR				THEME->GetMetricC("ScreenNetSelectMusic","GroupsBGColor")

#define SONGSBG_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","SongsBGWidth")
#define SONGSBG_HEIGHT				THEME->GetMetricF("ScreenNetSelectMusic","SongsBGHeight")
#define SONGSBG_COLOR				THEME->GetMetricC("ScreenNetSelectMusic","SongsBGColor")

#define DIFFBG_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","DiffBGWidth")
#define DIFFBG_HEIGHT				THEME->GetMetricF("ScreenNetSelectMusic","DiffBGHeight")
#define DIFFBG_COLOR				THEME->GetMetricC("ScreenNetSelectMusic","DiffBGColor")

#define EXINFOBG_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","ExInfoBGWidth")
#define EXINFOBG_HEIGHT				THEME->GetMetricF("ScreenNetSelectMusic","ExInfoBGHeight")
#define EXINFOBG_COLOR				THEME->GetMetricC("ScreenNetSelectMusic","ExInfoBGColor")

#define	NUM_GROUPS_SHOW				THEME->GetMetricI("ScreenNetSelectMusic","NumGroupsShow");
#define	NUM_SONGS_SHOW				THEME->GetMetricI("ScreenNetSelectMusic","NumSongsShow");

#define SEL_WIDTH					THEME->GetMetricF("ScreenNetSelectMusic","SelWidth")
#define SEL_HEIGHT					THEME->GetMetricF("ScreenNetSelectMusic","SelHeight")
#define SEL_COLOR					THEME->GetMetricC("ScreenNetSelectMusic","SelColor")

#define SUBTITLE_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","SongsSubtitleWidth")
#define ARTIST_WIDTH				THEME->GetMetricF("ScreenNetSelectMusic","SongsArtistWidth")

const int NUM_SCORE_DIGITS	=	9;

#define USERSBG_WIDTH				THEME->GetMetricF("ScreenNetEvaluation","UsersBGWidth")
#define USERSBG_HEIGHT				THEME->GetMetricF("ScreenNetEvaluation","UsersBGHeight")
#define USERSBG_COLOR				THEME->GetMetricC("ScreenNetEvaluation","UsersBGColor")

#define USERDX						THEME->GetMetricF("ScreenNetEvaluation","UserDX")
// #define USERDY						THEME->GetMetricF("ScreenNetEvaluation","UserDY")
#define USERDY						25

#define MAX_COMBO_NUM_DIGITS		THEME->GetMetricI("ScreenEvaluation","MaxComboNumDigits")

const ScreenMessage SM_NoSongs		= ScreenMessage(SM_User+3);
const ScreenMessage	SM_AddToChat	= ScreenMessage(SM_User+4);
const ScreenMessage SM_ChangeSong	= ScreenMessage(SM_User+5);
const ScreenMessage SM_BackFromOpts	= ScreenMessage(SM_User+6);
const ScreenMessage SM_BackFromReloadSongs			= ScreenMessage(SM_User+7);
const ScreenMessage SM_BackFromSelectSongs			= ScreenMessage(SM_User+8);
const ScreenMessage SM_ReloadConnectPack	        = ScreenMessage(SM_User+9);
const ScreenMessage SM_EscFromSelectSongs			= ScreenMessage(SM_User+10);
const CString AllGroups			= "[ALL MUSIC]";

ScreenNetSelectMusic::ScreenNetSelectMusic( const CString& sName ) : ScreenWithMenuElements( sName )
{
	
	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. */
	GAMESTATE->FinishStage();

	//ChatBox
	m_rectChatInputBox.SetDiffuse( CHATINPUT_COLOR ); 
	m_rectChatInputBox.SetName( "ChatInputBox" );
	m_rectChatInputBox.SetWidth( CHATINPUT_WIDTH );
	m_rectChatInputBox.SetHeight( CHATINPUT_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectChatInputBox );
	this->AddChild( &m_rectChatInputBox );

	m_rectChatOutputBox.SetDiffuse( CHATOUTPUT_COLOR );
	m_rectChatOutputBox.SetName( "ChatOutputBox" );
	m_rectChatOutputBox.SetWidth( CHATOUTPUT_WIDTH );
	m_rectChatOutputBox.SetHeight( CHATOUTPUT_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectChatOutputBox );
	this->AddChild( &m_rectChatOutputBox );

	m_textChatInput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatInput.SetHorizAlign( align_left );
	m_textChatInput.SetVertAlign( align_top );
	m_textChatInput.SetShadowLength( 0 );
	m_textChatInput.SetName( "ChatInput" );
	m_textChatInput.SetWrapWidthPixels( (int)(CHATINPUT_WIDTH * 2) );
	SET_XY_AND_ON_COMMAND( m_textChatInput );
	this->AddChild( &m_textChatInput );

	m_textOutHidden.LoadFromFont( THEME->GetPathF("ScreenNetSelectMusic","chat") );
	m_textOutHidden.SetWrapWidthPixels( (int)(CHATOUTPUT_WIDTH * 2) );

	m_textChatOutput.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textChatOutput.SetHorizAlign( align_left );
	m_textChatOutput.SetVertAlign( align_bottom );
	m_textChatOutput.SetShadowLength( 0 );
	m_textChatOutput.SetName( "ChatOutput" );
	SET_XY_AND_ON_COMMAND( m_textChatOutput );
	this->AddChild( &m_textChatOutput );

	//Display updated chat (maybe this should be a function)?
	m_textOutHidden.SetText( NSMAN->m_sChatText );
	vector <wstring> wLines;
	m_textOutHidden.GetLines( wLines );
	m_actualText = "";
	for (unsigned i = max(int(wLines.size()) - SHOW_CHAT_LINES, 0 ) ; i < wLines.size() ; ++i)
		m_actualText += WStringToCString( wLines[i] )+'\n';
	m_textChatOutput.SetText( m_actualText );

	//Groups

	m_rectGroupsBackground.SetDiffuse( GROUPSBG_COLOR );
	m_rectGroupsBackground.SetName( "GroupsBG" );
	m_rectGroupsBackground.SetWidth( GROUPSBG_WIDTH );
	m_rectGroupsBackground.SetHeight( GROUPSBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectGroupsBackground );
	this->AddChild( &m_rectGroupsBackground );

	m_textGroups.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textGroups.SetShadowLength( 0 );
	m_textGroups.SetName( "GroupsList" );
	m_textGroups.SetMaxWidth( GROUPSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textGroups );
	this->AddChild( &m_textGroups);

	//Songs
	m_rectSongsBackground.SetDiffuse( SONGSBG_COLOR );
	m_rectSongsBackground.SetName( "SongsBackground" );
	m_rectSongsBackground.SetWidth( SONGSBG_WIDTH );
	m_rectSongsBackground.SetHeight( SONGSBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectSongsBackground );
	this->AddChild( &m_rectSongsBackground );

	m_textSongs.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textSongs.SetShadowLength( 0 );
	m_textSongs.SetName( "SongsList" );
	m_textSongs.SetMaxWidth( SONGSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textSongs );
	this->AddChild( &m_textSongs);

	m_rectExInfo.SetDiffuse( EXINFOBG_COLOR );
	m_rectExInfo.SetName( "ExInfoBG" );
	m_rectExInfo.SetWidth( EXINFOBG_WIDTH );
	m_rectExInfo.SetHeight( EXINFOBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectExInfo );
	this->AddChild( &m_rectExInfo );

	m_textArtist.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textArtist.SetShadowLength( 0 );
	m_textArtist.SetName( "SongsArtist" );
	m_textArtist.SetMaxWidth( ARTIST_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textArtist );
	this->AddChild( &m_textArtist);

	m_textSubtitle.LoadFromFont( THEME->GetPathF(m_sName,"chat") );
	m_textSubtitle.SetShadowLength( 0 );
	m_textSubtitle.SetName( "SongsSubtitle" );
	m_textSubtitle.SetMaxWidth( SUBTITLE_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textSubtitle );
	this->AddChild( &m_textSubtitle );

	//SelectOptions Sprite
	m_sprSelOptions.Load( THEME->GetPathG( m_sName, "options" ) );
	m_sprSelOptions.SetName( "options" );
	SET_XY_AND_ON_COMMAND( m_sprSelOptions );
	this->AddChild( &m_sprSelOptions );

	//Diff Icon background
	m_rectDiff.SetDiffuse( DIFFBG_COLOR );
	m_rectDiff.SetName( "DiffBG" );
	m_rectDiff.SetWidth( DIFFBG_WIDTH );
	m_rectDiff.SetHeight( DIFFBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectDiff );
	this->AddChild( &m_rectDiff );


	FOREACH_EnabledPlayer (p)
	{
		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathG( "ScreenSelectMusic",
												   ssprintf("difficulty icons 1x%d",
															NUM_DIFFICULTIES)) );
		SET_XY( m_DifficultyIcon[p] );
		this->AddChild( &m_DifficultyIcon[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
		m_DC[p] = GAMESTATE->m_PreferredDifficulty[p];
		m_DifficultyIcon[p].SetFromDifficulty( p, m_DC[p] );

		m_DifficultyMeters[p].SetName( "DifficultyMeter", ssprintf("MeterP%d",p+1) );
		m_DifficultyMeters[p].Load();
		SET_XY_AND_ON_COMMAND( m_DifficultyMeters[p] );
		this->AddChild( &m_DifficultyMeters[p] );
	}

	m_SelectMode = SelectGroup;
	m_rectSelection.SetDiffuse( SEL_COLOR );
	m_rectSelection.SetName( "Sel" );
	m_rectSelection.SetWidth( SEL_WIDTH );
	m_rectSelection.SetHeight( SEL_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectSelection );
	this->AddChild( &m_rectSelection );

	SONGMAN->GetGroupNames( m_vGroups );
	if (m_vGroups.size()<1)
	{
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		SCREENMAN->SystemMessage( "You don't have any songs in the game" );
		//SCREENMAN->SendMessageToTopScreen( SM_NoSongs );
		return;
	}
	
	//Make the last group the full list group.
	//Must be last
	m_vGroups.push_back( AllGroups );
	m_iShowGroups = NUM_GROUPS_SHOW;
	m_iShowSongs = NUM_SONGS_SHOW;
	m_iGroupNum=m_vGroups.size()-1;	//Alphabetical

	UpdateGroupsListPos();
	UpdateSongsList();
	if(GAMESTATE->m_pCurSong==NULL)
	{
		GAMESTATE->m_pCurSong=m_vSongs[0];
	}
	if (GAMESTATE->m_pCurSong != NULL)
	{
		for ( unsigned i = 0 ; i<m_vSongs.size() ; ++i )
			if (m_vSongs[i]->GetFullDisplayTitle() == GAMESTATE->m_pCurSong->GetFullDisplayTitle())
				m_iSongNum = i;
				
		CString GroupName =  m_vSongs[m_iSongNum]->m_sGroupName;
		for(unsigned j = 0; j<m_vGroups.size(); ++j)
		{
			if(GroupName == m_vGroups[j])
			{
				m_iGroupNum = j;
				UpdateGroupsListPos();
				UpdateSongsList();
				for ( unsigned i = 0 ; i<m_vSongs.size() ; ++i )
					if (m_vSongs[i]->GetFullDisplayTitle() == GAMESTATE->m_pCurSong->GetFullDisplayTitle())
						m_iSongNum = i;
				m_iSongNum += m_vSongs.size();
				break;
			}
		}
	}	
	
	UpdateSongsListPos();
	CheckChangeSong();
	//================
	// unsigned i=0;
	// unsigned j=0;
	// bool find_song_flag = false;
	// if(GAMESTATE->m_pCurSong!=NULL){
	// 	for(j=0; j < m_vGroups.size(); j++){
	// 		if(find_song_flag)
	// 			break;
	// 		m_iGroupNum = j;
	// 		UpdateSongsList();
	// 		for ( i = 0; i < m_vSongs.size(); ++i)
	// 		{
	// 			if (m_vSongs[i]->GetFullDisplayTitle() == GAMESTATE->m_pCurSong->GetFullDisplayTitle())
	// 			{
	// 				find_song_flag = true;
	// 				break;
	// 			}
	// 		}
	// 	}
	// }
	// UpdateGroupsListPos();
	// m_iSongNum = i + m_vSongs.size();
	// UpdateSongsListPos();
	//================

	//Load SFX next
	m_soundChangeOpt.Load( THEME->GetPathToS("ScreenNetSelectMusic change opt"));
	m_soundChangeSel.Load( THEME->GetPathToS("ScreenNetSelectMusic change sel"));
	//=====user rect======
	int ShowSide;
	m_pActivePlayer = PLAYER_1;	

	FOREACH_PlayerNumber (pn)
		if ( GAMESTATE->IsPlayerEnabled( pn ) )
			m_pActivePlayer = pn;

	if (m_pActivePlayer == PLAYER_1)
		ShowSide = 2;
	else
		ShowSide = 1;
	m_rectUsersBG.SetWidth( USERSBG_WIDTH );
	m_rectUsersBG.SetHeight( USERSBG_HEIGHT );
	m_rectUsersBG.SetDiffuse( USERSBG_COLOR );
	m_rectUsersBG.SetName( "UsersBG" );
	//ON_COMMAND( m_rectUsersBG );
	
	m_rectUsersBG.SetXY(
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dX",ShowSide)),
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dY",ShowSide)) );

	this->AddChild( &m_rectUsersBG );

	float cx = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dX",ShowSide));
	float cy = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dY",ShowSide));
	
	m_iActivePlayers = 20;
	m_iCurrentPlayer = 0;

	for( int i=0; i<m_iActivePlayers; ++i )
	{
		//m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"names") );
		m_textUsers[i].SetVertAlign(align_top);
		//m_textUsers[i].SetHorizAlign( align_left );
		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"chat") );
		m_textUsers[i].SetName( "User" );
		m_textUsers[i].SetShadowLength( 1 );
		m_textUsers[i].SetXY( cx+30, cy );
		
		m_textUsersNum[i].SetVertAlign(align_top);
		// m_textUsersNum[i].SetHorizAlign( align_left );
		m_textUsersNum[i].LoadFromFont( THEME->GetPathF(m_sName,"chat") );
		m_textUsersNum[i].SetName( "UserNum" );
		m_textUsersNum[i].SetShadowLength( 1 );
		m_textUsersNum[i].SetXY( cx-80, cy );
		
		this->AddChild( &m_textUsers[i] );
		this->AddChild( &m_textUsersNum[i] );
		cx+=USERDX;
		cy+=USERDY;
	}
	//======================
	NSMAN->ReportNSSOnOff(1);
	NSMAN->ReportPlayerOptions();
	NSMAN->SendAskSong();
	return;
}

void ScreenNetSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{
	if (m_In.IsTransitioning() || m_Out.IsTransitioning())
		return;

	if ((type != IET_FIRST_PRESS) && (type != IET_SLOW_REPEAT) && (type != IET_FAST_REPEAT))
		return;

	if ((DeviceI.button == KEY_RIGHT || DeviceI.button == KEY_LEFT) ||
		(MenuI.button == MENU_BUTTON_RIGHT || MenuI.button == MENU_BUTTON_LEFT))
	{
		// TRICKY:  There's lots of weirdness that can happen here when tapping 
		// Left and Right quickly, like when changing sort.
		bool bLeftAndRightPressed =
			(INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LEFT)) &&
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RIGHT))) ||
				(INPUTMAPPER->IsButtonDown(MenuInput(MenuI.player, MENU_BUTTON_LEFT)) &&
					INPUTMAPPER->IsButtonDown(MenuInput(MenuI.player, MENU_BUTTON_RIGHT)));

		if (bLeftAndRightPressed)
		{
			if (m_SelectMode == SelectGroup || m_SelectMode == SelectSong)
			{
				NSMAN->ReportNSSOnOff(0);
				GAMESTATE->m_bEditing = true;
				int j = m_iSongNum % m_vSongs.size();
				GAMESTATE->m_pPreferredSong = m_vSongs[j];
				SCREENMAN->AddNewScreenToTop("ScreenSelectMusic", SM_BackFromSelectSongs);
				return;
			}
		}
	}
	if (DeviceI.button == KEY_LEFT)
	{
		MenuLeft(MenuI.player, type);
		return;
	}
	else if (DeviceI.button == KEY_RIGHT)
	{
		MenuRight(MenuI.player, type);
		return;
	}
	else if (DeviceI.button == KEY_UP)
	{
		MenuUp(MenuI.player, type);
		return;
	}
	else if (DeviceI.button == KEY_DOWN)
	{
		MenuDown(MenuI.player, type);
		return;
	}
	bool bHoldingShift =
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT));

	bool bHoldingCtrl =
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	//If we are disconnected, assume no chatting
	bool input_check = false;

	bool bHoldingF1 = INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_F1));
	if (bHoldingF1)
	{
		GAMESTATE->m_bLoadPackConnect = true;
		NSMAN->ReportNSSOnOff(2);
		GAMESTATE->m_bEditing = true;
		// SCREENMAN->PopTopScreen();
		SCREENMAN->AddNewScreenToTop("ScreenReloadSongs", SM_BackFromReloadSongs);
		return;
	}
	bool bHoldingF5 = INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_F5));
	if (bHoldingF5)
	{
		NSMAN->ReportNSSOnOff(2);
		GAMESTATE->m_bEditing = true;
		// SCREENMAN->PopTopScreen();
		SCREENMAN->AddNewScreenToTop("ScreenReloadSongs", SM_BackFromReloadSongs);
	}
	// bool bHoldingtab = INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_TAB));

	if (DeviceI.button == KEY_F8)
	{
		if (GAMESTATE->ScreenNetSelectMusicAlpha) {
			GAMESTATE->ScreenNetSelectMusicAlpha = false;
		}
		else
		{
			GAMESTATE->ScreenNetSelectMusicAlpha = true;
		}
	}
	switch (DeviceI.button)
	{
	case KEY_ENTER:
	case KEY_KP_ENTER:
		if (!bHoldingCtrl)
		{
			if (m_sTextInput != "")
				NSMAN->SendChat(m_sTextInput);
			m_sTextInput = "";
			UpdateTextInput();
			return;
		}
		break;
	case KEY_BACK:
		if (!m_sTextInput.empty())
			m_sTextInput = m_sTextInput.erase(m_sTextInput.size() - 1);
		UpdateTextInput();
		break;

		// case KEY_F1:
		// 	{
		// 		GAMESTATE->m_bLoadPackConnect=true;
		// 	    NSMAN->ReportNSSOnOff(2);
		// 		GAMESTATE->m_bEditing = true;
		// 		// SCREENMAN->PopTopScreen();
		// 		SCREENMAN->AddNewScreenToTop( "ScreenReloadSongs", SM_BackFromReloadSongs );
		// 	}
		// 	break;
	case KEY_F4:
	{
		CheckChangeSong();
	}
	break;
	// case KEY_F5:
	// 	{
	// 	    NSMAN->ReportNSSOnOff(2);
	// 		GAMESTATE->m_bEditing = true;
	// 		// SCREENMAN->PopTopScreen();
	// 		SCREENMAN->AddNewScreenToTop( "ScreenReloadSongs", SM_BackFromReloadSongs );
	// 	}
	break;
	default:
		char c;
		c = DeviceI.ToChar();
		if ((int)DeviceI.button > 0 && (int)DeviceI.button < 127)
		{
			input_check = true;
		}
		if (bHoldingShift && !bHoldingCtrl)
		{
			c = (char)toupper(c);

			switch (c)
			{
			case '`':	c = '~';	break;
			case '1':	c = '!';	break;
			case '2':	c = '@';	break;
			case '3':	c = '#';	break;
			case '4':	c = '$';	break;
			case '5':	c = '%';	break;
			case '6':	c = '^';	break;
			case '7':	c = '&';	break;
			case '8':	c = '*';	break;
			case '9':	c = '(';	break;
			case '0':	c = ')';	break;
			case '-':	c = '_';	break;
			case '=':	c = '+';	break;
			case '[':	c = '{';	break;
			case ']':	c = '}';	break;
			case '\'':	c = '"';	break;
			case '\\':	c = '|';	break;
			case ';':	c = ':';	break;
			case ',':	c = '<';	break;
			case '.':	c = '>';	break;
			case '/':	c = '?';	break;
			}
		}

		//Search list for given letter (to aide in searching)
		if (bHoldingCtrl)
		{
			c = (char)toupper(c);
			for (int i = 0; i < (int)m_vSongs.size(); ++i)
				if ((char)toupper(m_vSongs[i]->GetTranslitMainTitle().c_str()[0]) == (char)c)
				{
					m_iSongNum = i;
					UpdateSongsListPos();
					break;
				}
		}

		if ((c >= ' ') && (!bHoldingCtrl))
		{
			m_sTextInput += c;
			UpdateTextInput();
		}
		break;
	}
	if (!input_check && DeviceI.button != KEY_F1)
	{
		Screen::Input(DeviceI, type, GameI, MenuI, StyleI);	// default input handler
	}
}
bool ScreenNetSelectMusic::CheckHash(Song * temp_song)
{
	//=========test======
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
	vector <Steps *> MultiSteps;
	int notenum = 0;
	MultiSteps = temp_song->GetAllSteps( st );
	for(int i = 0; i<MultiSteps.size(); i++)
	{
		notenum=MultiSteps[i]->GetNumTapNotesformSetp();
		if(notenum==NSMAN->m_ihash)
		{
			//LOG->Info("find the song notenum %d",notenum);
			return true;
		}
	}
	return false;
}
void ScreenNetSelectMusic::CheckChangeSong()
{
	//===============
	m_iGroupNum=m_vGroups.size()-1;
	UpdateSongsList();
	unsigned i;
	for ( i = 0; i < m_vSongs.size(); ++i)
		if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
			( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
			( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
			{
				// break;
				if(CheckHash(m_vSongs[i])==true)
				{
					break;
				}
			}
			
	bool haveSong = i != m_vSongs.size();
	if(haveSong)
	{
		bool find_song_in_group = false;
		CString GroupName =  m_vSongs[i]->m_sGroupName;
		for(unsigned j = 0; j<m_vGroups.size(); ++j)
		{
			if(GroupName == m_vGroups[j])
			{
				m_iGroupNum = j;
				UpdateGroupsListPos();
				UpdateSongsList();
				for ( i = 0 ; i<m_vSongs.size() ; ++i )
				{
					if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
						( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
						( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
					{
						// find_song_in_group = true;
						// break;
						if(CheckHash(m_vSongs[i])==true)
						{
							find_song_in_group = true;
							break;
						}
					}
				}
				if(find_song_in_group)
					break;
			}
		}
	}
	//===============
	// unsigned i;
	// unsigned j;
	// bool find_song_flag = false;
	// if(GAMESTATE->m_pCurSong==NULL){
	// 	//LOG->Info("GAMESTATE->m_pCurSong NULL");
	// 	return;
	// }
	// //LOG->Info("GAMESTATE->m_pCurSong %s", GAMESTATE->m_pCurSong->GetTranslitMainTitle().c_str());
	// for(j=0; j < m_vGroups.size(); j++){
	// 	if(find_song_flag)
	// 		break;
	// 	m_iGroupNum = j;
	// 	UpdateSongsList();
	// 	for ( i = 0; i < m_vSongs.size(); ++i)
	// 	{
	// 		if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sCurArtist ) ) &&
	// 			( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sCurMainTitle ) ) &&
	// 			( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sCurSubTitle ) ) )
	// 		{
	// 			find_song_flag = true;
	// 			break;
	// 		}
	// 	}
	// }
	// bool haveSong = j != m_vGroups.size();
	if(haveSong)
	{
		UpdateGroupsListPos();
		m_iSongNum = i + m_vSongs.size();
		UpdateSongsListPos();
		NSMAN->SendHasSong(true);
	}
	else if(NSMAN->m_sCurMainTitle=="")
	{
		UpdateGroupsListPos();
		m_iSongNum = i + m_vSongs.size();
		UpdateSongsListPos();
	}
}
void ScreenNetSelectMusic::ResetSongList()
{
	//=============
	m_vGroups.clear();
	m_vSongs.clear();
	SONGMAN->GetGroupNames( m_vGroups );
	//Make the last group the full list group.
	//Must be last
	m_vGroups.push_back( AllGroups );
	m_iShowGroups = NUM_GROUPS_SHOW;
	m_iShowSongs = NUM_SONGS_SHOW;
	m_iGroupNum=m_vGroups.size()-1;	//Alphabetical

	UpdateGroupsListPos();
	UpdateSongsList();
	UpdateSongsListPos();
	GAMESTATE->m_pPreferredSong=NULL;
	GAMESTATE->m_pPreferredCourse=NULL;
	
	CheckChangeSong();
	//==================
}
void ScreenNetSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SOUND->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenStage" );
		break;
	case SM_NoSongs:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_AddToChat:
		{
			//m_textOutHidden.SetText2( NSMAN->m_sChatText );
			m_textOutHidden.SetText( NSMAN->m_sChatText );
			vector <wstring> wLines;
			m_textOutHidden.GetLines( wLines );
			m_actualText = "";
			for (unsigned i = max(int(wLines.size()) - SHOW_CHAT_LINES, 0 ) ; i < wLines.size() ; ++i)
			{
				m_actualText += WStringToCString( wLines[i] )+'\n';
			}
			// m_textChatOutput.SetText2( m_actualText );
			m_textChatOutput.SetText( m_actualText );
			break;
		}
	case SM_ChangeSong:
		{
			// //We always need to find the song
			// m_iGroupNum=m_vGroups.size()-1;	//Alphabetical 
			// //m_vGroups.size()-1allsongs? 
			// //so we need to search the pack of the song

			// UpdateGroupsListPos();
			// UpdateSongsList();
			
			// unsigned i;
	
			// for ( i = 0; i < m_vSongs.size(); ++i)
			// 	if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
			// 		 ( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
			// 		 ( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
			// 		 break;
			
			// bool haveSong = i != m_vSongs.size();//serch allsongs_group to the end
			//==========================
			m_iGroupNum=m_vGroups.size()-1;
			UpdateSongsList();
			unsigned i;
			for ( i = 0; i < m_vSongs.size(); ++i)
				if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
					 ( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
					 ( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
					 {
						// break;
						if(CheckHash(m_vSongs[i])==true)
						{
							break;
						}
					 }
			bool haveSong = i != m_vSongs.size();
			if(haveSong)
			{
				bool find_song_in_group = false;
				CString GroupName =  m_vSongs[i]->m_sGroupName;
				for(unsigned j = 0; j<m_vGroups.size(); ++j)
				{
					if(GroupName == m_vGroups[j])
					{
						m_iGroupNum = j;
						UpdateGroupsListPos();
						UpdateSongsList();
						for ( i = 0 ; i<m_vSongs.size() ; ++i )
						{
							if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
								 ( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
								 ( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
							{
								// find_song_in_group = true;
								// break;
								if(CheckHash(m_vSongs[i])==true)
								{
									find_song_in_group = true;
									break;
								}
							}
						}
						if(find_song_in_group)
							break;
					}
				}
			}
			//==========================
			// unsigned i;
			// unsigned j;
			// bool find_song_flag = false;

			// for(j=0; j < m_vGroups.size(); j++){
			// 	if(find_song_flag)
			// 		break;
			// 	m_iGroupNum = j;
			// 	UpdateSongsList();
			// 	for ( i = 0; i < m_vSongs.size(); ++i)
			// 	{
			// 		if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sCurArtist ) ) &&
			// 			( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sCurMainTitle ) ) &&
			// 			( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sCurSubTitle ) ) )
			// 		{
			// 			find_song_flag = true;
			// 			break;
			// 		}
			// 	}
			// }
			// bool haveSong = j != m_vGroups.size();
			// if(haveSong)
			// 	UpdateGroupsListPos();
			//==========================
			static int pre_m_iSelectMode=0;
			if(pre_m_iSelectMode==2 && NSMAN->m_iSelectMode==2)
			{
				break;
			}
			pre_m_iSelectMode=NSMAN->m_iSelectMode;

			switch (NSMAN->m_iSelectMode)
			{
			case 3:
				StartSelectedSong();
				break;
			case 2: //We need to do cmd 1 as well here
				if (haveSong)
				{
					m_iSongNum = i + m_vSongs.size();
					UpdateSongsListPos();
					StartSelectedSong();
				}
				break;
			case 1:	//Scroll to song as well
				if (haveSong)
				{
					m_iSongNum = i + m_vSongs.size();
					UpdateSongsListPos();
				}
				//don't break here
			case 0:	//See if client has song
				if (haveSong)
					NSMAN->m_iSelectMode = 0;
				else
					NSMAN->m_iSelectMode = 1;
				NSMAN->SelectUserSong();
			}
		}
		break;
	case SM_BackFromOpts:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_bEditing = false;
		NSMAN->ReportPlayerOptions();
		NSMAN->SendAskSong();
		break;
	case SM_BackFromReloadSongs:
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_bEditing = false;
		NSMAN->ReportPlayerOptions();
		ResetSongList();
		NSMAN->SendAskSong();
		CheckChangeSong();
		break;
	case SM_BackFromSelectSongs:
		{
			// LOG->Info("back form ScreenSelectMusic");
			// NSMAN->ReportNSSOnOff(3);
			GAMESTATE->m_bEditing = false;
			NSMAN->ReportPlayerOptions();
			ResetSongList();
				
			if ((strcmp(NSMAN->m_sCurMainTitle, NSMAN->m_sMainTitle) == 0) &&
				(strcmp(NSMAN->m_sCurSubTitle, NSMAN->m_sSubTitle) == 0) &&
				(strcmp(NSMAN->m_sCurArtist, NSMAN->m_sArtist) == 0))
			{
				//choose the same song
				if(NSMAN->m_sCurMainTitle!="")
				{
					CheckChangeSong();
					break;
				}	
			}
			NSMAN->SelectUserSong ();
			NSMAN->SendAskSong();
			break;
		}
		
	case SM_ReloadConnectPack:
		GAMESTATE->m_bLoadPackConnect=true;
		NSMAN->ReportNSSOnOff(2);
		GAMESTATE->m_bEditing = true;
		// SCREENMAN->PopTopScreen();
		SCREENMAN->AddNewScreenToTop( "ScreenReloadSongs", SM_BackFromReloadSongs );
		break;
	case SM_EscFromSelectSongs:
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_bEditing = false;
		NSMAN->ReportPlayerOptions();
		ResetSongList();
		NSMAN->SendAskSong();
		CheckChangeSong();
		break;
	}

}
vector <Steps *> ScreenNetSelectMusic::SortStep(vector <Steps *> MultiSteps)
{
	vector <Steps *> sort;
	sort.assign(MultiSteps.begin(), MultiSteps.end());
	
	for ( int i=(int)sort.size()-1; i>0; --i )
	{
		for(int j=0; j<i; ++j)
		{
			if(sort[j]->GetDifficulty() > sort[j+1]->GetDifficulty())
			{
				Steps *tmp = sort[j];
				sort[j] = sort[j+1];
				sort[j+1] = tmp;
			}
		}
	}
	//MultiSteps.assign(sort.begin(), sort.end());
	return sort;
}
void ScreenNetSelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	int i;
	m_soundChangeOpt.Play();
	switch (m_SelectMode)
	{
	case SelectGroup:
		m_iGroupNum--;
		m_iSongNum=0;
		UpdateGroupsListPos();
		UpdateSongsList();
		UpdateSongsListPos();
		break;
	case SelectSong:
		m_iSongNum--;
		UpdateSongsListPos();
		break;
	case SelectDifficulty:
		{
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			vector <Steps *> MultiSteps;
			MultiSteps = GAMESTATE->m_pCurSong->GetAllSteps( st );
			if (MultiSteps.size() == 0)
				m_DC[pn] = NUM_DIFFICULTIES;//no lv?
			else
			{
				MultiSteps = SortStep(MultiSteps);
				
				for ( i=0; i<(int)MultiSteps.size(); ++i )
					if ( MultiSteps[i]->GetDifficulty() >= m_DC[pn] )
						break;

				if ( i == (int)MultiSteps.size() )
					m_DC[pn] = MultiSteps[i-1]->GetDifficulty();
				else
					if (i > 0)	//If we are at the easiest difficulty, do nothign
						m_DC[pn] = MultiSteps[i-1]->GetDifficulty();
			}
			UpdateDifficulties( pn );
			GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		}
		break;
	case SelectOptions:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(2);
		GAMESTATE->m_bEditing = true;
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromOpts );
		break;
	}
}

void ScreenNetSelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	int i;
	m_soundChangeOpt.Play();
	switch (m_SelectMode)
	{
	case SelectGroup:
		m_iGroupNum++;
		m_iSongNum=0;
		UpdateGroupsListPos();
		UpdateSongsList();
		UpdateSongsListPos();
		break;
	case SelectSong:
		m_iSongNum++;
		UpdateSongsListPos();
		break;
	case SelectDifficulty:
		{
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			vector <Steps *> MultiSteps;
			MultiSteps = GAMESTATE->m_pCurSong->GetAllSteps( st );
			if (MultiSteps.size() == 0)
				m_DC[pn] = NUM_DIFFICULTIES;
			else
			{
				MultiSteps = SortStep(MultiSteps);

				for ( i=0; i<(int)MultiSteps.size(); ++i )
					if ( MultiSteps[i]->GetDifficulty() >= m_DC[pn] )
						break;

				if ( i == (int)MultiSteps.size() )
					m_DC[pn] = MultiSteps[i-1]->GetDifficulty();
				else
					if (i < (int)MultiSteps.size() - 1 )	//If we are at the hardest difficulty, do nothign
						m_DC[pn] = MultiSteps[i+1]->GetDifficulty();
			}
			UpdateDifficulties( pn );
			GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		}
		break;
	case SelectOptions:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(2);
		GAMESTATE->m_bEditing = true;
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromOpts );
		break;
	}
}

void ScreenNetSelectMusic::MenuUp( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetScreenSelectModes) ( ( (int)m_SelectMode - 1) % (int)SelectModes);
	if ( (int) m_SelectMode < 0) 
		m_SelectMode = (NetScreenSelectModes) (SelectModes - 1);
	COMMAND( m_rectSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetSelectMusic::MenuDown( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetScreenSelectModes) ( ( (int)m_SelectMode + 1) % (int)SelectModes);
	COMMAND( m_rectSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetSelectMusic::MenuStart( PlayerNumber pn )
{
	GAMESTATE->m_bEditing=false;
	if ( NSMAN->useSMserver )
	{
		int j = m_iSongNum % m_vSongs.size();
		NSMAN->m_sArtist = m_vSongs[j]->GetTranslitArtist();
		NSMAN->m_sMainTitle = m_vSongs[j]->GetTranslitMainTitle();
		NSMAN->m_sSubTitle = m_vSongs[j]->GetTranslitSubTitle();
		NSMAN->m_iSelectMode = 2; //Command for user selecting song
		//=========hash======
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		NoteField cur_song_note_date;
		Steps * SelectStep;
		SelectStep = m_vSongs[j]->GetStepsByDifficulty(st, m_DC[pn]);
		int notenum = 0;
		notenum = SelectStep->GetNumTapNotesformSetp();
		// LOG->Info("m_iNumTracks %d", notenum);
		if(notenum<=0)return;
		NSMAN->m_ihash = notenum;
		//===================
		NSMAN->SelectUserSong ();
	}
	else
		StartSelectedSong();
}

void ScreenNetSelectMusic::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();
	TweenOffScreen();

	Back( SM_GoToPrevScreen );
}


void ScreenNetSelectMusic::TweenOffScreen()
{
	OFF_COMMAND( m_rectChatInputBox );
	OFF_COMMAND( m_rectChatOutputBox );
	OFF_COMMAND( m_textChatInput );
	OFF_COMMAND( m_textChatOutput );

	OFF_COMMAND( m_rectSelection );
	OFF_COMMAND( m_textGroups );
	OFF_COMMAND( m_rectGroupsBackground );

	OFF_COMMAND( m_textSongs );
	OFF_COMMAND( m_rectSongsBackground );

	OFF_COMMAND( m_textArtist );
	OFF_COMMAND( m_textSubtitle );

	OFF_COMMAND( m_rectExInfo );
	OFF_COMMAND( m_rectDiff );

	OFF_COMMAND( m_sprSelOptions );

	FOREACH_EnabledPlayer (pn)
	{
		OFF_COMMAND( m_DifficultyMeters[pn] );
		OFF_COMMAND( m_DifficultyIcon[pn] );
	}
	NSMAN->ReportNSSOnOff(0);
}


void ScreenNetSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	UpdateUsersStates();
}
void ScreenNetSelectMusic::UpdateUsersStates()
{
	float alpha = 1;
	if(GAMESTATE->ScreenNetSelectMusicAlpha)
	{
		alpha = 1;
	}else{
		alpha = 0;
	}

	m_rectUsersBG.SetDiffuse(RageColor(0,1,0.9,alpha*0.2));
	for( int i=0; i<20; ++i )
	{
		m_textUsers[i].SetText( "" );
		m_textUsersNum[i].SetText( "" );
	}
	//RageColor(R,G,B,A)
	for( int i=0; i<NSMAN->m_PlayerNames.size()/2; ++i )
	{
		if(NSMAN->m_PlayerCondition.size()<=i)
			return;
		if(NSMAN->m_PlayerCondition[i]==0)
		{
			m_textUsers[i].SetDiffuse(RageColor(1,1,1,alpha));
			m_textUsersNum[i].SetDiffuse(RageColor(1,1,1,alpha));
		}else if(NSMAN->m_PlayerCondition[i]==1)
		{
			m_textUsers[i].SetDiffuse(RageColor(0.9,0,0,alpha));
			m_textUsersNum[i].SetDiffuse(RageColor(0.9,0,0,alpha));
		}else
		{
			m_textUsers[i].SetDiffuse(RageColor(0.3,0.3,0.3,alpha));
			m_textUsersNum[i].SetDiffuse(RageColor(0.3,0.3,0.3,alpha));
		}
		
		CString Num;
		Num.Format("%d", i);
		CString Display_Num;
		if(i==0)
		{
			Display_Num="HOST";
		}else
		{
			Display_Num+=Num;
			Display_Num+=".";
		}
		m_textUsersNum[i].SetText( Display_Num );
		//=========
		CString temp_PlayerName="";
		if(NSMAN->m_PlayerNames[i*2]!=""
		 &&NSMAN->m_PlayerNames[i*2+1]=="")
		{
			temp_PlayerName=NSMAN->m_PlayerNames[i*2];
		}
		else if(NSMAN->m_PlayerNames[i*2]==""
		 &&NSMAN->m_PlayerNames[i*2+1]!="")
		{
			temp_PlayerName=NSMAN->m_PlayerNames[i*2+1];
		}
		else if(NSMAN->m_PlayerNames[i*2]!=""
		 &&NSMAN->m_PlayerNames[i*2+1]!="")
		{
			temp_PlayerName=NSMAN->m_PlayerNames[i*2];
			temp_PlayerName+="&";
			temp_PlayerName+=NSMAN->m_PlayerNames[i*2+1];
		}
		m_textUsers[i].SetText( temp_PlayerName );
		// m_textUsers[i].SetDiffuse(RageColor(r,g,b,a));
		
		// if ( NSMAN->m_EvalPlayerData[i].grade < GRADE_TIER_3 )	//Yes, hardcoded (I'd like to leave it that way)
		// 	m_textUsers[i].TurnRainbowOn();
		// else
		// 	m_textUsers[i].TurnRainbowOff();
		// ON_COMMAND( m_textUsers[i] );
	}
}
void ScreenNetSelectMusic::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenNetSelectMusic::UpdateTextInput()
{
	// m_textChatInput.SetText2( m_sTextInput );
	m_textChatInput.SetText( m_sTextInput );  
}

void ScreenNetSelectMusic::StartSelectedSong()
{
	Song * pSong = m_vSongs[m_iSongNum%m_vSongs.size()];
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType; //STEPS_TYPE_DANCE_SINGLE;
	FOREACH_EnabledPlayer (pn)
	{
		GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		Steps * pSteps = pSong->GetStepsByDifficulty(st,m_DC[pn]);
		GAMESTATE->m_pCurSteps[pn] = pSteps;
	}
	
	TweenOffScreen();
	StartTransitioning( SM_GoToNextScreen );
}

void ScreenNetSelectMusic::UpdateDifficulties( PlayerNumber pn )
{
	if ( ( m_DC[pn] < DIFFICULTY_EDIT ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyIcon[pn].SetFromDifficulty( pn, m_DC[pn] );
	else
		m_DifficultyIcon[pn].SetFromSteps( pn, NULL );	//It will blank it out 

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

	if ( ( m_DC[pn] < NUM_DIFFICULTIES ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyMeters[pn].SetFromSteps( GAMESTATE->m_pCurSong->GetStepsByDifficulty( st, m_DC[pn] ) );
	else
		m_DifficultyMeters[pn].SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER ); 
}

void ScreenNetSelectMusic::UpdateSongsListPos()
{
	int i,j;
	if (m_iSongNum<m_iShowSongs)
		m_iSongNum+=m_vSongs.size();
	CString SongsDisplay="";
	for (i=m_iSongNum-m_iShowSongs; i<=m_iSongNum+m_iShowSongs; ++i)
	{
		j= i % m_vSongs.size();
		SongsDisplay+=m_vSongs.at(j)->GetDisplayMainTitle();
		if (i<m_iSongNum+m_iShowSongs)
			SongsDisplay+='\n';
	}
	m_textSongs.SetText( SongsDisplay );

	j= m_iSongNum % m_vSongs.size();

	m_textArtist.SetText( m_vSongs[j]->GetDisplayArtist() );
	m_textSubtitle.SetText( m_vSongs[j]->GetDisplaySubTitle() );
	GAMESTATE->m_pCurSong = m_vSongs[j];

	//Update the difficulty Icons
	//Handle difficulty
	FOREACH_EnabledPlayer (pn)
	{
		m_DC[pn] = GAMESTATE->m_PreferredDifficulty[pn];
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		vector <Steps *> MultiSteps;
		MultiSteps = m_vSongs[j]->GetAllSteps( st );
		
		MultiSteps = SortStep(MultiSteps);

		if (MultiSteps.size() == 0)
			m_DC[pn] = NUM_DIFFICULTIES;
		else
		{
			for ( i=0; i<(int)MultiSteps.size(); ++i )
				if ( MultiSteps[i]->GetDifficulty() >= m_DC[pn] )
					break;

			if ( i == (int)MultiSteps.size() )
				m_DC[pn] = MultiSteps[i-1]->GetDifficulty();
			else
				m_DC[pn] = MultiSteps[i]->GetDifficulty();
		}
		UpdateDifficulties( pn );
	}

	//Then handle sound (Copied from MusicBannerWheel)
	static Song* pre_Song;
	static time_t now = time(0);
	if(pre_Song == m_vSongs[j] && time(0)-now<2)
	{
	}else
	{
		SOUND->StopMusic();
		Song* pSong = m_vSongs[j];
		if( pSong  &&  pSong->HasMusic() )
		{
			SOUND->StopMusic();
			// SOUND->PlayMusic(pSong->GetMusicPath(), true,
			// 	pSong->m_fMusicSampleStartSeconds,
			// 	pSong->m_fMusicSampleLengthSeconds
			// 	);
			SOUND->PlayMusic2(pSong->GetMusicPath(), true,
				pSong->m_fMusicSampleStartSeconds,
				pSong->m_fMusicLengthSeconds-pSong->m_fMusicSampleStartSeconds,
				pSong->m_fMusicLengthSeconds,
				true);
		}
	}
	now = time(0);
	pre_Song = m_vSongs[j];

}

void ScreenNetSelectMusic::UpdateGroupsListPos()
{
	int i,j;
	if (m_iGroupNum<m_iShowGroups)
		m_iGroupNum+=m_vGroups.size();
	CString GroupsDisplay="";
	for (i=m_iGroupNum-m_iShowGroups; i<=m_iGroupNum+m_iShowGroups; ++i)
	{
		j=i%m_vGroups.size();
		GroupsDisplay+=m_vGroups[j];
		if (i<m_iGroupNum+m_iShowGroups)
			GroupsDisplay+='\n';
	}	
	m_textGroups.SetText( GroupsDisplay );
}

void ScreenNetSelectMusic::UpdateSongsList()
{
	m_vSongs.clear();
	SONGMAN->SortSongs();
	if (m_iGroupNum<m_iShowGroups)
		m_iGroupNum+=m_vGroups.size();
	int j=m_iGroupNum%m_vGroups.size();
	if ( m_vGroups[j]==AllGroups )
		SONGMAN->GetSongs( m_vSongs );
	else
		SONGMAN->GetSongs( m_vSongs, m_vGroups[j] );
	//SongUtil::SortSongPointerArrayByTitle( m_vSongs );
}

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *		Based off of ScreenEz2SelectMusic by Frieza
 *      Elements from ScreenTextEntry
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
