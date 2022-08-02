require "data/maps/constants"

math.randomseed(os.time())

-- some globals
wave_table = {
	{time=180, wave_size=3},
	{time=300, wave_size=5},
	{time=420, wave_size=7},
	{time=480, wave_size=9}
}
start_time = os.time()
unit_offset = 0;
buildings = {get_buildings_for_team(1)} -- buildings[1] is the ID
building_to_build_from = buildings[1]

-- send n units to attack the base
function send_wave(num_units)
	send_message("Sending a wave of "..num_units.." units!")
	units = {get_units_for_team(1)}
	for i = 2+unit_offset,1+num_units+unit_offset,1
	do
		give_command(units[i], commands.attack_move, 10, 10)
	end
	unit_offset = unit_offset + num_units
end

-- check if a time has been passed
function check_time_passed(time_bound)
	return os.time()-start_time >= time_bound
end

-- this is called in an infinite loop at the end of the file
function main()
	if check_time_passed(wave_table[1].time)	-- if we've passed the first time in the wave table
	then
		send_wave(wave_table[1].wave_size)	-- send the wave
		table.remove(wave_table, 1)			-- remove that row from the wave table
	end
	give_command(building_to_build_from, commands.build_unit, unit_types.archer)	-- instruct the EnemySpawner to build a new unit
	sleep(6)
end

while 1 do
	main()
end