require "data/maps/constants"

math.randomseed(os.time())

-- x,y,z = get_entities_of_type()
-- build_and_add_entity(unit_types.knight,y,z)

wave_table = {
	{time=180, wave_size=3},
	{time=300, wave_size=5},
	{time=420, wave_size=7},
	{time=480, wave_size=9}
}

buildings = {get_buildings_for_team(1)} -- buildings[1] is the ID
building_to_build_from = buildings[1]
player_buildings = {get_buildings_for_team(0)}

previous_time = os.time()
unit_offset = 0;

function send_wave(time, n)
	if os.time()-time >= wave_table[1].time	-- if we've passed the first time in the wave table
	then
		-- send n units
		units = {get_units_for_team(1)}
		send_message("hit wave send: "..wave_table[1].wave_size)
		for i = 2+unit_offset,1+wave_table[1].wave_size+unit_offset,1
		do
			give_command(units[i], commands.attack_move, 10, 10)
		end
		unit_offset = unit_offset + wave_table[1].wave_size
		table.remove(wave_table, 1)
	end
end

while 1 do
	give_command(building_to_build_from, commands.build_unit, unit_types.archer)
	sleep(6)
	send_wave(previous_time)
end