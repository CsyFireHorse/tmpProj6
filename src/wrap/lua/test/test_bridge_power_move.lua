-- Single-axis flow/power/move integration test with end-state assert.
-- AI modification signature: Composer
local helpers = require("helpers")

local target_pos = -5.0
local power_wait_ticks = 100
local move_cycles = 400
local pos_tol = 1e-3

local bridge, status, axfb = helpers.open_bridge()

local flow = axfb.packet()
flow:set_flow_config(1, 8, 1)
bridge:push_packet(flow)

local power = axfb.packet()
power:set_axis(0)
power:set_speed(20.0)
power:set_accel(100.0)
power:set_decel(100.0)
power:set_jerk(0.0)
power:set_direction(axfb.direction.finite)
power:power_on()
bridge:push_packet(power)

local inp = string.rep("\0", 16)
local oup = string.rep("\0", 16)
local total_cycles = power_wait_ticks + move_cycles
local latest = nil

for cycle = 1, total_cycles do
    if cycle == power_wait_ticks + 1 then
        local move = axfb.packet()
        move:move_abs(target_pos)
        bridge:push_packet(move)
    end

    oup = helpers.run_cycle(bridge, inp, oup)
    helpers.drain_status(status, bridge)
    local status_ret, state = status:get_axis_state2(0)
    if status_ret == 0 then
        latest = state
    end
end

helpers.assert_true(latest ~= nil, "expected axis state2 after cycles")
helpers.assert_near(target_pos, latest.act_pos, pos_tol, "act_pos")
helpers.assert_near(target_pos, latest.set_pos, pos_tol, "set_pos")

-- Verify the actual binding returns only the unified terminal-result fields.
local ret, results = status:get_cmd_results()
helpers.assert_eq(0, ret, "get_cmd_results ret")
local completed = false
for _, result in ipairs(results) do
    if result.command_type == 0x43 then -- FBM_MOVE_ABSOLUTE.
        local fields = {"id", "command_type", "target_index", "target_kind", "status",
                        "io_count", "error_code", "tick", "data", "data_ext"}
        local count = 0
        for _ in pairs(result) do count = count + 1 end
        helpers.assert_eq(#fields, count, "result field count")
        for _, field in ipairs(fields) do
            helpers.assert_true(type(result[field]) == "number", field)
        end
        helpers.assert_eq(1, result.target_kind, "Axis target")
        helpers.assert_eq(0, result.target_index, "axis index")
        helpers.assert_eq(4, result.status, "DONE")
        helpers.assert_eq(0, result.error_code, "result error")
        helpers.assert_true(result.id ~= 0, "command ID")
        helpers.assert_true((result.data & 0xffffffff) <= result.tick, "start cycle")
        completed = true
    end
end
helpers.assert_true(completed, "absolute move terminal result")

bridge:close()
print("test_bridge_power_move OK")
