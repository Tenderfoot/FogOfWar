
commands = {
	move=0, 
	attack=1,
	gather=2,
	build_unit=3,
	build_building=4,
	attack_move=5,
	chop=6
}

unit_types = {
	gatherer=20,
	knight=21, 
	skeleton=22, 
	archer=26
}

building_types = {
	townhall=18, 
	goldmine=19, 
	farm=23,
	barracks=24,
	enemyspawner=25
}

function sleep(n)
  if n > 0 then os.execute("ping -n " .. tonumber(n+1) .. " localhost > NUL") end
end