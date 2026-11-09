# 64-bit client add-on package

Packages the Release x64 client as an **add-on for an existing SWG Source 3.0 client folder**. Players unzip it into the folder that holds `SwgClient_r.exe` and start `SwgClient_x64.bat`. Their normal 32-bit client is not changed.

## Why an add-on, in a subfolder

- **Sound and movies need the 3.0 install.** The x64 client plays sound and Bink movies through the x86 helper `miles-host.exe`, which loads the original 32-bit Miles and Bink libraries. The player already has those in the 3.0 folder, so the package never redistributes them: `setup.bat` copies the player's own `Mss32.dll`, `binkw32.dll` and `miles\` next to the helper. Miles looks for its providers relative to `Mss32.dll`, so they must sit in `helper-x86\miles\`.
- **The game data doesn't change.** The x64 client reads the same `.tre`/`.toc` files and configs, so the download is only the program files (about 14 MB).
- **The x64 DLLs need their own folder.** The renderers and support DLLs have the same names as the 32-bit ones. Windows loads DLLs from the executable's folder first, so `x64\SwgClient_r.exe` gets the x64 copies even though the 32-bit ones are in the client folder.
- **The launcher keeps the client folder as the working directory.** `SwgClient_x64.bat` changes to the client folder, then starts `x64\SwgClient_r.exe -- @x64\x64.cfg`. The client loads `client.cfg` (and through it `login.cfg`, `live.cfg`, `options.cfg`, `user.cfg`) as usual, then `x64.cfg`, which only selects the helper. The helper inherits the same working directory, so the relative paths in `x64.cfg` resolve inside `x64\helper-x86\`.

## Layout

```
SwgClient_x64.bat                 launcher
x64\SwgClient_r.exe               Release x64 game
x64\gl05_r.dll gl06_r.dll gl07_r.dll dpvs.dll libxml2.dll
x64\x64.cfg                       [ClientAudio] devMilesHost / devMilesDll / devMilesUploadBudget
x64\setup.bat                     one-time copy of the player's Miles/Bink files
x64\README-x64.txt                player guide
x64\VERSION.txt                   build label
x64\MANIFEST.sha256               hashes of every packaged file
x64\helper-x86\miles-host.exe     Release x86 media helper
```

`DllExport.dll` is never shipped: the renderers' delay-load hook resolves it to the game executable. The renderers need Microsoft's x64 `d3dx9_43.dll` and `d3dx9_31.dll`, which come with the DirectX End-User Runtime (June 2010); the player guide points there.

## Build and package

1. Build the Release x64 game, renderers, DPVS and libxml2 with the Miles development backend, and the Release x86 host. The steps and MSBuild properties are in `tools/miles-bridge/README.md` (Explicit development project build) and `tools/build-client-deps/README.md`. Pass `--configuration Release` to every bridge target.
2. Put the six x64 outputs (`SwgClient_r.exe`, `gl05_r.dll`, `gl06_r.dll`, `gl07_r.dll`, `dpvs.dll`, `libxml2.dll`) in one folder.
3. Package:

   ```
   python tools\package-client-x64\package.py --game-dir C:\build\x64-release --host C:\build\miles\host\x86\Release\<invocation>\miles-host.exe --out C:\build\SwgClient-x64-beta.zip --label "<version text>"
   ```

The script checks that the six game files are x64 and the host is x86, writes the templates with Windows line endings, and records SHA-256 hashes in `x64\MANIFEST.sha256`. The zip uses fixed timestamps, so the same inputs give the same zip.

## Checked so far

The first package was built from the Release binaries recorded in `tools/test-wire-compatibility/live-session.md` (game `307042f7…`, media host `67c2c17c…`). The game source at this branch's base is identical to that build, and the host source is identical to the repaired host. In the Windows build VM, on a copy of a 3.0 client:

- the zip unzipped into the client folder as laid out above;
- `setup.bat` copied `Mss32.dll`, `binkw32.dll` and all 12 Miles providers;
- `x64\SwgClient_r.exe -- @x64\x64.cfg`, started from the client folder, loaded `dpvs.dll` and `libxml2.dll` from `x64\` and got past audio setup. A first run without `devMilesUploadBudget` stopped with the bridge's explicit missing-budget error, which is why `x64.cfg` sets it.

That check ran without a desktop session, so it stops before the game window. Logging in and playing from this layout still needs an interactive Windows run.
