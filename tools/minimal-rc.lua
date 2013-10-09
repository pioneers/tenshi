-- Standard awesome library
require("awful")
require("awful.autofocus")
require("awful.rules")

awful.tag({ 1 }, 1, awful.layout.suit.tile)

-- {{{ Rules
awful.rules.rules = {
    -- All clients will match this rule.
    { rule = { },
      properties = { focus = true } },
}
-- }}}

client.add_signal("manage", function (c, startup)
    if not startup then
        -- Put windows in a smart way, only if they does not set an initial position.
        if not c.size_hints.user_position and not c.size_hints.program_position then
            awful.placement.no_overlap(c)
            awful.placement.no_offscreen(c)
        end
    end
end)

