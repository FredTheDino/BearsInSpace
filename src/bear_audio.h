#pragma once
typedef int16 SND;

const uint32 spec_freq = 44100; // Move this.

// TODO(Ed): I need a floor and ceil function.
// TODO(Ed): We need to support OGG files for all those HUGE music files, eventually.

#define BEAR_MAX_AUDIO_BUFFERS 512
#define BEAR_MAX_AUDIO_SOURCES 64

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
	int16 uid_counter = 0;

	int32 free_buffer = 0;
	int32 max_buffer = -1;
	AudioBuffer *buffers;

	int32 free_source = 0;
	int32 max_source = -1;
	AudioSource *sources;
};

