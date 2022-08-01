math.randomseed(os.time())

x,y,z = get_entities_of_type();
build_and_add_entity(x,y,z);
buildings = {get_buildings_for_team(1)}; -- building[1] is the ID


while 1 do
	send_message("Test");
end