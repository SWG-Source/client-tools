SWG Source 64-bit client (beta)
===============================

This is an add-on for your existing SWG Source 3.0 client. It adds a
64-bit version of the game next to the normal one. Your normal client is
not changed, and you can switch back at any time.

The 64-bit client isn't limited to the 4 GB of memory that a 32-bit
program can use.


What you need
-------------

1. A working SWG Source 3.0 client (the folder with SwgClient_r.exe).
2. 64-bit Windows 10 or 11.
3. The Microsoft DirectX End-User Runtime (June 2010). Most gaming PCs
   already have it. If the game says d3dx9_43.dll or d3dx9_31.dll is
   missing, install it from Microsoft:
   https://www.microsoft.com/en-us/download/details.aspx?id=8109
   (or search for "DirectX End-User Runtimes June 2010").


Install
-------

1. Unzip this package into your SWG Source client folder, the one that
   has SwgClient_r.exe in it. You should end up with:

       SwgClient_r.exe        (your normal client, unchanged)
       SwgClient_x64.bat      (new: starts the 64-bit client)
       x64\                   (new: the 64-bit client's files)

2. Double-click SwgClient_x64.bat.

   The first time, it copies your client's sound and movie libraries into
   the x64 folder (this takes a second). Then the game starts and you log
   in as usual.


Going back to the normal client
-------------------------------

Start SwgClient_r.exe (or your usual launcher) as before.

To remove the 64-bit client completely, delete SwgClient_x64.bat and the
x64 folder.


Good to know
------------

- It uses the same game files, settings, login and characters as your
  normal client.
- Sound and movies go through a small helper program,
  x64\helper-x86\miles-host.exe, because they only exist as 32-bit
  libraries. It starts and stops with the game.
- In-game voice chat is not available in the 64-bit client.
- This is a beta. If something goes wrong, use the normal client and
  report it in the SWG Source Discord: say what you were doing, and
  attach any SwgClient_r*.txt or *.mdmp file from the client folder.
