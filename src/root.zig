const std = @import("std");
const mach = @import("mach");
const main = @import("main.zig");

pub const App = main.App;
pub const app = mach.App(App);
pub const target = mach.Target{ .title = "F-22 Game" };
