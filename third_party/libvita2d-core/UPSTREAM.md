libvita2d core, MIT, https://github.com/xerpi/libvita2d
Commit a8f15ab09d5233f0a4e4ad0e8f6ade0da888cbed
Only core drawing/texture/util sources and upstream compiled shader objects.
Local changes: reject failed GXM initialization; drain and detach scanout before framebuffer cleanup.
GPU command pool and allocation fallbacks use uncached memory; release allocation on base/map failures.
