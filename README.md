# MHW-ClassPropDump
A hasilty written program to dump the reflection/debug information ("DTI") from MT Framework games.

The majority of this code was written just to "make it work" while reverse engineering. Expectations of the code quality should be set properly (read: it's bad code).

## Usage
1. Get the build for your specific game, or build it yourset (see `MT_GAME` setting while configuring CMake)
2. Run the game to the title screen ("Press Key to Continue")
3. Inject the compiled DLL into the game using any DLL injector (Xenos, Cheat Engine, etc)
    * The program will try to instanciate every class in the game binary (with DTI info) in order to populate the property list. This will cause many DirectX/Directinput/DXGI errors, as multiple class constructors and property initializers require certain state. Crashes are mostly avoided via SEH handling, but error messageboxes must be closed manually.
    * When finished, it will dump the following files into the game directory:
        * `dti_dumper.log`
        * `dti_map_propless.json`
        * `dti_map_with_props.json`
        * `mt_types.json`
4. Run the post-processor script against the game directory:
    ```
    PS C:\dev\MHW-ClassPropDump> py -3 scripts/process_dump.py mm11 "C:\Program Files (x86)\Steam\steamapps\common\Mega Man 11\dti_map_with_props.json" mm11_clean_dump.json
    Game profile: Mega Man 11
    Wrote clean dump mm11_clean_dump.json
    Game profile: Mega Man 11
    Generating class records [0/1609]
    Generating class records [100/1609]
    Generating class records [200/1609]
    Generating class records [300/1609]
    Generating class records [400/1609]
    Generating class records [500/1609]
    Generating class records [600/1609]
    Generating class records [700/1609]
    Generating class records [800/1609]
    Generating class records [900/1609]
    Generating class records [1000/1609]
    Generating class records [1100/1609]
    Generating class records [1200/1609]
    Generating class records [1300/1609]
    Generating class records [1400/1609]
    Generating class records [1500/1609]
    Generating class records [1600/1609]
    Wrote flat dump mm11_clean_dump.json
    Wrote ./flat_dump.h
    Generating class records [0/1609]
    Generating class records [100/1609]
    Generating class records [200/1609]
    Generating class records [300/1609]
    Generating class records [400/1609]
    Generating class records [500/1609]
    Generating class records [600/1609]
    Generating class records [700/1609]
    Generating class records [800/1609]
    Generating class records [900/1609]
    Generating class records [1000/1609]
    Generating class records [1100/1609]
    Generating class records [1200/1609]
    Generating class records [1300/1609]
    Generating class records [1400/1609]
    Generating class records [1500/1609]
    Generating class records [1600/1609]
    Wrote ./flat_dump_legacy_format.h
    PS C:\dev\MHW-ClassPropDump>
    ```
5. See the `flat_dump.h` / `flat_dump_legacy_format.h` produced.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)