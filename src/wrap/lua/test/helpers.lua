-- Shared helpers for zmc.axfb Lua integration tests.
-- fail_if is reserved for bridge init and run_cycle only.
-- AI modification signature: Composer
local axfb = require("zmc.axfb")

local M = {}

function M.fail_if(ret, step)
    if ret ~= 0 then
        error(step .. " failed ret=" .. tostring(ret), 2)
    end
end

function M.assert_eq(expected, actual, msg)
    if expected ~= actual then
        error(string.format("%s: expected=%s actual=%s",
            msg or "assert_eq", tostring(expected), tostring(actual)), 2)
    end
end

function M.assert_near(expected, actual, tol, msg)
    local diff = expected - actual
    if diff < 0 then
        diff = -diff
    end
    if diff > tol then
        error(string.format("%s: expected near %.6f actual=%.6f tol=%.6f",
            msg or "assert_near", expected, actual, tol), 2)
    end
end

function M.assert_true(cond, msg)
    if not cond then
        error(msg or "assert_true failed", 2)
    end
end

function M.default_ini_path()
    return "src/mhelper/data/zmc_motapp_demo_fak0.ini"
end

function M.default_eni_path()
    return "src/mhelper/data/pse_servo1.xml"
end

-- Open bridge + status. Uses fail_if only on init path.
function M.open_bridge(opts)
    opts = opts or {}
    local ini_path = opts.ini_path or M.default_ini_path()
    local eni_path = opts.eni_path or M.default_eni_path()
    local axis_num = opts.axis_num or 1

    local bridge, ret = axfb.new()
    M.fail_if(ret, "new")
    M.fail_if(bridge:load_eni(eni_path), "load_eni")
    M.fail_if(bridge:configure({
        ini_path = ini_path,
        ini_content = "",
        axis_num = axis_num,
    }), "configure")
    local status
    status, ret = axfb.status()
    M.fail_if(ret, "status")
    return bridge, status, axfb
end

function M.drain_status(status, bridge)
    while true do
        local pop_ret = status:pop(bridge)
        if pop_ret ~= 0 then
            break
        end
    end
end

function M.run_cycle(bridge, inp, oup)
    local ret, next_oup = bridge:run_cycle(inp, oup)
    M.fail_if(ret, "run_cycle")
    return next_oup
end

return M
