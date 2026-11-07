-- Single-axis qcmd packet demo driven by bridge:run_cycle.
-- fail_if is reserved for init and run_cycle only.
-- AI modification signature: Composer
local axfb = require("zmc.axfb")

local function fail_if(ret, step)
    if ret ~= 0 then
        error(step .. " failed ret=" .. tostring(ret), 2)
    end
end

local function default_ini_path()
    return "src/mhelper/data/zmc_motapp_demo_fak0.ini"
end

local function default_eni_path()
    return "src/mhelper/data/pse_servo1.xml"
end

local function submit_flow_packet(bridge)
    local packet = axfb.packet()
    packet:set_flow_config(1, 8, 1)
    bridge:push_packet(packet)
end

local function submit_power_packet(bridge)
    local packet = axfb.packet()
    packet:set_axis(0)
    packet:set_speed(20.0)
    packet:set_accel(100.0)
    packet:set_decel(100.0)
    packet:set_jerk(0.0)
    packet:set_direction(axfb.direction.finite)
    packet:power_on()
    bridge:push_packet(packet)
end

local function submit_move_packet(bridge, target_pos)
    local packet = axfb.packet()
    packet:move_abs(target_pos)
    bridge:push_packet(packet)
end

local ini_path = arg and arg[1] or default_ini_path()
local target_pos = arg and tonumber(arg[2]) or -52.0
local move_cycles = arg and tonumber(arg[3]) or 500
local eni_path = arg and arg[4] or default_eni_path()
local power_wait_ticks = 100

local bridge, ret = axfb.new()
fail_if(ret, "new")
fail_if(bridge:load_eni(eni_path), "load_eni")
fail_if(bridge:configure({
    ini_path = ini_path,
    ini_content = "",
    axis_num = 1,
}), "configure")
local status
status, ret = axfb.status()
fail_if(ret, "status")

submit_flow_packet(bridge)
submit_power_packet(bridge)

local inp = string.rep("\0", 16)
local oup = string.rep("\0", 16)
local total_cycles = power_wait_ticks + move_cycles
local latest = nil
for cycle = 1, total_cycles do
    if cycle == power_wait_ticks + 1 then
        submit_move_packet(bridge, target_pos)
    end

    ret, oup = bridge:run_cycle(inp, oup)
    fail_if(ret, "run_cycle")
    while true do
        local pop_ret = status:pop(bridge)
        if pop_ret ~= 0 then
            break
        end
    end
    local status_ret, state = status:get_axis_state2(0)
    if status_ret == 0 then
        latest = state
    end
    if latest and (cycle == 1 or cycle % 10 == 0 or cycle == total_cycles) then
        print(string.format(
            "cycle=%d axis=%d state=%d act=%.6f set=%.6f vel=%.6f acc=%.6f tick=%d",
            cycle,
            latest.axis_id,
            latest.axis_state,
            latest.act_pos,
            latest.set_pos,
            latest.set_vel,
            latest.set_acc,
            latest.tick))
    end
end

bridge:close()
