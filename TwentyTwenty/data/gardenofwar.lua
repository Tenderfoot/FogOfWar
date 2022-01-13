math.randomseed(os.time())

local value = build_and_add_entity(math.random(20,22), math.random(1,128), math.random(1,128))
io.write(string.format("spawned new unit\n"))