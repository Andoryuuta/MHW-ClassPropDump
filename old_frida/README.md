# MHW-dti-dump

(Backup of old frida script before force push to existing repo.)

A frida/python script to dump the DTI type information from MHW

## Usage
1. Install Python 3.8+ (64 bit)
2. Install Frida
    `py -3 -m pip install frida`
3. Place the `.py` and `steam_appid.txt` in your MHW install folder.
4. Dump the DTI data:
    `py -3 dump_dti.py`
5. Parse the DTI data into a usable format:
    `py -3 parse_dti_dump.py`
6. Look at your output `dti_class_dump.h`