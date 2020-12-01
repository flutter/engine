Flutter Accessibility Library
==============

This accessibility library is a fork of the [chromium](https://www.chromium.org) accessibility code at commit
[4579d5538f06c5ef615a15bc67ebb9ac0523a973](https://chromium.googlesource.com/chromium/src/+/4579d5538f06c5ef615a15bc67ebb9ac0523a973).

You can find the corresponding upstream files for all of the files in
`ax/`, `ax_build/`, `base/`, and `gfx/` directory.

`ax/`: https://source.chromium.org/chromium/chromium/src/+/master:ui/accessibility/
`ax_build/`: https://source.chromium.org/chromium/chromium/src/+/master:build/
`base/`: https://source.chromium.org/chromium/chromium/src/+/master:base/
`gfx/`: https://source.chromium.org/chromium/chromium/src/+/master:ui/gfx/

Update to this Library
==============
Bug fixes to the forked files in the the four directories should proceed as usual.
New features or changes that change the behavior of the class are discouraged. All
of these changes will increase our tech debts, please only proceed as the last resort.

If you do need to make such change, please log the change at the end of this readme.

To Use this Library
==============
To use this library, each platform must provide their own implementations of
`FlutterAccessibility` and override the static method
`FlutterAccessibility::Create` to return their own subclasses. They can then
create an accessibility bridge using the `AccessibilityBridge` class.

The accessibility bridge can be updated by consuming the semantics updates sent
from the embedder API and produces accessibility tree in platform native format
to be sent to the native accessibility service.
