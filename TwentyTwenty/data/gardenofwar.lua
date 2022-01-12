io.write("Calling howdy() ...\n")
local value = howdy("First", "Second", 112233)
io.write(string.format("howdy() returned: %s\n", tostring(value)))

spawn_time = 0
math.randomseed(os.time())

while(true)
do
   current_time = os.time()
   if(current_time-spawn_time > 1) then
      spawn_time = os.time()
      local value = build_and_add_entity(math.random(20,22), math.random(1,128), math.random(1,128))
      io.write(string.format("spawned new unit"))
   end
end