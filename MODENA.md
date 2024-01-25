# Modena

Modena is a city in Italy.

## Running Modena

`mod <original-installation-path>`

Then answer questions, and the wizard will finally tell you where you should go — e.g., if your character's name is
Charlie, then, once you're in DOSBox, `cd Charlie` down from the original game directory and run the game as usual.
**Don't create your character again from within the game** — simply load the Modena-created save file (there will be
only one, and upon pressing 'L' the game will pick it up automatically).

## Known issues

1. Only the console interactive mode is currently available. Prospective command-line options have been documented,
but none (except for the installation path) has been implemented so far.

2. Only the DOS version released in 1990 is currently supported. Modena is smart enough to unpack it if needed and only if
neede, but coordinate edits (fortress and port locations) are written at fixed offsets. More flexible logic is coming soon.

3. There is currently no way to run the same map with newly randomized treasures and roaming armies, as one random number
generator seed controls both. Also, the nearest rebel lord is always the weakest one; you may consider it a bug or a feature.

## Attributions

The godspeed quotes belong to Mark Twain and Winston Churchill, as all famous quotes do.
