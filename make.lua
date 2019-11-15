local lm = require 'luamake'

lm.rootdir = 'src'
lm:lua_library 'lml' {
    sources = "*.cpp"
}

lm:build "test" {
    "$luamake", "lua", "test/test.lua",
    deps = { "lml" }
}
