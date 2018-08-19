template <typename T>
static T mix_from_one_channel(T *sampler, int32 buffer_length, float32 current_sample, float32 pitch)
{
	float32 sub_sample = current_sample - floor(current_sample);
	int32 a_offset = floor(current_sample);
	int32 b_offset = floor(current_sample + pitch);
	a_offset = minimum(a_offset, buffer_length - 1);
	b_offset = minimum(b_offset, buffer_length - 1);
	T a_sample = sampler[a_offset];
	T b_sample = sampler[b_offset];
	T sample = lerp(a_sample, b_sample, sub_sample);
	return sample;
}

template <typename T>
static T mix_from_two_channel(T *sampler, int32 buffer_length, float32 current_sample, float32 pitch, bool left)
{
	float32 sub_sample = current_sample - floor(current_sample);
	int32 a_offset = floor(current_sample * 2);
	a_offset -= (a_offset + left) % 2;
	int32 b_offset = a_offset + (floor(current_sample * 2 + pitch) - a_offset) * 2;
	a_offset = minimum(a_offset, buffer_length - 1);
	b_offset = minimum(b_offset, buffer_length - 1);
	T a_sample = sampler[a_offset];
	T b_sample = sampler[b_offset];
	T sample = lerp(a_sample, b_sample, sub_sample); 
	return sample;
}

extern "C"
void sound(int16 *out_buffer, int32 num_samples)
{
	// TODO(Ed): This is SLOW!!!! We will probably have to SIMD this.
	for (int32 i = 0; i < num_samples; i++)
		out_buffer[i] = 0;

	if (!world) return;

	// Samples for 1 channel.
	num_samples *= 0.5f;

	Audio *audio = &world->audio;
	for (int32 i = 0; i < audio->max_source; i++)
	{
		AudioSource source = audio->sources[i];
		if (source.id.uid < 0) continue;
		AudioBuffer buffer = get_buffer(audio, source.buffer_id);
		if (buffer.id.uid < 0)
		{
			// Don't play invalid audio.
			stop_audio(audio, source.id);
			continue;
		}
		ASSERT(buffer.channels < 3);

		int32 samples_left = (int32) (buffer.num_samples - source.current_sample) / buffer.channels;
		int32 length;
		if (source.loop)
			length = num_samples;
		else
			length = minimum(num_samples, samples_left);

		float32 pitch = source.pitch;
		float32 volume = source.volume;

		Vec3f distance = source.position - audio->position;
		float32 angle_factor = dot(audio->left, 
				normalize(distance));
		float32 distance_falloff = 1.0f / length_cubed(distance);

		float32 left_volume  = volume * maximum(1.0f, 1.0f + angle_factor) * distance_falloff;
		float32 right_volume = volume * maximum(1.0f, 1.0f - angle_factor) * distance_falloff;

		// Yeah, stopping the audio before we play it! Woot!;
		if (length == samples_left && !source.loop)
			stop_audio(audio, source.id);

		if (buffer.channels == 1)
		{
			// One channel
			if (buffer.bitdepth == 32)
			{
				int16 *ptr = out_buffer;
				float32 *sampler = buffer.data32;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					int16 sample = mix_from_one_channel(sampler, buffer.num_samples, current_sample, pitch) * 0x7FFF;
					current_sample += pitch;
					*(ptr++) += sample * left_volume;
					*(ptr++) += sample * right_volume;
					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
			else if (buffer.bitdepth == 16)
			{
				int16 *ptr = out_buffer;
				int16 *sampler = buffer.data16;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					int16 sample = mix_from_one_channel(sampler, buffer.num_samples, current_sample, pitch);
					current_sample += pitch;
					*(ptr++) += sample * left_volume;
					*(ptr++) += sample * right_volume;
					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
			else if (buffer.bitdepth == 8)
			{
				int16 *ptr = out_buffer;

				int8 *sampler = buffer.data8;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					int16 sample = mix_from_one_channel(sampler, buffer.num_samples, current_sample, pitch) * 2;
					current_sample += pitch;
					*(ptr++) += sample * left_volume;
					*(ptr++) += sample * right_volume;
					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
		}
		else
		{
			// Two channels
			if (buffer.bitdepth == 32)
			{
				int16 *ptr = out_buffer;
				float32 *sampler = buffer.data32;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					float32 left_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, false);
					*(ptr++) += left_sample  * volume * 0x7FFF;
					float32 right_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, true);
					*(ptr++) += right_sample * volume * 0x7FFF;
					current_sample += pitch;

					if (source.loop && buffer.num_samples < current_sample * 2)
					{
						current_sample = 0;
					}

					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
			else if (buffer.bitdepth == 16)
			{
				int16 *ptr = out_buffer;
				int16 *sampler = buffer.data16;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					int16 left_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, false);
					*(ptr++) += left_sample  * volume;
					int16 right_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, true);
					*(ptr++) += right_sample * volume;
					current_sample += pitch;

					if (source.loop && buffer.num_samples < current_sample * 2)
					{
						current_sample = 0;
					}

					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
			else if (buffer.bitdepth == 8)
			{
				int16 *ptr = out_buffer;

				int8 *sampler = buffer.data8;
				float32 current_sample = source.current_sample;

				while (length > 0)
				{
					int8 left_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, false);
					*(ptr++) += left_sample  * volume * 2;
					int8 right_sample = mix_from_two_channel(sampler, buffer.num_samples, current_sample, pitch, true);
					*(ptr++) += right_sample * volume * 2;

					if (source.loop && buffer.num_samples < current_sample * 2)
					{
						current_sample = 0;
					}

					current_sample += pitch;
					length--;
				}
				audio->sources[i].current_sample = current_sample;
			}
		}
	}
}

