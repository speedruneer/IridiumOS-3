-- rwfs_cli.lua
-- Pure Lua RWFS CLI tool
local SECTOR_SIZE = 512

-- Block magics
local MAGIC_FS       = 0x8362
local MAGIC_FILE_F    = 0xAA55
local MAGIC_FILE_M    = 0x2631
local MAGIC_FILE_L    = 0x7361
local MAGIC_DIR       = 0x7788

-- Helpers
local function pad(str,len)
    if #str < len then return str .. string.rep("\0",len-#str) else return str:sub(1,len) end
end

local function read_dword(s,pos)
    local b1,b2,b3,b4 = s:byte(pos,pos+3)
    return b1 + b2*256 + b3*65536 + b4*16777216
end

local function write_dword(n)
    return string.char(n%256, math.floor(n/256)%256, math.floor(n/65536)%256, math.floor(n/16777216)%256)
end

-- Create new empty partition
local function cmd_new(outfile, total_blocks)
    total_blocks = total_blocks or 1024
    local f = assert(io.open(outfile,"wb"))
    f:write(write_dword(MAGIC_FS))
    f:write(write_dword(total_blocks))
    f:write(string.rep("\0", SECTOR_SIZE-8))
    for i=2,total_blocks do
        f:write(string.rep("\0", SECTOR_SIZE))
    end
    f:close()
    print("Created new partition:", outfile)
end

-- Utility to read a block
local function read_block(f, n)
    f:seek("set",(n-1)*SECTOR_SIZE)
    return f:read(SECTOR_SIZE)
end

local function write_block(f, n, data)
    f:seek("set",(n-1)*SECTOR_SIZE)
    f:write(pad(data, SECTOR_SIZE))
end

-- Find free block
local function find_free_block(f, total_blocks)
    for i=2,total_blocks do
        local block = read_block(f,i)
        local magic = block:byte(1) + block:byte(2)*256
        if magic ~= MAGIC_FILE_F and magic ~= MAGIC_FILE_M and magic ~= MAGIC_FILE_L and magic ~= MAGIC_DIR then
            return i
        end
    end
    return nil
end

-- List files recursively
local function cmd_listfiles(srcfile)
    local f = assert(io.open(srcfile,"rb"))
    local header = read_block(f,1)
    local total_blocks = read_dword(header,5)
    print("Scanning partition with", total_blocks, "blocks")
    for i=2,total_blocks do
        local block = read_block(f,i)
        local magic = block:byte(1) + block:byte(2)*256
        if magic == MAGIC_FILE_F then
            local fname = block:sub(3,18)
            print("File:", fname)
        elseif magic == MAGIC_DIR then
            local dname = block:sub(3,18)
            print("Dir:", dname)
        end
    end
    f:close()
end

local function file_exists(path)
    local ok, pipe
    if package.config:sub(1,1) == "\\" then
        -- Windows
        pipe = io.popen('if exist "'..path..'" (echo yes) else (echo no)')
    else
        -- Unix / Linux / macOS
        pipe = io.popen('test -f "'..path..'" && echo yes || echo no')
    end
    local result = pipe:read("*l")
    pipe:close()
    return result == "yes"
end


-- Add file
local function cmd_add(file, destfile)
    if not file_exists(file) then print("Source file not found") return end
    local fdata = io.open(file,"rb"):read("*all")
    local df = assert(io.open(destfile,"r+b"))
    local header = read_block(df,1)
    local total_blocks = read_dword(header,5)
    local first_block = find_free_block(df, total_blocks)
    if not first_block then print("No free blocks") df:close() return end
    -- Write first block
    local fname = pad(file:match("([^/\\]+)$"),16)
    local block = string.char(MAGIC_FILE_F%256, math.floor(MAGIC_FILE_F/256)) .. fname .. write_dword(0) .. pad(fdata, SECTOR_SIZE-22)
    write_block(df, first_block, block)
    df:close()
    print("Added", file, "at block", first_block)
end

-- Remove file
local function cmd_remove(filename, destfile)
    local df = assert(io.open(destfile,"r+b"))
    local header = read_block(df,1)
    local total_blocks = read_dword(header,5)
    for i=2,total_blocks do
        local block = read_block(df,i)
        local magic = block:byte(1) + block:byte(2)*256
        if magic == MAGIC_FILE_F then
            local fname = block:sub(3,18)
            if fname:match("^[^\0]+") == filename then
                write_block(df,i,string.rep("\0",SECTOR_SIZE))
                print("Removed file:", filename)
                break
            end
        end
    end
    df:close()
end

-- Make partition from directory (simplified: flat, no subdirs)
local lfs = require("lfs")
local function cmd_makepart(sourcedir,destfile)
    cmd_new(destfile,1024)
    local df = assert(io.open(destfile,"r+b"))
    local header = read_block(df,1)
    local total_blocks = read_dword(header,5)
    for file in lfs.dir(sourcedir) do
        if file~="." and file~=".." then
            local fpath = sourcedir.."/"..file
            local attr = lfs.attributes(fpath)
            if attr.mode=="file" then
                cmd_add(fpath,destfile)
            end
        end
    end
    df:close()
end

-- Convert partition to directory (extract)
local function cmd_convert(outdir, srcfile)
    os.execute("mkdir -p "..outdir)
    local f = assert(io.open(srcfile,"rb"))
    local header = read_block(f,1)
    local total_blocks = read_dword(header,5)
    for i=2,total_blocks do
        local block = read_block(f,i)
        local magic = block:byte(1) + block:byte(2)*256
        if magic == MAGIC_FILE_F then
            local fname = block:sub(3,18):match("^[^%z]+")
            local data = block:sub(23)
            local out = io.open(outdir.."/"..fname,"wb")
            out:write(data)
            out:close()
        end
    end
    f:close()
end

-- CLI dispatcher
local cmd = arg[1]
if cmd=="new" then
    cmd_new(arg[2])
elseif cmd=="listfiles" then
    cmd_listfiles(arg[2])
elseif cmd=="add" then
    cmd_add(arg[2], arg[3])
elseif cmd=="remove" then
    cmd_remove(arg[2], arg[3])
elseif cmd=="makepart" then
    cmd_makepart(arg[2], arg[3])
elseif cmd=="convert" then
    cmd_convert(arg[2], arg[3])
else
    print("Commands: new <file>, add <file> <destfile>, remove <filename> <destfile>, listfiles <srcfile>, makepart <dir> <destfile>, convert <outdir> <srcfile>")
end
