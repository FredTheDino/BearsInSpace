
void init_phy(World *world)
{
	//world->phy.normal = {1.0f, 1.0f, 1.0f};
	world->phy.body_limits = create_array<BodyLimit>(50);
}

void destroy_phy(World *world)
{
	delete_array(&world->phy.body_limits);
}

