-- Golden path: fields string + memory.script dual-write on one live image.
-- AI modification signature: Composer
local helpers = require("helpers")
local assemble = require("memory_pou_assemble")

local fields = [[
AXIS_REF axis;
FbtPower power;
FbtHalt halt;
FbtMoveRelative moveRel;
FbtMoveAdditive moveAdd;
FbtMoveAbsolute move;
]]

local script =
    "power.Enable = true; " ..
    "power.bRegulatorOn = true; " ..
    "power.bDriveStart = true; " ..
    "move.Position = 15; " ..
    "move.Velocity = 20; " ..
    "move.Acceleration = 40; " ..
    "move.Deceleration = 40; " ..
    "delay 6; " ..
    "move.Execute = true; " ..
    "delay 150; " ..
    "out move.Done; " ..
    "expect move.Done = true"

local function stmt_off(text, needle)
    local pos = text:find(needle, 1, true)
    helpers.assert_true(pos ~= nil, "needle not found: " .. needle)
    return pos - 1
end

local pou, image, exec_list, axis = assemble.assemble(fields, { cycle_time = 10000 })
helpers.fail_if(pou:parse_script(script), "parse_script")

local cycles = 200
for _ = 1, cycles do
    assemble.run_cycle(pou, exec_list, axis)
end

helpers.fail_if(pou:error(), "pou error after run")

local pos = axis:act_position()
helpers.assert_near(15.0, pos, 0.5, "act_position after memory pou move")

local out_off = stmt_off(script, "out move.Done")
local expect_off = stmt_off(script, "expect move.Done = true")
local count = pou:capture_count()
helpers.assert_true(count >= 2, "capture_count")

local out_cap = pou:capture_at(0)
local exp_cap = pou:capture_at(1)
helpers.assert_true(out_cap.line == 1, "out line")
helpers.assert_true(out_cap.stmtOff == out_off, "out stmtOff")
helpers.assert_true(out_cap.kind == 0, "out kind")
helpers.assert_true(#out_cap.value == 1 and string.byte(out_cap.value) ~= 0, "out value")

helpers.assert_true(exp_cap.line == 1, "expect line")
helpers.assert_true(exp_cap.stmtOff == expect_off, "expect stmtOff")
helpers.assert_true(exp_cap.kind == 1, "expect-ok kind")
helpers.assert_true(#exp_cap.value == 1 and string.byte(exp_cap.value) ~= 0, "expect value")

assemble.destroy_fbs(exec_list)
print("test_memory_pou_fields ok pos=" .. tostring(pos))
