
/****************************************************************

   This file was created automatically by `FlexCat 2.18'
   from "iGameStrings.cd".

   Do NOT edit by hand!

****************************************************************/

/****************************************************************
    This file uses the auto initialization features of
    Dice, gcc and SAS/C, respectively.

    Dice does this by using the __autoinit and __autoexit
    keywords, whereas SAS/C uses names beginning with _STI
    or _STD, respectively. gcc uses the asm() instruction
    to emulate C++ constructors and destructors.

    Using this file you don't have *all* the benefits of
    locale.library (no Locale or Language arguments are
    supported when opening the catalog). However, these are
    *very* rarely used, so this should be sufficient for most
    applications.
****************************************************************/

/*
    Include files and compiler specific stuff
*/

#include <exec/memory.h>
#include <libraries/locale.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/iffparse.h>

#include <stdlib.h>
#include <string.h>



#include "iGameStrings_cat.h"


/*
    Variables
*/

struct FC_String iGameStrings_Strings[77] = {
    { (STRPTR) "A front-end to WHDLoad", 0 },
    { (STRPTR) "Dimitris Panokostas 2018", 1 },
    { (STRPTR) "iGame", 2 },
    { (STRPTR) "File", 3 },
    { (STRPTR) "Scan Repositories", 4 },
    { (STRPTR) "R_", 5 },
    { (STRPTR) "Add non-WHDLoad game...", 6 },
    { (STRPTR) "A_", 7 },
    { (STRPTR) "Show/Hide hidden entries", 8 },
    { (STRPTR) "Open List...", 9 },
    { (STRPTR) "O_", 10 },
    { (STRPTR) "Save List", 11 },
    { (STRPTR) "S_", 12 },
    { (STRPTR) "Save List As...", 13 },
    { (STRPTR) "Export List to Text file...", 14 },
    { (STRPTR) "About...", 15 },
    { (STRPTR) "Quit", 16 },
    { (STRPTR) "Q_", 17 },
    { (STRPTR) "Edit", 18 },
    { (STRPTR) "Duplicate...", 19 },
    { (STRPTR) "Properties...", 20 },
    { (STRPTR) "P_", 21 },
    { (STRPTR) "Delete", 22 },
    { (STRPTR) "D_", 23 },
    { (STRPTR) "Settings", 24 },
    { (STRPTR) "Settings...", 25 },
    { (STRPTR) "Game Repositories...", 26 },
    { (STRPTR) "MUI Settings...", 27 },
    { (STRPTR) "Filter:", 28 },
    { (STRPTR) "Genres", 29 },
    { (STRPTR) "Game Properties", 30 },
    { (STRPTR) "Title", 31 },
    { (STRPTR) "Genre:", 32 },
    { (STRPTR) "Unknown", 33 },
    { (STRPTR) "Favorite:", 34 },
    { (STRPTR) "Hidden:", 35 },
    { (STRPTR) "Times Played:", 36 },
    { (STRPTR) "Slave Path:", 37 },
    { (STRPTR) "Tooltypes", 38 },
    { (STRPTR) "OK", 39 },
    { (STRPTR) "Cancel", 40 },
    { (STRPTR) "Game Repositories", 41 },
    { (STRPTR) "Add", 42 },
    { (STRPTR) "Remove", 43 },
    { (STRPTR) "Close", 44 },
    { (STRPTR) "Add a non-WHDLoad Game", 45 },
    { (STRPTR) "Title:", 46 },
    { (STRPTR) "Path:", 47 },
    { (STRPTR) "Genre:", 48 },
    { (STRPTR) "Unknown", 49 },
    { (STRPTR) "OK", 50 },
    { (STRPTR) "Cancel", 51 },
    { (STRPTR) "About iGame", 52 },
    { (STRPTR) "iGame version 2.0b\n\n(C) Emmanuel Vasilakis\nmrzammler@gmail.com\n\nUpdates by Dimitris Panokostas\nmidwan@gmail.com\n\n", 53 },
    { (STRPTR) "OK", 54 },
    { (STRPTR) "iGame Settings", 55 },
    { (STRPTR) "Show Screenshots", 56 },
    { (STRPTR) "Screenshots", 57 },
    { (STRPTR) "Use GuiGfx", 58 },
    { (STRPTR) "Screenshot Size:", 59 },
    { (STRPTR) "160x128", 60 },
    { (STRPTR) "320x256", 61 },
    { (STRPTR) "Custom", 62 },
    { (STRPTR) "Width", 63 },
    { (STRPTR) "Height", 64 },
    { (STRPTR) "Titles", 65 },
    { (STRPTR) "Titles From:", 66 },
    { (STRPTR) "Slave Contents", 67 },
    { (STRPTR) "Directories", 68 },
    { (STRPTR) "Use Smart Spaces", 69 },
    { (STRPTR) "Misc", 70 },
    { (STRPTR) "Save Stats on Exit", 71 },
    { (STRPTR) "Use Enter to Filter", 72 },
    { (STRPTR) "Hide Side panel", 73 },
    { (STRPTR) "Save", 74 },
    { (STRPTR) "Use", 75 },
    { (STRPTR) "Cancel", 76 }
};

STATIC struct Catalog *iGameStringsCatalog = NULL;
#ifdef LOCALIZE_V20
STATIC STRPTR iGameStringsStrings = NULL;
STATIC ULONG iGameStringsStringsSize;
#endif


#if defined(_DCC)
STATIC __autoexit VOID _STDCloseiGameStringsCatalog(VOID)
#elif defined(__SASC)
VOID _STDCloseiGameStringsCatalog(VOID)
#elif defined(__GNUC__)
STATIC VOID __attribute__ ((destructor)) _STDCloseiGameStringsCatalog(VOID)
#else
VOID CloseiGameStringsCatalog(VOID)
#endif

{
    if (iGameStringsCatalog) {
	CloseCatalog(iGameStringsCatalog);
    }
#ifdef LOCALIZE_V20
    if (iGameStringsStrings) {
	FreeMem(iGameStringsStrings, iGameStringsStringsSize);
    }
#endif
}


#if defined(_DCC)
STATIC __autoinit VOID _STIOpeniGameStringsCatalog(VOID)
#elif defined(__SASC)
VOID _STIOpeniGameStringsCatalog(VOID)
#elif defined(__GNUC__)
VOID __attribute__ ((constructor)) _STIOpeniGameStringsCatalog(VOID)
#else
VOID OpeniGameStringsCatalog(VOID)
#endif

{
    if (LocaleBase) {
	if ((iGameStringsCatalog = OpenCatalog(NULL, (STRPTR) "iGameStrings.catalog",
				     OC_BuiltInLanguage, "english",
				     OC_Version, -1,
				     TAG_DONE))) {
	    struct FC_String *fc;
	    int i;

	    for (i = 0, fc = iGameStrings_Strings;  i < 77;  i++, fc++) {
		 fc->msg = GetCatalogStr(iGameStringsCatalog, fc->id, (STRPTR) fc->msg);
	    }
	}
    }
}




#ifdef LOCALIZE_V20
VOID InitiGameStringsCatalog(STRPTR language)

{
    struct IFFHandle *iffHandle;

    /*
    **  Use iffparse.library only, if we need to.
    */
    if (LocaleBase  ||  !IFFParseBase  ||  !language  ||
	Stricmp(language, "english") == 0) {
	return;
    }

    if ((iffHandle = AllocIFF())) {
	char path[128];  /* Enough to hold 4 path items (dos.library V40) */
	strcpy(path, "PROGDIR:Catalogs");
	AddPart((STRPTR) path, language, sizeof(path));
	AddPart((STRPTR) path, "iGameStrings.catalog", sizeof(path));
	if (!(iffHandle->iff_Stream = Open((STRPTR) path, MODE_OLDFILE))) {
	    strcpy(path, "LOCALE:Catalogs");
	    AddPart((STRPTR) path, language, sizeof(path));
	    AddPart((STRPTR) path, language, sizeof(path));
	    iffHandle->iff_Stream = Open((STRPTR) path, MODE_OLDFILE);
	}

	if (iffHandle->iff_Stream) {
	    InitIFFasDOS(iffHandle);
	    if (!OpenIFF(iffHandle, IFFF_READ)) {
		if (!PropChunk(iffHandle, MAKE_ID('C','T','L','G'),
			       MAKE_ID('S','T','R','S'))) {
		    struct StoredProperty *sp;
		    int error;

		    for (;;) {
			if ((error = ParseIFF(iffHandle, IFFPARSE_STEP))
				   ==  IFFERR_EOC) {
			    continue;
			}
			if (error) {
			    break;
			}

			if ((sp = FindProp(iffHandle, MAKE_ID('C','T','L','G'),
					   MAKE_ID('S','T','R','S')))) {
			    /*
			    **  Check catalog and calculate the needed
			    **  number of bytes.
			    **  A catalog string consists of
			    **      ID (LONG)
			    **      Size (LONG)
			    **      Bytes (long word padded)
			    */
			    LONG bytesRemaining;
			    LONG *ptr;

			    iGameStringsStringsSize = 0;
			    bytesRemaining = sp->sp_Size;
			    ptr = (LONG *) sp->sp_Data;

			    while (bytesRemaining > 0) {
				LONG skipSize, stringSize;

				ptr++;                  /* Skip ID */
				stringSize = *ptr++;
				skipSize = ((stringSize+3) >> 2);

				iGameStringsStringsSize += stringSize+1;  /* NUL */
				bytesRemaining -= 8 + (skipSize << 2);
				ptr += skipSize;
			    }

			    if (!bytesRemaining  &&
				(iGameStringsStrings = AllocMem(iGameStringsStringsSize, MEMF_ANY))) {
				STRPTR sptr;

				bytesRemaining = sp->sp_Size;
				ptr = (LONG *) sp->sp_Data;
				sptr = iGameStringsStrings;

				while (bytesRemaining) {
				    LONG skipSize, stringSize, id;
				    struct FC_String *fc;
				    int i;

				    id = *ptr++;
				    stringSize = *ptr++;
				    skipSize = ((stringSize+3) >> 2);

				    CopyMem(ptr, sptr, stringSize);
				    bytesRemaining -= 8 + (skipSize << 2);
				    ptr += skipSize;

				    for (i = 0, fc = iGameStrings_Strings;  i < 77;  i++, fc++) {
					if (fc->id == id) {
					    fc->msg = sptr;
					}
				    }

				    sptr += stringSize;
				    *sptr++ = '\0';
				}
			    }
			    break;
			}
		    }
		}
		CloseIFF(iffHandle);
	    }
	    Close(iffHandle->iff_Stream);
	}
	FreeIFF(iffHandle);
    }
}
#endif
