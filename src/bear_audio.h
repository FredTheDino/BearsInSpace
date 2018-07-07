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
	uint16 pos;

	bool operator== (AudioID other)
	{
		return uid == other.uid && pos == other.pos;
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
	int64 current_sample;

	float32 pitch;
	float32 volume; // We do some fading.
	// float32 curr_volume;
	// bool loop;
};

struct Audio
{
	int16 uid_counter = 1;

	int32 free_buffer = 0;
	uint32 max_buffer = 0;
	AudioBuffer buffers[BEAR_MAX_AUDIO_BUFFERS];

	int32 free_source = 0;
	uint32 max_source = 0;
	AudioSource sources[BEAR_MAX_AUDIO_SOURCES];
};


