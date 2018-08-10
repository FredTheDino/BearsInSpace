#pragma once

Array<float32> to_float32(Array<Vec3f> v_arr)
{
	Array<float32> f_arr = static_array<float32>(size(v_arr) * 3);
	for (uint64 i = 0; i < size(v_arr); i++)
	{
		append(&f_arr, v_arr[i].x);
		append(&f_arr, v_arr[i].y);
		append(&f_arr, v_arr[i].z);
	}

	return f_arr;
}
