-- document.lua
local output_file = "funcs.txt"
local funcs = {}

-- Get all .s and .inc files recursively
local function get_s_files(root)
    local files = {}
    local cmd
    if package.config:sub(1,1) == "\\" then
        cmd = 'dir /s /b "'..root..'\\*.s" "'..root..'\\*.inc"'
    else
        cmd = 'find "'..root..'" -type f \\( -name "*.s" -o -name "*.inc" \\)'
    end
    local p = io.popen(cmd)
    if p then
        for line in p:lines() do table.insert(files,line) end
        p:close()
    end
    return files
end

-- Process each file and extract functions
local function process_file(file)
    local f = io.open(file,"r")
    if not f then return end
    local lines = {}
    for line in f:lines() do table.insert(lines,line) end
    f:close()

    local i = 1
    while i <= #lines do
        local line = lines[i]
        if line:match("^%s*;%s*type:%s*function") then
            local comments = {}
            i = i + 1
            line = lines[i]
            while line and line:match("^%s*;") do
                table.insert(comments,line)
                i = i + 1
                line = lines[i]
            end
            if line and line:match("^%s*([_%a][_%w]*)%s*:") then
                local func_name = line:match("^%s*([_%a][_%w]*)%s*:")
                funcs[func_name] = {comments=comments, file=file}
            end
        else
            i = i + 1
        end
    end
end

-- Write functions to output file in a clear format
local function write_output()
    local f = io.open(output_file,"w")
    local keys = {}
    for k in pairs(funcs) do table.insert(keys,k) end
    table.sort(keys)

    for _,func in ipairs(keys) do
        local data = funcs[func]
        f:write("====================================\n")
        f:write("Function: "..func.."\n")
        f:write("Defined in: "..data.file.."\n\n")

        local desc, inputs, outputs = {}, {}, {}
        for _,c in ipairs(data.comments) do
            c = c:gsub("^%s*;%s*","") -- remove leading "; "
            if c:match("^type:") then
                -- skip marker line
            elseif c:match("^->") then
                table.insert(inputs, "  "..c)
            elseif c:match("^<-") then
                table.insert(outputs, "  "..c)
            else
                table.insert(desc, "  "..c)
            end
        end

        if #desc > 0 then
            f:write("Description:\n")
            for _,d in ipairs(desc) do f:write(d.."\n") end
            f:write("\n")
        end

        if #inputs > 0 then
            f:write("Inputs:\n")
            for _,inpt in ipairs(inputs) do f:write(inpt.."\n") end
            f:write("\n")
        end

        if #outputs > 0 then
            f:write("Outputs:\n")
            for _,outp in ipairs(outputs) do f:write(outp.."\n") end
            f:write("\n")
        end
    end
    f:close()
    print("Generated "..output_file)
end

-- Main
local root_dir = arg[1] or "."
local files = get_s_files(root_dir)
for _,file in ipairs(files) do process_file(file) end
write_output()
