#include "bear_audio.h"

AudioID add_sound_source(Audio *audio, AudioSource source)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	AudioID id;
	id.uid = audio->uid_counter++;
	if (audio->uid_counter < 0) audio->uid_counter = 1;
	
	if (audio->free_source < 0)
	{
		id.pos = -audio->free_source - 1;
		audio->free_source = audio->sources[id.pos].id.pos;
	}
	else
	{
		id.pos = audio->free_source++;
	}

	audio->max_source = maximum(audio->max_source, (int32) id.pos);
	source.id = id;
	audio->sources[id.pos] = source;
	return id;
}

AudioID play_sound(Audio *audio, AudioID buffer_id, float32 volume, float32 pitch, Vec3f position={0.0f, 0.0f, 0.0f})
{ 
 	AudioSource source { buffer_id, buffer_id, 0, volume, pitch, position};
	return add_sound_source(audio, source);
}

AudioID play_music(Audio *audio, AudioID buffer_id, float32 volume, float32 pitch, bool loop)
{
 	AudioSource source { buffer_id, buffer_id, 0, volume, pitch};
	source.loop = loop;
	return add_sound_source(audio, source);
}

void stop_audio(Audio *audio, AudioID id)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	AudioSource source = audio->sources[id.pos];
	if (source.id != id) return;
	
	uint32 pos = id.pos;
	id.pos = audio->free_source;
	id.uid = -1;
	audio->free_source = -pos - 1;
	audio->sources[pos].id = id;

	if (pos == audio->max_source)
	{
		while (audio->sources[audio->max_source].id.uid < 0 && 0 <= audio->max_source)
			audio->max_source--;
	}
}

AudioBuffer get_buffer(Audio *audio, AudioID id)
{
	AudioBuffer buffer = audio->buffers[id.pos];
	if (id == buffer.id)
		return buffer;
	return {-1, -1};
}

AudioID add_buffer(Audio *audio, AudioBuffer buffer)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	AudioID id;
	id.uid = audio->uid_counter++;
	if (audio->uid_counter < 0) audio->uid_counter = 1;
	
	if (audio->free_buffer < 0)
	{
		// We need to swap shit
		id.pos = -audio->free_buffer - 1;
		audio->free_buffer = -audio->buffers[id.pos].id.uid;
	}
	else
	{
		// We need to append
		id.pos = audio->max_buffer++;
	}
	ASSERT(id.pos < BEAR_MAX_AUDIO_BUFFERS);

	buffer.id = id;
	audio->buffers[id.pos] = buffer;
	return id;
}

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

	ASSERT(header->format == 1);

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
	ptr -= chunk->size;

	AudioBuffer buffer;
	buffer.channels = header->channels;
	buffer.bitdepth = header->bitdepth;
	buffer.sample_rate = header->sample_rate;
	buffer.length = chunk->size / (header->bitdepth / 8); // Num samples

	uint32 length = chunk->size;
	buffer.data8 = MALLOC2(int8, length);
	int8 *to = buffer.data8;
	int8 *from = (int8 *) ptr;
	while (length--)
		*to++ = *from++;

	world->plt.free_file(file);

	return add_buffer(audio, buffer);
}

void free_sound(Audio *audio, AudioID id)
{
	// TODO: This should be refactored into a new data structure. 
	// Since we have this in 3 places. (Add/Remove Entity, Buffer and Source.
	ASSERT(0 < id.pos && id.pos < BEAR_MAX_AUDIO_BUFFERS);
	AudioBuffer buffer = audio->buffers[id.pos];
	if (buffer.id != id) return;
	
	uint32 pos = id.pos;
	id.pos = audio->free_buffer;
	id.uid = -1;
	audio->free_buffer = -pos - 1;
	audio->buffers[pos].id = id;

	FREE(audio->buffers[pos].data);

	if (pos == audio->max_buffer)
	{
		while (audio->buffers[audio->max_buffer].id.uid < 0 && 0 <= audio->max_buffer)
			audio->max_buffer--;
	}
}


