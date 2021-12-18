local termios = require "termios"
local byte, char, format = string.byte, string.char, string.format
local stdin, stdout, stderr = io.stdin, io.stdout, io.stderr

local M = {}

local function read_key()
	local c = stdin:read(1)
	while c == nil do
		c = stdin:read(1)
	end
	return c
end

function M.move_left(s)
	s = s or 999
	stdout:write("\x1b[" .. s .. "D")
end

function M.move_right(s)
	s = s or 999
	stdout:write("\x1b[" .. s .. "C")
end

function M.move_up(s)
	s = s or 999
	stdout:write("\x1b[" .. s .. "A")
end

function M.move_down(s)
	s = s or 999
	stdout:write("\x1b[" .. s .. "B")
end

function M.clear_line(arg)
	if arg == nil or arg == "r" then
		stdout:write "\x1b[0K"
	elseif arg == "l" then
		stdout:write "\x1b[1K"
	elseif arg == "a" then
		stdout:write "\x1b[2K"
	end
end

function M.next_key()
	local c = read_key()
	if c == "\x1b" then
		-- escape code
		local a, b = read_key(), read_key()
		if a == "[" then
			if b == "A" then
				return "Up"
			elseif b == "B" then
				return "Down"
			elseif b == "C" then
				return "Right"
			elseif b == "D" then
				return "Left"
			end
		end
	elseif byte(c) == 127 then
		return "Backspace"
	elseif byte(c) == 10 or byte(c) == 13 then
		return "Enter"
	elseif byte(c) == 9 then
		return "Tab"
	elseif c == " " then
		return "Space"
	elseif byte(c) <= 26 then
		return format("^%s", char(64 + byte(c)))
	else
		return c
	end
end

local FOREGROUND = {
	black = 30,
	red = 31,
	green = 32,
	yellow = 33,
	blue = 34,
	purple = 35,
	cyan = 36,
	white = 37,
}

local BACKGROUND = {
	black = 40,
	red = 41,
	green = 42,
	yellow = 43,
	blue = 44,
	purple = 45,
	cyan = 46,
	white = 47,
}

function M.with_color(fg, bg)
	local t = {}
	if fg ~= nil then
		local c = FOREGROUND[fg]
		if c == nil then
			return nil
		end
		table.insert(t, c)
	end

	if bg ~= nil then
		local c = FOREGROUND[bg]
		if c == nil then
			return nil
		end
		table.insert(t, c)
	end

	local param = string.format("\x1b[%sm", table.concat(t, ";"))

	return function(input)
		stdout:write(param)
		stdout:write(input)
		stdout:write "\x1b[0m"
	end
end

return function(f)
	termios.enable_raw_mode()
	f(M)
	termios.disable_raw_mode()
end
