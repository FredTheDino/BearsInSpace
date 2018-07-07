#pragma once
typedef int16 SND;

const uint32 spec_freq = 44100; // The frequency we output.

// TODO(Ed): I need a floor and ceil function.
// TODO(Ed): Ambience might be nice.
// TODO(Ed): We need to support OGG files for all those HUGE music files.

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

	uint8 channels;
	uint8 bitdepth;
	uint16 sample_rate;
	int32 length; // Enough audio for 811h.
	union
	{
		void *data;
		int8 *data8;
		int16 *data16;
		int32 *data32;
	};
};

struct AudioSource
{
	AudioID source_id;
	AudioID buffer_id;
	int64 current_sample;

	// For both music and sound fx.
	float32 pitch; // How does this work?
	float32 volume; // We do some fading.
	// float32 curr_volume;
	// bool loop;
};

struct Audio
{
	uint32 num_buffers;
	int32 last_free_buffer;
	AudioBuffer buffers[BEAR_MAX_AUDIO_BUFFERS];
	int32 last_free_source;
	AudioSource sources[BEAR_MAX_AUDIO_SOURCES];
};


