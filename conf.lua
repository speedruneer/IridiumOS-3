local term = require "term"
local colors = term.colors

-- Terminal utilities
local function readkey()
    os.execute("stty raw -echo")
    local c = io.read(1)
    if c == "\27" then
        local c2 = io.read(2)
        if c2 == "[A" then return "up"
        elseif c2 == "[B" then return "down"
        end
    elseif c == "\r" or c == "\n" then
        return "enter"
    elseif c == " " then
        return "space"
    elseif c == "\127" then
        return "backspace"
    elseif c == "\27" then
        return "esc"
    elseif c == "\3" then
        return "ctrlc"
    end
    return nil
end

local function prompt_input(prompt)
    os.execute("stty sane")
    io.write(prompt .. ": ")
    local val = io.read()
    os.execute("stty raw -echo")
    return val
end

-- Config loader
local function load_config(path)
    local conf = {}
    local f = io.open(path, "r")
    if not f then return conf end
    for line in f:lines() do
        local k,v = line:match("(%w+)%s*=%s*(.+)")
        if k and v then
            if v == "true" then v = true
            elseif v == "false" then v = false
            elseif tonumber(v) then v = tonumber(v)
            end
            conf[k] = v
        end
    end
    f:close()
    return conf
end

-- Menu system
local Menu = {}
Menu.__index = Menu

function Menu:new(title, items, parent)
    return setmetatable({title=title, items=items or {}, current=1, parent=parent}, self)
end

-- Draw menu with colors per type
function Menu:draw()
    term.clear()
    term.cursor.jump(1,1)
    print(colors.bright .. self.title .. colors.reset)
    for i, item in ipairs(self.items) do
        term.cursor.goleft(600)
        local line = ""
        if item.type == "bool" then
            local valstr = item.value and "[*]" or "[ ]"
            line = valstr .. " " .. colors.green .. item.label .. colors.reset
        elseif item.type == "string" then
            line = colors.cyan .. "(" .. tostring(item.value or "") .. ")" .. colors.reset .. " " .. item.label
        elseif item.type == "number" then
            line = colors.yellow .. tostring(item.value or 0) .. colors.reset .. " : " .. item.label
        elseif item.type == "menu" then
            line = colors.magenta .. "> " .. item.label .. colors.reset
        elseif item.type == "back" then
            line = colors.red .. "< Back" .. colors.reset
        end

        if i == self.current then
            io.write(colors.reverse .. line .. colors.reset .. "\n")
        else
            io.write(line .. "\n")
        end
    end
    io.flush()
end

-- Run menu loop
function Menu:run()
    local running = true
    while running do
        self:draw()
        local key
        repeat key = readkey() until key
        local item = self.items[self.current]

        if key == "up" and self.current > 1 then
            self.current = self.current - 1
        elseif key == "down" and self.current < #self.items then
            self.current = self.current + 1
        elseif key == "space" then
            if item.type == "bool" then
                item.value = not item.value
            end
        elseif key == "enter" then
            if item.type == "menu" and item.value then
                item.value:run()
            elseif item.type == "string" or item.type == "number" then
                local input = prompt_input(item.label)
                if item.type == "number" then input = tonumber(input) end
                item.value = input
            elseif item.type == "bool" then
                item.value = not item.value
            elseif item.type == "back" then
                return
            end
        elseif key == "esc" or key == "backspace" then
            if self.parent then return end
        elseif key == "ctrlc" then
            os.execute("stty sane")
            os.exit()
        end
    end
end

-- Load initial config
local config = load_config("config.mk")
local function save_config(menu, path)
    local f = io.open(path, "w")
    if not f then error("Failed to open "..path.." for writing") end

    local function write_menu(m)
        for _, item in ipairs(m.items) do
            if item.type == "menu" and item.value then
                write_menu(item.value)
            elseif item.type ~= "back" then
                f:write(string.format("%s=%s\n", item.label:upper():gsub("%s","_"), tostring(item.value)))
            end
        end
    end

    write_menu(menu)
    f:close()
end

-- Example nested menu
local sub_menu = Menu:new("Advanced Options", {
    {label="Enable Logging", type="bool", value=config.ENABLE_LOGGING or false},
    {label="Set Timeout", type="number", value=config.TIMEOUT or 30},
    {label="< Back", type="back"}
})

local main_menu = Menu:new("Main Menu", {
    {label="USB Support", type="bool", value=config.USB_SUPPORT or false},
    {label="Filesystem", type="string", value=config.FILESYSTEM or "ext4"},
    {label="Advanced", type="menu", value=sub_menu},
    {label="Exit", type="back"}
})

sub_menu.parent = main_menu

-- Run main menu
main_menu:run()
os.execute("stty sane")

-- Print final selections
local function print_menu(menu, indent)
    indent = indent or ""
    for _, item in ipairs(menu.items) do
        if item.type == "menu" and item.value then
            print(indent .. item.label .. ":")
            print_menu(item.value, indent .. "  ")
        elseif item.type ~= "back" then
            print(indent .. item.label .. " = " .. tostring(item.value))
        end
    end
end

print_menu(main_menu)
save_config(main_menu, "config.mk")
term.clear()
term.cursor.goto(1,1)