#include "bear_audio.h"

// Unknown if needed.
AudioID load_sound(const char *path); // NOTE(Ed): Assumes WAV
AudioID play_sound(AudioID buffer_id, float32 volume, float32 pitch);

struct WAVHeader
{
	char  riff[4];
	int32 size;
	char  wave[4];

	// FMT chunk
	char  fmt[4];
	int32 fmt_size;
	int16 format;
	int16 channels;
	int32 sample_rate;
	int32 byte_rate;
	int16 block_align;
	int16 bitdepth;

};

struct WAVChunk
{
	char  type[4];
	int32 size;
};

void copy(uint8 *to, uint8 *from, uint32 length)
{
	while (length--)
		*to++ = *from++;
}

AudioID load_sound(Audio *audio, const char *path) // NOTE(Ed): Assumes WAV
{
	OSFile file = world->plt.read_file(path);
	uint8 *ptr = (uint8 *) file.data;
	WAVHeader *header = (WAVHeader *) ptr;
	ptr += sizeof(WAVHeader);

	ASSERT(header->riff[0] == 'r');
	ASSERT(header->riff[1] == 'i');
	ASSERT(header->riff[2] == 'f');
	ASSERT(header->riff[3] == 'f');

	ASSERT(header->wave[0] == 'w');
	ASSERT(header->wave[1] == 'a');
	ASSERT(header->wave[2] == 'v');
	ASSERT(header->wave[3] == 'e');

	ASSERT(header->fmt[0] == 'f');
	ASSERT(header->fmt[1] == 'm');
	ASSERT(header->fmt[2] == 't');

	// Just get me the data chunk.
	WAVChunk *chunk;
	do 
	{
		ASSERT(*ptr != 0);
		chunk = (WAVChunk *) ptr;
		ptr += sizeof(WAVChunk) + chunk->size;
	} while (
			chunk->type[0] != 'd' ||
			chunk->type[1] != 'a' ||
			chunk->type[2] != 't' ||
			chunk->type[3] != 'a');

	AudioBuffer buffer;
	buffer.channels = header->channels;
	buffer.bitdepth = header->bitdepth;
	buffer.sample_rate = header->sample_rate;
	buffer.length = chunk->size / (header->channels * header->bitdepth / 8); // Num samples

	if (buffer.bitdepth == 8)
	{
		uint16 length = buffer.length * buffer.channels;
		int8 *to = MALLOC2(int8, length);
		int8 *from = ptr;
		while (length--)
			*to++ = *from++;
	}
	else if (buffer.bitdepth == 16)
	{
		uint16 length = buffer.length * buffer.channels;
		int16 *to = MALLOC2(int16, length);
		int16 *from = ptr;
		while (length--)
			*to++ = *from++;
	}
	else
	{
		ASSERT(!"Unsupported bitdepth!");
	}

	world->plt.free_file(file);
	return {0, 0};
}

