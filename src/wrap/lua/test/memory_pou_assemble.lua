-- POU assembler: fields string -> live image + FB exec list (dynamic only in Lua).
-- AI modification signature: Composer
local axfb = require("zmc.axfb")

local M = {}

-- Fbt* type name -> mc_fb create/exec/destroy (strategy lives only here).
M.FB = {
    FbtPower = {
        create = axfb.mc.Power_create,
        exec = axfb.mc.Power_exec,
        destroy = axfb.mc.Power_destroy,
    },
    FbtHalt = {
        create = axfb.mc.Halt_create,
        exec = axfb.mc.Halt_exec,
        destroy = axfb.mc.Halt_destroy,
    },
    FbtMoveRelative = {
        create = axfb.mc.MoveRelative_create,
        exec = axfb.mc.MoveRelative_exec,
        destroy = axfb.mc.MoveRelative_destroy,
    },
    FbtMoveAdditive = {
        create = axfb.mc.MoveAdditive_create,
        exec = axfb.mc.MoveAdditive_exec,
        destroy = axfb.mc.MoveAdditive_destroy,
    },
    FbtMoveAbsolute = {
        create = axfb.mc.MoveAbsolute_create,
        exec = axfb.mc.MoveAbsolute_exec,
        destroy = axfb.mc.MoveAbsolute_destroy,
    },
}

local function fail_if(ret, step)
    if ret ~= 0 then
        error(step .. " failed ret=" .. tostring(ret), 2)
    end
end

-- Assemble live image from MEMORY_POU_DEMO_FIELDS body + optional axis ref inject.
-- Returns pou, image, exec_list, axis
function M.assemble(fields, opts)
    opts = opts or {}
    local pou, ret = axfb.memory_pou()
    fail_if(ret, "memory_pou")
    fail_if(pou:load_layout(opts.layout), "load_layout")
    fail_if(pou:configure(fields), "configure")

    local image
    image, ret = axfb.image(pou:root_size(), pou:max_align())
    fail_if(ret, "image")

    local exec_list = {}
    local axis_off = nil
    local count = pou:decl_count()
    for i = 0, count - 1 do
        local t, name, off, sz = pou:decl_at(i)
        if t == "AXIS_REF" and name == "axis" then
            axis_off = off
        end
        local ops = M.FB[t]
        if ops then
            local slot = image:at(off, sz)
            fail_if(ops.create(slot), "create " .. name)
            exec_list[#exec_list + 1] = {
                name = name,
                type = t,
                offset = off,
                size = sz,
                slot = slot,
                ops = ops,
            }
        end
    end

    local axis = nil
    if axis_off ~= nil then
        axis, ret = axfb.axis_virtual({ cycle_time = opts.cycle_time or 10000 })
        fail_if(ret, "axis_virtual")
        fail_if(image:write_ptr(axis_off, axis:ref()), "write axis")
        -- Wire Axis members via symbol copy (power.Axis = axis, ...).
        for _, fb in ipairs(exec_list) do
            local axis_sym = fb.name .. ".Axis"
            local off, sz = pou:symbol_offset(axis_sym)
            if off ~= nil and sz == 8 then
                fail_if(image:write_ptr(off, axis:ref()), "write " .. axis_sym)
            end
        end
    end

    fail_if(pou:bind(image), "bind")
    return pou, image, exec_list, axis
end

function M.destroy_fbs(exec_list)
    for i = #exec_list, 1, -1 do
        local fb = exec_list[i]
        fb.ops.destroy(fb.slot)
    end
end

function M.run_cycle(pou, exec_list, axis)
    fail_if(pou:cmd_advance(), "cmd_advance")
    for _, fb in ipairs(exec_list) do
        fail_if(fb.ops.exec(fb.slot), "exec " .. fb.name)
    end
    if axis then
        fail_if(axis:run_cycle(), "axis run_cycle")
    end
end

return M
