# Royal Reward

_"The High Command's maps never change... Never!"_ (_Ilya Kormiltsev_)

The Royal Reward project has been started to work around the aforementioned inconvenience.

## Goals

* Our short-term goal has already been achieved. New maps (consistent with original tileset
requirements) can be generated and installed and the gameplay stays feature complete, up to
the point of solving the puzzle and unearthing the Golden Key.
* Our medium-term goal is releasing a configurable mission generator (and possibly a manual
mission editor) that can be used in a cross-platform fashion on all major desktop platforms
(Windows, Mac and Linux) and is accessible to an average user. (See: [Modena](MODENA.md).)
* Our long-term goal is a reimplementation of the original game — one that does not use any
of the original artwork or proper names and can be tailored to a theme of the user's choice
— we can readily think of the Middle East, the Edo period in Japan, the British Empire, the
Soviet Union, or any other fictional world.

### Isn't that exactly what the OpenKB project is about?

First, while the OpenKB project has achieved some progress in research, its development and
release progress leaves much to be desired.

Second, GPL belongs in the dustbin of history.

## Means

### Dependencies

* [Meson](https://mesonbuild.com)
* [Boost](https://boost.org) (we currently use `filesystem`, which requires `system`)

#### Prospective dependencies

We are considering [IUP](https://www.tecgraf.puc-rio.br/iup/) for the map editor and similar GUI tools.

We are in search of a permissive-license graphical game engine that works best for a step-by-step,
mostly keyboard-controlled game.
(There are a few options we are currently considering which this margin is too narrow to contain.)

### Building and running

To build, run `meson build && cd build && ninja` on the command line.

`build/mod <path-to-original-game-installation>` is the interactive tool that installs new maps.

If you are unable to run the game, consult [Modena's help page](MODENA.md) regarding the steps
needed to clear the cache directory.

#### I am on Windows (10/11), shall I wait for a port?

Let's assume you have the original game at `c:\Users\BelovedMe\Downloads\KB`
and already have [DOSBox](https://www.dosbox.com/download.php?main=1) installed.

Install WSL (old way: `cmd.exe` -> "Run as administrator", `wsl --install`; new way: find WSL in the Windows Store).
It comes with Ubuntu, complete with Git and a C++ compiler. In the Ubuntu terminal, run:

```
sudo apt-get install meson libboost-filesystem-dev
git clone https://github.com/treeswift/royalreward
cd royalreward
meson build && cd build && ninja
build/mod /mnt/c/Users/BelovedMe/Downloads/KB
```

You only need to do it once. Lines beginning with `sudo` will ask for your just-chosen password.

To re-generate a game, enter the Ubuntu terminal again and re-type (or re-paste):

```
~/royalreward/build/mod /mnt/c/Users/BelovedMe/Downloads/KB
```

You can pin `Ubuntu` and `DOSBox` to the taskbar (right click -> "Pin") for convenience.

#### I am on a Mac

There should be `meson`, `git` and `boost` available in Macports.

## Status

### 2024-01-26 — update from @treeswift

It's gotten to the point at which it's more rewarding to play it than to tinker with it, and it's
going to be like that for a while. I like the ascetic counterintuitiveness of the new labyrinths;
I absolutely love the new Desertia, now more of an impact crater than a caldera.

![woodlands](https://raw.githubusercontent.com/treeswift/royalreward/master/mapshots/amazonas.png)![crater](https://raw.githubusercontent.com/treeswift/royalreward/master/mapshots/crater.png)

Also, I have promises to keep and lines to code before I sleep.

My next steps towards the long-term goal will most likely involve writing a GUI generator-analyzer
tool, at the same time trying out a sufficiently permissive-licensed GUI library (I have promising
candidates) in the process. Following the urban theme introduced by _Modena_ (as well as the Swiss
theme introduced by "Lenin's Prize"), I am going to call this next tool _Geneva_.

For now,
* let me know if you have any issues building or using Modena;
* let me know if you have any issues _playing_ on "modenized" maps;
* if you get me a 1995 DOS version or a port for another OS (other than Mac OS 9; I have that one),
odds are Modena will eventually support it, too.

Other than that… Pray for Ukraine and Russia. Pray for Israel and the Middle East. Pray for the United States.

Antony Bulatovich is a saint, and the world is a book.

Dina Talaat is beautiful.

# Legal and administrative

## Regarding our contributions

Everything contributed to this repository has been released into the public domain worldwide.
(If your jurisdiction does not recognize public domain,
[these four boxes](https://infogalactic.com/info/Four_boxes_of_liberty) may be of some use.)

## Regarding other people's contributions

_King's Bounty_ is a title released by (and has long been a registered trademark of) New World Computing.
We **DO NOT** distribute the original game. It used to be available on [GOG](https://gog.com).

We relied on the following information sources to learn more about the original game:
* [Let's Play](https://lparchive.org/Kings-Bounty/)
* [Modding Wiki](https://moddingwiki.shikadi.net/wiki/King%27s_Bounty)
* [RPGC Shrines](https://shrines.rpgclassics.com/genesis/kingbounty/)
* [GameFAQs](https://gamefaqs.gamespot.com/pc/577442-kings-bounty/faqs/7416)
* …the original game manual (if all else fails, read the manual)

## Regarding your contributions

Everything contributed to this repository is released into the public domain. It means that the only right
you retain regarding your contribution is the right to establish the fact that you have been the author;
you cannot in any way limit or restrict how your contribution is being used.

The exact legal wording we use is the [Unlicense License](LICENSE).

Public domain allows unlimited redistribution and/or creation of derivative works. You are therefore free
to fork this repo and license your fork on your own terms, or build a software package and distribute it
on your own terms. Just keep in mind that it won't in any way restrict anyone's freedom to use the contents
of this original repository.
