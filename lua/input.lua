local raw = require "tty"
local stdout = io.stdout

local M = {}
function M.radio(prompt, opts)
	local idx = 1
	local selected = 0
	local N = #opts

	raw(function(term)
		local cyan = term.with_color "cyan"
		cyan(prompt)
		stdout:write "\r\n"
		while true do
			for i, v in ipairs(opts) do
				term.clear_line "a"
				cyan(idx == i and "> " or "  ")
				cyan(selected == i and "× " or "· ")
				stdout:write(v)
				stdout:write "\r\n"
			end

			local key = term.next_key()
			if key == "Up" or key == "k" then
				idx = math.max(1, idx - 1)
			elseif key == "Down" or key == "j" then
				idx = math.min(N, idx + 1)
			elseif key == "Enter" and selected ~= 0 then
				return nil
			elseif key == "Space" then
				selected = idx
			end
			term.move_up(3)
			term.move_left()
		end
	end)

	return opts[selected]
end

function M.selector(prompt, opts)
	local idx = 1
	local selected = {}
	local N = #opts

	raw(function(term)
		local cyan = term.with_color "cyan"
		cyan(prompt)
		stdout:write "\r\n"
		while true do
			for i, v in ipairs(opts) do
				term.clear_line "a"
				cyan(idx == i and "> " or "  ")
				cyan(selected[i] and "[·] " or "[ ] ")
				stdout:write(v)
				stdout:write "\r\n"
			end

			local key = term.next_key()
			if key == "Up" or key == "k" then
				idx = math.max(1, idx - 1)
			elseif key == "Down" or key == "j" then
				idx = math.min(N, idx + 1)
			elseif key == "Enter" then
				return nil
			elseif key == "Space" then
				selected[idx] = not selected[idx]
			end
			term.move_up(3)
			term.move_left()
		end
	end)

	local out = {}
	for index, value in ipairs(opts) do
		if selected[index] then
			table.insert(out, value)
		end
	end

	return out
end

function M.text_input(prompt)
	local idx = 1
	local start = 1
	local input = {}

	raw(function(term)
		local blue = term.with_color "cyan"
		blue(prompt .. ": ")
		start = prompt:len() + 2

		local function draw()
			term.move_left()
			term.move_right(start)
			term.clear_line "r"
			stdout:write(table.concat(input))
			term.move_left()
			term.move_right(start + idx - 1)
		end
		while true do
			local c = term.next_key()
			if c == "Enter" then
				return nil
			elseif c == "Left" then
				idx = math.max(1, idx - 1)
				draw()
			elseif c == "Right" then
				idx = idx + 1
				draw()
			elseif c == "Backspace" then
				idx = idx - 1
				table.remove(input, idx)
				draw()
			else
				if c == "Space" then
					c = " "
				elseif c == "Tab" then
					c = "\t"
				end
				table.insert(input, idx, c)
				idx = idx + 1
				draw()
			end
		end
	end)
	print ""
	return table.concat(input)
end

--
function M._test()
	local text_input = M.text_input
	local radio = M.radio
	local selector = M.selector
	print "== text input =="
	local res = text_input "write your name"
	print("output:", res)

	print "== multiple select =="
	print "press space to toggle, press 'Enter' to continue"
	local sel = selector("Select zero or more", {
		"option 1",
		"option 2",
		"option 3",
	})

	print("selected:", unpack(sel))

	print "== single select =="
	print "press space to toggle, press 'Enter' to continue"
	sel = radio("Select one", {
		"option 1",
		"option 2",
		"option 3",
	})

	print("selected:", sel)
end

return M
