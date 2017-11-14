v1 = vec.new(1, 0)
v2 = vec.new(0, 1)

-- observe usage of getter/setter
print("v1:", v1.x, v1.y)
print("v2:", v2.x, v2.y)
-- gets 0, 1 by doing lookup into getter function
print("changing v2...")
v2.x = 3
v2[1] = 5
-- can use [0] [1] like popular
-- C++-style math vector classes
print("v1:", v1.x, v1.y)
print("v2:", v2.x, v2.y)
-- both obj.name and obj["name"]
-- are equivalent lookup methods
-- and both will trigger the getter
-- if it can't find 'name' on the object
assert(v1["x"] == v1.x)
assert(v1[0] == v1.x)
assert(v1["x"] == v1[0])
assert(v1["y"] == v1.y)
assert(v1[1] == v1.y)
assert(v1["y"] == v1[1])
