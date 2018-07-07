#include "bear_audio.h"

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

AudioID play_sound(Audio *audio, AudioID buffer_id, float32 volume, float32 pitch) 
{ 
	AudioSource source { buffer_id, buffer_id, 0, volume, pitch };

	AudioID id;
	id.uid = audio->uid_counter++;
	if (audio->uid_counter < 0) audio->uid_counter = 1;
	
	if (audio->free_source < 0)
	{
		// We need to swap shit
		id.pos = -audio->free_source;
		audio->free_source = -audio->sources[id.pos].id.uid;
	}
	else
	{
		// We need to append
		id.pos = audio->max_source++;
	}

	source.id = id;
	audio->sources[id.pos] = source;
	return id;
}

AudioBuffer get_buffer(Audio *audio, AudioID id)
{
	AudioBuffer buffer = audio->buffers[id.pos];
	if (id == buffer.id)
		return buffer;
	return {};
}

AudioID add_buffer(Audio *audio, AudioBuffer buffer)
{
	AudioID id;
	id.uid = audio->uid_counter++;
	if (audio->uid_counter < 0) audio->uid_counter = 1;
	
	if (audio->free_buffer < 0)
	{
		// We need to swap shit
		id.pos = -audio->free_buffer;
		audio->free_buffer = -audio->buffers[id.pos].id.uid;
	}
	else
	{
		// We need to append
		id.pos = audio->max_buffer++;
	}

	buffer.id = id;
	audio->buffers[id.pos] = buffer;
	return id;
}

AudioID load_sound(Audio *audio, const char *path) // NOTE(Ed): Assumes WAV
{
	OSFile file = world->plt.read_file(path);
	uint8 *ptr = (uint8 *) file.data;
	WAVHeader *header = (WAVHeader *) ptr;
	ptr += sizeof(WAVHeader);

	ASSERT(header->riff[0] == 'R');
	ASSERT(header->riff[1] == 'I');
	ASSERT(header->riff[2] == 'F');
	ASSERT(header->riff[3] == 'F');

	ASSERT(header->wave[0] == 'W');
	ASSERT(header->wave[1] == 'A');
	ASSERT(header->wave[2] == 'V');
	ASSERT(header->wave[3] == 'E');

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
	buffer.length = chunk->size / (header->bitdepth / 8); // Num samples

	if (buffer.bitdepth == 8)
	{
		uint16 length = buffer.length * buffer.channels;
		int8 *to = MALLOC2(int8, length);
		int8 *from = (int8 *) ptr;
		for (uint16 i = 0; i < length; i++)
			to[i] = from[i];
		buffer.data8 = to;
	}
	else if (buffer.bitdepth == 16)
	{
		uint16 length = buffer.length * buffer.channels;
		int16 *to = MALLOC2(int16, length);
		int16 *from = (int16 *) ptr;
		for (uint16 i = 0; i < length; i++)
			to[i] = from[i];
		buffer.data16 = to;
	}
	else if (buffer.bitdepth == 32)
	{
		uint16 length = chunk->size / (buffer.bitdepth / 8);
		float32 *to = MALLOC2(float32, length);
		float32 *from = (float32 *) ptr;
		for (uint16 i = 0; i < length; i++)
			to[i] = from[i];
		buffer.data32 = to;
	}
	else
	{
		ASSERT(!"Unsupported bitdepth!");
	}

	return add_buffer(audio, buffer);
}

