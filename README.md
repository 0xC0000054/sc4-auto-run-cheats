# sc4-auto-run-cheats

A DLL Plugin for SimCity 4 that automatically executes cheat codes when loading the game or a city.

The plugin can be downloaded from the Releases tab: https://github.com/0xC0000054/sc4-auto-run-cheats/releases

## System Requirements

* Windows 10 or later

The plugin may work on Windows 7 or later with the [Microsoft Visual C++ 2022 x86 Redistribute](https://aka.ms/vs/17/release/vc_redist.x86.exe)
installed, but I do not have the ability to test that.

## Installation

1. Close SimCity 4.
2. Copy `SC4AutoRunCheats.dll` and `SC4AutoRunCheats.ini` into the Plugins folder in the SimCity 4 installation directory.
3. Configure the plugin settings, see the *Configuring the plugin* section.

## Configuring the plugin

1. Open `SC4AutoRunCheats.ini` in a text editor (e.g. Notepad).
Note that depending on the permissions of your SimCity 4 installation directory you may need to start the text editor with administrator permissions to be able to save the file.

2. Adjust the settings in the `[Startup]`, `[Tile]`,`[EstablishedTile]` and `[UnestablishedTile]` sections to your preferences.

3. Save the file and start the game.

### Settings overview

The commands or cheat codes will be applied at the specified stages in the game's loading process.
The `RunOnce` versions will only be applied for the first matching city tile that is loaded.

The command strings can use one of two formats:

* A comma-separated list of commands/cheats.
* A `File:<path>` string that specifies an external file containing the commands/cheats.

[Startup]: The commands in this section will be run once when the game starts up.    
[Tile]: The commands in this section will be run when loading either an established or unestablished city tile.    
[EstablishedTile]: The commands in this section will be run when loading an established city tile or when a city is established.    
[UnestablishedTile]: The commands in this section will be run when loading an unestablished city tile.

#### External File Format

The external file must place each command or cheat on its own line.
The cheats/commands will be executed sequentially starting from the top of the file.

Example:

```
You don't deserve it
TerrainQuery
```

### ZoneBitmap command

The ZoneBitmap command can be used to load a zone bitmap that will be applied when loading a city.
The command syntax is `ZoneBitmap <path>`.

The required zone bitmap size varies depending on the city size. The zone bitmap is one row taller
than the city tile because the last row is used to specify the colors for each zone and a few other options.

Small city tile (4096 cells): 64x65    
Medium city tile (16384 cells): 128x129    
Large city tile (65536 cells): 256x257   

The command will write a text file containing the zone colors in the same location as the image file.
The color information can be used to create an override plugin for the Zone Manager's displayed zone colors.

#### Control Row

The last row in the zone bitmap is the control row. The first 16 pixels from the bottom left indicate which colors
map to the game's 16 zone types. Currently only zone types 1 through 9 are usable.
The next 3 pixels control various zone layout options, these options are enabled if the corresponding pixel is set
to a RGB value of 255,165,0.

Pixel 16 - Custom Size: Use a custom size for the zone grid. Equivalent to holding down Control when using the zoning tool.   
Pixel 17 - Alternate Layout: Use an alternate layout when placing the zone grid. Equivalent to holding down Alt when using the zoning tool.   
Pixel 18 - Place Streets: Place streets in the zoned areas. Equivalent to not pressing Shift when using the zoning tool.   

## Troubleshooting

The plugin should write a `SC4AutoRunCheats.log` file in the same folder as the plugin.    
The log contains status information for the most recent run of the plugin.

# License

This project is licensed under the terms of the MIT License.    
See [LICENSE.txt](LICENSE.txt) for more information.

## 3rd party code

[gzcom-dll](https://github.com/nsgomez/gzcom-dll/tree/master) Located in the vendor folder, MIT License.    
[Windows Implementation Library](https://github.com/microsoft/wil) - MIT License    
[Boost.Algorithm](https://www.boost.org/doc/libs/1_84_0/libs/algorithm/doc/html/index.html) - Boost Software License, Version 1.0.    
[Boost.PropertyTree](https://www.boost.org/doc/libs/1_84_0/doc/html/property_tree.html) - Boost Software License, Version 1.0.

# Source Code

## Prerequisites

* Visual Studio 2022

## Building the plugin

* Open the solution in the `src` folder
* Update the post build events to copy the build output to you SimCity 4 application plugins folder.
* Build the solution

## Debugging the plugin

Visual Studio can be configured to launch SimCity 4 on the Debugging page of the project properties.
I configured the debugger to launch the game in a window with the following command line:    
`-intro:off -CPUCount:1 -CPUPriority:high -w -CustomResolution:enabled -r1920x1080x32`

You may need to adjust the resolution for your screen.
