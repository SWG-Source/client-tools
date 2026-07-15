# Multi-controller Input

The `x64-dx9-vanilla-inputreborn` branch uses SDL 3.4.10 for native multi-device controller input while retaining DirectInput 8 for keyboard and mouse. Up to eight independently bound devices can be active at once, including separate flight sticks, throttles, rudder pedals, and mapped Xbox, PlayStation, or Switch-style gamepads.

## Runtime

`SDL3.dll` is tracked under `deps/x64/bin` and copied by both x64 staging scripts. The build uses the matching headers and import library from `deps/x64/include/SDL3` and `deps/x64/lib/SDL3.lib`; no external SDL installation or reference checkout is required.

SDL input is enabled by default. To temporarily return joystick handling to DirectInput, add this setting to `client.cfg`:

```ini
[ClientDirectInput]
useSdlInput=false
```

Keyboard and mouse input remain on DirectInput in either mode.

## Keymaps

The input-map format advances from version `0006` to `0007`. Version `0007` stores a controller slot on each joystick binding and a stable SDL GUID/name record for each assigned slot. Version `0006` keymaps remain readable and are migrated when saved.

Use the existing Find Controllers action after attaching or removing hardware. Hot-plug changes also trigger a rescan and clear held input so a disconnected axis cannot leave a ship turning or moving.

## Provenance

The controller implementation was ported from `swgsais/client` commit `f31c236b53bf70640cba589aab2370fd51424969`, referenced by `swgsais/GalaxiesReborn` commit `d349c418caf3bded6ffe4059ac17a1440dc56616`. Only the multi-device input changes and related UI null guards were carried over; that checkout is not a build-time or runtime dependency.

SDL 3.4.10 is distributed under the zlib license. The exact upstream license is included at `deps/x64/licenses/SDL3.txt`. The vendored development archive is `SDL3-devel-3.4.10-VC.zip` from the official SDL release and has SHA-256 `e2b336b10b037934af98308027410732ef7b22f2c6697d58092aa1c209fae7d7`.
