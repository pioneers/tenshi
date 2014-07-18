local units = {}

-- SI prefixes
units.mega = 1e6
units.kilo = 1e3
units.mili = 1e-3
units.micro = 1e-6
units.nano = 1e-9

-- Conversion factors into standard units used in the API
units.inch = 2.54               -- to cm
units.pound = 453.592           -- to g
units.deg = math.pi / 180.0     -- to radians

return units
