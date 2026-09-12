![Logo for Taggy (inspired by BFDI)](TaggyLogo.png)
Taggy simple standalone CLI tool that reads & writes ID3v2 tags to .mp3 files.
This is just a hobby project for adding tags to `.mp3` files and will
be rather lacking in advanced support. 

# Installation
You can install Taggy from the pre-built binaries in the Releases tab, 
or build it yourself with CMake. 
If you do end up building from the source, 
the executable can be found at `cmake-build-release/Taggy.exe`.

# Usage
Taggy can be used just by running the executable through the command line:
```
Taggy.exe [input-file] [options] [-o [output-file]]
```
If only the input file is provided, Taggy will just read out the data in the 
`.mp3` files to the console.

If an output file is provided, Taggy will create a new `.mp3` file
with the raw _music data_ from the input, but without any tags.

Tags can be further added through the options:

```
-h, --help            Display the help screen
-o                    Writes the destination file to the path specified after
--artist=<name>       Sets the artist name
--title=<name>        Sets the title
--album=<name>        Sets the album name
--albart=<name>       Sets the album artist name
--disc=<number>       Sets the disc number
--track=<number>      Sets the track number
--date=<date>         Sets the release date, format is YYYY-MM-DD
--explicit            Adds the explicit tag
--lyrics=<lrcfile>    Sets the (unsycned) lyrics from a file source
--slyrics=<lrcfile>   Sets the (sycned) lyrics from a file source
--cover=<imagefile>   Sets the images from a file source
-T...=<value>         Adds a custom text field tag
```