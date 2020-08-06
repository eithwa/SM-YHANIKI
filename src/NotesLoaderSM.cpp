#include "global.h"
#include "NotesLoaderSM.h"
#include "GameManager.h"
#include "RageException.h"
#include "MsdFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "RageFileManager.h"
#include "GameState.h"
#define MAX_EDIT_SIZE_BYTES  20*1024	// 20 KB

void SMLoader::LoadFromSMTokens( 
	CString sStepsType, 
	CString sDescription,
	CString sDifficulty,
	CString sMeter,
	CString sRadarValues,
	CString sNoteData,
	CString sAttackData,
	Steps &out
)
{
	TrimLeft(sStepsType); 
	TrimRight(sStepsType); 
	TrimLeft(sDescription); 
	TrimRight(sDescription); 
	TrimLeft(sDifficulty); 
	TrimRight(sDifficulty); 


//	LOG->Trace( "Steps::LoadFromSMTokens()" );

	out.m_StepsType = GameManager::StringToStepsType(sStepsType);
	out.SetDescription(sDescription);
	out.SetDifficulty(StringToDifficulty( sDifficulty ));

	// HACK:  We used to store SMANIAC as DIFFICULTY_HARD with special description.
	// Now, it has its own DIFFICULTY_CHALLENGE
	if( sDescription.CompareNoCase("smaniac") == 0 ) 
		out.SetDifficulty( DIFFICULTY_CHALLENGE );
	// HACK:  We used to store CHALLENGE as DIFFICULTY_HARD with special description.
	// Now, it has its own DIFFICULTY_CHALLENGE
	if( sDescription.CompareNoCase("challenge") == 0 ) 
		out.SetDifficulty( DIFFICULTY_CHALLENGE );

	out.SetMeter(atoi(sMeter));
	CStringArray saValues;
	split( sRadarValues, ",", saValues, true );
	if( saValues.size() == NUM_RADAR_CATEGORIES )
	{
		RadarValues v;
		FOREACH_RadarCategory(rc)
			v[rc] = strtof( saValues[rc], NULL );
		out.SetRadarValues( v ); 
	}
    
	out.SetSMNoteData(sNoteData, sAttackData);

	out.TidyUpData();
}

void SMLoader::GetApplicableFiles( CString sPath, CStringArray &out )
{
	GetDirListing( sPath + CString("*.sm"), out );
}

bool SMLoader::LoadTimingFromFile( const CString &fn, TimingData &out )
{
	MsdFile msd;
	if( !msd.ReadFile( fn ) )
	{
		LOG->Warn( "Couldn't load %s, \"%s\"", fn.c_str(), msd.GetError().c_str() );
		return false;
	}

	out.m_sFile = fn;
	LoadTimingFromSMFile( msd, out );
	return true;
}
float binarySearch(float find)
{ 
	int MAX = GAMESTATE->m_fBeatNormalization.size();
    int low = 0; 
    int upper = MAX - 1;
	vector<float> number;
	number.assign(GAMESTATE->m_fBeatNormalization.begin(), GAMESTATE->m_fBeatNormalization.end());
    while(low <= upper) { 
		int mid = (low + upper) / 2;
		//===========
		if((mid+1)<=upper)
		{
			if(number[mid] < find && number[mid+1] > find)
			{
				if(abs(number[mid]-find)<abs(number[mid+1]-find))
				{
					return number[mid];
				}else
				{
					return number[mid+1];
				}
			}
		}
		if((mid-1)>=low)
		{
			if(number[mid-1] < find && number[mid] > find)
			{
				if(abs(number[mid-1]-find)<abs(number[mid]-find))
				{
					return number[mid-1];
				}else
				{
					return number[mid];
				}
			}
		}
		//===========
        
        if(number[mid] < find) 
            low = mid+1; 
        else if(number[mid] > find) 
            upper = mid - 1; 
        else //mid = find
            return number[mid]; 
    } 
    return 999; 
} 
void SMLoader::LoadTimingFromSMFile( const MsdFile &msd, TimingData &out )
{
	out.m_fBeat0OffsetInSeconds = 0;
	out.m_BPMSegments.clear();
	out.m_StopSegments.clear();

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		const CString sValueName = sParams[0];

		if( 0==stricmp(sValueName,"OFFSET") )
			out.m_fBeat0OffsetInSeconds = strtof( sParams[1], NULL );
		else if( 0==stricmp(sValueName,"STOPS") || 0==stricmp(sValueName,"FREEZES") )
		{
			CStringArray arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );

			for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				/* XXX: Once we have a way to display warnings that the user actually
				 * cares about (unlike most warnings), this should be one of them. */
				if( arrayFreezeValues.size() != 2 )
				{
					LOG->Warn( "Invalid #%s value \"%s\" (must have exactly one '='), ignored",
						sValueName.c_str(), arrayFreezeExpressions[f].c_str() );
					continue;
				}

				const float fFreezeBeat = strtof( arrayFreezeValues[0], NULL );
				const float fFreezeSeconds = strtof( arrayFreezeValues[1], NULL );

				StopSegment new_seg;

				float stop_beat_temp = fFreezeBeat;
				float tmp = binarySearch((float)(fFreezeBeat -(int)fFreezeBeat));
				if(tmp!=999)
				{
					stop_beat_temp = (float)((int)fFreezeBeat + tmp);
				}
				
				if (tmp != 999) 
				{
					new_seg.m_fStartBeat = stop_beat_temp;
					// LOG->Info("normalization %f", new_seg.m_fStartBeat);
				}
				else 
				{
					new_seg.m_fStartBeat = fFreezeBeat;
				}
				// new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fStopSeconds = fFreezeSeconds;
				// LOG->Info("get stop %f", new_seg.m_fStartBeat);
//				LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

				out.AddStopSegment( new_seg );
			}
		}

		else if( 0==stricmp(sValueName,"BPMS") )
		{
			CStringArray arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );

			for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				/* XXX: Once we have a way to display warnings that the user actually
				 * cares about (unlike most warnings), this should be one of them. */
				if(arrayBPMChangeValues.size() != 2)
				{
					LOG->Warn( "Invalid #%s value \"%s\" (must have exactly one '='), ignored",
						sValueName.c_str(), arrayBPMChangeExpressions[b].c_str() );
					continue;
				}

				const float fBeat = strtof( arrayBPMChangeValues[0], NULL );
				const float fNewBPM = strtof( arrayBPMChangeValues[1], NULL );			

				BPMSegment new_seg;
				
				float bpm_beat_temp = fBeat;
				float tmp = binarySearch((float)(fBeat - (int)fBeat));
				if (tmp != 999)
				{
					bpm_beat_temp = (float)((int)fBeat + tmp);
				}
	
				if (tmp != 999)
				{
					new_seg.m_fStartBeat = bpm_beat_temp;
				}
				else
				{
					new_seg.m_fStartBeat = fBeat;
				}
				// new_seg.m_fStartBeat = fBeat;
				new_seg.m_fBPM = fNewBPM;
				
				out.AddBPMSegment( new_seg );
			}
		}
	}
}

bool LoadFromBGChangesString( BackgroundChange &change, const CString &sBGChangeExpression )
{
	CStringArray aBGChangeValues;
	split( sBGChangeExpression, "=", aBGChangeValues );

	switch( aBGChangeValues.size() )
	{
	case 6:
		change.m_fRate = strtof( aBGChangeValues[2], NULL );
		change.m_bFadeLast = atoi( aBGChangeValues[3] ) != 0;
		change.m_bRewindMovie = atoi( aBGChangeValues[4] ) != 0;
		change.m_bLoop = atoi( aBGChangeValues[5] ) != 0;
		// fall through
	case 2:
		change.m_fStartBeat = strtof( aBGChangeValues[0], NULL );
		change.m_sBGName = aBGChangeValues[1];
		return true;
	default:
		LOG->Warn("Invalid #BGCHANGES value \"%s\" was ignored", sBGChangeExpression.c_str());
		return false;
	}
}

bool SMLoader::LoadFromSMFile( CString sPath, Song &out )
{
	LOG->Trace( "Song::LoadFromSMFile(%s)", sPath.c_str() );

	MsdFile msd;
	if( !msd.ReadFile( sPath ) )
		RageException::Throw( "Error opening file \"%s\": %s", sPath.c_str(), msd.GetError().c_str() );

	out.m_Timing.m_sFile = sPath;
	LoadTimingFromSMFile( msd, out.m_Timing );

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		const CString sValueName = sParams[0];

		// handle the data
		/* Don't use GetMainAndSubTitlesFromFullTitle; that's only for heuristically
		 * splitting other formats that *don't* natively support #SUBTITLE. */
		if( 0==stricmp(sValueName,"TITLE") )
			out.m_sMainTitle = sParams[1];

		else if( 0==stricmp(sValueName,"SUBTITLE") )
			out.m_sSubTitle = sParams[1];

		else if( 0==stricmp(sValueName,"ARTIST") )
			out.m_sArtist = sParams[1];

		else if( 0==stricmp(sValueName,"TITLETRANSLIT") )
			out.m_sMainTitleTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"SUBTITLETRANSLIT") )
			out.m_sSubTitleTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"ARTISTTRANSLIT") )
			out.m_sArtistTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"CREDIT") )
			out.m_sCredit = sParams[1];

		else if( 0==stricmp(sValueName,"BANNER") )
			out.m_sBannerFile = sParams[1];

		else if( 0==stricmp(sValueName,"BACKGROUND") )
			out.m_sBackgroundFile = sParams[1];

		/* Save "#LYRICS" for later, so we can add an internal lyrics tag. */
		else if( 0==stricmp(sValueName,"LYRICSPATH") )
			out.m_sLyricsFile = sParams[1];

		else if( 0==stricmp(sValueName,"CDTITLE") )
			out.m_sCDTitleFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSIC") )
			out.m_sMusicFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSICLENGTH") )
		{
			if(!FromCache)
				continue;
			out.m_fMusicLengthSeconds = strtof( sParams[1], NULL );
		}

		else if( 0==stricmp(sValueName,"MUSICBYTES") )
			; /* ignore */

		/* We calculate these.  Some SMs in circulation have bogus values for
		 * these, so make sure we always calculate it ourself. */
		else if( 0==stricmp(sValueName,"FIRSTBEAT") )
		{
			if(!FromCache)
				continue;
			out.m_fFirstBeat = strtof( sParams[1], NULL );
		}

		else if( 0==stricmp(sValueName,"LASTBEAT") )
		{
			if(!FromCache)
				LOG->Trace("Ignored #LASTBEAT (cache only)");
			out.m_fLastBeat = strtof( sParams[1], NULL );
		}
		else if( 0==stricmp(sValueName,"SONGFILENAME") )
		{
			if( FromCache )
				out.m_sSongFileName = sParams[1];
		}
		else if( 0==stricmp(sValueName,"HASMUSIC") )
		{
			if( FromCache )
				out.m_bHasMusic = atoi( sParams[1] ) != 0;
		}
		else if( 0==stricmp(sValueName,"HASBANNER") )
		{
			if( FromCache )
				out.m_bHasBanner = atoi( sParams[1] ) != 0;
		}

		else if( 0==stricmp(sValueName,"SAMPLESTART") )
			out.m_fMusicSampleStartSeconds = HHMMSSToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLELENGTH") )
			out.m_fMusicSampleLengthSeconds = HHMMSSToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"DISPLAYBPM") )
		{
			// #DISPLAYBPM:[xxx][xxx:xxx]|[*]; 
			if( sParams[1] == "*" )
				out.m_DisplayBPMType = Song::DISPLAY_RANDOM;
			else 
			{
				out.m_DisplayBPMType = Song::DISPLAY_SPECIFIED;
				out.m_fSpecifiedBPMMin = strtof( sParams[1], NULL );
				if( sParams[2].empty() )
					out.m_fSpecifiedBPMMax = out.m_fSpecifiedBPMMin;
				else
					out.m_fSpecifiedBPMMax = strtof( sParams[2], NULL );
			}
		}

		else if( 0==stricmp(sValueName,"SELECTABLE") )
		{
			if(!stricmp(sParams[1],"YES"))
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			else if(!stricmp(sParams[1],"NO"))
				out.m_SelectionDisplay = out.SHOW_NEVER;
			else if(!stricmp(sParams[1],"ROULETTE"))
				out.m_SelectionDisplay = out.SHOW_ROULETTE;
			else
				LOG->Warn( "The song file '%s' has an unknown #SELECTABLE value, '%s'; ignored.", sPath.c_str(), sParams[1].c_str());
		}

		else if( 0==stricmp(sValueName,"BGCHANGES") || 0==stricmp(sValueName,"ANIMATIONS") )
		{
			CStringArray aBGChangeExpressions;
			split( sParams[1], ",", aBGChangeExpressions );

			for( unsigned b=0; b<aBGChangeExpressions.size(); b++ )
			{
				BackgroundChange change;
				if( LoadFromBGChangesString( change, aBGChangeExpressions[b] ) )
					out.AddBackgroundChange( change );
			}
		}

		else if( 0==stricmp(sValueName,"FGCHANGES") )
		{
			CStringArray aFGChangeExpressions;
			split( sParams[1], ",", aFGChangeExpressions );

			for( unsigned b=0; b<aFGChangeExpressions.size(); b++ )
			{
				BackgroundChange change;
				if( LoadFromBGChangesString( change, aFGChangeExpressions[b] ) )
					out.AddForegroundChange( change );
			}
		}

		else if( 0==stricmp(sValueName,"NOTES") )
		{
			if( iNumParams < 7 )
			{
				LOG->Trace( "The song file '%s' is has %d fields in a #NOTES tag, but should have at least %d.", sPath.c_str(), iNumParams, 7 );
				continue;
			}

			Steps* pNewNotes = new Steps;
			ASSERT( pNewNotes );

			LoadFromSMTokens( 
				sParams[1], sParams[2], sParams[3], sParams[4], sParams[5], sParams[6], (iNumParams>=8)?sParams[7]:CString(""),
				*pNewNotes);

			out.AddSteps( pNewNotes );
		}
		else if( 0==stricmp(sValueName,"OFFSET") || 0==stricmp(sValueName,"BPMS") ||
				 0==stricmp(sValueName,"STOPS") || 0==stricmp(sValueName,"FREEZES") )
				 ;
		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.c_str() );
	}

	return true;
}


bool SMLoader::LoadFromDir( CString sPath, Song &out )
{
	CStringArray aFileNames;
	GetApplicableFiles( sPath, aFileNames );

	if( aFileNames.size() > 1 )
		RageException::Throw( "There is more than one SM file in '%s'.  There should be only one!", sPath.c_str() );

	/* We should have exactly one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( aFileNames.size() == 1 );

	return LoadFromSMFile( sPath + aFileNames[0], out );
}

bool SMLoader::LoadEdit( CString sEditFilePath, ProfileSlot slot )
{
	LOG->Trace( "Song::LoadEdit(%s)", sEditFilePath.c_str() );

	int iBytes = FILEMAN->GetFileSizeInBytes( sEditFilePath );
	if( iBytes > MAX_EDIT_SIZE_BYTES )
	{
		LOG->Warn( "The edit '%s' is unreasonably large.  It won't be loaded.", sEditFilePath.c_str() );
		return false;
	}

	MsdFile msd;
	if( !msd.ReadFile( sEditFilePath ) )
		RageException::Throw( "Error opening file \"%s\": %s", sEditFilePath.c_str(), msd.GetError().c_str() );

	Song* pSong = NULL;

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		const CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"SONG") )
		{
			if( pSong )
			{
				LOG->Warn( "The edit file '%s' has more than one #SONG tag.", sEditFilePath.c_str() );
				return false;
			}

			CString sSongFullTitle = sParams[1];
			sSongFullTitle.Replace( '\\', '/' );

			pSong = SONGMAN->FindSong( sSongFullTitle );
			if( pSong == NULL )
			{
				LOG->Warn( "The edit file '%s' required a song '%s' that isn't present.", sEditFilePath.c_str(), sSongFullTitle.c_str() );
				return false;
			}

			if( pSong->GetNumStepsLoadedFromProfile(slot) >= MAX_EDITS_PER_SONG_PER_PROFILE )
			{
				LOG->Warn( "The song '%s' already has the maximum number of edits allowed for ProfileSlotP%d.", sSongFullTitle.c_str(), slot+1 );
				return false;
			}
		}

		else if( 0==stricmp(sValueName,"NOTES") )
		{
			if( pSong == NULL )
			{
				LOG->Warn( "The edit file '%s' has doesn't have a #SONG tag preceeding the first #NOTES tag.", sEditFilePath.c_str() );
				return false;
			}

			if( iNumParams < 7 )
			{
				LOG->Trace( "The song file '%s' is has %d fields in a #NOTES tag, but should have at least %d.", sEditFilePath.c_str(), iNumParams, 7 );
				continue;
			}

			Steps* pNewNotes = new Steps;
			ASSERT( pNewNotes );

			LoadFromSMTokens( 
				sParams[1], sParams[2], sParams[3], sParams[4], sParams[5], sParams[6], (iNumParams>=8)?sParams[7]:CString(""),
				*pNewNotes);

			pNewNotes->SetLoadedFromProfile( slot );
			pNewNotes->SetDifficulty( DIFFICULTY_EDIT );


			if( pSong->IsEditAlreadyLoaded(pNewNotes) )
			{
				LOG->Warn( "The edit file '%s' is a duplicate of another edit that was already loaded.", sEditFilePath.c_str() );
				SAFE_DELETE( pNewNotes );
				return false;
			}

			pSong->AddSteps( pNewNotes );
			return true;	// Only allow one Steps per edit file!
		}
		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.c_str() );
	}

	return true;
	
}

void SMLoader::TidyUpData( Song &song, bool cache )
{
	/*
	 * Hack: if the song has any changes at all (so it won't use a random BGA)
	 * and doesn't end with "-nosongbg-", add a song background BGC.  Remove
	 * "-nosongbg-" if it exists.
	 *
	 * This way, songs that were created earlier, when we added the song BG
	 * at the end by default, will still behave as expected; all new songs will
	 * have to add an explicit song BG tag if they want it.  This is really a
	 * formatting hack only; nothing outside of SMLoader ever sees "-nosongbg-".
	 */
	vector<BackgroundChange> &bg = song.m_BackgroundChanges;
	if( !bg.empty() )
	{
		/* BGChanges have been sorted.  On the odd chance that a BGChange exists
		 * with a very high beat, search the whole list. */
		bool bHasNoSongBgTag = false;

		for( unsigned i = 0; !bHasNoSongBgTag && i < bg.size(); ++i )
		{
			if( !bg[i].m_sBGName.CompareNoCase("-nosongbg-") )
			{
				bg.erase( bg.begin()+i );
				bHasNoSongBgTag = true;
			}
		}

		/* If there's no -nosongbg- tag, add the song BG. */
		if( !bHasNoSongBgTag ) do
		{
			/* If we're loading cache, -nosongbg- should always be in there.  We must
			 * not call IsAFile(song.GetBackgroundPath()) when loading cache. */
			if( cache )
				break;

			/* If BGChanges already exist after the last beat, don't add the background
			 * in the middle. */
			if( !bg.empty() && bg.back().m_fStartBeat-0.0001f >= song.m_fLastBeat )
				break;

			/* If the last BGA is already the song BGA, don't add a duplicate. */
			if( !bg.empty() && !bg.back().m_sBGName.CompareNoCase(song.m_sBackgroundFile) )
				break;

			if( !IsAFile( song.GetBackgroundPath() ) )
				break;

			bg.push_back( BackgroundChange(song.m_fLastBeat,song.m_sBackgroundFile) );
		} while(0);
	}
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
