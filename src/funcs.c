/*
  funcs.c
  Misc functions for iGame
  
  Copyright (c) 2016, Emmanuel Vasilakis
  
  This file is part of iGame.

  iGame is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  iGame is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with iGame. If not, see <http://www.gnu.org/licenses/>.
*/

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/icon_protos.h>
#include <libraries/mui.h>
#include <MUI/Guigfx_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <clib/muimaster_protos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <exec/memory.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <workbench/workbench.h>
#include <clib/graphics_protos.h>

#include "iGameGUI.h"
#include "iGameExtern.h"
#include "iGameStrings_cat.h"
#include <libraries/asl.h>
#include "iGameExtern.h"

extern char* strdup(const char* s);
extern struct ObjApp* app;
extern char* executable_name;

/* global variables */
int total_hidden = 0;
int showing_hidden = 0;
int total_games;
int no_of_genres;
char gamestooltypes[1024];
int SS_WIDTH, SS_HEIGHT;
int NOGUIGFX;
int FILTERUSEENTER;
int NOSCREENSHOT;
int SAVESTATSONEXIT;
int TITLESFROMDIRS;
int NOSMARTSPACES;
int NOSIDEPANEL;
int IntroPic = 0;

/* function definitions */
char** my_split(char* str, char* spl);
int get_genre(char* title, char* genre);
void get_path(char* title, char* path);
void followthread(BPTR lock, int tab_level);
void refresh_list(int check_exists);
int hex2dec(char* hexin);
void msg_box(const char* msg);
int get_title_from_slave(char* slave, char* title);
int check_dup_title(char* title);
int get_delimiter_position(const char* str);
const char* get_directory_name(const char* str);
const char* get_executable_name(int argc, char** argv);
const char* add_spaces_to_string(const char* input);
void strip_path(const char* path, char* naked_path);
char* get_slave_from_path(char* slave, int start, char* path);

/* structures */
struct EasyStruct msgbox;
struct FileRequester* request;
char fname[255];

typedef struct games
{
	char title[200];
	char genre[30];
	int index;
	char path[256];
	int favorite;
	int times_played;
	int last_played; //indicates whether this one was the last game played
	int exists; //indicates whether this game still exists after a scan
	int hidden; //game is hidden from normal operation
	int deleted; // indicates this entry should be deleted when the list is saved
	struct games* next;
} games_list;

games_list *item_games = NULL, *games = NULL;

typedef struct repos
{
	char repo[256];
	struct repos* next;
} repos_list;

repos_list *item_repos = NULL, *repos = NULL;

typedef struct genres
{
	char genre[256];
	struct genres* next;
} genres_list;

genres_list *item_genres = NULL, *genres = NULL;

const unsigned char* GetMBString(const unsigned char* ref)
{
	if (ref[1] == '\0')
		return &ref[2];
	return ref;
}

void status_show_total()
{
	char helper[200];
	set(app->LV_GamesList, MUIA_List_Quiet, FALSE);
	sprintf(helper, (const char*)GetMBString(MSG_TotalNumberOfGames), total_games);
	set(app->TX_Status, MUIA_Text_Contents, helper);
}

void add_games_to_listview()
{
	total_games = 0;
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (item_games->hidden != 1 && item_games->deleted != 1)
		{
			total_games++;
			DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
		}
	}
	status_show_total();
}

void load_games_list(const char* filename)
{
	char file_line[500];
	FILE* fpgames = fopen(filename, "re");
	if (fpgames)
	{
		if (games != NULL)
		{
			free(games);
			games = NULL;
		}

		do
		{
			if (fgets(file_line, sizeof file_line, fpgames) == NULL)
				break;

			file_line[strlen(file_line) - 1] = '\0';

			if (strlen(file_line) == 0)
				continue;

			item_games = (games_list *)calloc(1, sizeof(games_list));
			item_games->next = NULL;

			if (!strcmp((char *)file_line, "index=0"))
			{
				item_games->index = 0;
				item_games->exists = 0;
				item_games->deleted = 0;

				do
				{
					if (fgets(file_line, sizeof file_line, fpgames) == NULL)
						break;

					file_line[strlen(file_line) - 1] = '\0';

					if (strlen(file_line) == 0)
						break;

					//this is to make sure that gameslist goes ok from 1.2 to 1.3
					item_games->hidden = 0;

					if (!strncmp(file_line, "title=", 6))
						strcpy(item_games->title, file_line + 6);
					else if (!strncmp(file_line, "genre=", 6))
						strcpy(item_games->genre, file_line + 6);
					else if (!strncmp(file_line, "path=", 5))
						strcpy(item_games->path, file_line + 5);
					else if (!strncmp(file_line, "favorite=", 9))
						item_games->favorite = atoi(file_line + 9);
					else if (!strncmp(file_line, "timesplayed=", 12))
						item_games->times_played = atoi(file_line + 12);
					else if (!strncmp(file_line, "lastplayed=", 11))
						item_games->last_played = atoi(file_line + 11);
					else if (!strncmp(file_line, "hidden=", 7))
						item_games->hidden = atoi(file_line + 7);
				}
				while (1);

				if (games == NULL)
				{
					games = item_games;
				}
				else
				{
					item_games->next = games;
					games = item_games;
				}
			}
		}
		while (1); //read of gameslist ends here

		add_games_to_listview();
		fclose(fpgames);
	}
}

void load_repos(const char* filename)
{
	STRPTR file_line = malloc(500 * sizeof(char));
	if (file_line == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	BPTR fprepos = Open((CONST_STRPTR)filename, MODE_OLDFILE);
	if (fprepos)
	{
		while (FGets(fprepos, file_line, 500))
		{
			item_repos = (repos_list *)calloc(1, sizeof(repos_list));
			item_repos->next = NULL;
			strcpy(item_repos->repo, file_line);

			if (repos == NULL)
			{
				repos = item_repos;
			}
			else
			{
				item_repos->next = repos;
				repos = item_repos;
			}

			DoMethod(app->LV_GameRepositories, MUIM_List_InsertSingle, item_repos->repo, 1, MUIV_List_Insert_Bottom);
		}

		Close(fprepos);
	}
	if (file_line)
		free(file_line);
}

void load_genres(const char* filename)
{
	STRPTR file_line = malloc(500 * sizeof(char));
	if (file_line == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	int i;
	BPTR fpgenres = Open((CONST_STRPTR)filename, MODE_OLDFILE);
	if (fpgenres)
	{
		no_of_genres = 0;
		while (FGets(fpgenres, file_line, 500))
		{
			item_genres = (genres_list *)calloc(1, sizeof(genres_list));
			item_genres->next = NULL;
			strcpy(item_genres->genre, file_line);

			if (genres == NULL)
			{
				genres = item_genres;
			}
			else
			{
				item_genres->next = genres;
				genres = item_genres;
			}

			no_of_genres++;
			DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, item_genres->genre, MUIV_List_Insert_Sorted);
		}

		for (i = 0; i < no_of_genres; i++)
		{
			DoMethod(app->LV_GenresList, MUIM_List_GetEntry, i + 5, &app->CY_PropertiesGenreContent[i]);
		}

		app->CY_PropertiesGenreContent[i] = (unsigned char*)GetMBString(MSG_UnknownGenre);
		app->CY_PropertiesGenreContent[i + 1] = NULL;
		set(app->CY_PropertiesGenre, MUIA_Cycle_Entries, app->CY_PropertiesGenreContent);

		Close(fpgenres);
	}
	if (file_line)
		free(file_line);
}

void add_default_filters()
{
	DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, GetMBString(MSG_FilterShowAll), MUIV_List_Insert_Bottom);
	DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, GetMBString(MSG_FilterFavorites), MUIV_List_Insert_Bottom);
	DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, GetMBString(MSG_FilterLastPlayed), MUIV_List_Insert_Bottom);
	DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, GetMBString(MSG_FilterMostPlayed), MUIV_List_Insert_Bottom);
	DoMethod(app->LV_GenresList, MUIM_List_InsertSingle, GetMBString(MSG_FilterNeverPlayed), MUIV_List_Insert_Bottom);
}

void app_start()
{
	load_games_list(DEFAULT_GAMELIST_FILE);
	load_repos(DEFAULT_REPOS_FILE);
	add_default_filters();
	load_genres(DEFAULT_GENRES_FILE);

	IntroPic = 1;

	set(app->WI_MainWindow, MUIA_Window_Open, TRUE);
	set(app->WI_MainWindow, MUIA_Window_ActiveObject, app->LV_GamesList);
}

void clear_gameslist()
{
	// Erase list
	DoMethod(app->LV_GamesList, MUIM_List_Clear);
	set(app->LV_GamesList, MUIA_List_Quiet, TRUE);
}

void list_show_all(char* str)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	clear_gameslist();
	total_games = 0;

	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);
				for (int i = 0; i <= length - 1; i++) helper[i] = tolower(helper[i]);

				if (strstr(helper, (char *)str))
				{
					if (item_games->hidden != 1)
					{
						DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title,
						         MUIV_List_Insert_Sorted);
						total_games++;
					}
				}
			}
		}
	}

	status_show_total();

	if (helper)
		free(helper);
}

void list_show_favorites(char* str)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	clear_gameslist();
	total_games = 0;

	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);
				for (int i = 0; i <= length - 1; i++) helper[i] = tolower(helper[i]);

				if (item_games->favorite == 1 && item_games->hidden != 1 && strstr(helper, (char *)str))
				{
					DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
					total_games++;
				}
			}
		}
	}
	status_show_total();

	if (helper)
		free(helper);
}

void list_show_last_played(char* str)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	clear_gameslist();
	total_games = 0;

	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);
				for (int i = 0; i <= length - 1; i++) helper[i] = tolower(helper[i]);

				if (item_games->last_played == 1 && strstr(helper, (char *)str))
				{
					DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
					total_games++;
				}
			}
		}
	}
	status_show_total();

	if (helper)
		free(helper);
}

void list_show_most_played(char* str)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	int max = 0;
	clear_gameslist();
	total_games = 0;

	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);

				for (int i = 0; i <= length - 1; i++)
					helper[i] = tolower(helper[i]);

				if (item_games->times_played && strstr(helper, (char *)str))
				{
					if (item_games->times_played >= max && item_games->hidden != 1)
					{
						max = item_games->times_played;
						DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Top);
						total_games++;
					}
					else
					{
						DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title,
						         MUIV_List_Insert_Bottom);
						total_games++;
					}
				}
			}
		}
	}
	status_show_total();

	if (helper)
		free(helper);
}

void list_show_never_played(char* str)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	clear_gameslist();
	total_games = 0;

	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);

				for (int i = 0; i <= length - 1; i++)
					helper[i] = tolower(helper[i]);

				if (item_games->times_played == 0 && item_games->hidden != 1 && strstr(helper, (char *)str))
				{
					DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
					total_games++;
				}
			}
		}
	}
	status_show_total();

	if (helper)
		free(helper);
}

void list_show_filtered(char* str, char* str_gen)
{
	char* helper = malloc(200 * sizeof(char));
	if (helper == NULL)
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	clear_gameslist();
	total_games = 0;

	// Find the entries in Games and update the list
	if (games)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				strcpy(helper, item_games->title);
				const int length = strlen(helper);
				for (int i = 0; i <= length - 1; i++)
					helper[i] = tolower(helper[i]);

				if (!strcmp(item_games->genre, str_gen) && item_games->hidden != 1 && strstr(helper, (char *)str))
				{
					DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
					total_games++;
				}
			}
		}
	}
	status_show_total();

	if (helper)
		free(helper);
}

void filter_change()
{
	char* str = NULL;
	char* str_gen = NULL;

	get(app->STR_Filter, MUIA_String_Contents, &str);
	DoMethod(app->LV_GenresList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &str_gen);

	if (str && strlen(str) != 0)
		for (int i = 0; i <= strlen((char *)str) - 1; i++)
			str[i] = tolower(str[i]);

	if (str_gen == NULL || !strcmp(str_gen, GetMBString(MSG_FilterShowAll)))
		list_show_all(str);

	else if (!strcmp(str_gen, GetMBString(MSG_FilterFavorites)))
		list_show_favorites(str);

	else if (!strcmp(str_gen, GetMBString(MSG_FilterLastPlayed)))
		list_show_last_played(str);

	else if (!strcmp(str_gen, GetMBString(MSG_FilterMostPlayed)))
		list_show_most_played(str);

	else if (!strcmp(str_gen, GetMBString(MSG_FilterNeverPlayed)))
		list_show_never_played(str);

	else
		list_show_filtered(str, str_gen);
}

void string_to_lower(char* slave)
{
	for (int i = 0; i <= strlen(slave) - 1; i++)
		slave[i] = tolower(slave[i]);
}

/*
*   Executes whdload with the slave
*/
void launch_game()
{
	struct Library* icon_base;
	struct DiskObject* disk_obj;
	char *game_title = NULL, exec[256], *tool_type;
	int success, i, whdload = 0;
	char str2[512], fullpath[800], helperstr[250], to_check[256];

	//clear vars:
	str2[0] = '\0';
	fullpath[0] = '\0';

	DoMethod(app->LV_GamesList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &game_title);
	if (game_title == NULL || strlen(game_title) == 0)
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
		return;
	}

	char* path = malloc(256 * sizeof(char));
	if (path != NULL)
		get_path(game_title, path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	sprintf(helperstr, (const char*)GetMBString(MSG_RunningGameTitle), game_title);
	set(app->TX_Status, MUIA_Text_Contents, helperstr);

	unsigned char* naked_path = malloc(256 * sizeof(char));
	if (naked_path != NULL)
		strip_path(path, (char*)naked_path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	char* slave = malloc(256 * sizeof(char));
	if (slave != NULL)
		slave = get_slave_from_path(slave, strlen(naked_path), path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}
	string_to_lower(slave);
	if (strstr(slave, ".slave"))
		whdload = 1;

	const BPTR oldlock = Lock((CONST_STRPTR)PROGDIR, ACCESS_READ);
	const BPTR lock = Lock(naked_path, ACCESS_READ);
	if (lock)
		CurrentDir(lock);
	else
	{
		msg_box((const char*)GetMBString(MSG_DirectoryNotFound));
		return;
	}

	if (whdload == 1)
		sprintf(exec, "whdload ");
	else
		strcpy(exec, path);

	//tooltypes only for whdload games
	if (whdload == 1)
	{
		if ((icon_base = (struct Library *)OpenLibrary((CONST_STRPTR)ICON_LIBRARY, 0)))
		{
			//scan the .info files in the current dir.
			//one of them should be the game's project icon.

			/*  allocate space for a FileInfoBlock */
			struct FileInfoBlock* m = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);

			success = Examine(lock, m);
			if (m->fib_DirEntryType <= 0)
			{
				/*  We don't allow "opta file", only "opta dir" */
				return;
			}

			while ((success = ExNext(lock, m)))
			{
				if (strstr(m->fib_FileName, ".info"))
				{
					NameFromLock(lock, str2, 512);
					sprintf(fullpath, "%s/%s", str2, m->fib_FileName);

					//lose the .info
					for (i = strlen(fullpath) - 1; i >= 0; i--)
					{
						if (fullpath[i] == '.')
							break;
					}
					fullpath[i] = '\0';

					if ((disk_obj = GetDiskObject((STRPTR)fullpath)))
					{
						if (MatchToolValue(FindToolType(disk_obj->do_ToolTypes, "SLAVE"), (STRPTR)slave)
							|| MatchToolValue(FindToolType(disk_obj->do_ToolTypes, "slave"), (STRPTR)slave))
						{
							for (char** tool_types = (char **)disk_obj->do_ToolTypes; (tool_type = *tool_types); ++
							     tool_types)
							{
								if (!strncmp(tool_type, "IM", 2)) continue;
								if (tool_type[0] == ' ') continue;
								if (tool_type[0] == '(') continue;
								if (tool_type[0] == '*') continue;
								if (tool_type[0] == ';') continue;
								if (tool_type[0] == '\0') continue;

								/* Must check here for numerical values */
								/* Those (starting with $ should be transformed to dec from hex) */
								char** temp_tbl = my_split((char *)tool_type, "=");
								if (temp_tbl == NULL
									|| temp_tbl[0] == NULL
									|| !strcmp((char *)temp_tbl[0], " ")
									|| !strcmp((char *)temp_tbl[0], ""))
									continue;

								if (temp_tbl[1] != NULL)
								{
									sprintf(to_check, "%s", temp_tbl[1]);
									if (to_check[0] == '$')
									{
										const int dec_rep = hex2dec(to_check);
										sprintf(tool_type, "%s=%d", temp_tbl[0], dec_rep);
									}
								}
								if (temp_tbl)
									free(temp_tbl);
								//req: more free's

								sprintf(exec, "%s %s", exec, tool_type);
							}

							FreeDiskObject(disk_obj);
							break;
						}
						FreeDiskObject(disk_obj);
					}
				}
			}

			CloseLibrary(icon_base);
		}

		//if we're still here, and exec contains just whdload, add the slave and execute
		if (!strcmp(exec, "whdload "))
		{
			sprintf(exec, "%s %s", exec, path);
		}
	}

	// Cleanup the memory allocations
	if (slave)
		free(slave);
	if (path)
		free(path);
	if (naked_path)
		free(naked_path);

	//set the counters for this game
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (!strcmp(item_games->title, game_title))
		{
			item_games->last_played = 1;
			item_games->times_played++;
		}

		if (item_games->last_played == 1 && strcmp(item_games->title, game_title))
		{
			item_games->last_played = 0;
		}
	}

	if (SAVESTATSONEXIT == 0)
		save_list(0);

	success = Execute(exec, 0, 0);

	if (success == 0)
		msg_box((const char*)GetMBString(MSG_ErrorExecutingWhdload));

	CurrentDir(oldlock);
	status_show_total();
}

/*
* Scans the repos for games
*/
void scan_repositories()
{
	char repotemp[256], helperstr[256];

	if (repos)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			//only apply the not exists hack to slaves that are in the current repos, that will be scanned later
			//Binaries (that are added through add-non-whdload, should be handled afterwards
			if (strstr(item_games->path, ".slave") || strlen(item_games->path) == 0)
				item_games->exists = 0;
			else
				item_games->exists = 1;
		}

		const BPTR currentlock = Lock((CONST_STRPTR)PROGDIR, ACCESS_READ);

		for (item_repos = repos; item_repos != NULL; item_repos = item_repos->next)
		{
			sprintf(repotemp, "%s", item_repos->repo);
			sprintf(helperstr, (const char*)GetMBString(MSG_ScanningPleaseWait), repotemp);
			set(app->TX_Status, MUIA_Text_Contents, helperstr);
			const BPTR oldlock = Lock(repotemp, ACCESS_READ);

			if (oldlock != 0)
			{
				CurrentDir(oldlock);
				followthread(oldlock, 0);
				CurrentDir(currentlock);
			}
			else
			{
				//could not lock
			}
		}

		save_list(1);
		refresh_list(1);
	}
}

void refresh_sidepanel()
{
	DoMethod(app->GR_sidepanel, MUIM_Group_InitChange);
	DoMethod(app->GR_sidepanel, OM_REMMEMBER, app->IM_GameImage_0);
	DoMethod(app->GR_sidepanel, OM_REMMEMBER, app->Space_Sidepanel);
	DoMethod(app->GR_sidepanel, OM_REMMEMBER, app->LV_GenresList);
	MUI_DisposeObject(app->IM_GameImage_0);
	DoMethod(app->GR_sidepanel, OM_ADDMEMBER, app->IM_GameImage_0 = app->IM_GameImage_1);
	DoMethod(app->GR_sidepanel, OM_ADDMEMBER, app->Space_Sidepanel);
	DoMethod(app->GR_sidepanel, OM_ADDMEMBER, app->LV_GenresList);
	DoMethod(app->GR_sidepanel, MUIM_Group_ExitChange);
}

void strip_path(const char* path, char* naked_path)
{
	int i, k;
	/* strip the path from the slave file and get the rest */
	for (i = strlen(path) - 1; i >= 0; i--)
	{
		if (path[i] == '/')
			break;
	}

	for (k = 0; k <= i - 1; k++)
		naked_path[k] = path[k];
	naked_path[k] = '\0';
}

void game_click()
{
	if (NOSIDEPANEL || NOSCREENSHOT)
		return;

	char* game_title = NULL;
	DoMethod(app->LV_GamesList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &game_title);

	if (game_title) //for some reason, game_click is called and game_title is null??
	{
		char* path = malloc(256 * sizeof(char));
		if (path != NULL)
			get_path(game_title, path);
		else
		{
			msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
			return;
		}

		char* naked_path = malloc(300 * sizeof(char));
		if (naked_path != NULL)
			strip_path(path, naked_path);
		else
		{
			msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
			return;
		}

		//Check the string, when filter is populated there is trouble
		if (strlen(naked_path) != 0)
		{
			sprintf(naked_path, "%s/igame.iff", (const char*)naked_path);
			BPTR fp = Open((CONST_STRPTR)naked_path, MODE_OLDFILE);
			if (!fp) //no igame.iff, try .info and newicons
			{
				if (strstr(path, ".slave")) //check for whdload game
				{
					path[strlen(path) - 6] = '\0';
					sprintf(naked_path, "%s.info", (const char*)path);
					fp = Open((CONST_STRPTR)naked_path, MODE_OLDFILE);
				}
			}

			if (fp)
			{
				// We don't need the file open anymore
				Close(fp);

				if (NOGUIGFX)
				{
					app->IM_GameImage_1 = MUI_NewObject(Dtpic_Classname,
					                                    MUIA_Dtpic_Name, naked_path,
					                                    MUIA_Frame, MUIV_Frame_ImageButton,
					End;
				}
				else
				{
					app->IM_GameImage_1 = GuigfxObject, MUIA_Guigfx_FileName, naked_path,
					                                  MUIA_Guigfx_Quality, MUIV_Guigfx_Quality_Best,
					                                  MUIA_Guigfx_ScaleMode, NISMF_SCALEFREE,
					                                  MUIA_Guigfx_Transparency, 0,
					                                  MUIA_Frame, MUIV_Frame_ImageButton,
					                                  MUIA_FixHeight, SS_HEIGHT,
					                                  MUIA_FixWidth, SS_WIDTH,
					End;
				}

				if (app->IM_GameImage_1)
				{
					refresh_sidepanel();
					IntroPic = 0;
				}
				else
					//in case it failed to load something, lets hope it gets picked up here and is forced to load the default igame.iff
				{
					goto loaddef;
				}
			}
			else //no pic found
			{
			loaddef:
				if (IntroPic == 0)
				{
					if (NOGUIGFX)
					{
						app->IM_GameImage_1 = (Object *)MUI_NewObject(Dtpic_Classname,
						                                              MUIA_Dtpic_Name, DEFAULT_SCREENSHOT_FILE,
						                                              MUIA_Frame, MUIV_Frame_ImageButton,
						End;
					}
					else
					{
						app->IM_GameImage_1 = GuigfxObject, MUIA_Guigfx_FileName, DEFAULT_SCREENSHOT_FILE,
						                                  MUIA_Guigfx_Quality, MUIV_Guigfx_Quality_Best,
						                                  MUIA_Guigfx_ScaleMode, NISMF_SCALEFREE,
						                                  MUIA_Guigfx_Transparency, 0,
						                                  MUIA_Frame, MUIV_Frame_ImageButton,
						                                  MUIA_FixHeight, SS_HEIGHT,
						                                  MUIA_FixWidth, SS_WIDTH,
						End;
					}
					if (app->IM_GameImage_1)
					{
						refresh_sidepanel();
						IntroPic = 1;
					}
				}
			}
			if (path)
				free(path);
			if (naked_path)
				free(naked_path);
		}
	}
}

/* Retrieves the Genre from the file, using the Title */
int get_genre(char* title, char* genre)
{
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (item_games->deleted != 1)
		{
			if (!strcmp(title, item_games->title))
			{
				strcpy(genre, item_games->genre);
				break;
			}
		}
	}

	return 0;
}

void get_path(char* title, char* path)
{
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (item_games->deleted != 1)
		{
			if (!strcmp(title, item_games->title))
			{
				strcpy(path, item_games->path);
				break;
			}
		}
	}
}

/*
* Adds a repository (path on the disk)
* to the list of repositories
*/
void repo_add()
{
	char* repo_path = NULL;
	get(app->PA_RepoPath, MUIA_String_Contents, &repo_path);

	if (repo_path && strlen(repo_path) != 0)
	{
		item_repos = (repos_list *)calloc(1, sizeof(repos_list));
		item_repos->next = NULL;

		strcpy(item_repos->repo, repo_path);

		if (repos == NULL)
			repos = item_repos;
		else
		{
			item_repos->next = repos;
			repos = item_repos;
		}

		DoMethod(app->LV_GameRepositories, MUIM_List_InsertSingle, item_repos->repo, 1, MUIV_List_Insert_Bottom);
	}
}

void repo_remove()
{
	DoMethod(app->LV_GameRepositories, MUIM_List_Remove, MUIV_List_Remove_Active);
}

/*
* Writes the repositories to the repo.prefs file
*/
void repo_stop()
{
	BPTR fprepos = Open((CONST_STRPTR)DEFAULT_REPOS_FILE, MODE_NEWFILE);
	if (!fprepos)
	{
		msg_box((const char*)GetMBString(MSG_CouldNotCreateReposFile));
	}
	else
	{
		CONST_STRPTR repo_path = NULL;
		for (int i = 0;; i++)
		{
			DoMethod(app->LV_GameRepositories, MUIM_List_GetEntry, i, &repo_path);
			if (!repo_path)
				break;

			FPuts(fprepos, repo_path);
		}
		Close(fprepos);
	}
}

int title_exists(char* game_title)
{
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (!strcmp(game_title, item_games->title) && item_games->deleted != 1)
		{
			// Title found
			return 1;
		}
	}
	// Title not found
	return 0;
}

char* get_slave_from_path(char* slave, int start, char* path)
{
	int z = 0;
	for (int k = start + 1; k <= strlen(path); k++)
	{
		slave[z] = path[k];
		z++;
	}
	return slave;
}

//shows and inits the GameProperties Window
void game_properties()
{
	int open;
	//if window is already open
	get(app->WI_Properties, MUIA_Window_Open, &open);
	if (open) return;

	char* game_title = NULL;
	//set the elements on the window
	DoMethod(app->LV_GamesList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &game_title);
	if (game_title == NULL || strlen(game_title) == 0)
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
		return;
	}

	const int found = title_exists(game_title);
	if (!found)
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
		return;
	}

	char helperstr[512];
	int i;
	struct Library* icon_base;
	struct DiskObject* disk_obj;
	char* tool_type;
	char str2[512], fullpath[800];

	set(app->STR_PropertiesGameTitle, MUIA_String_Contents, game_title);
	set(app->TX_PropertiesSlavePath, MUIA_Text_Contents, item_games->path);

	sprintf(helperstr, "%d", item_games->times_played);
	set(app->TX_PropertiesTimesPlayed, MUIA_Text_Contents, helperstr);

	//set the genre
	for (i = 0; i < no_of_genres; i++)
		if (!strcmp(app->CY_PropertiesGenreContent[i], item_games->genre))
			break;
	set(app->CY_PropertiesGenre, MUIA_Cycle_Active, i);

	if (item_games->favorite == 1)
		set(app->CH_PropertiesFavorite, MUIA_Selected, TRUE);
	else
		set(app->CH_PropertiesFavorite, MUIA_Selected, FALSE);

	if (item_games->hidden == 1)
		set(app->CH_PropertiesHidden, MUIA_Selected, TRUE);
	else
		set(app->CH_PropertiesHidden, MUIA_Selected, FALSE);

	//set up the tooltypes
	char* path = malloc(256 * sizeof(char));
	if (path != NULL)
		get_path(game_title, path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	char* naked_path = malloc(256 * sizeof(char));
	if (naked_path != NULL)
		strip_path(path, naked_path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	char* slave = malloc(256 * sizeof(char));
	if (slave != NULL)
		slave = get_slave_from_path(slave, strlen(naked_path), path);
	else
	{
		msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
		return;
	}

	string_to_lower(slave);

	const BPTR oldlock = Lock((CONST_STRPTR)PROGDIR, ACCESS_READ);
	const BPTR lock = Lock((CONST_STRPTR)naked_path, ACCESS_READ);
	if (lock)
		CurrentDir(lock);
	else
	{
		msg_box((const char*)GetMBString(MSG_DirectoryNotFound));
		return;
	}

	memset(&gamestooltypes[0], 0, sizeof gamestooltypes);

	if ((icon_base = (struct Library *)OpenLibrary((CONST_STRPTR)ICON_LIBRARY, 0)))
	{
		//scan the .info files in the current dir.
		//one of them should be the game's project icon.

		/*  allocate space for a FileInfoBlock */
		struct FileInfoBlock* m = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);

		int success = Examine(lock, m);
		if (m->fib_DirEntryType <= 0)
		{
			/*  We don't allow "opta file", only "opta dir" */
			return;
		}

		while ((success = ExNext(lock, m)))
		{
			if (strstr(m->fib_FileName, ".info"))
			{
				NameFromLock(lock, str2, 512);
				sprintf(fullpath, "%s/%s", str2, m->fib_FileName);

				//lose the .info
				for (i = strlen(fullpath) - 1; i >= 0; i--)
				{
					if (fullpath[i] == '.')
						break;
				}
				fullpath[i] = '\0';

				if ((disk_obj = GetDiskObject((STRPTR)fullpath)))
				{
					if (MatchToolValue(FindToolType(disk_obj->do_ToolTypes, (STRPTR)SLAVE_STRING), (STRPTR)slave))
					{
						for (char** tool_types = (char **)disk_obj->do_ToolTypes; (tool_type = *tool_types); ++
						     tool_types)
						{
							if (!strncmp(tool_type, "IM", 2))
								continue;

							sprintf(gamestooltypes, "%s%s\n", gamestooltypes, tool_type);
							set(app->TX_PropertiesTooltypes, MUIA_TextEditor_ReadOnly, FALSE);
						}

						FreeDiskObject(disk_obj);
						break;
					}
					FreeDiskObject(disk_obj);
				}
			}
		}

		CloseLibrary(icon_base);
	}

	// Cleanup the memory allocations
	if (slave)
		free(slave);
	if (path)
		free(path);
	if (naked_path)
		free(naked_path);

	if (strlen(gamestooltypes) == 0)
	{
		sprintf(gamestooltypes, "No .info file");
		set(app->TX_PropertiesTooltypes, MUIA_TextEditor_ReadOnly, TRUE);
	}

	set(app->TX_PropertiesTooltypes, MUIA_TextEditor_Contents, gamestooltypes);
	CurrentDir(oldlock);
	set(app->WI_Properties, MUIA_Window_Open, TRUE);
}

//when ok is pressed in GameProperties
void game_properties_ok()
{
	char* game_title = NULL;
	char* path = NULL;
	int fav = 0, genre = 0, hid = 0;
	struct Library* icon_base;
	struct DiskObject* disk_obj;
	char* tool_type;
	int i;
	int new_tool_type_count = 1, old_tool_type_count = 0, old_real_tool_type_count = 0;
	char str2[512], fullpath[800];

	get(app->STR_PropertiesGameTitle, MUIA_String_Contents, &game_title);
	get(app->TX_PropertiesSlavePath, MUIA_Text_Contents, &path);
	get(app->CY_PropertiesGenre, MUIA_Cycle_Active, &genre);
	get(app->CH_PropertiesFavorite, MUIA_Selected, &fav);
	get(app->CH_PropertiesHidden, MUIA_Selected, &hid);


	//find the entry, and update it:
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (!strcmp(path, item_games->path))
		{
			if (strcmp(item_games->title, game_title))
			{
				//check dup for title
				if (check_dup_title(game_title))
				{
					msg_box((const char*)GetMBString(MSG_TitleAlreadyExists));
					return;
				}
			}
			strcpy(item_games->title, game_title);
			strcpy(item_games->genre, app->CY_PropertiesGenreContent[genre]);
			if (fav == 1) item_games->favorite = 1;
			else item_games->favorite = 0;

			//if it was previously not hidden, hide now
			if (hid == 1 && item_games->hidden != 1)
			{
				item_games->hidden = 1;
				DoMethod(app->LV_GamesList, MUIM_List_Remove, MUIV_List_Remove_Selected);
				total_games = total_games - 1;
				status_show_total();
			}

			if (hid == 0 && item_games->hidden == 1)
			{
				item_games->hidden = 0;
				DoMethod(app->LV_GamesList, MUIM_List_Remove, MUIV_List_Remove_Selected);
				total_hidden--;
				status_show_total();
			}

			break;
		}
	}

	//update the games tooltypes
	STRPTR tools = (STRPTR)DoMethod(app->TX_PropertiesTooltypes, MUIM_TextEditor_ExportText);

	//tooltypes changed
	if (strcmp((char *)tools, gamestooltypes))
	{
		char* naked_path = malloc(256 * sizeof(char));
		if (naked_path != NULL)
			strip_path(path, naked_path);
		else
		{
			msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
			return;
		}
		strip_path(path, naked_path);
		char* slave = malloc(256 * sizeof(char));
		if (slave != NULL)
			get_slave_from_path(slave, strlen(naked_path), path);
		else
		{
			msg_box((const char*)GetMBString(MSG_NotEnoughMemory));
			return;
		}
		string_to_lower(slave);

		BPTR oldlock = Lock((CONST_STRPTR)PROGDIR, ACCESS_READ);
		const BPTR lock = Lock((CONST_STRPTR)naked_path, ACCESS_READ);
		CurrentDir(lock);

		if ((icon_base = (struct Library *)OpenLibrary((CONST_STRPTR)ICON_LIBRARY, 0)))
		{
			//scan the .info files in the current dir.
			//one of them should be the game's project icon.

			/*  allocate space for a FileInfoBlock */
			struct FileInfoBlock* m = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);

			int success = Examine(lock, m);
			if (m->fib_DirEntryType <= 0)
			{
				/*  We don't allow "opta file", only "opta dir" */
				return;
			}

			while ((success = ExNext(lock, m)))
			{
				if (strstr(m->fib_FileName, ".info"))
				{
					NameFromLock(lock, str2, 512);
					sprintf(fullpath, "%s/%s", str2, m->fib_FileName);

					//lose the .info
					for (i = strlen(fullpath) - 1; i >= 0; i--)
					{
						if (fullpath[i] == '.')
							break;
					}
					fullpath[i] = '\0';

					if ((disk_obj = GetDiskObject((STRPTR)fullpath)))
					{
						if (MatchToolValue(FindToolType(disk_obj->do_ToolTypes, (STRPTR)SLAVE_STRING), (STRPTR)slave))
						{
							//check numbers for old and new tooltypes
							for (i = 0; i <= strlen(tools); i++)
								if (tools[i] == '\n') new_tool_type_count++;

							//add one for the last tooltype that doesnt end with \n
							new_tool_type_count++;

							for (i = 0; i <= strlen(gamestooltypes); i++)
								if (gamestooltypes[i] == '\n') old_tool_type_count++;

							for (char** tool_types = (char **)disk_obj->do_ToolTypes; (tool_type = *tool_types); ++
							     tool_types)
								old_real_tool_type_count++;

							unsigned char** new_tool_types = AllocVec(new_tool_type_count * sizeof(char *),
							                                          MEMF_FAST | MEMF_CLEAR);
							unsigned char** newptr = new_tool_types;

							char** temp_tbl = my_split((char *)tools, "\n");
							if (temp_tbl == NULL
								|| temp_tbl[0] == NULL
								|| !strcmp((char *)temp_tbl[0], " ")
								|| !strcmp((unsigned char *)temp_tbl[0], ""))
								break;

							for (i = 0; i <= new_tool_type_count - 2; i++)
								*newptr++ = (unsigned char*)temp_tbl[i];

							*newptr = NULL;

							disk_obj->do_ToolTypes = new_tool_types;
							PutDiskObject((STRPTR)fullpath, disk_obj);
							FreeDiskObject(disk_obj);

							if (temp_tbl)
								free(temp_tbl);
							if (new_tool_types)
								free(new_tool_types);

							break;
						}
						FreeDiskObject(disk_obj);
					}
				}
			}

			CloseLibrary(icon_base);
		}
		// Cleanup the memory allocations
		if (slave)
			free(slave);
		if (path)
			free(path);
		if (naked_path)
			free(naked_path);
	}
	FreeVec(tools);

	DoMethod(app->LV_GamesList, MUIM_List_Redraw, MUIV_List_Redraw_Active);
	set(app->WI_Properties, MUIA_Window_Open, FALSE);
	save_list(0);
}

void list_show_hidden()
{
	if (showing_hidden == 0)
	{
		set(app->LV_GamesList, MUIA_List_Quiet, TRUE);
		total_hidden = 0;
		DoMethod(app->LV_GamesList, MUIM_List_Clear);

		set(app->LV_GenresList, MUIA_Disabled, TRUE);
		set(app->STR_Filter, MUIA_Disabled, TRUE);
		if (games)
		{
			/* Find the entries in Games and update the list */
			for (item_games = games; item_games != NULL; item_games = item_games->next)
			{
				if (item_games->hidden == 1 && item_games->deleted != 1)
				{
					DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
					total_hidden++;
				}
			}
		}

		status_show_total();
		showing_hidden = 1;
	}
	else
	{
		set(app->LV_GenresList, MUIA_Disabled, FALSE);
		set(app->STR_Filter, MUIA_Disabled, FALSE);
		showing_hidden = 0;
		refresh_list(0);
	}
}

void app_stop()
{
	if (SAVESTATSONEXIT == 1)
		save_list(0);

	memset(&fname[0], 0, sizeof fname);
	memset(&gamestooltypes[0], 0, sizeof gamestooltypes);

	if (games)
	{
		free(games);
		games = NULL;
	}
	if (repos)
	{
		free(repos);
		repos = NULL;
	}
	if (genres)
	{
		free(genres);
		genres = NULL;
	}
}

void genres_click()
{
	filter_change();
}

/*
* Splits a string using spl
*/
char** my_split(char* str, char* spl)
{
	char **ret, *buffer[256], buf[4096];
	int i;

	if (!spl)
	{
		ret = (char **)malloc(2 * sizeof(char *));
		ret[0] = (char *)strdup(str);
		ret[1] = NULL;
		return (ret);
	}

	int count = 0;

	char* fptr = str;
	const int spl_len = strlen(spl);
	char* sptr = strstr(fptr, spl);
	while (sptr)
	{
		i = sptr - fptr;
		memcpy(buf, fptr, i);
		buf[i] = '\0';
		buffer[count++] = (char *)strdup(buf);
		fptr = sptr + spl_len;
		sptr = strstr(fptr, spl);
	}
	sptr = strchr(fptr, '\0');
	i = sptr - fptr;
	memcpy(buf, fptr, i);
	buf[i] = '\0';
	buffer[count++] = (char *)strdup(buf);

	ret = (char **)malloc((count + 1) * sizeof(char *));

	for (i = 0; i < count; i++)
	{
		ret[i] = buffer[i];
	}
	ret[count] = NULL;

	return ret;
}

void followthread(BPTR lock, int tab_level)
{
	int exists = 0, j;
	char str[512], fullpath[512], temptitle[256];

	/*  if at the end of the road, don't print anything */
	if (!lock)
		return;

	/*  allocate space for a FileInfoBlock */
	struct FileInfoBlock* m = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);

	int success = Examine(lock, m);
	if (m->fib_DirEntryType <= 0)
		return;

	/*  The first call to Examine fills the FileInfoBlock with information
	about the directory.  If it is called at the root level, it contains
	the volume name of the disk.  Thus, this program is only printing
	the output of ExNext rather than both Examine and ExNext.  If it
	printed both, it would list directory entries twice! */

	while ((success = ExNext(lock, m)))
	{
		/*  Print what we've got: */
		if (m->fib_DirEntryType > 0)
		{
		}

		//make m->fib_FileName to lower
		const int kp = strlen((char *)m->fib_FileName);
		for (int s = 0; s < kp; s++) m->fib_FileName[s] = tolower(m->fib_FileName[s]);
		if (strstr(m->fib_FileName, ".slave"))
		{
			NameFromLock(lock, str, 511);
			sprintf(fullpath, "%s/%s", str, m->fib_FileName);

			/* add the slave to the gameslist (if it does not already exist) */
			for (item_games = games; item_games != NULL; item_games = item_games->next)
			{
				if (!strcmp(item_games->path, fullpath))
				{
					exists = 1;
					item_games->exists = 1;
					break;
				}
			}
			if (exists == 0)
			{
				item_games = (games_list *)calloc(1, sizeof(games_list));
				item_games->next = NULL;

				/* strip the path from the slave file and get the rest */
				for (j = strlen(str) - 1; j >= 0; j--)
				{
					if (str[j] == '/' || str[j] == ':')
						break;
				}

				//CHANGE 2007-12-03: init n=0 here
				int n = 0;
				for (int k = j + 1; k <= strlen(str) - 1; k++)
					temptitle[n++] = str[k];
				temptitle[n] = '\0';

				if (TITLESFROMDIRS)
				{
					// If the TITLESFROMDIRS tooltype is enabled, set Titles from Directory names
					const char* title = get_directory_name(fullpath);
					if (title != NULL)
					{
						if (NOSMARTSPACES)
						{
							strcpy(item_games->title, title);
						}
						else
						{
							const char* title_with_spaces = add_spaces_to_string(title);
							strcpy(item_games->title, title_with_spaces);
						}
					}
				}
				else
				{
					// Default behavior: set Titles by the .slave contents
					if (get_title_from_slave(fullpath, item_games->title))
						strcpy(item_games->title, temptitle);
				}

				while (check_dup_title(item_games->title))
				{
					strcat(item_games->title, " Alt");
				}

				strcpy(item_games->genre, GetMBString(MSG_UnknownGenre));
				strcpy(item_games->path, fullpath);
				item_games->favorite = 0;
				item_games->times_played = 0;
				item_games->last_played = 0;
				item_games->exists = 1;
				item_games->hidden = 0;

				if (games == NULL)
				{
					games = item_games;
				}
				else
				{
					item_games->next = games;
					games = item_games;
				}
			}
			exists = 0;
		}

		/*  If we have a directory, then enter it: */
		if (m->fib_DirEntryType > 0)
		{
			/*  Since it is a directory, get a lock on it: */
			const BPTR newlock = Lock((CONST_STRPTR)m->fib_FileName, ACCESS_READ);

			/*  cd to this directory: */
			const BPTR oldlock = CurrentDir(newlock);

			/*  recursively follow the thread down to the bottom: */
			followthread(newlock, tab_level + 1);

			/*  cd back to where we used to be: */
			CurrentDir(oldlock);

			/*  Unlock the directory we just visited: */
			if (newlock)
				UnLock(newlock);
		}
	}
	FreeMem(m, sizeof(struct FileInfoBlock));
}

void refresh_list(const int check_exists)
{
	DoMethod(app->LV_GamesList, MUIM_List_Clear);
	total_games = 0;
	set(app->LV_GamesList, MUIA_List_Quiet, TRUE);

	if (check_exists == 0)
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (strlen(item_games->path) != 0
				&& item_games->hidden != 1
				&& item_games->deleted != 1)
			{
				total_games++;
				DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
			}
		}
	}
	else
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (strlen(item_games->path) != 0
				&& item_games->hidden != 1
				&& item_games->exists == 1
				&& item_games->deleted != 1)
			{
				total_games++;
				DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
			}
			else
			{
				printf("[%s] [%s] [%d]\n", item_games->title, item_games->path, item_games->exists);
			}
		}
	}

	status_show_total();
}

BOOL get_filename(const char* title, const char* positive_text, const BOOL save_mode)
{
	BOOL result = FALSE;
	if ((request = MUI_AllocAslRequest(ASL_FileRequest, NULL)) != NULL)
	{
		if (MUI_AslRequestTags(request,
		                       ASLFR_TitleText, title,
		                       ASLFR_PositiveText, positive_text,
		                       ASLFR_DoSaveMode, save_mode,
		                       ASLFR_InitialDrawer, PROGDIR,
		                       TAG_DONE))
		{
			memset(&fname[0], 0, sizeof fname);
			strcat(fname, request->rf_Dir);
			if (fname[strlen(fname) - 1] != (UBYTE)58) /* Check for : */
				strcat(fname, "/");
			strcat(fname, request->rf_File);

			result = TRUE;
		}

		if (request)
			MUI_FreeAslRequest(request);
	}

	return result;
}

/*
* Saves the current Games struct to disk
*/
void save_to_file(const char* filename, const int check_exists)
{
	const char* saving_message = (const char*)GetMBString(MSG_SavingGamelist);
	set(app->TX_Status, MUIA_Text_Contents, saving_message);

	FILE* fpgames = fopen(filename, "w");
	if (!fpgames)
	{
		msg_box((const char*)GetMBString(MSG_FailedOpeningGameslist));
	}
	else
	{
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (item_games->deleted != 1)
			{
				//printf("Saving: %s\n", item_games->Title);
				if (check_exists == 1)
				{
					if (item_games->exists == 1)
					{
						fprintf(fpgames, "index=%d\n", item_games->index);
						fprintf(fpgames, "title=%s\n", item_games->title);
						fprintf(fpgames, "genre=%s\n", item_games->genre);
						fprintf(fpgames, "path=%s\n", item_games->path);
						fprintf(fpgames, "favorite=%d\n", item_games->favorite);
						fprintf(fpgames, "timesplayed=%d\n", item_games->times_played);
						fprintf(fpgames, "lastplayed=%d\n", item_games->last_played);
						fprintf(fpgames, "hidden=%d\n\n", item_games->hidden);
					}
					else
					{
						strcpy(item_games->path, "");
					}
				}
				else
				{
					fprintf(fpgames, "index=%d\n", item_games->index);
					fprintf(fpgames, "title=%s\n", item_games->title);
					fprintf(fpgames, "genre=%s\n", item_games->genre);
					fprintf(fpgames, "path=%s\n", item_games->path);
					fprintf(fpgames, "favorite=%d\n", item_games->favorite);
					fprintf(fpgames, "timesplayed=%d\n", item_games->times_played);
					fprintf(fpgames, "lastplayed=%d\n", item_games->last_played);
					fprintf(fpgames, "hidden=%d\n\n", item_games->hidden);
				}
			}
		}
		fflush(fpgames);
		fclose(fpgames);
	}

	saving_message = "Done.";
	set(app->TX_Status, MUIA_Text_Contents, saving_message);
}

void save_list(const int check_exists)
{
	save_to_file(DEFAULT_GAMELIST_FILE, check_exists);
}

void save_list_as()
{
	//TODO Check if file exists, warn the user about overwriting it
	if (get_filename("Save List As...", "Save", TRUE))
		save_to_file(fname, 0);
}

void export_list()
{
	//TODO implement this
	msg_box("This feature is not yet implemented...");
}

void open_list()
{
	if (get_filename("Open List", "Open", FALSE))
	{
		clear_gameslist();
		load_games_list(fname);
	}
}

//function to return the dec eq of a hex no.
int hex2dec(char* hexin)
{
	int dec;
	//lose the first $ character
	hexin++;
	sscanf(hexin, "%x", &dec);
	return dec;
}

int getlistindex(Object* obj)
{
	int index = 0;
	get(obj, MUIA_List_Active, &index);
	return index;
}

void game_duplicate()
{
	char* str = NULL;
	DoMethod(app->LV_GamesList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &str);

	if (str == NULL || strlen(str) == 0)
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
		return;
	}

	int found = 0;
	for (item_games = games; item_games != NULL; item_games = item_games->next)
	{
		if (!strcmp(str, item_games->title) && item_games->deleted != 1)
		{
			found = 1;
			break;
		}
	}

	if (!found)
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
		return;
	}

	char title_copy[200];
	char path_copy[256];
	char genre_copy[30];
	strcpy(title_copy, item_games->title);
	strcat(title_copy, " copy");
	strcpy(path_copy, item_games->path);
	strcpy(genre_copy, item_games->genre);

	item_games = (games_list *)calloc(1, sizeof(games_list));
	item_games->next = NULL;
	item_games->index = 0;
	item_games->exists = 0;
	item_games->deleted = 0;
	strcpy(item_games->title, title_copy);
	strcpy(item_games->genre, genre_copy);
	strcpy(item_games->path, path_copy);
	item_games->favorite = 0;
	item_games->times_played = 0;
	item_games->last_played = 0;
	item_games->hidden = 0;

	if (games == NULL)
		games = item_games;
	else
	{
		item_games->next = games;
		games = item_games;
	}

	total_games++;
	DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
	status_show_total();
}

void game_delete()
{
	char* str = NULL;
	if (getlistindex(app->LV_GamesList) >= 0)
	{
		DoMethod(app->LV_GamesList, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &str);
		for (item_games = games; item_games != NULL; item_games = item_games->next)
		{
			if (!strcmp(str, item_games->title))
			{
				item_games->deleted = 1;
				break;
			}
		}

		DoMethod(app->LV_GamesList, MUIM_List_Remove, MUIV_List_Remove_Active);

		total_games--;
		status_show_total();
	}
	else
	{
		msg_box((const char*)GetMBString(MSG_SelectGameFromList));
	}
}

void setting_filter_use_enter_changed()
{
	//TODO
}

void setting_save_stats_on_exit_changed()
{
	//TODO
}

void setting_smart_spaces_changed()
{
	//TODO
}

void setting_titles_from_changed()
{
	//TODO
}

void setting_show_screenshot_changed()
{
	//TODO
}

void setting_use_gui_gfx_changed()
{
	//TODO
}

void setting_screenshot_size_changed()
{
	//TODO
}

void settings_save()
{
	//TODO
}

void setttings_use()
{
	//TODO
}

void setting_hide_side_panel_changed()
{
	//TODO
}

void msg_box(const char* msg)
{
	msgbox.es_StructSize = sizeof msgbox;
	msgbox.es_Flags = 0;
	msgbox.es_Title = (UBYTE*)GetMBString(MSG_WI_MainWindow);
	msgbox.es_TextFormat = (unsigned char*)msg;
	msgbox.es_GadgetFormat = (UBYTE*)GetMBString(MSG_BT_AboutOK);

	EasyRequest(NULL, &msgbox, NULL);
}

void get_screen_size(int* width, int* height)
{
	struct Screen* wbscreen;
	struct Library* intuition_base;
	struct Library* gfx_base;

	if ((intuition_base = (struct Library *)OpenLibrary((CONST_STRPTR)INTUITION_LIBRARY, 36)))
	{
		if ((gfx_base = (struct Library *)OpenLibrary((CONST_STRPTR)GRAPHICS_LIBRARY, 36)))
		{
			if ((wbscreen = LockPubScreen((CONST_STRPTR)WB_PUBSCREEN_NAME)))
			{
				/* Using intuition.library/GetScreenDrawInfo(), we get the pen
				* array we'll use for the screen clone the easy way. */
				struct DrawInfo* drawinfo = GetScreenDrawInfo(wbscreen);

				struct ViewPort* vp = &wbscreen->ViewPort;
				/* Use graphics.library/GetVPModeID() to get the ModeID of the
				* Workbench screen. */
				if (GetVPModeID(vp) != (long)INVALID_ID)
				{
					*width = wbscreen->Width;
					*height = wbscreen->Height;
				}
				else
				{
					*width = -1;
					*height = -1;
				}

				FreeScreenDrawInfo(wbscreen, drawinfo);
				UnlockPubScreen(NULL, wbscreen);
			}
			else
			{
				*width = -1;
				*height = -1;
			}
			CloseLibrary(gfx_base);
		}
		CloseLibrary(intuition_base);
	}
}

void read_tool_types()
{
	struct Library* icon_base;
	struct DiskObject* disk_obj;

	int screen_width, screen_height;

	SS_HEIGHT = -1;
	SS_WIDTH = -1;
	NOGUIGFX = 0;
	FILTERUSEENTER = 0;
	NOSCREENSHOT = 0;
	SAVESTATSONEXIT = 0;
	TITLESFROMDIRS = 0;
	NOSMARTSPACES = 0;
	NOSIDEPANEL = 0;

	if ((icon_base = (struct Library *)OpenLibrary((CONST_STRPTR)ICON_LIBRARY, 0)))
	{
		char* filename = strcat(PROGDIR, executable_name);
		if ((disk_obj = GetDiskObject((STRPTR)filename)))
		{
			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_SCREENSHOT))
			{
				char** tool_types = (char **)disk_obj->do_ToolTypes;
				char* tool_type = *tool_types;

				char** temp_tbl = my_split((char *)tool_type, "=");
				if (temp_tbl == NULL
					|| temp_tbl[0] == NULL
					|| !strcmp(temp_tbl[0], " ")
					|| !strcmp(temp_tbl[0], ""))
				{
					msg_box((const char*)GetMBString(MSG_BadTooltype));
					exit(0);
				}

				if (temp_tbl[1] != NULL)
				{
					char** temp_tbl2 = my_split((char *)temp_tbl[1], "x");
					if (temp_tbl2[0]) SS_WIDTH = atoi((char *)temp_tbl2[0]);
					if (temp_tbl2[1]) SS_HEIGHT = atoi((char *)temp_tbl2[1]);

					free(temp_tbl2[0]);
					free(temp_tbl2[1]);
					free(temp_tbl2);
					free(temp_tbl[0]);
					free(temp_tbl[1]);
					free(temp_tbl);
				}
			}

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_NOGUIGFX))
				NOGUIGFX = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_FILTERUSEENTER))
				FILTERUSEENTER = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_NOSCREENSHOT))
				NOSCREENSHOT = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_SAVESTATSONEXIT))
				SAVESTATSONEXIT = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_TITLESFROMDIRS))
				TITLESFROMDIRS = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_NOSMARTSPACES))
				NOSMARTSPACES = 1;

			if (FindToolType(disk_obj->do_ToolTypes, (STRPTR)TOOLTYPE_NOSIDEPANEL))
				NOSIDEPANEL = 1;
		}

		CloseLibrary(icon_base);
	}

	if (!NOSIDEPANEL)
	{
		//check screen res and adjust image box accordingly
		if (SS_HEIGHT == -1 && SS_WIDTH == -1)
		{
			get_screen_size(&screen_width, &screen_height);

			//if values are ok from the previous function, and user has not provided his own values, calculate a nice size
			if (screen_width != -1 && screen_height != -1)
			{
				//for hi-res screens (800x600 or greater) we'll use 320x256
				if (screen_width >= 800 && screen_height >= 600)
				{
					SS_WIDTH = 320;
					SS_HEIGHT = 256;
				}
				else
				{
					// for anything less, we'll go with half that
					SS_WIDTH = 160;
					SS_HEIGHT = 128;
				}
			}
			else
			{
				SS_WIDTH = 160;
				SS_HEIGHT = 128;
			}
		}
	}
}

void add_non_whdload()
{
	set(app->STR_AddTitle, MUIA_String_Contents, NULL);
	set(app->PA_AddGame, MUIA_String_Contents, NULL);
	set(app->WI_AddNonWHDLoad, MUIA_Window_Open, TRUE);
}

void non_whdload_ok()
{
	char *str, *str_title;

	get(app->PA_AddGame, MUIA_String_Contents, &str);
	get(app->STR_AddTitle, MUIA_String_Contents, &str_title);

	if (strlen(str_title) == 0)
	{
		msg_box((const char*)GetMBString(MSG_NoTitleSpecified));
		return;
	}
	if (strlen(str) == 0)
	{
		msg_box((const char*)GetMBString(MSG_NoExecutableSpecified));
		return;
	}

	//add the game to the list
	item_games = (games_list *)calloc(1, sizeof(games_list));
	item_games->next = NULL;
	item_games->index = 0;
	strcpy(item_games->title, (char *)str_title);
	strcpy(item_games->genre, GetMBString(MSG_UnknownGenre));
	strcpy(item_games->path, (char *)str);
	item_games->favorite = 0;
	item_games->times_played = 0;
	item_games->last_played = 0;

	if (games == NULL)
	{
		games = item_games;
	}
	else
	{
		item_games->next = games;
		games = item_games;
	}

	DoMethod(app->LV_GamesList, MUIM_List_InsertSingle, item_games->title, MUIV_List_Insert_Sorted);
	total_games++;
	status_show_total();

	save_list(0);

	set(app->WI_AddNonWHDLoad, MUIA_Window_Open, FALSE);
}

/*
* Gets title from a slave file
* returns 0 on success, 1 on fail
*/
int get_title_from_slave(char* slave, char* title)
{
	char slave_title[100];

	struct slave_info
	{
		unsigned long security;
		char id[8];
		unsigned short version;
		unsigned short flags;
		unsigned long base_mem_size;
		unsigned long exec_install;
		unsigned short game_loader;
		unsigned short current_dir;
		unsigned short dont_cache;
		char keydebug;
		char keyexit;
		unsigned long exp_mem;
		unsigned short name;
		unsigned short copy;
		unsigned short info;
	};

	struct slave_info sl;

	FILE* fp = fopen(slave, "rbe");
	if (fp == NULL)
	{
		return 1;
	}

	//seek to +0x20
	fseek(fp, 32, SEEK_SET);
	fread(&sl, 1, sizeof sl, fp);

	//sl.Version = (sl.Version>>8) | (sl.Version<<8);
	//sl.name = (sl.name>>8) | (sl.name<<8);

	//printf ("[%s] [%d]\n", sl.ID, sl.Version);

	//sl.name holds the offset for the slave name
	fseek(fp, sl.name + 32, SEEK_SET);
	//title = calloc (1, 100);
	//fread (title, 1, 100, fp);

	if (sl.version < 10)
	{
		return 1;
	}

	for (int i = 0; i <= 99; i++)
	{
		slave_title[i] = fgetc(fp);
		if (slave_title[i] == '\n')
		{
			slave_title[i] = '\0';
			break;
		}
	}

	strcpy(title, slave_title);
	fclose(fp);

	return 0;
}

/*
* Checks if the title already exists
* returns 1 if yes, 0 otherwise
*/
int check_dup_title(char* title)
{
	for (games_list* check_games = games; check_games != NULL; check_games = check_games->next)
	{
		if (!strcmp(check_games->title, title))
		{
			return 1;
		}
	}
	return 0;
}

int get_delimiter_position(const char* str)
{
	char* delimiter = strrchr(str, '/');
	if (!delimiter)
		delimiter = strrchr(str, ':');

	if (!delimiter)
	{
		return 0;
	}

	const int pos = delimiter - str;
	return pos;
}

// Get the Directory part from a full path containing a file
const char* get_directory_name(const char* str)
{
	int pos1 = get_delimiter_position(str);
	if (!pos1)
		return NULL;

	char full_path[100];
	strncpy(full_path, str, pos1);
	full_path[pos1] = '\0';

	const int pos2 = get_delimiter_position(full_path);
	if (!pos2)
		return NULL;

	char* dir_name = malloc(sizeof full_path);
	int c = 0;
	for (unsigned int i = pos2 + 1; i <= sizeof full_path; i++)
	{
		dir_name[c] = full_path[i];
		c++;
	}
	dir_name[c] = '\0';

	return dir_name;
}

// Get the application's executable name
const char* get_executable_name(int argc, char** argv)
{
	// argc is zero when run from the Workbench,
	// positive when run from the CLI
	if (argc == 0)
	{
		// in AmigaOS, argv is a pointer to the WBStartup message
		// when argc is zero (run under the Workbench)
		struct WBStartup* argmsg = (struct WBStartup *)argv;
		struct WBArg* wb_arg = argmsg->sm_ArgList; /* head of the arg list */

		executable_name = wb_arg->wa_Name;
	}
	else
	{
		// Run from the CLI
		executable_name = argv[0];
	}

	return executable_name;
}

// Add spaces to a string, based on letter capitalization and numbers
// E.g. input "A10TankKiller2Disk" -> "A10 Tank Killer 2 Disk"
const char* add_spaces_to_string(const char* input)
{
	char input_string[100];
	strcpy(input_string, input);
	char* output = (char*)malloc(sizeof input_string * 2);

	// Special case for the first character, we don't touch it
	output[0] = input_string[0];

	unsigned int output_index = 1;
	unsigned int input_index = 1;
	unsigned int capital_found = 0;
	unsigned int number_found = 0;
	while (input_string[input_index])
	{
		if (isspace(input_string[input_index]))
			return input;

		if (isdigit(input_string[input_index]))
		{
			if (number_found < input_index - 1)
			{
				output[output_index] = ' ';
				output_index++;
				output[output_index] = input_string[input_index];
			}
			number_found = input_index;
		}

		else if (isupper(input_string[input_index]))
		{
			if (capital_found < input_index - 1)
			{
				output[output_index] = ' ';
				output_index++;
				output[output_index] = input_string[input_index];
			}
			capital_found = input_index;
		}

		output[output_index] = input_string[input_index];
		output_index++;
		input_index++;
	}
	output[output_index] = '\0';

	return output;
}
