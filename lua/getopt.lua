local NOARG = 1
local ACCEPT = 2

local function parse_opts(optstr)
    local options = {}
    local i = 1
    while i <= optstr:len() do
        local c, n = optstr:sub(i, i), optstr:sub(i + 1, i + 1)
        if c ~= ':' then
            options[c] = n == ':' and ACCEPT or NOARG
        end
        i = i + 1
    end

    return options
end

return function(argv, optstr)
    -- build option table
    local options = parse_opts(optstr)

    local idx = 1
    return function()
        repeat
            local arg = argv[idx]
            idx = idx + 1
            if arg == nil then
                return nil
            end
            if arg:sub(1, 1) == '-' and arg:len() == 2 then
                -- potential option 
                local c = options[arg:sub(2, 2)]
                if c == NOARG then
                    return arg:sub(2, 2), nil
                elseif c == ACCEPT then
                    idx = idx + 1
                    return arg:sub(2, 2), argv[idx - 1]
                end
            end
        until true
    end
end
