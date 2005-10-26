/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "config.h"

#include "twolame.h"
#include "common.h"
#include "util.h"


// Return string containg version number
// of this library
const char* get_twolame_version( void )
{
	const char* str = PACKAGE_VERSION;
	
	return str;
}

// Return string containg version number
// of this library
const char* get_twolame_url( void )
{
	const char* str = "http://twolame.sourceforge.net";
	
	return str;
}


/* 1: MPEG-1, 0: MPEG-2 LSF, 1995-07-11 shn */
static const int bitrate_table[2][15] = {
{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384}
};


// *  Returns the index associated with a bitrate for
// *  the specified version of MPEG.
// *
// *  Returns -1 for invalid bitrates
//
int twolame_get_bitrate_index(int bitrate, TWOLAME_MPEG_version version)
{
	int index = 0;
	int found = 0;

	// MFC sanity check.
	if (version!=0 && version!=1) {
		fprintf(stderr,"twolame_get_bitrate_index error %i\n",version);
		return -1;
	}

	while (!found && index < 15) {
		if (bitrate_table[version][index] == bitrate)
			found = 1;
		else
			++index;
	}
	
	if (found)  return (index);
	else {
		fprintf (stderr,
			"twolame_get_bitrate_index: %d is not a legal bitrate for version %i\n",
			bitrate, version);
		return -1;
	}
}

// convert samp frq in Hz to index
// legal rates 16000, 22050, 24000, 32000, 44100, 48000
//  -1 is returned for invalid samplerates
int twolame_get_samplerate_index (long sample_rate)
{

	switch (sample_rate) {
		case 44100L: return 0;
		case 48000L: return 1;
		case 32000L: return 2;
		case 22050L: return 0;
		case 24000L: return 1;
		case 16000L: return 2;
	}

	// Invalid choice of samplerate	
	fprintf (stderr, "twolame_get_samplerate_index: %ld is not a legal sample rate\n", sample_rate);
	return -1;
}


// Return the MPEG Version to use for the specified samplerate
//  -1 is returned for invalid samplerates
int twolame_get_version_for_samplerate (long sample_rate)
{

	switch (sample_rate) {
		case 48000L: return TWOLAME_MPEG1;
		case 44100L: return TWOLAME_MPEG1;
		case 32000L: return TWOLAME_MPEG1;
		case 24000L: return TWOLAME_MPEG2;
		case 22050L: return TWOLAME_MPEG2;
		case 16000L: return TWOLAME_MPEG2;
	}

	// Invalid choice of samplerate	
	fprintf (stderr, "twolame_get_version_for_samplerate: %ld is not a legal sample rate\n", sample_rate);
	return -1;
}



// Print the library version and 
//  encoder parameter settings to STDERR
void twolame_print_config(twolame_options *glopts)
{
	FILE* fd = stderr;

    fprintf(fd, "TwoLame version %s (%s)\n", get_twolame_version(), get_twolame_url());

	fprintf (stderr, "---------------------------------------------------------\n");
	fprintf (stderr, "Input : %d Hz, %d channels\n",
				twolame_get_in_samplerate(glopts),
				twolame_get_num_channels(glopts));
	fprintf (stderr, "Output: %d Hz, %s\n",
				twolame_get_out_samplerate(glopts), 
				twolame_get_mode_name(glopts));
	fprintf (stderr, "%d kbps ", twolame_get_bitrate(glopts) );
	fprintf (stderr, "%s Layer II ", twolame_get_version_name(glopts));
	fprintf (stderr, "psycho model=%d \n", twolame_get_psymodel(glopts));
	
	fprintf (stderr, "[De-emph:%s    Copyright:%s   Original:%s]\n",
	((twolame_get_emphasis(glopts)) ? "On " : "Off"),
	((twolame_get_copyright(glopts)) ? "Yes" : "No "),
	((twolame_get_original(glopts)) ? "Yes" : "No "));
	
	fprintf (stderr, "[Padding:%s CRC:%s         DAB:%s     ]\n",
	((twolame_get_padding(glopts)) ? "Normal" : "Off   "),
	((twolame_get_error_protection(glopts)) ? "On " : "Off"),
	((twolame_get_DAB(glopts)) ? "On " : "Off")); 
	
	if (twolame_get_VBR(glopts))
	fprintf (stderr, "VBR Enabled. Using MNR boost of %f\n", twolame_get_VBR_q(glopts));
	fprintf(stderr,"ATH adjustment %f\n", twolame_get_ATH_level(glopts));
	fprintf(stderr,"Reserving %i Ancillary bits\n", twolame_get_num_ancillary_bits(glopts));
	
    if (glopts->num_channels == 2 && glopts->mode == TWOLAME_MONO ) {
		fprintf(fd, "Autoconverting from stereo to mono.\n");
    }
	
	fprintf (stderr, "---------------------------------------------------------\n");
}

