# MHW-ClassPropDump
A hasilty written program to dump DTI and property information from MHW.

The majority of this code was written just to "make it work" while reverse engineering. Expectations of the code quality should be set properly (read: it's bad code).

## Usage
1. Disable all existing plugins (if any)
2. Run the game to the title screen ("Press Key to Continue")
3. Inject the compiled DLL into the game using any DLL injector (Xenos, Cheat Engine, etc)

The program will try to instanciate every class in the game binary (with DTI info) in order to populate the property list. This will cause many DirectX/Directinput/DXGI errors, as multiple class constructors and property initializers require certain state. Crashes are mostly avoided via SEH handling, but error messageboxes must be closed manually.

When finished, it will dump a file `dti_prop_dump.h` into the game folder. 

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)