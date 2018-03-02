package.cpath = package.cpath .. [[;.\..\bin\Debug\?.dll]]

local lml = require 'lml'
local print_r = require 'print_r'

function LOAD(filename)
	local f = assert(io.open(filename, 'rb'))
	local r = lml(f:read 'a')
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
	local r = lml(script, name)
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
]==]
,
{
'', false,
{ 'TEST' },
}
)

TEST(
[==[
TEST: STATE
]==]
,
{
'', false,
{ 'TEST', 'STATE' },
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

TEST(
[==[
TEST: STATE
  A
  B
]==]
,
{
'', false,
{ 'TEST', 'STATE', {'A'}, {'B'}}
}
)


TEST(
[==[
TEST: STATE
  A: STATE_A
  B: STATE_B
]==]
,
{
'', false,
{ 'TEST', 'STATE', {'A', 'STATE_A'}, {'B', 'STATE_B'}}
}
)

TEST(
[==[
TEST: STATE
  A: STATE_A
    A1
    A2
  B: STATE_B
    B1
    B2
]==]
,
{
'', false,
{ 'TEST', 'STATE', {'A', 'STATE_A', {'A1'}, {'A2'}}, {'B', 'STATE_B', {'B1'}, {'B2'}}}
}
)


TEST(
[==[
'TE:ST': '''A000'''
]==]
,
{
'', false,
{ 'TE:ST', "'A000'"}
}
)

print('test ok!')
