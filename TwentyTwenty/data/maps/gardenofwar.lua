math.randomseed(os.time())

function sleep(n)
  if n > 0 then os.execute("ping -n " .. tonumber(n+1) .. " localhost > NUL") end
end

x,y,z = get_entities_of_type()
build_and_add_entity(x,y,z)
buildings = {get_buildings_for_team(1)} -- buildings[1] is the ID

building_to_build_from = buildings[1]
previous_time = os.time()

while 1 do
	give_command(building_to_build_from, 3, 22)
	sleep(10);
	units = {get_units_for_team(1)}
	for i = 2,units[1],1
	do
		send_message("test: "..units[i])
	end
end