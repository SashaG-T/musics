# Musics!
Musics is inspired from the QBasic PLAY command.
Improvements are you can set the ADSR Filter and add a Reverb!
I would like to improve this into a suite of command line tools to generate some chipetune music with mutliple channels.
It was also an exercise in Endianness.

# Compiling
	gcc musics.c -o musics.exe
(or however you you like to compile your shtufs)

# Usage
	./musics.exe [filename]
* filename is the name of the output file if not provided will default to "wavefile.wav".

Pipe into it! (Recommended):

	cat "Canon in D.txt" | ./musics.exe "Canon in D.wav"

# Input Structure
Musics reads from standard input!

	[WaveType]
	[AttackTime] [DecayTime] [SustainLevel] [ReleaseTime]
	[ReverbDelay] [ReverbDecayFactor] [Mix]
	[SongData]
	.

* WaveType: 0 for sin, 1 for square. (integer)
* AttackTime: Attack Ramp in Samples. (integer)
* DecayTime: Decay Ramp in Samples. (integer)
* SustainLevel: Level to sustain wave after attack and decay. (0.0 - 1.0 float)
* RelaseTime: Release Ramp in Samples. (integer)

* SongData:
  * t[number] to set tempo (in Beats Per Minute)
  * o[number] to set octave
  * l[number] to set length of precceding notes (l1 for whole note, l2 for half note, l4 for quarter note, etc.)
  * < to go down an octave
  * \> to go up an octave
  * \- to set precceding note as flat
  * \+ to set precceding note as sharp
  * a-g for notes (no uppercase and currently set to an arbitrary max of 100 notes)
  * \` for a pause (l affects pauses too!)
  * . is used to indicate end of data (you can put whatever you want after it such as notes)
  * Any other character is ignored so you can used unused characters to help visually organize the file.
* All whitespace is treated the same (it can all be on one line if you really wanted)

# Examples
Check out "lostwoods.txt" or "Canon in D.txt".
