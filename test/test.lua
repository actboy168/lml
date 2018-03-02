package.cpath = package.cpath .. [[;.\..\bin\Debug\?.dll]]

local lni = require 'lml'
local print_r = require 'print_r'

function LOAD(filename)
	local f = assert(io.open(filename, 'rb'))
	local r = lni(f:read 'a')
	f:close()
	return r
end

local function EQUAL(a, b)
	for k, v in pairs(a) do
		if type(v) == 'table' then
			EQUAL(v, b[k])
		else
			assert(v == b[k])
		end
	end
end

local n = 0
local function TEST(script, t)
	n = n + 1
	local name = 'TEST-' .. n
	local r = lni(script, name)
	local ok, e = pcall(EQUAL, r, t)
	if not ok then
		print(script)
		print('--------------------------')
		print_r(r)
		print('--------------------------')
		print_r(t)
		print('--------------------------')
		error(name)
	end
	local ok, e = pcall(EQUAL, t, r)
	if not ok then
		print(script)
		print('--------------------------')
		print_r(r)
		print('--------------------------')
		print_r(t)
		print('--------------------------')
		error(name)
	end
end

TEST(
[==[
TEST
  A
TEST
  B
]==]
,
{
'', false,
{ 'TEST', false, {'A'}},
{ 'TEST', false, {'B'}},
}
)

TEST(
[==[
TEST
  A
  B
]==]
,
{
'', false,
{ 'TEST', false, {'A'}, {'B'}}
}
)

print('test ok!')
