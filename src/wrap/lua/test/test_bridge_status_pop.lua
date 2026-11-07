-- Status pop/query smoke after a few run_cycle ticks.
-- AI modification signature: Composer
local helpers = require("helpers")

local bridge, status, axfb = helpers.open_bridge()

local flow = axfb.packet()
flow:set_flow_config(1, 8, 1)
bridge:push_packet(flow)

local power = axfb.packet()
power:set_axis(0)
power:set_speed(20.0)
power:set_accel(100.0)
power:set_decel(100.0)
power:power_on()
bridge:push_packet(power)

local inp = string.rep("\0", 16)
local oup = string.rep("\0", 16)
local got_state = false

for _ = 1, 32 do
    oup = helpers.run_cycle(bridge, inp, oup)
    helpers.drain_status(status, bridge)
    local basic_ret, basic = status:get_axis_state(0)
    if basic_ret == 0 and basic ~= nil then
        helpers.assert_true(type(basic.error_id) == "number", "basic error_id")
        helpers.assert_true(type(basic.tick) == "number", "basic tick")
        helpers.assert_true(type(basic.act_pos) == "number", "basic act_pos")
        helpers.assert_true(type(basic.set_pos) == "number", "basic set_pos")
    end
    local status_ret, state = status:get_axis_state2(0)
    if status_ret == 0 and state ~= nil then
        got_state = true
        helpers.assert_eq(0, state.axis_id, "axis_id")
        helpers.assert_true(type(state.error_id) == "number", "extended error_id")
        helpers.assert_true(type(state.tick) == "number", "extended tick")
        helpers.assert_true(type(state.set_vel) == "number", "extended set_vel")
        helpers.assert_true(type(state.set_acc) == "number", "extended set_acc")
        helpers.assert_true(type(state.set_position) == "number", "extended set_position")
        helpers.assert_true(type(state.act_position) == "number", "extended act_position")
    end
end

helpers.assert_true(got_state, "expected at least one get_axis_state2 success")

local seq_ret, seq = status:ack_seq()
helpers.assert_eq(0, seq_ret, "ack_seq ret")
helpers.assert_true(type(seq) == "number", "ack_seq value")

local flow_ret, flow_st = status:get_flow_status()
helpers.assert_eq(0, flow_ret, "get_flow_status ret")
helpers.assert_true(type(flow_st) == "table", "get_flow_status value")

bridge:close()
print("test_bridge_status_pop OK")
