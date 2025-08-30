--[[
    This file is encode by "GBK"
]]
local sys = require "sys"

log.info("base", "LuatOS@Air640w ˢ������")

if rtos.bsp() ~= "win32" then
    log.info("base", "��ǰ��֧��win32����")
    os.exit()
end

Base_CWD = lfs.currentdir()
tool_debug = false

local self_conf = {
    basic = {
        COM_PORT = "COM20",
        USER_PATH = "user\\",
        LIB_PATH = "lib\\",
        MAIN_LUA_DEBUG = "false",
        LUA_DEBUG = "false",
        TOOLS_PATH = "tools\\"
    }
}

local function trim5(s)
    return s:match'^%s*(.*%S)' or ''
end

local function read_local_ini()
    local f = io.open("/local.ini")
    if f then
        local key = "basic"
        for line in f:lines() do
            line = trim5(line)
            if #line > 0 then
                if line:byte() == '[' and line:byte(line:len()) then
                    key = line:sub(2, line:len() - 1)
                    if key == "air640w" then key = "basic" end
                    if self_conf[key] == nil then
                        self_conf[key] = {}
                    end
                elseif line:find("=") then
                    local skey = line:sub(1, line:find("=") - 1)
                    local val = line:sub(line:find("=") + 1)
                    if skey and val then
                        self_conf[key][trim5(skey)] = trim5(val)
                    end
                end
            end
        end
        f:close()
    end
    log.info("config", json.encode(self_conf))
end
read_local_ini()

local function lsdir(path, files, shortname)
    local exe = io.popen("dir /b " .. (shortname and " " or " /s ") .. path)
    if exe then
        for line in exe:lines() do
            table.insert(files, line)
        end
        exe:close()
    end
end

local function oscall(cmd, quite, cwd)
    if cwd and Base_CWD then
        lfs.chdir(cwd)
    end
    if tool_debug then
        log.info("cmd", cmd)
    end
    local exe = io.popen(cmd)
    if exe then
        for line in exe:lines() do
            if not quite then
                log.info("cmd", line)
            end
        end
        exe:close()
    end
    if cwd and Base_CWD then
        lfs.chdir(Base_CWD)
    end
end

local function create_tmproot()
    --local tmproot = os.getenv("Temp") .. "\\luatos"
    local tmproot = "tmp\\"
    log.info("tmpdir", tmproot)
    oscall("rmdir /S /Q \"" .. tmproot .. "\"")
    oscall("mkdir \"" .. tmproot .. "\"")
    return tmproot
end

local cmds = {}
cmds["help"] = function(arg)
    log.info("help", "==============================")
    log.info("help", "lfs      ����ļ�ϵͳ")
    log.info("help", "dlrom    ���صײ�̼�")
    log.info("help", "dlfs     �����ļ�ϵͳ")
    log.info("help", "dlfull   ���صײ�̼����ļ�ϵͳ")
    log.info("help", "==============================")
end

local function build_flashx(rootpath)
    local files = {}
    lsdir(rootpath, files, true)
    local buff = zbuff.create(64*1024)
    -- ͷ��ħ��
    buff:writeI16(0x1234)
    -- ����ļ�д��
    for _, name in ipairs(files) do
        if name ~= "flashx.bin" then
            -- д���ļ���
            buff:writeI16(0x0101)
            buff:writeI16(0x0)
            buff:writeI32(name:len())
            buff:write(name)
            -- д���ļ�
            local f = io.open(rootpath .. "\\" .. name, "rb")
            if f == nil then
                log.error("�ļ���ȡʧ��!!!!!!!" .. name)
                os.exit(2)
                return -- impossible
            end
            local data = f:read("*a")
            buff:writeI16(0x0202)
            buff:writeI16(0x0)
            buff:writeI32(data:len())
            buff:write(data)
            f:close()
        end
    end
    local fdata = buff:toStr(0, buff:seek(0, zbuff.SEEK_CUR))
    io.writeFile(rootpath .. "\\flashx.bin", fdata)
    log.info("lfs", "flashx.bin is done", crypto.sha256(fdata))
end

cmds["lfs"] = function()
    -- ����, ���û����ļ���ȡһ��
    local files = {}
    lsdir(self_conf["basic"]["USER_PATH"] or "user\\", files)
    lsdir(self_conf["basic"]["LIB_PATH"] or "lib\\", files)
    for index, value in ipairs(files) do
        log.info("file", index ,value)
    end
    -- ������ʱĿ¼, ���ļ���������ȥ
    local tmproot = create_tmproot()
    oscall("mkdir \"" .. tmproot .. "\"\\luafile")
    oscall("mkdir \"" .. tmproot .. "\"\\disk")
    for index, value in ipairs(files) do
        log.info("copy", index ,value)
        oscall("copy \"" .. value .. "\" " .. "\"" .. tmproot .. "\\luafile\\")
    end
    -- ��lua�ļ�����luac����, ��lua�ļ�������
    files = {}
    lsdir(tmproot .. "\\luafile", files, true)

    --local tools_path = self_conf["basic"]["TOOLS_PATH"]
    local luac_exe = self_conf["basic"]["TOOLS_PATH"] .. "luac_536_32bits.exe"
    local main_lua_debug = self_conf["basic"]["MAIN_LUA_DEBUG"] == "true"
    local lua_debug = self_conf["basic"]["LUA_DEBUG"] == "true"
    for _, name in ipairs(files) do
        local srcpath = tmproot .. "\\luafile\\" .. name
        if name:match(".+lua$") then
            local dstpath = tmproot .. "\\disk\\" .. name .. "c"
            local debug = lua_debug
            if name == "main.lua" and main_lua_debug then
                debug = true
            end
            oscall(luac_exe .. (debug and " -o " or " -s -o ") .. dstpath .." "..  srcpath, true)
        else
            local dstpath = tmproot .. "\\disk\\" .. name
            oscall("copy " .. srcpath .. " " .. dstpath)
        end
    end

    -- �ļ��Ѿ�ȫ������%Temp%\luatos\disk ������

    -- ������, ����flashx.bin
    build_flashx(tmproot .. "\\disk")
    -- Ȼ�󿽱�һ��, ���������ļ���

end

cmds["dlfs"] = function ()
    -- ����diskĿ¼�ڲ���

    -- ��ʼ����

    -- TODO ���볿��������ǰд��tools.lua, �����ļ����豸ȥ
end

cmds["dlrom"] = function ()
    -- ���FLS�ļ��Ƿ����

    -- ͨ��xmodemЭ�鷢�͹̼�
end

cmds["dlfull"] = function ()
    cmds["dlrom"]()
    cmds["dlfs"]()
end

sys.taskInit(function()
    sys.wait(10)
    for _, arg in ipairs(win32.args()) do
        if cmds[arg] then
            cmds[arg]()
        elseif cmds["-" .. arg] then
            cmds[arg]()
        elseif arg:byte() == '-' and arg:find("=") then
            local skey = arg:sub(2, arg:find("=") - 1)
            local val = arg:sub(arg:find("=") + 1)
            if skey and val then
                self_conf["basic"][trim5(skey)] = trim5(val)
            end
        end
    end
    os.exit(0)
end)

sys.run()
