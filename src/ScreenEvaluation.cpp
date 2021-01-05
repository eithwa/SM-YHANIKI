#include "global.h"
#include "ScreenEvaluation.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "Steps.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GrooveRadar.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "ActorUtil.h"
#include "RageTimer.h"
#include "UnlockSystem.h"
#include "Course.h"
#include "LightsManager.h"
#include "ProfileManager.h"
#include "song.h"
#include "StageStats.h"
#include "Grade.h"
#include "CodeDetector.h"
#include "RageDisplay.h"
#include "StepMania.h"
#include "CryptManager.h"
#include "Style.h"
#include "NetworkSyncManager.h"

const int NUM_SCORE_DIGITS	=	9;


// metrics that are common to all ScreenEvaluation classes
#define BANNER_WIDTH						THEME->GetMetricF(m_sName,"BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF(m_sName,"BannerHeight")
const char* JUDGE_STRING[NUM_JUDGE_LINES] =
{
	"Marvelous", "Perfect", "Great", "Good", "Boo", "Miss", "OK", "MaxCombo", "TotalError"
};
const char* STATS_STRING[NUM_STATS_LINES] =
{
	"Jumps", "Holds", "Mines", "Hands"
};

#define SPIN_GRADES							THEME->GetMetricB(m_sName,"SpinGrades")
#define CHEER_DELAY_SECONDS					THEME->GetMetricF(m_sName,"CheerDelaySeconds")
#define BAR_ACTUAL_MAX_COMMAND				THEME->GetMetric (m_sName,"BarActualMaxCommand")

// metrics that are specific to classes derived from ScreenEvaluation
#define FAILED_SCREEN						THEME->GetMetric (m_sName, "FailedScreen")
#define NEXT_SCREEN							THEME->GetMetric (m_sName,"NextScreen")
#define END_SCREEN							THEME->GetMetric (m_sName,"EndScreen")
#define SHOW_BANNER_AREA					THEME->GetMetricB(m_sName,"ShowBannerArea")
#define SHOW_GRADE_AREA						THEME->GetMetricB(m_sName,"ShowGradeArea")
#define SHOW_POINTS_AREA					THEME->GetMetricB(m_sName,"ShowPointsArea")
#define SHOW_BONUS_AREA						THEME->GetMetricB(m_sName,"ShowBonusArea")
#define SHOW_SURVIVED_AREA					THEME->GetMetricB(m_sName,"ShowSurvivedArea")
#define SHOW_WIN_AREA						THEME->GetMetricB(m_sName,"ShowWinArea")
#define SHOW_JUDGMENT( l )					THEME->GetMetricB(m_sName,ssprintf("Show%s",JUDGE_STRING[l]))
#define SHOW_STAT( s )						THEME->GetMetricB(m_sName,ssprintf("Show%s",STATS_STRING[l]))
#define SHOW_SCORE_AREA						THEME->GetMetricB(m_sName,"ShowScoreArea")
#define SHOW_TOTAL_SCORE_AREA				THEME->GetMetricB(m_sName,"ShowTotalScoreArea")
#define SHOW_TIME_AREA						THEME->GetMetricB(m_sName,"ShowTimeArea")
#define SHOW_GRAPH_AREA						THEME->GetMetricB(m_sName,"ShowGraphArea")
#define SHOW_COMBO_AREA						THEME->GetMetricB(m_sName,"ShowComboArea")
#define SHOW_PER_DIFFICULTY_AWARD			THEME->GetMetricB(m_sName,"ShowPerDifficultyAward")
#define SHOW_PEAK_COMBO_AWARD				THEME->GetMetricB(m_sName,"ShowPeakComboAward")
#define GRAPH_START_HEIGHT					THEME->GetMetricF(m_sName,"GraphStartHeight")
#define TYPE								THEME->GetMetric (m_sName,"Type")
#define PASSED_SOUND_TIME					THEME->GetMetricF(m_sName,"PassedSoundTime")
#define FAILED_SOUND_TIME					THEME->GetMetricF(m_sName,"FailedSoundTime")
#define NUM_SEQUENCE_SOUNDS					THEME->GetMetricI(m_sName,"NumSequenceSounds")
#define SOUNDSEQ_TIME( i )					THEME->GetMetricF(m_sName,ssprintf("SoundSeqTime%d", i+1))
#define SOUNDSEQ_NAME( i )					THEME->GetMetric (m_sName,ssprintf("SoundSeqName%d", i+1))
#define MAX_COMBO_NUM_DIGITS				THEME->GetMetricI(m_sName,"MaxComboNumDigits")
#define PLAYER_OPTIONS_SEPARATOR			THEME->GetMetric (m_sName,"PlayerOptionsSeparator")


static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

const ScreenMessage SM_PlayCheer				=	ScreenMessage(SM_User+6);


ScreenEvaluation::ScreenEvaluation( CString sClassName ) : ScreenWithMenuElements(sClassName)
{
	LOG->Trace( "ScreenEvaluation::ScreenEvaluation" );
	Init(); // work around horrible gcc bug 3187
	LOG->Trace( "exiting ScreenEvaluation::ScreenEvalaution" );
}

void ScreenEvaluation::Init()
{
	//
	// debugging
	//
	
	if( PREFSMAN->m_bScreenTestMode )
	{
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_1);
		PROFILEMAN->LoadFirstAvailableProfile(PLAYER_2);

		GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;
		GAMESTATE->m_pCurStyle = GAMEMAN->GameAndStringToStyle( GAMEMAN->GetDefaultGame(), "versus" );
		GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;
		GAMESTATE->m_bSideIsJoined[PLAYER_2] = true;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong = SONGMAN->GetRandomSong();
		GAMESTATE->m_pCurCourse = SONGMAN->GetRandomCourse();
		GAMESTATE->m_pCurSteps[PLAYER_1] = GAMESTATE->m_pCurSong->GetAllSteps()[0];
		GAMESTATE->m_pCurSteps[PLAYER_2] = GAMESTATE->m_pCurSong->GetAllSteps()[0];
		g_CurStageStats.vpSteps[PLAYER_1].push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		g_CurStageStats.vpSteps[PLAYER_2].push_back( GAMESTATE->m_pCurSteps[PLAYER_2] );
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 2;
		GAMESTATE->m_PlayerOptions[PLAYER_2].m_fScrollSpeed = 2;
		GAMESTATE->m_iCurrentStageIndex = 0;
		GAMESTATE->m_PlayerOptions[PLAYER_1].ChooseRandomMofifiers();
		GAMESTATE->m_PlayerOptions[PLAYER_2].ChooseRandomMofifiers();

		for( float f = 0; f < 100.0f; f += 1.0f )
		{
			float fP1 = fmodf(f/100*4+.3f,1);
			g_CurStageStats.SetLifeRecordAt( PLAYER_1, fP1, f );
			g_CurStageStats.SetLifeRecordAt( PLAYER_2, 1-fP1, f );
		}
	
		g_CurStageStats.iActualDancePoints[PLAYER_1] = rand()%3;
		g_CurStageStats.iPossibleDancePoints[PLAYER_1] = 2;
		g_CurStageStats.iActualDancePoints[PLAYER_2] = rand()%2;
		g_CurStageStats.iPossibleDancePoints[PLAYER_2] = 1;
		g_CurStageStats.iCurCombo[PLAYER_1] = 0;
		g_CurStageStats.UpdateComboList( PLAYER_1, 0, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 1;
		g_CurStageStats.UpdateComboList( PLAYER_1, 1, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 50;
		g_CurStageStats.UpdateComboList( PLAYER_1, 25, false );
		g_CurStageStats.iCurCombo[PLAYER_1] = 250;
		g_CurStageStats.UpdateComboList( PLAYER_1, 100, false );
		if( rand()%2 )
		{
			g_CurStageStats.iCurCombo[PLAYER_1] = rand()%11000;
			g_CurStageStats.UpdateComboList( PLAYER_1, 110, false );
		}
		g_CurStageStats.iCurCombo[PLAYER_2] = 0;
		g_CurStageStats.UpdateComboList( PLAYER_2, 0, false );
		g_CurStageStats.iCurCombo[PLAYER_2] = 1;
		g_CurStageStats.UpdateComboList( PLAYER_2, 1, false );
		g_CurStageStats.iCurCombo[PLAYER_2] = 50;
		g_CurStageStats.UpdateComboList( PLAYER_2, 25, false );
		g_CurStageStats.iCurCombo[PLAYER_2] = 250;
		g_CurStageStats.UpdateComboList( PLAYER_2, 100, false );
		if( rand()%2 )
		{
			g_CurStageStats.iCurCombo[PLAYER_2] = rand()%11000;
			g_CurStageStats.UpdateComboList( PLAYER_2, 110, false );
		}

		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_MARVELOUS] = rand()%3;
		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_PERFECT] = rand()%3;
		g_CurStageStats.iTapNoteScores[PLAYER_1][TNS_GREAT] = rand()%3;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_MARVELOUS] = rand()%3;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_PERFECT] = rand()%3;
		g_CurStageStats.iTapNoteScores[PLAYER_2][TNS_GREAT] = rand()%3;

		g_vPlayedStageStats.clear();
	}

	LOG->Trace( "ScreenEvaluation::ScreenEvaluation()" );

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );

	m_bFailed = g_CurStageStats.AllFailed();

	ZERO( m_bSavedScreenshot );


	CString sType = TYPE;
	if( !sType.CompareNoCase("stage") )
		m_Type = stage;
	else if( !sType.CompareNoCase("summary") )
		m_Type = summary;
	else if( !sType.CompareNoCase("course") )
		m_Type = course;
	else
		RageException::Throw("Unknown evaluation type \"%s\"", TYPE.c_str() );
		

	//
	// Figure out which statistics and songs we're going to display
	//
	StageStats stageStats;
	switch( m_Type )
	{
	case summary:
		GAMESTATE->GetFinalEvalStats( stageStats );
		break;
	case stage:
	case course:
		stageStats = g_CurStageStats;
		break;
	default:
		ASSERT(0);
	}

	LOG->Trace( "total error: %i, %i", stageStats.iTotalError[0], stageStats.iTotalError[1] );

/*
	//
	// Debugging
	//
	{
		FOREACH_PlayerNumber( p )	// foreach line
			for( int r=0; r<NUM_RADAR_CATEGORIES; r++ )	// foreach line
			{
				stageStats.radarPossible[p][r] = 0.5f + r/10.0f;
				stageStats.radarActual[p][r] = 0.5f + r/10.0f;
			}
	}
*/

	//
	// Calculate grades
	//
	Grade grade[NUM_PLAYERS];

	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsPlayerEnabled(p) )
			grade[p] = stageStats.GetGrade( p );
		else
			grade[p] = GRADE_FAILED;

		if( PREFSMAN->m_iScoringType == PrefsManager::SCORING_5TH )
		{
			const int ScoreBonuses[] = { 10000000, 10000000, 1000000, 100000, 10000, 1000, 100 };
			if( grade[p] < (int) RageARRAYSIZE(ScoreBonuses) )
			{
				g_CurStageStats.iBonus[p] += ScoreBonuses[(int)grade[p] ];
				stageStats.iBonus[p] += ScoreBonuses[(int)grade[p] ];
			}
		}
	}


	//
	// update persistent statistics
	//
	int iPersonalHighScoreIndex[NUM_PLAYERS];
	int iMachineHighScoreIndex[NUM_PLAYERS];
	RankingCategory rc[NUM_PLAYERS];;
	PerDifficultyAward pdaToShow[NUM_PLAYERS];
	PeakComboAward pcaToShow[NUM_PLAYERS];
	CommitScores( stageStats, iPersonalHighScoreIndex, iMachineHighScoreIndex, rc, pdaToShow, pcaToShow );

	m_bTryExtraStage = 
		GAMESTATE->HasEarnedExtraStage()  && 
		m_Type==stage;
 

	//
	// load pass/fail sound
	//
//	if(m_Type==stage)
//	{
		int snd=0;
		for(snd=0;snd<NUM_SEQUENCE_SOUNDS;snd++) // grab in any sound sequence the user may want to throw onto this screen
		{
			EvalSoundSequence temp;
			temp.fTime = SOUNDSEQ_TIME(snd);
			temp.sSound.Load(THEME->GetPathToS(SOUNDSEQ_NAME(snd),false));
			m_SoundSequences.push_back(temp);
		}
//	}
	
	m_bgOverlay.LoadFromAniDir( THEME->GetPathB(m_sName, "overlay") );
	m_bgOverlay.SetDrawOrder( 1 ); /* draw on top of everything except transitions */
	this->AddChild( &m_bgOverlay );

	m_bPassFailTriggered = false; // the sound hasn't been triggered yet
	if( m_bFailed )
 		m_sndPassFail.Load( THEME->GetPathS(m_sName, "failed") );
	else
		m_sndPassFail.Load( THEME->GetPathS(m_sName, "passed") );
	m_sndPassFail.Play();

	m_bgCondBga.Load(m_sName);

	//
	// load other sounds
	//
	m_soundStart.Load( THEME->GetPathToS("ScreenEvaluation start") );

	
	//
	// init player name area
	//
	FOREACH_EnabledPlayer( p )
	{
		m_textPlayerName[p].LoadFromFont( THEME->GetPathF(m_sName,"PlayerName") );
		m_textPlayerName[p].SetName( ssprintf("PlayerNameP%d",p+1) );
		m_textPlayerName[p].SetText( GAMESTATE->GetPlayerDisplayName(p) );
		SET_XY_AND_ON_COMMAND( m_textPlayerName[p] );
		this->AddChild( &m_textPlayerName[p] );
	}

	//
	// init banner area
	//
	if( SHOW_BANNER_AREA )
	{
		switch( m_Type )
		{
		case stage:
			{
				m_LargeBanner.LoadFromSong( GAMESTATE->m_pCurSong );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame->SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( m_sprLargeBannerFrame );

				m_sprStage.Load( THEME->GetPathToG("ScreenEvaluation stage "+GAMESTATE->GetStageText()) );
				m_sprStage->SetName( "Stage" );
				SET_XY_AND_ON_COMMAND( m_sprStage );
				this->AddChild( m_sprStage );
			}
			break;
		case summary:
			{
				for( unsigned i=0; i<stageStats.vpSongs.size(); i++ )
				{
					Song *pSong = stageStats.vpSongs[i];

					m_SmallBanner[i].LoadFromSong( pSong );
					m_SmallBanner[i].ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
					m_SmallBanner[i].SetName( ssprintf("SmallBanner%d",i+1) );
					SET_XY_AND_ON_COMMAND( m_SmallBanner[i] );
					this->AddChild( &m_SmallBanner[i] );

					m_sprSmallBannerFrame[i].Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
					m_sprSmallBannerFrame[i]->SetName( ssprintf("SmallBanner%d",i+1) );
					SET_XY_AND_ON_COMMAND( m_sprSmallBannerFrame[i] );
					this->AddChild( m_sprSmallBannerFrame[i] );
				}
			}
			break;
		case course:
			{
				m_LargeBanner.LoadFromCourse( GAMESTATE->m_pCurCourse );
				m_LargeBanner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
				m_LargeBanner.SetName( "LargeBanner" );
				SET_XY_AND_ON_COMMAND( m_LargeBanner );
				this->AddChild( &m_LargeBanner );

				m_sprLargeBannerFrame.Load( THEME->GetPathToG("ScreenEvaluation banner frame") );
				m_sprLargeBannerFrame->SetName( "LargeBannerFrame" );
				SET_XY_AND_ON_COMMAND( m_sprLargeBannerFrame );
				this->AddChild( m_sprLargeBannerFrame );
			}
			break;
		default:
			ASSERT(0);
		}


		switch( m_Type )
		{
		case stage:
		case course:
			{
				FOREACH_EnabledPlayer( p )
				{
					m_DifficultyIcon[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation difficulty icons")) );
					switch( m_Type )
					{
					case stage:
						m_DifficultyIcon[p].SetFromSteps( p, GAMESTATE->m_pCurSteps[p] );
						break;
					case course:
						m_DifficultyIcon[p].SetFromTrail( p, GAMESTATE->m_pCurTrail[p] );
						break;
					default:
						ASSERT(0);
					}
					m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_DifficultyIcon[p] );
					this->AddChild( &m_DifficultyIcon[p] );
					
					m_DifficultyMeter[p].SetName( ssprintf("ScreenEvaluation DifficultyMeterP%d",p+1) );
					m_DifficultyMeter[p].Load();
					switch( m_Type )
					{
					case stage:
						m_DifficultyMeter[p].SetFromSteps( GAMESTATE->m_pCurSteps[p] );
						break;
					case course:
						m_DifficultyMeter[p].SetFromTrail( GAMESTATE->m_pCurTrail[p] );
						break;
					default:
						ASSERT(0);
					}
					m_DifficultyMeter[p].SetName( ssprintf("DifficultyMeterP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_DifficultyMeter[p] );
					this->AddChild( &m_DifficultyMeter[p] );
					
					m_textPlayerOptions[p].LoadFromFont( THEME->GetPathToF("Common normal") );
					CString sPO;
					// if(GAMESTATE->m_SongOptions.m_fMusicRate!=1)
					// {
					// 	CString s = ssprintf( "%2.2f", GAMESTATE->m_SongOptions.m_fMusicRate );
					// 	if( s[s.GetLength()-1] == '0' )
					// 		s.erase(s.GetLength()-1);
					// 	sPO += s + "xMusic ";
					// }
					sPO += GAMESTATE->m_PlayerOptions[p].GetThemedString();
					sPO.Replace( ", ", PLAYER_OPTIONS_SEPARATOR );
					m_textPlayerOptions[p].SetName( ssprintf("PlayerOptionsP%d",p+1) );
					SET_XY_AND_ON_COMMAND( m_textPlayerOptions[p] );
					m_textPlayerOptions[p].SetText( sPO );
					this->AddChild( &m_textPlayerOptions[p] );

					if( GAMESTATE->IsDisqualified((PlayerNumber)p) )
					{
						m_sprDisqualified[p].Load( THEME->GetPathToG("ScreenEvaluation disqualified") );
						m_sprDisqualified[p]->SetName( ssprintf("DisqualifiedP%d",p+1) );
						SET_XY_AND_ON_COMMAND( m_sprDisqualified[p] );
						this->AddChild( m_sprDisqualified[p] );
					}
				}
			}
			break;
		case summary:
			break;
		default:
			ASSERT(0);
		}
	}

	//
	// init graph area
	//
	if( SHOW_GRAPH_AREA )
	{
		enum { VALUE_RESOLUTION=100 };
		FOREACH_EnabledPlayer( p )
		{
			m_sprGraphFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation graph frame p%d",p+1)) );
			m_sprGraphFrame[p]->SetName( ssprintf("GraphFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGraphFrame[p] );
			this->AddChild( m_sprGraphFrame[p] );

			m_Graph[p].SetName( ssprintf("GraphP%i",p+1) );
			m_Graph[p].Load( THEME->GetPathToG(ssprintf("%s graph p%i",m_sName.c_str(), p+1)), GRAPH_START_HEIGHT );
			//=========
			m_Graph[p].LoadFromStageStats( stageStats, (PlayerNumber)p );
			for(int i=0; i<VALUE_RESOLUTION; i++)
			{
				GAMESTATE->m_PlayerGraph[p][i]=m_Graph[p].Get_m_DestValues(i)*10000;
			}
			//=========
			SET_XY_AND_ON_COMMAND( m_Graph[p] );
			this->AddChild( &m_Graph[p] );
		}
		NSMAN->ReportGraph();
	}

	if( SHOW_COMBO_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_Combo[p].SetName( m_sName, ssprintf("ComboP%i",p+1) );
			m_Combo[p].Load( ssprintf("ScreenEvaluationStage combo p%i", p+1), stageStats, (PlayerNumber)p );
			SET_XY_AND_ON_COMMAND( m_Combo[p] );

			this->AddChild( &m_Combo[p] );
		}
	}

	//
	// init grade area
	//
	if( SHOW_GRADE_AREA )
	{
		// Touch all possible grade files to make sure they're all present
		for( int g=GRADE_TIER_1; g<NUM_GRADES; g++ )
			THEME->GetPathToG( "ScreenEvaluation grade "+ GradeToString((Grade)g) );

		
		FOREACH_EnabledPlayer( p )
		{
			m_sprGradeFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation grade frame p%d",p+1)) );
			m_sprGradeFrame[p]->SetName( ssprintf("GradeFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprGradeFrame[p] );
			this->AddChild( m_sprGradeFrame[p] );

			m_Grades[p].Load( THEME->GetPathToG("ScreenEvaluation grades") );
			m_Grades[p].SetGrade( (PlayerNumber)p, grade[p] );
			m_Grades[p].SetName( ssprintf("GradeP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_Grades[p] );
			if( SPIN_GRADES )
				m_Grades[p].Spin();
			this->AddChild( &m_Grades[p] );

			m_sprGrade[p].Load( THEME->GetPathToG( "ScreenEvaluation grade "+ GradeToString(grade[p]) ) );
			m_sprGrade[p]->SetName( ssprintf("GradeP%d",p+1) );
			
			SET_XY_AND_ON_COMMAND( m_sprGrade[p] );
			this->AddChild( m_sprGrade[p] );
		}
	}

	//
	// init points area
	//
	if( SHOW_POINTS_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprPercentFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation percent frame p%d",p+1)) );
			m_sprPercentFrame[p]->SetName( ssprintf("PercentFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprPercentFrame[p] );
			this->AddChild( m_sprPercentFrame[p] );

			/* Use "ScreenEvaluation Percent" for the [metric set], but position and
			 * tween it with "PercentP1X", etc. */
			m_Percent[p].SetName( "ScreenEvaluation Percent" );
			m_Percent[p].Load( (PlayerNumber) p, &g_CurStageStats, true );
			m_Percent[p].SetXY( THEME->GetMetricF(m_sName, ssprintf("PercentP%dX",p+1)),
				THEME->GetMetricF(m_sName,ssprintf("PercentP%dY",p+1)) );
			m_Percent[p].Command( THEME->GetMetric(m_sName,ssprintf("PercentP%dOnCommand",p+1)) );
			// m_Percent[p].m_textPercent_sNumToDisplay;
			GAMESTATE->m_PlayerPercentage[p] = m_Percent[p].m_textPercent_sNumToDisplay;
			this->AddChild( &m_Percent[p] );
		}
		NSMAN->ReportPercentage();
	}

	//
	// init bonus area
	//
	if( SHOW_BONUS_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprBonusFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bonus frame p%d",p+1)) );
			m_sprBonusFrame[p]->SetName( ssprintf("BonusFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprBonusFrame[p] );
			this->AddChild( m_sprBonusFrame[p] );

			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				m_sprPossibleBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar possible p%d",p+1)) );
				m_sprPossibleBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.radarPossible[p][r] );
				m_sprPossibleBar[p][r].SetName( ssprintf("BarPossible%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprPossibleBar[p][r] );
				this->AddChild( &m_sprPossibleBar[p][r] );

				m_sprActualBar[p][r].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation bar actual p%d",p+1)) );
				// should be out of the possible bar, not actual (whatever value that is at)
				m_sprActualBar[p][r].SetWidth( m_sprPossibleBar[p][r].GetUnzoomedWidth() * stageStats.radarActual[p][r] );
				
				float value = (float)100 * m_sprActualBar[p][r].GetUnzoomedWidth() / m_sprPossibleBar[p][r].GetUnzoomedWidth();
				LOG->Trace("Radar bar %d of 5 - %f percent", r,  value);
				
				m_sprActualBar[p][r].SetName( ssprintf("BarActual%dP%d",r+1,p+1) );
				SET_XY_AND_ON_COMMAND( m_sprActualBar[p][r] );
				
				// .99999 is fairly close to 1.00, so we use that 
				if( stageStats.radarActual[p][r] > 0.99999f )
					m_sprActualBar[p][r].Command( BAR_ACTUAL_MAX_COMMAND );
				this->AddChild( &m_sprActualBar[p][r] );
			}
		}
	}

	//
	// init survived area
	//
	if( SHOW_SURVIVED_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprSurvivedFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenEvaluation survived frame p%d",p+1)) );
			m_sprSurvivedFrame[p]->SetName( ssprintf("SurvivedFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprSurvivedFrame[p] );
			this->AddChild( m_sprSurvivedFrame[p] );

			m_textSurvivedNumber[p].LoadFromFont( THEME->GetPathF(m_sName, "stage") );
			m_textSurvivedNumber[p].SetShadowLength( 0 );
			// curewater: edited the "# stages cleared" text so it deducts one if you failed.
			// Should be accurate, but I'm not sure if its "standard" that (bool)true = 1.  (assumption)
			m_textSurvivedNumber[p].SetText( ssprintf("%02d", stageStats.iSongsPlayed[p] - (int)stageStats.bFailed[p]) );
			m_textSurvivedNumber[p].SetName( ssprintf("SurvivedNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textSurvivedNumber[p] );
			this->AddChild( &m_textSurvivedNumber[p] );
		}
	}
	
	//
	// init win area
	//
	if( SHOW_WIN_AREA )
	{
		FOREACH_EnabledPlayer( p )
		{
			m_sprWinFrame[p].Load( THEME->GetPathG(m_sName,ssprintf("win frame p%d",p+1)) );
			m_sprWinFrame[p]->SetName( ssprintf("WinFrameP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprWinFrame[p] );
			this->AddChild( m_sprWinFrame[p] );

			m_sprWin[p].Load( THEME->GetPathG(m_sName,ssprintf("win p%d 1x3",p+1)) );
			m_sprWin[p].StopAnimating();
			int iFrame = GAMESTATE->GetStageResult( p );
			m_sprWin[p].SetState( iFrame );
			m_sprWin[p].SetName( ssprintf("WinP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_sprWin[p] );
			this->AddChild( &m_sprWin[p] );
		}
	}
	
	//
	// init judgment area
	//
	int l;
	for( l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( l == 0  && !GAMESTATE->ShowMarvelous() )
			continue;	// skip

		if( SHOW_JUDGMENT(l) )
		{
			m_sprJudgeLabels[l].Load( THEME->GetPathToG( "ScreenEvaluation judge labels" ) );
			m_sprJudgeLabels[l].StopAnimating();
			m_sprJudgeLabels[l].SetState( l );
			m_sprJudgeLabels[l].SetName( ssprintf("%sLabel",JUDGE_STRING[l]) );
			SET_XY_AND_ON_COMMAND( m_sprJudgeLabels[l] );
			this->AddChild( &m_sprJudgeLabels[l] );

			FOREACH_EnabledPlayer( p )
			{
				m_textJudgeNumbers[l][p].LoadFromFont( THEME->GetPathF(m_sName, "judge") );
				m_textJudgeNumbers[l][p].SetShadowLength( 0 );
				m_textJudgeNumbers[l][p].SetDiffuse( PlayerToColor(p) );
				m_textJudgeNumbers[l][p].SetName( ssprintf("%sNumberP%d",JUDGE_STRING[l],p+1) );
				SET_XY_AND_ON_COMMAND( m_textJudgeNumbers[l][p] );
				this->AddChild( &m_textJudgeNumbers[l][p] );

				int iValue;
				switch( l )
				{
				case marvelous:	iValue = stageStats.iTapNoteScores[p][TNS_MARVELOUS];	break;
				case perfect:	iValue = stageStats.iTapNoteScores[p][TNS_PERFECT];		break;
				case great:		iValue = stageStats.iTapNoteScores[p][TNS_GREAT];		break;
				case good:		iValue = stageStats.iTapNoteScores[p][TNS_GOOD];		break;
				case boo:		iValue = stageStats.iTapNoteScores[p][TNS_BOO];			break;
				case miss:		iValue = stageStats.iTapNoteScores[p][TNS_MISS];		break;
				case ok:		iValue = stageStats.iHoldNoteScores[p][HNS_OK];			break;
				case max_combo:	iValue = stageStats.GetMaxCombo(p).cnt;					break;
				case error:		iValue = stageStats.iTotalError[p];						break;
				default:	iValue = 0;	ASSERT(0);
				}

				// UGLY... generalize this
				int iNumDigits = (l==max_combo) ? MAX_COMBO_NUM_DIGITS : 4;
				m_textJudgeNumbers[l][p].SetText( ssprintf("%*d",iNumDigits,iValue) );
			}
		}
	}

	for( l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		m_sprStatsLabel[l].Load( THEME->GetPathToG( ssprintf("ScreenEvaluation label %s", STATS_STRING[l]) ) );
		m_sprStatsLabel[l]->StopAnimating();
		m_sprStatsLabel[l]->SetName( ssprintf("%sLabel",STATS_STRING[l]) );
		SET_XY_AND_ON_COMMAND( m_sprStatsLabel[l] );
		this->AddChild( m_sprStatsLabel[l] );

		FOREACH_EnabledPlayer( p )
		{
			m_textStatsText[l][p].LoadFromFont( THEME->GetPathToF("ScreenEvaluation stats") );
			m_textStatsText[l][p].SetDiffuse( PlayerToColor(p) );
			m_textStatsText[l][p].SetName( ssprintf("%sTextP%d",STATS_STRING[l],p+1) );
			SET_XY_AND_ON_COMMAND( m_textStatsText[l][p] );
			this->AddChild( &m_textStatsText[l][p] );

			const int indeces[NUM_STATS_LINES] =
			{
				RADAR_NUM_JUMPS, RADAR_NUM_HOLDS, RADAR_NUM_MINES, RADAR_NUM_HANDS
			};
			const int ind = indeces[l];
			const int iActual = (int) roundf(stageStats.radarActual[p][ind]);
			const int iPossible = (int) roundf(stageStats.radarPossible[p][ind]);

			m_textStatsText[l][p].SetText( ssprintf("%3d/%3d",iActual,iPossible) );
		}
	}

	//
	// init score area
	//
	if( SHOW_SCORE_AREA )
	{
		m_sprScoreLabel.Load( THEME->GetPathToG("ScreenEvaluation score label") );
		m_sprScoreLabel->SetName( "ScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprScoreLabel );
		this->AddChild( m_sprScoreLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textScore[p].LoadFromFont( THEME->GetPathF(m_sName, "score") );
			m_textScore[p].SetShadowLength( 0 );
			m_textScore[p].SetDiffuse( PlayerToColor(p) );
			m_textScore[p].SetName( ssprintf("ScoreNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textScore[p] );
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, stageStats.iScore[p]) );
			this->AddChild( &m_textScore[p] );
		}
	}

	if( SHOW_TOTAL_SCORE_AREA )
	{
		m_sprTotalScoreLabel.Load( THEME->GetPathToG("ScreenEvaluation totalscore label") );
		m_sprTotalScoreLabel->SetName( "TotalScoreLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTotalScoreLabel );
		this->AddChild( m_sprTotalScoreLabel );

		FOREACH_EnabledPlayer( p )
		{
			int iTotalScore=0;
			for( unsigned i=0; i<g_vPlayedStageStats.size(); i++ )
				iTotalScore += g_vPlayedStageStats[i].iScore[p];

			//iTotalScore += stageStats.iScore[p];

			m_textTotalScore[p].LoadFromFont( THEME->GetPathF(m_sName, "totalscore") );
			m_textTotalScore[p].SetShadowLength( 0 );
			m_textTotalScore[p].SetDiffuse( PlayerToColor(p) );
			m_textTotalScore[p].SetName( ssprintf("TotalScoreNumberP%d",p+1) );
			m_textTotalScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS+2, iTotalScore) );
			SET_XY_AND_ON_COMMAND( m_textTotalScore[p] );

			this->AddChild( &m_textTotalScore[p] );
		}
	}

	//
	// init time area
	//
	if( SHOW_TIME_AREA )
	{
		m_sprTimeLabel.Load( THEME->GetPathToG("ScreenEvaluation time label") );
		m_sprTimeLabel->SetName( "TimeLabel" );
		SET_XY_AND_ON_COMMAND( m_sprTimeLabel );
		this->AddChild( m_sprTimeLabel );

		FOREACH_EnabledPlayer( p )
		{
			m_textTime[p].LoadFromFont( THEME->GetPathF(m_sName, "time") );
			m_textTime[p].SetShadowLength( 0 );
			m_textTime[p].SetDiffuse( PlayerToColor(p) );
			m_textTime[p].SetName( ssprintf("TimeNumberP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_textTime[p] );
			m_textTime[p].SetText( SecondsToMMSSMsMs(stageStats.fAliveSeconds[p]) );
			this->AddChild( &m_textTime[p] );
		}
	}


	//
	// init extra area
	//
	FOREACH_PlayerNumber( p )
	{
		if( iMachineHighScoreIndex[p] != -1 )
		{
			m_sprMachineRecord[p].Load( THEME->GetPathG( "ScreenEvaluation", ssprintf("MachineRecord %02d",iMachineHighScoreIndex[p]+1) ) );
			m_sprMachineRecord[p]->SetName( ssprintf("MachineRecordP%d",p+1) );
			m_sprMachineRecord[p]->StopAnimating();
			SET_XY_AND_ON_COMMAND( m_sprMachineRecord[p] );
			this->AddChild( m_sprMachineRecord[p] );
		}
		if( iPersonalHighScoreIndex[p] != -1 )
		{
			m_sprPersonalRecord[p].Load( THEME->GetPathG( "ScreenEvaluation", ssprintf("PersonalRecord %02d",iPersonalHighScoreIndex[p]+1) ) );
			m_sprPersonalRecord[p]->SetName( ssprintf("PersonalRecordP%d",p+1) );
			m_sprPersonalRecord[p]->StopAnimating();
			SET_XY_AND_ON_COMMAND( m_sprPersonalRecord[p] );
			this->AddChild( m_sprPersonalRecord[p] );
		}
		if( SHOW_PER_DIFFICULTY_AWARD && pdaToShow[p]!=PER_DIFFICULTY_AWARD_INVALID )
		{
			CString sAward = PerDifficultyAwardToString( pdaToShow[p] );

			m_PerDifficultyAward[p].Load( THEME->GetPathG(m_sName,"PerDifficultyAward "+sAward) );
			m_PerDifficultyAward[p]->SetName( ssprintf("PerDifficultyAwardP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_PerDifficultyAward[p] );
			this->AddChild( m_PerDifficultyAward[p] );
		}
		if( SHOW_PEAK_COMBO_AWARD && pcaToShow[p]!=PEAK_COMBO_AWARD_INVALID )
		{
			CString sAward = PeakComboAwardToString( pcaToShow[p] );

			m_PeakComboAward[p].Load( THEME->GetPathG(m_sName,"PeakComboAward "+sAward) );
			m_PeakComboAward[p]->SetName( ssprintf("PeakComboAwardP%d",p+1) );
			SET_XY_AND_ON_COMMAND( m_PeakComboAward[p] );
			this->AddChild( m_PeakComboAward[p] );
		}
	}

	bool bOneHasNewTopRecord = false;
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->IsPlayerEnabled(p) && (iMachineHighScoreIndex[p] != -1 || iPersonalHighScoreIndex[p] != -1) )
			bOneHasNewTopRecord = true;

	Grade best_grade = GRADE_NO_DATA;
	FOREACH_PlayerNumber( p )
		best_grade = min( best_grade, grade[p] ); 
	
	if( PREFSMAN->m_bAllowExtraStage && m_bTryExtraStage )
	{
		m_sprTryExtraStage.Load( THEME->GetPathToG(GAMESTATE->IsExtraStage()?"ScreenEvaluation try extra2":"ScreenEvaluation try extra1") );
		m_sprTryExtraStage->SetName( "TryExtraStage" );
		SET_XY_AND_ON_COMMAND( m_sprTryExtraStage );
		this->AddChild( m_sprTryExtraStage );

		if( GAMESTATE->IsExtraStage() )
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra2") );
		else
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEvaluation try extra1") );
	}
	else if( bOneHasNewTopRecord  &&  ANNOUNCER->HasSoundsFor("evaluation new record") )
	{
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation new record") );
	}
	else
	{	
		switch( m_Type )
		{
		case stage:
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_BATTLE:
				{
					bool bWon = GAMESTATE->GetStageResult(GAMESTATE->m_MasterPlayerNumber) == RESULT_WIN;
					if( bWon )
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation win") );
					else
						SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation lose") );
				}
				break;
			default:
				SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation "+GradeToOldString(best_grade)) );
				break;
			}
			break;
		case course:
		case summary:
			SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation final "+GradeToOldString(best_grade)) );
			break;
		default:
			ASSERT(0);
		}
	}

	switch( best_grade )
	{
	case GRADE_TIER_1:
	case GRADE_TIER_2:	
	case GRADE_TIER_3:	
		this->PostScreenMessage( SM_PlayCheer, CHEER_DELAY_SECONDS );	
		break;
	}

	this->SortByDrawOrder();


	SOUND->PlayMusic( THEME->GetPathToS("ScreenEvaluation music") );
	m_timerSoundSequences.SetZero(); // zero the sound sequence timer
	m_timerSoundSequences.Touch(); // set the timer going :]
	m_fScreenCreateTime = RageTimer::GetTimeSinceStart();
}


void ScreenEvaluation::CommitScores(
	const StageStats &stageStats, 
	int iPersonalHighScoreIndexOut[NUM_PLAYERS], 
	int iMachineHighScoreIndexOut[NUM_PLAYERS], 
	RankingCategory rcOut[NUM_PLAYERS],
	PerDifficultyAward pdaToShowOut[NUM_PLAYERS],
	PeakComboAward pcaToShowOut[NUM_PLAYERS] )
{
	{
		FOREACH_PlayerNumber( pn )
		{
			iPersonalHighScoreIndexOut[pn] = -1;
			iMachineHighScoreIndexOut[pn] = -1;
			rcOut[pn] = RANKING_INVALID;
			pdaToShowOut[pn] = PER_DIFFICULTY_AWARD_INVALID;
			pcaToShowOut[pn] = PEAK_COMBO_AWARD_INVALID;
		}
	}

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_BATTLE:
	case PLAY_MODE_RAVE:
		return; /* don't save scores in battle */
	}

	if( PREFSMAN->m_bScreenTestMode )
	{
		FOREACH_PlayerNumber( pn )
		{
			iPersonalHighScoreIndexOut[pn] = 0;
			iMachineHighScoreIndexOut[pn] = 0;
		}
	}

	// don't save scores if the player chose not to
	if( !GAMESTATE->m_SongOptions.m_bSaveScore )
		return;

	LOG->Trace( "saving stats and high scores" );

	{
		FOREACH_HumanPlayer( p )
		{
			// don't save scores if the player is disqualified
			if( GAMESTATE->IsDisqualified(p) || GAMESTATE->m_bUsingAutoPlay)
				continue;

			Song* pSong = GAMESTATE->m_pCurSong;
			Steps* pSteps = GAMESTATE->m_pCurSteps[p];

			// whether or not to save scores when the stage was failed
			// depends on if this is a course or not ... it's handled
			// below in the switch

			HighScore &hs = m_HighScore[p];
			hs.sName = RANKING_TO_FILL_IN_MARKER[p];
			hs.grade = stageStats.GetGrade( p );
			hs.iScore = stageStats.iScore[p];
			hs.fPercentDP = stageStats.GetPercentDancePoints( p );
			hs.fSurviveSeconds = stageStats.fAliveSeconds[p];
			hs.sModifiers = GAMESTATE->m_PlayerOptions[p].GetString2();
			hs.dateTime = DateTime::GetNowDateTime();
			hs.sPlayerGuid = PROFILEMAN->IsUsingProfile(p) ? PROFILEMAN->GetProfile(p)->m_sGuid : CString("");
			hs.sMachineGuid = PROFILEMAN->GetMachineProfile()->m_sGuid;
			hs.iProductID = PREFSMAN->m_iProductID;
			memcpy( hs.iTapNoteScores, stageStats.iTapNoteScores[p], sizeof(hs.iTapNoteScores) );
			memcpy( hs.iHoldNoteScores, stageStats.iHoldNoteScores[p], sizeof(hs.iHoldNoteScores) );
			hs.radarValues = stageStats.radarActual[p];

			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

			switch( m_Type )
			{
			case stage:
				{
					// don't save scores for a failed song
					if( stageStats.bFailed[p] )
						continue;

					ASSERT( pSteps );

					PROFILEMAN->AddStepsScore( pSong, pSteps, p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
				}
				break;

			case summary:
				{
					// don't save scores if any stage was failed
					if( stageStats.bFailed[p] ) 
						continue;

					int iAverageMeter = stageStats.GetAverageMeter(p);
					rcOut[p] = AverageMeterToRankingCategory( iAverageMeter );

					PROFILEMAN->AddCategoryScore( st, rcOut[p], p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
					
					// TRICKY:  Increment play count here, and not on ScreenGameplay like the others.
					PROFILEMAN->IncrementCategoryPlayCount( st, rcOut[p], p );
				}
				break;

			case course:
				{
					Course* pCourse = GAMESTATE->m_pCurCourse;
					ASSERT( pCourse );
					Trail* pTrail = GAMESTATE->m_pCurTrail[p];

					PROFILEMAN->AddCourseScore( pCourse, pTrail, p, hs, iPersonalHighScoreIndexOut[p], iMachineHighScoreIndexOut[p] );
				}
				break;
			default:
				ASSERT(0);
			}
		}
	}

	LOG->Trace( "done saving stats and high scores" );

	// If both players get a machine high score in the same HighScoreList,
	// then one player's score may have bumped the other player.  Look in 
	// the HighScoreList and re-get the high score index.
	{
		FOREACH_HumanPlayer( p )
		{
			if( iMachineHighScoreIndexOut[p] == -1 )	// no record
				continue;	// skip

			HighScore &hs = m_HighScore[p];
			Profile* pProfile = PROFILEMAN->GetMachineProfile();
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

			const HighScoreList *pHSL = NULL;
			switch( m_Type )
			{
			case stage:
				{
					Song* pSong = GAMESTATE->m_pCurSong;
					Steps* pSteps = GAMESTATE->m_pCurSteps[p];
					pHSL = &pProfile->GetStepsHighScoreList( pSong, pSteps );
				}
				break;
			case summary:
				{
					pHSL = &pProfile->GetCategoryHighScoreList( st, rcOut[p] );
				}
				break;
			case course:
				{
					Course* pCourse = GAMESTATE->m_pCurCourse;
					ASSERT( pCourse );
					Trail *pTrail = GAMESTATE->m_pCurTrail[p];
					ASSERT( pTrail );
					pHSL = &pProfile->GetCourseHighScoreList( pCourse, pTrail );
				}
				break;
			default:
				ASSERT(0);
			}
			
			vector<HighScore>::const_iterator iter = find( pHSL->vHighScores.begin(), pHSL->vHighScores.end(), hs );
			if( iter == pHSL->vHighScores.end() )
				iMachineHighScoreIndexOut[p] = -1;
			else
				iMachineHighScoreIndexOut[p] = iter - pHSL->vHighScores.begin();
		}
	}

	
	LOG->Trace( "hand out awards" );
	
	{
		FOREACH_HumanPlayer( p )
		{
			deque<PerDifficultyAward> &vPdas = GAMESTATE->m_vLastPerDifficultyAwards[p];

			LOG->Trace( "per difficulty awards" );

			// per-difficulty awards
			switch( m_Type )
			{
			case stage:
				// don't give per-difficutly awards if using easy mods
				if( !GAMESTATE->IsDisqualified(p) )
				{
					if( stageStats.FullComboOfScore( p, TNS_GREAT ) )
						vPdas.push_back( AWARD_FULL_COMBO_GREATS );
					if( stageStats.SingleDigitsOfScore( p, TNS_GREAT ) )
						vPdas.push_back( AWARD_SINGLE_DIGIT_GREATS );
					if( stageStats.FullComboOfScore( p, TNS_PERFECT ) )
						vPdas.push_back( AWARD_FULL_COMBO_PERFECTS );
					if( stageStats.SingleDigitsOfScore( p, TNS_PERFECT ) )
						vPdas.push_back( AWARD_SINGLE_DIGIT_PERFECTS );
					if( stageStats.FullComboOfScore( p, TNS_MARVELOUS ) )
						vPdas.push_back( AWARD_FULL_COMBO_MARVELOUSES );
					
					if( stageStats.OneOfScore( p, TNS_GREAT ) )
						vPdas.push_back( AWARD_ONE_GREAT );
					if( stageStats.OneOfScore( p, TNS_PERFECT ) )
						vPdas.push_back( AWARD_ONE_PERFECT );

					float fPercentGreats = stageStats.GetPercentageOfTaps(p, TNS_GREAT);
					if( fPercentGreats >= 0.8f )
						vPdas.push_back( AWARD_GREATS_80_PERCENT );
					if( fPercentGreats >= 0.9f )
						vPdas.push_back( AWARD_GREATS_90_PERCENT );
					if( fPercentGreats >= 1.f )
						vPdas.push_back( AWARD_GREATS_100_PERCENT );
				}
			}

			// Max one PDA per stage
			if( !vPdas.empty() )
				vPdas.erase( vPdas.begin(), vPdas.end()-1 );
			
			if( !vPdas.empty() )
				pdaToShowOut[p] = vPdas.back();

			LOG->Trace( "done with per difficulty awards" );

			// DO give peak combo awards if using easy mods
			int iComboAtStartOfStage = stageStats.GetComboAtStartOfStage( p );
			int iPeakCombo = stageStats.GetMaxCombo(p).cnt;

			FOREACH_PeakComboAward( pca )
			{
			      int iLevel = 1000 * (pca+1);
				bool bCrossedLevel = iComboAtStartOfStage < iLevel && iPeakCombo >= iLevel;
				LOG->Trace( "pca = %d, iLevel = %d, bCrossedLevel = %d", pca, iLevel, bCrossedLevel );
				if( bCrossedLevel )
				{
					GAMESTATE->m_vLastPeakComboAwards[p].push_back( pca );
				}
			}

			if( !GAMESTATE->m_vLastPeakComboAwards[p].empty() )
				pcaToShowOut[p] = GAMESTATE->m_vLastPeakComboAwards[p].back();

			LOG->Trace( "done with per combo awards" );
		}
	}

	LOG->Trace( "done handing out awards." );
}


void ScreenEvaluation::TweenOffScreen()
{
	// player name area
	FOREACH_EnabledPlayer( p ) 
	{
		OFF_COMMAND( m_textPlayerName[p] );
	}

	// large banner area
	OFF_COMMAND( m_LargeBanner );
	OFF_COMMAND( m_sprLargeBannerFrame );
	OFF_COMMAND( m_sprStage );
	FOREACH_EnabledPlayer( p )
	{
		OFF_COMMAND( m_DifficultyIcon[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
		OFF_COMMAND( m_textPlayerOptions[p] );
		OFF_COMMAND( m_sprDisqualified[p] );
	}

	// small banner area
	for( int i=0; i<MAX_SONGS_TO_SHOW; i++ )
	{
		OFF_COMMAND( m_SmallBanner[i] );
		OFF_COMMAND( m_sprSmallBannerFrame[i] );
	}

	// grade area
	if( SHOW_GRADE_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprGradeFrame[p] );
			OFF_COMMAND( m_Grades[p] );
			OFF_COMMAND( m_sprGrade[p] );
		}
	}

	if( SHOW_GRAPH_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprGraphFrame[p] );
			OFF_COMMAND( m_Graph[p] );
		}
	}

	if( SHOW_COMBO_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_Combo[p] );
		}
	}

	// points area
	if( SHOW_POINTS_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprPercentFrame[p] );
			m_Percent[p].Command( THEME->GetMetric(m_sName,ssprintf("PercentP%dOffCommand",p+1)) );
			m_Percent[p].TweenOffScreen();
		}
	}

	// bonus area
	if( SHOW_BONUS_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprBonusFrame[p] );
			for( int r=0; r<NUM_SHOWN_RADAR_CATEGORIES; r++ )	// foreach line
			{
				OFF_COMMAND( m_sprPossibleBar[p][r] );
				OFF_COMMAND( m_sprActualBar[p][r] );
			}
		}
	}

	// survived area
	if( SHOW_SURVIVED_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprSurvivedFrame[p] );
			OFF_COMMAND( m_textSurvivedNumber[p] );
		}
	}

	// win area
	if( SHOW_WIN_AREA )
	{
		FOREACH_EnabledPlayer( p ) 
		{
			OFF_COMMAND( m_sprWinFrame[p] );
			OFF_COMMAND( m_sprWin[p] );
		}
	}

	// judgement area
	for( int l=0; l<NUM_JUDGE_LINES; l++ ) 
	{
		if( !SHOW_JUDGMENT(l) )
			continue;

		OFF_COMMAND( m_sprJudgeLabels[l] );

		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textJudgeNumbers[l][p] );
	}

	// stats area
	for( int l=0; l<NUM_STATS_LINES; l++ ) 
	{
		if( !SHOW_STAT(l) )
			continue;

		OFF_COMMAND( m_sprStatsLabel[l] );

		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textStatsText[l][p] );
	}

	// score area
	if( SHOW_SCORE_AREA )
	{
		OFF_COMMAND( m_sprScoreLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textScore[p] );
	}

	// total score area
	if( SHOW_TOTAL_SCORE_AREA )
	{
		OFF_COMMAND( m_sprTotalScoreLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textTotalScore[p] );
	}

	// time area
	if( SHOW_TIME_AREA )
	{
		OFF_COMMAND( m_sprTimeLabel );
		FOREACH_EnabledPlayer( p ) 
			OFF_COMMAND( m_textTime[p] );
	}

	// extra area
	FOREACH_EnabledPlayer( p ) 
	{
		OFF_COMMAND( m_sprMachineRecord[p] );
		OFF_COMMAND( m_sprPersonalRecord[p] );
		if( m_PerDifficultyAward[p].IsLoaded() )
			OFF_COMMAND( m_PerDifficultyAward[p] );
		if( m_PeakComboAward[p].IsLoaded() )
			OFF_COMMAND( m_PeakComboAward[p] );
	}
	OFF_COMMAND( m_sprTryExtraStage );
}

void ScreenEvaluation::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	m_bgCondBga.Update(fDeltaTime);
	if( !m_bPassFailTriggered )
	{
		float fTime = m_bFailed? FAILED_SOUND_TIME:PASSED_SOUND_TIME;
		if(	m_timerSoundSequences.Ago() > fTime )
		{
			if( !m_sndPassFail.IsPlaying() )
				m_sndPassFail.Play();
			m_bPassFailTriggered = true;
		}
	}

	for( unsigned snd=0; snd<m_SoundSequences.size(); snd++ )
	{
		if(m_SoundSequences[snd].fTime != -1) // already played? skip...
		{
			if(m_timerSoundSequences.Ago() > m_SoundSequences[snd].fTime )
			{
				m_SoundSequences[snd].fTime = -1; // -1 indicates already started playing
				m_SoundSequences[snd].sSound.Play();
			}
		}
	}
	
	FOREACH_EnabledPlayer( p ) 
	{
		if( g_CurStageStats.iBonus[p] == 0 )
			continue;

		if( GAMESTATE->IsCourseMode() )
			continue;

		// wait a few seconds before adding bonus
		if( RageTimer::GetTimeSinceStart() - m_fScreenCreateTime  < 1.5f )
			continue;

		int increment = g_CurStageStats.iBonus[p]/10;
		/* XXX: What's this supposed to do?  If i < 1, then min(i, 1024) is i ... */
		if( increment < 1 )
			increment = min( 1024, g_CurStageStats.iBonus[p] );

		g_CurStageStats.iBonus[p] -= increment;
		g_CurStageStats.iScore[p] += increment;

		if( SHOW_SCORE_AREA )
			m_textScore[p].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, g_CurStageStats.iScore[p]) );
	}
}

void ScreenEvaluation::DrawPrimitives()
{
	m_bgCondBga.DrawPrimitives();

	Screen::DrawPrimitives();
}

void ScreenEvaluation::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenEvaluation::Input()" );

	if( IsTransitioning() )
		return;

	if( GameI.IsValid() )
	{
		PlayerNumber pn = GAMESTATE->GetCurrentStyle()->ControllerToPlayerNumber( GameI.controller );
		HighScore &hs = m_HighScore[pn];


		if( CodeDetector::EnteredCode(GameI.controller, CodeDetector::CODE_SAVE_SCREENSHOT) )
		{
			if( !m_bSavedScreenshot[pn]  &&	// only allow one screenshot
				PROFILEMAN->IsUsingProfile(pn) )
			{
				Profile* pProfile = PROFILEMAN->GetProfile(pn);
				CString sDir = PROFILEMAN->GetProfileDir((ProfileSlot)pn) + "Screenshots/";
				int iScreenshotIndex = pProfile->GetNextScreenshotIndex();
				CString sFileName = SaveScreenshot( sDir, true, true, iScreenshotIndex );
				CString sPath = sDir+sFileName;
				
				if( !sFileName.empty() )
				{
					Screenshot screenshot;
					screenshot.sFileName = sFileName;
					screenshot.sMD5 = CRYPTMAN->GetMD5( sPath );
					screenshot.highScore = hs;
					pProfile->AddScreenshot( screenshot );
				}

				m_bSavedScreenshot[pn] = true;
				return;	// handled
			}
		}
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		EndScreen();
		break;
	case SM_GoToNextScreen:
	{
		if( PREFSMAN->m_bEventMode )
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		else
		{
			/* Go to FAILED_SCREEN if we failed a non-extra stage. */
			bool bReallyFailed = m_bFailed && !(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2());

			// If failed and not in event mode, go to the game over screen.
			if( bReallyFailed )
			{
				SCREENMAN->SetNewScreen( FAILED_SCREEN );
				break;
			}

			/* We passed.  If we have another stage to play, go to NEXT_SCREEN. */
			switch( m_Type )
			{
			case stage:
				if( m_bTryExtraStage || !(GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() ) )
				{
					/* Hack: finish the stage before moving on.  This will increment the stage
					 * counter.  If we don't do this, the stage counter will be wrong for
					 * ScreenSelectMusic code which is called before the ctor.  Watch out;
					 * calling this will change the results of IsFinalStage(), etc. */
					GAMESTATE->FinishStage();

					SCREENMAN->SetNewScreen( NEXT_SCREEN );
					break;
				}

			case summary:
			case course:
				SCREENMAN->SetNewScreen( END_SCREEN );
				break;
			}
		}

		if(	m_sndPassFail.IsPlaying() )
			m_sndPassFail.Stop();
		break;
	}
	case SM_PlayCheer:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("evaluation cheer") );
		break;
	}
}

void ScreenEvaluation::MenuLeft( PlayerNumber pn )
{
	// What is the purpose of this?  I keep my feet on the pads and 
	// was wondering why the grades weren't spinning. -Chris
	// To be able to see the grade without having to wait for it to
	// stop spinning.  (I was hitting left repeatedly and wondering
	// why it kept spinning ...)
	//m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuRight( PlayerNumber pn )
{
	// What is the purpose of this?  I keep my feet on the pads and 
	// was wondering why the grades weren't spinning. -Chris
	//m_Grades[pn].SettleQuickly();
}

void ScreenEvaluation::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}

void ScreenEvaluation::MenuStart( PlayerNumber pn )
{
	m_soundStart.Play();

	EndScreen();
}

void ScreenEvaluation::EndScreen()
{
	TweenOffScreen();

	FOREACH_PlayerNumber( p )
		m_Grades[p].SettleImmediately();

	if( !PREFSMAN->m_bEventMode )
	{
		switch( m_Type )
		{
		case stage:
			if( !m_bTryExtraStage && (GAMESTATE->IsFinalStage() || GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2()) )
			{
				/* Tween the screen out, but leave the MenuElements where they are.
				 * Play the "swoosh" sound manually (would normally be played by the ME
				 * tween out). */
				TweenOffScreen();
			}
			break;
		}
	}
	StartTransitioning( SM_GoToNextScreen );
}

/*
 * (c) 2001-2004 Chris Danford
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
