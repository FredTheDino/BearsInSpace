//#include "audio/bear_audio.h"

struct AudioBuffer
{
	uint8 channels;
	uint8 bitdepth; // This should allways be 16 bit, otherwise it should be converted.
	uint16 sample_rate; // This should also allways be the same... Or should it?
	int32 num_samples; // Enough audio for 811h.
	int8 *data;
};

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

AudioBuffer load_wav(const char *path, void *(*alloc)(size_t))
{
	FILE *file = fopen(path, "r");
	fseek(file, 0, SEEK_END);
	uint64 size = ftell(file);
	fseek(file, 0, SEEK_SET);
	uint8 *ptr = (uint8 *) alloc(size + 1);
	fread(ptr, size, 1, file);
	fclose(file);

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
	buffer.num_samples = chunk->size / (header->bitdepth / 8); // Num samples

	uint32 length = chunk->size;
	buffer.data = (int8 *) alloc(length);
	int8 *to = buffer.data;
	int8 *from = (int8 *) ptr;
	while (length--)
		*to++ = *from++;

	return buffer;
}
