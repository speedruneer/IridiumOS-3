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

-- Load config.mk (flat key=value)
local function load_config(path)
    local conf = {}
    local f = io.open(path, "r")
    if not f then return conf end
    for line in f:lines() do
        line = line:match("^%s*(.-)%s*$")
        if line ~= "" and not line:match("^[;#]") then
            local k, v = line:match("^(.-)%s*=%s*(.-)$")
            if k and v then
                k = k:gsub("%s","_")
                if v == "true" then
                    v = true
                elseif v == "false" then
                    v = false
                elseif tonumber(v) then
                    v = tonumber(v)
                end
                conf[k] = v
            end
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

function Menu:draw()
    term.clear()
    term.cursor.jump(1,1)
    print(colors.bright .. self.title .. colors.reset)
    for i, item in ipairs(self.items) do
        term.cursor.goleft(600)
        local line = ""
        local label = item.label
        if item.type == "bool" then
            local valstr = item.value and "[*]" or "[ ]"
            line = valstr .. " " .. colors.green .. label .. colors.reset
        elseif item.type == "string" then
            line = colors.cyan .. "(" .. tostring(item.value or "") .. ")" .. colors.reset .. " " .. label
        elseif item.type == "number" then
            line = colors.yellow .. tostring(item.value or 0) .. colors.reset .. " : " .. label
        elseif item.type == "menu" then
            line = colors.magenta .. "> " .. label .. colors.reset
        elseif item.type == "back" then
            line = colors.red .. "< Back" .. colors.reset
        end

        if i == self.current then
            io.write(colors.reverse .. line .. colors.reset .. "\n")
        else
            io.write(line .. "\n")
        end
    end
    term.cursor.goleft(600)
    io.flush()
end

function Menu:run()
    while true do
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

-- Save config back to config.mk
local function save_config(menu, path)
    local f = io.open(path, "w")
    if not f then error("Failed to open "..path.." for writing") end

    local function write_menu(m)
        for _, item in ipairs(m.items) do
            if item.type == "menu" and item.value then
                write_menu(item.value)
            elseif item.type ~= "back" then
                if item.key then
                    f:write(string.format("%s=%s\n", item.key, tostring(item.value)))
                end
            end
        end
    end
    write_menu(menu)
    f:close()
end

-- Load menus from menu.ini
local function load_menus_from_ini(path, config)
    local menu_sections = {}
    local current_section = nil

    for line in io.lines(path) do
        line = line:match("^%s*(.-)%s*$")
        if line ~= "" and not line:match("^[;#]") then
            local section = line:match("^%[(.-)%]$")
            if section then
                current_section = section
                menu_sections[current_section] = {}
            elseif current_section then
                local key, val, label = line:match("^(.-)%s*=%s*([^:]+):?(.*)$")
                if key and val then
                    table.insert(menu_sections[current_section], {
                        key   = key:gsub("%s","_"), -- canonical key
                        val   = val,                        -- type
                        label = (label ~= "" and label) or key -- human label
                    })
                end
            end
        end
    end

    local menus = {}
    local function build_menu(tag, parent)
        tag = tag:gsub("_", " ")
        if menus[tag] then return menus[tag] end
        local items_raw = menu_sections[tag]
        if not items_raw then error("No section ["..tag.."] found") end
        local items = {}
        local menu_obj = Menu:new(tag, items, parent)
        menus[tag] = menu_obj

        for _, item in ipairs(items_raw) do
            local itype = item.val:lower()
            if itype == "menu" then
                local sub_menu = build_menu(item.key, menu_obj)
                table.insert(items, {key=item.key, label=item.label, type="menu", value=sub_menu})
            elseif itype == "bool" then
                table.insert(items, {key=item.key, label=item.label, type="bool", value=config[item.key] or false})
            elseif itype == "string" then
                table.insert(items, {key=item.key, label=item.label, type="string", value=config[item.key] or ""})
            elseif itype == "number" then
                table.insert(items, {key=item.key, label=item.label, type="number", value=config[item.key] or 0})
            elseif itype == "back" then
                table.insert(items, {label="< Back", type="back"})
            end
        end

        if parent then
            table.insert(items, {label="< Back", type="back"})
        end
        for _, it in ipairs(items) do
            if it.type == "menu" and it.value then
                it.value.parent = menu_obj
            end
        end
        menu_obj.items = items
        return menu_obj
    end

    return build_menu("Main Menu", nil)
end

-- Load config and menu
local config = load_config("config.mk")
local main_menu = load_menus_from_ini("menu.ini", config)

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
            print(indent .. item.key .. " = " .. tostring(item.value))
        end
    end
end

print_menu(main_menu)
save_config(main_menu, "config.mk")
term.clear()
term.cursor.goto(1,1)
