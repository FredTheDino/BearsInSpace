#pragma once
// TODO(Ed): We need to support OGG files for all those HUGE music files, eventually.

#define MAX_AUDIO_BUFFERS 512
#define MAX_AUDIO_SOURCES 64

struct AudioID
{
	int16 uid;
	int16 pos;

	bool operator== (AudioID other)
	{
		return uid == other.uid && pos == other.pos;
	}

	bool operator!= (AudioID other)
	{
		return !(*this == other);
	}
};

struct AudioBuffer
{
	AudioID id;

	uint8 channels;
	uint8 bitdepth;
	uint16 sample_rate;
	int32 length; // Enough audio for 811h.
	union
	{
		void *data;
		int8 *data8;
		int16 *data16;
		float32 *data32;
	};
};

struct AudioSource
{
	AudioID id;
	AudioID buffer_id;
	float32 current_sample;

	float32 volume; // We do some fading.
	float32 pitch;
	Vec3f position;

	bool loop;
};

struct Audio
{
	Vec3f left;
	Vec3f position;
	int16 uid_counter;

	int32 free_buffer;
	int32 max_buffer;
	AudioBuffer buffers[MAX_AUDIO_BUFFERS];

	int32 free_source;
	int32 max_source;
	AudioSource sources[MAX_AUDIO_SOURCES];
};

