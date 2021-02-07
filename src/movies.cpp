/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2021 Julian Xhokaxhiu                                   //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#include "movies.h"
#include "renderer.h"
#include "gl.h"
#include "patch.h"
#include "field.h"
#include "ff7/defs.h"
#include "video/movies.h"

// Required by > 15 FPS movies
short movie_fps_ratio = 1;
int (*old_ofst)();
int (*old_asped)();
int (*old_msped)();
int (*old_pmvie)();
int (*old_canm1)();
int (*old_canm2)();
int (*old_anim1)();
int (*old_anim2)();
int (*old_anime1)();
int (*old_anime2)();
int (*old_dfanm)();
int (*old_wait)();
int (*old_mvief)();

int script_WAIT()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
	{
		if (
			// Some scenes do not need this patch, it makes it worser
			*ff7_externals.field_id != 133 &&
			*ff7_externals.field_id != 240
		)
		{
			set_field_parameter_word(0, get_field_parameter_word(0) * movie_fps_ratio);
		}
	}

	return old_wait();
}

int script_OFST()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_word(9, get_field_parameter_word(9) * movie_fps_ratio);

	return old_ofst();
}

int script_ASPED()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_word(1, get_field_parameter_word(1) / movie_fps_ratio);

	return old_asped();
}

int script_MSPED()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_word(1, get_field_parameter_word(1) - (1024 / movie_fps_ratio));

	return old_msped();
}

int script_CANM1()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
	{
		set_field_parameter_word(1, get_field_parameter_word(1) * movie_fps_ratio);
		set_field_parameter_word(2, get_field_parameter_word(2) * movie_fps_ratio);
		set_field_parameter_byte(3, get_field_parameter_byte(3) * movie_fps_ratio);
	}

	return old_canm1();
}

int script_CANM2()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
	{
		set_field_parameter_word(1, get_field_parameter_word(1) * movie_fps_ratio);
		set_field_parameter_word(2, get_field_parameter_word(2) * movie_fps_ratio);
		set_field_parameter_byte(3, get_field_parameter_byte(3) * movie_fps_ratio);
	}

	return old_canm2();
}

int script_ANIM1()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_byte(1, get_field_parameter_byte(1) * movie_fps_ratio);

	return old_anim1();
}

int script_ANIM2()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_byte(1, get_field_parameter_byte(1) * movie_fps_ratio);

	return old_anim2();
}

int script_ANIME1()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_byte(1, get_field_parameter_byte(1) * movie_fps_ratio);

	return old_anime1();
}

int script_ANIME2()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_byte(1, get_field_parameter_byte(1) * movie_fps_ratio);

	return old_anime2();
}

int script_DFANM()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		set_field_parameter_byte(1, get_field_parameter_byte(1) * movie_fps_ratio);

	return old_dfanm();
}

int script_MVIEF()
{
	if (ff7_externals.movie_object->is_playing && movie_fps_ratio > 1)
		*ff7_externals.current_movie_frame = (WORD)ceil(*ff7_externals.current_movie_frame * movie_fps_ratio);

	return old_mvief();
}
// ---------------------------

uint32_t ff7_prepare_movie(char *name, uint32_t loop, struct dddevice **dddevice, uint32_t dd2interface)
{
	char drivename[4];
	char dirname[256];
	char filename[128];
	char fmvName[512];
	char newFmvName[512];

	if(trace_all || trace_movies) trace("prepare_movie %s\n", name);

	ff7_externals.movie_object->loop = loop;
	ff7_externals.movie_sub_415231(name);

	ff7_externals.movie_object->field_1F8 = 1;
	ff7_externals.movie_object->is_playing = 0;
	ff7_externals.movie_object->movie_end = 0;
	ff7_externals.movie_object->global_movie_flag = 0;
	ff7_externals.movie_object->field_E0 = !((struct ff7_game_obj *)common_externals.get_game_object())->field_968;

	_splitpath(name, drivename, dirname, filename, NULL);

	_snprintf(fmvName, sizeof(fmvName), "%s%s%s.%s", drivename, dirname, filename, ffmpeg_video_ext.c_str());

	int redirect_status = attempt_redirection(fmvName, newFmvName, sizeof(newFmvName));

	if (redirect_status == 0) ffmpeg_prepare_movie(newFmvName);
	else ffmpeg_prepare_movie(fmvName);

	ff7_externals.movie_object->global_movie_flag = 1;

	// Required by > 15 FPS movies
	movie_fps_ratio = ffmpeg_get_fps_ratio();
	if (movie_fps_ratio > 1)
	{
		if (strcmp(filename, "opening") == 0)
		{
			*ff7_externals.opening_movie_music_start_frame = *ff7_externals.opening_movie_music_start_frame * movie_fps_ratio;
		}
	}
	// ---------------------------

	return true;
}

uint32_t ff7_update_movie_sample(LPDIRECTDRAWSURFACE surface)
{
	uint32_t movie_end;

	ff7_externals.movie_object->movie_end = 0;

	if(!ff7_externals.movie_object->is_playing) return false;

retry:
	movie_end = !ffmpeg_update_movie_sample();

	if(movie_end)
	{
		if(trace_all || trace_movies) trace("movie end\n");
		if(ff7_externals.movie_object->loop)
		{
			ffmpeg_loop();
			goto retry;
		}

		ff7_externals.movie_object->movie_end = 1;
		return true;
	}

	return true;
}

uint32_t ff7_start_movie()
{
	if(trace_all || trace_movies) trace("start_movie\n");

	if(ff7_externals.movie_object->is_playing) return true;

	ff7_externals.movie_object->is_playing = 1;

	return ff7_update_movie_sample(0);
}

uint32_t ff7_stop_movie()
{
	if(trace_all || trace_movies) trace("stop_movie\n");

	if(ff7_externals.movie_object->is_playing)
	{
		ff7_externals.movie_object->is_playing = 0;
		ff7_externals.movie_object->movie_end = 0;

		ffmpeg_stop_movie();
	}

	return true;
}

void ff7_release_movie_objects()
{
	if(trace_all || trace_movies) trace("release_movie_objects\n");

	ff7_stop_movie();

	ffmpeg_release_movie_objects();

	ff7_externals.movie_object->global_movie_flag = 0;
}

void draw_current_frame()
{
	if (trace_all || trace_movies) trace("draw_current_frame\n");

	ffmpeg_draw_current_frame();

	// FF8 on Steam sometimes forgets to release the movie objects, so we do ensure it's done anyway
	ffmpeg_release_movie_objects();
}

uint32_t ff7_get_movie_frame()
{
	if(!ff7_externals.movie_object->is_playing) return 0;

	return ffmpeg_get_movie_frame();
}

uint32_t ff8_movie_frames;

void ff8_prepare_movie(uint32_t disc, uint32_t movie)
{
	char fmvName[512];
	char newFmvName[512];
	char camName[512];
	char dataPath[260]{0};
	FILE *camFile;
	uint32_t camOffset = 0;

	// The only movie which is translated needs to be loaded from specific language path
	if (disc == 3u && movie == 5u)
	{
		get_data_lang_path(dataPath);
	}
	else
	{
		strcpy(dataPath, basedir);
		PathAppendA(dataPath, "data");
	}

	// Unexpected cases default to current disk
	if (disc >= 5u) {
		disc = ff8_currentdisk - 1;
	}

	_snprintf(fmvName, sizeof(fmvName), "%s/movies/disc%02i_%02ih.%s", dataPath, disc, movie, ffmpeg_video_ext.c_str());
	_snprintf(camName, sizeof(camName), "%s/data/movies/disc%02i_%02i.cam", basedir, disc, movie);

	if(trace_all || trace_movies) trace("prepare_movie %s\n", fmvName);

	if(disc != 4)
	{
		camFile = fopen(camName, "rb");

		if(!camFile)
		{
			error("could not load camera data from %s\n", camName);
			return;
		}

		while(!feof(camFile) && !ferror(camFile))
		{
			uint32_t res = fread(&ff8_externals.movie_object->camdata_buffer[camOffset], 1, 4096, camFile);

			if(res > 0) camOffset += res;
		}

		ff8_externals.movie_object->movie_intro_pak = false;
	}
	else ff8_externals.movie_object->movie_intro_pak = true;

	ff8_externals.movie_object->camdata_start = (struct ff8_camdata *)(&ff8_externals.movie_object->camdata_buffer[8]);
	ff8_externals.movie_object->camdata_pointer = ff8_externals.movie_object->camdata_start;

	ff8_externals.movie_object->movie_current_frame = 0;

	int redirect_status = attempt_redirection(fmvName, newFmvName, sizeof(newFmvName));

	if (redirect_status == 0) ff8_movie_frames = ffmpeg_prepare_movie(newFmvName);
	else ff8_movie_frames = ffmpeg_prepare_movie(fmvName);
}

void ff8_release_movie_objects()
{
	if(trace_all || trace_movies) trace("release_movie_objects\n");

	ffmpeg_release_movie_objects();
}

void ff8_update_movie_sample()
{
	if(trace_all || trace_movies) trace("update_movie_sample\n");

	if(!ffmpeg_update_movie_sample())
	{
		if(ff8_externals.movie_object->movie_intro_pak) ff8_stop_movie();
		else ff8_externals.sub_5304B0();
	}

	ff8_externals.movie_object->movie_current_frame = ffmpeg_get_movie_frame();

	if(ff8_externals.movie_object->camdata_pointer->flag & 0x8) *ff8_externals.byte_1CE4907 = 0;
	if(ff8_externals.movie_object->camdata_pointer->flag & 0x10) *ff8_externals.byte_1CE4907 = 1;
	if(ff8_externals.movie_object->camdata_pointer->flag & 0x20)
	{
		*ff8_externals.byte_1CE4901 = 1;
		ff8_externals.movie_object->field_4C4B0 = 1;
	}
	else
	{
		*ff8_externals.byte_1CE4901 = 0;
		ff8_externals.movie_object->field_4C4B0 = 0;
	}

	if(ff8_externals.movie_object->camdata_pointer->flag & 0x1)
	{
		*ff8_externals.byte_1CE4901 = 1;
		ff8_externals.movie_object->field_4C4B0 = 1;
	}

	*ff8_externals.byte_1CE490D = ff8_externals.movie_object->camdata_pointer->flag & 0x40;

	if(ff8_externals.movie_object->movie_current_frame > ff8_externals.movie_object->movie_total_frames)
	{
		if(ff8_externals.movie_object->movie_intro_pak) ff8_stop_movie();
		else ff8_externals.sub_5304B0();

		return;
	}

	ff8_externals.movie_object->camdata_pointer = &ff8_externals.movie_object->camdata_start[ff8_externals.movie_object->movie_current_frame];
}

void ff8_start_movie()
{
	if(trace_all || trace_movies) trace("start_movie\n");

	if(ff8_externals.movie_object->movie_intro_pak) ff8_externals.movie_object->movie_total_frames = ff8_movie_frames;
	else
	{
		ff8_externals.movie_object->movie_total_frames = ((WORD *)ff8_externals.movie_object->camdata_buffer)[3];
		trace("%i frames\n", ff8_externals.movie_object->movie_total_frames);
	}

	ff8_externals.movie_object->field_4C4B0 = 0;

	*ff8_externals.enable_framelimiter = false;

	ff8_update_movie_sample();

	ff8_externals.movie_object->movie_is_playing = true;
}

void ff8_stop_movie()
{
	if(trace_all || trace_movies) trace("stop_movie\n");

	ffmpeg_stop_movie();

	ff8_externals.movie_object->movie_is_playing = false;

	ff8_externals.movie_object->field_4C4AC = 0;
	ff8_externals.movie_object->field_4C4B0 = 0;

	*ff8_externals.enable_framelimiter = true;
}

void movie_init()
{
	if(!ff8)
	{
		// Fix sync with movies > 15 FPS
		old_wait = (int (*)())common_externals.execute_opcode_table[0x24];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0x24], (DWORD)&script_WAIT);

		old_ofst = (int (*)())common_externals.execute_opcode_table[0xC3];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xC3], (DWORD)&script_OFST);

		old_asped = (int (*)())common_externals.execute_opcode_table[0xBD];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xBD], (DWORD)&script_ASPED);

		old_msped = (int (*)())common_externals.execute_opcode_table[0xB2];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xB2], (DWORD)&script_MSPED);

		old_mvief = (int (*)())common_externals.execute_opcode_table[0xFA];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xFA], (DWORD)&script_MVIEF);

		old_canm1 = (int (*)())common_externals.execute_opcode_table[0xB1];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xB1], (DWORD)&script_CANM1);

		old_canm2 = (int (*)())common_externals.execute_opcode_table[0xBC];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xBC], (DWORD)&script_CANM2);

		old_anim1 = (int (*)())common_externals.execute_opcode_table[0xAF];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xAF], (DWORD)&script_ANIM1);

		old_anim2 = (int (*)())common_externals.execute_opcode_table[0xBA];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xBA], (DWORD)&script_ANIM2);

		old_anime1 = (int (*)())common_externals.execute_opcode_table[0xA3];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xA3], (DWORD)&script_ANIME1);

		old_anime2 = (int (*)())common_externals.execute_opcode_table[0xAE];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xAE], (DWORD)&script_ANIME2);

		old_dfanm = (int (*)())common_externals.execute_opcode_table[0xA2];
		patch_code_dword((uint32_t)&common_externals.execute_opcode_table[0xA2], (DWORD)&script_DFANM);
		// -----------------------------

		replace_function(common_externals.prepare_movie, ff7_prepare_movie);
		replace_function(common_externals.release_movie_objects, ff7_release_movie_objects);
		replace_function(common_externals.start_movie, ff7_start_movie);
		replace_function(common_externals.update_movie_sample, ff7_update_movie_sample);
		replace_function(common_externals.stop_movie, ff7_stop_movie);
		replace_function(common_externals.get_movie_frame, ff7_get_movie_frame);
	}
	else
	{
		replace_function(common_externals.prepare_movie, ff8_prepare_movie);
		replace_function(common_externals.release_movie_objects, ff8_release_movie_objects);
		replace_function(common_externals.start_movie, ff8_start_movie);
		replace_function(common_externals.update_movie_sample, ff8_update_movie_sample);
		replace_function(ff8_externals.draw_movie_frame, draw_current_frame);
		replace_function(common_externals.stop_movie, ff8_stop_movie);
	}

	ffmpeg_movie_init();
}
