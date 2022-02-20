# ChomikBox MPRIS2 server
This program will let you control ChomikBox's media player, running under Wine, with any MPRIS2 client (media controls in most desktop environments, [KDE Connect](https://kdeconnect.kde.org/), etc).

It will probably only work with ChomikBox 2.0.8.2 exactly (`sha256(chomikbox.exe) = 13a500de0c270d289c30d772283fd332d4b427d196c80f0b166d8c53762a6386`) because of the numerous hardcoded offsets.

## Building
On Debian 11, install the packages listed in [bdeps.sh](./bdeps.sh#L6). Adjust as appropriate for your distribution. Then, a simple `make` should result in:
- hook.dll (PE)
- withdll.exe (PE)
- mpris-server.dll (ELF, despite the name. Wine can load it.)

## Usage
Copy the build outputs (or at least mpris-server.dll, you can specify paths to the other ones) to the directory chomikbox.exe is in. Then launch the application with:

    $ wine withdll.exe /d:hook.dll chomikbox.exe

## Limitations
- Playlists are not supported (not exposed over MPRIS)
- The `Seeked` signal is not emitted
- Looping one track is not supported (it will loop the whole playlist)
