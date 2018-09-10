#pragma once

void debug_camera_control(Camera *camera)
{
	float32 planar_speed = 15.0f * delta;
	float32 vertical_speed = 15.0f * delta;
	float32 rotational_speed = 1.5f * delta;
	Vec3f movement = {};
	movement.x = AXIS_VAL("xmove") * planar_speed;
	movement.z = AXIS_VAL("zmove") * planar_speed;
	PRINT("%.4f\n", movement.x);
	movement = camera.transform.orientation * movement;
	movement.y += (AXIS_VAL("up") - AXIS_VAL("down")) * vertical_speed;
	camera->transform.position += movement;
	camera->transform.orientation = 
		toQ(0.0f, -AXIS_VAL("xrot") * rotational_speed, 0.0f) *
		camera.transform.orientation *
		toQ(-AXIS_VAL("yrot") * rotational_speed, 0.0f, 0.0f);
}
