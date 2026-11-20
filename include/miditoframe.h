#ifndef GETMIDIFRAME_H
#define GETMIDIFRAME_H

#define TML_IMPLEMENTATION
#include "tml.h" // using tml.h --> https://github.com/schellingb/TinySoundFont/blob/main/tml.h

#include "videodata.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

typedef struct NotesList {
	uint32_t* framestime;
	uint16_t amount;
} NotesList;

NotesList GetMidiFrames(const char* midipath, VideoData vdata) {

	printf("Getting midi frames...\n");

	tml_message *midi = tml_load_filename(midipath);
	assert(midi);

	// first integer is going to be the amount of notes there is, the rest is the array.
	NotesList ret;
	int totalnotes = tml_get_info(midi, NULL, NULL, NULL, NULL, NULL);
	ret.amount = totalnotes;
	ret.framestime = malloc(totalnotes * sizeof(uint32_t));

	int i = 0;
	int ffps1;
	int ffps2;
	sscanf(vdata.fps, "%d/%d", &ffps1, &ffps2);
	
	for (tml_message *msg = midi; msg != NULL; msg = msg->next) {
		if (msg->type == TML_NOTE_ON && msg->velocity > 0) {		
			ret.framestime[i] = (uint32_t)round(
				msg->time/1000.0*( (float)ffps1 / (float)ffps2 ) // ffmpeg fps is in this format
			);
			++i;
		}
	}

	printf("Amount of notes: %d\n", totalnotes);

	tml_free(midi);
	
	return ret;
}

#endif
