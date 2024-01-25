# Map generator

The map generator's goal is producing a map that looks _loosely_ like one of the original maps but is still fun to play on.
It's specified in `algos/ground` and is (supposed to be) used both by [Modena](MODENA.md) and the public domain reimplementation.

## Data structures

`map::Geology`, defined in `geology.h`, is the _specification_ structure that can be used to initialize the implementation object, as well as copied from one.
It is a mere parameter container. The most firmly defined parameters in no realistic need of tweaking, such as map dimensions, are softcoded in `map_defs.h`.

`map::Continent`, defined in `geography.h`, is the _implementation_ structure used to represent a complete generated map.

`map::MapHolder` is a template class that stores the contents of a 2D array in a 1D vector. (We don't use any matrix library at the moment, and STL
does not provide dynamically sized and strongly typed 2D containers on its own.) `map::Map` is a templated 2D array. `map::Continent::map` is the field
that stores the generated map authoritatively; its legend (e.g. that `cPlain` codes a road tile) is provided in `map_defs.h`. The legend is chosen in such
a way that it should be possible to dump out the map in human-readable character format, which `operator<<(FILE& out, const ChrMap& map)` and
`operator<<(std::ostream& out, const ChrMap& map)` do.

The rest of `map::Continent` fields are either relevant before land mass formation (e.g. `minerals`) or contain politically/economically important objects
placed on the map after individual landmasses have been defined.

## Steps

There are four major stages of map generation:

1. Scattering primaeval elements and forming continental plates during `geo_formation.cpp`;
2. Refinement of the shoreline and impenetrable terrains (woods, mountains and deserts) in `geo_landscape.cpp`;
3. Placement of fortresses and roads in `geo_power.cpp`;
4. Placements of ports, roaming gangs, treasures and other special tiles in `geo_wealth.cpp`.

### Geological formation

`map::Continent::minerals` can be understood as mineral pigments, or colors, scattered into the primaeval ocean.
Each tile on the map is under influence of all of them, but the influence decays following the Gaussian law
(exponent of distance square). Land only emerges where there is a clear winner; if there is no dominant color,
the square becomes an ocean square. In addition, `map::Continent::segregate()` marks tiles neighboring a color
different from theirs as ocean tiles. Distance between continents can be further increased at later step.

The decay rate can either be uniform or depend on the color; the dependence is determined by the decay polynomial,
which can be constant, linear or quadratic; its coefficients are `map::Geology::{kDecay|kDPow1|kDPow2}`, respectively.
A uniform decay rate produces Archipelia-like islands; a steeply uneven decay rate produces a dominant continent
of color 1 with outlying islands (like Britain off the shores of Europe) of greater color IDs. Color 0 denotes ocean.
Use `map::Continent::setComposition()` to regulate the diversity of mineral pigments/colors; an argument of 1 would
produce a single, Saharia-type large continent occupying most of the map.

Typically, you'd want to place the capital (king's castle) on the first continent ("Continentia"); if you don't,
the character is going to play without human units (which is fine) and without promotions (which is so-so).
`map::Geology::kGround` controls how that last patch of yet civilized land will be formed. Assign `1/kOnLand` to
it to make it a (likely) part of a dominant continent, `0/kOcean` to start on a bridge mid-sea, `kIsland` to produce
a separate island or kMature to skip its generation completely (usually applies to higher continents). Also,
if the character is in need of learning magic, set `kWizard` to something different from `kMature`; in addition to
the above options, e.g. `kIsland` (same island as the king's castle), you can use `kHermit` to generate a completely
separate island for the magic school.

There will always be a bridge leading to the king's castle gate because I found it unnecessarily complicated to
generate realistically placed "magical borders" around the capital, and if I don't, the hero gets attacked by roaming
units right out of the door, which is not intended by the original design. (The benefit of a bridge is that followers
can't enter it; the added benefit is that a boat can appear there and navigate it; "the best kind of a home", as my
wife said when she saw it.)

### Landscape formation

(Configuration description)

### Geopolitics

(Configuration description)

### Geoeconomics

(Configuration description)

## Example

For an example of `map::Geology` setup to closely match the layout of the original Four Continents, see `mod.cpp`.
