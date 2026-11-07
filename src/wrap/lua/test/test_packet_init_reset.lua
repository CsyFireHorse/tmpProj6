-- packet:init clears capacity; packet:reset appends FBM_RESET.
-- AI modification signature: Composer
local helpers = require("helpers")
local axfb = require("zmc.axfb")

local probe_len = 32

local packet = axfb.packet()
helpers.assert_eq(0, packet:init(), "init")

local ret, can = packet:can_append(probe_len)
helpers.assert_eq(0, ret, "can_append after init")
helpers.assert_eq(1, can, "room after init")

-- Fill payload until can_append reports full for probe_len.
local fill_guard = 0
while true do
    fill_guard = fill_guard + 1
    helpers.assert_true(fill_guard < 4096, "fill loop overflow")
    ret, can = packet:can_append(probe_len)
    helpers.assert_eq(0, ret, "can_append fill")
    if can == 0 then
        break
    end
    packet:set_axis(0)
    packet:move_abs(0.1 * fill_guard)
end

helpers.assert_eq(0, packet:init(), "init clears full packet")
ret, can = packet:can_append(probe_len)
helpers.assert_eq(0, ret, "can_append after clear")
helpers.assert_eq(1, can, "room restored after init")

packet:set_axis(0)
packet:move_abs(-12.5)
ret, can = packet:can_append(probe_len)
helpers.assert_eq(0, ret, "can_append before reset")
helpers.assert_eq(1, can, "room before reset")

local before_ret, before_can = packet:can_append(probe_len)
helpers.assert_eq(0, before_ret, "snapshot can_append")
helpers.assert_eq(0, packet:reset(), "reset appends FBM_RESET")
-- After append, room for the same probe must not increase.
local after_ret, after_can = packet:can_append(probe_len)
helpers.assert_eq(0, after_ret, "can_append after reset")
helpers.assert_true(after_can <= before_can, "reset must consume payload room")

helpers.assert_eq(0, packet:init(), "final init")
ret, can = packet:can_append(probe_len)
helpers.assert_eq(0, ret, "can_append final")
helpers.assert_eq(1, can, "room after final init")

print("test_packet_init_reset OK")
