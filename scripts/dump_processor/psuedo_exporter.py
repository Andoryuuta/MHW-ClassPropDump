from pprint import pprint
import base64
import datetime
import json
import sys
import zlib
from .game_profiles import mt_game_profiles

# Offset value used by the legacy format for non-offset based properties.
LEGACY_INVALID_OFFSET = 0x7FFFFFFFFFFFFFFF

def jamcrc(data):
    return zlib.crc32(data) ^ 0xFFFFFFFF

class DumpProcessor():
    def __init__(self):
        self._game_profile = None
        self._game_profile_type_names = {}
        self._game_profile_prop_flag_values = {}
        self._dump = None
        self._cr_by_id = None
        self._cr_by_name = None
    
    def load_profile(self, profile):
        print(f"Game profile: {profile['game_name']}")
        self._game_profile = profile

        # Create a fast lookup table of property type ID -> property type name
        self._game_profile_type_names = {}
        for prop_type in self._game_profile['property_types']:
            self._game_profile_type_names[prop_type['id']] = prop_type['name']

        # Create lookup table of property flag name -> property flag (int)
        self._game_profile_prop_flag_values = {}
        for val, name in self._game_profile['property_flags'].items():
            self._game_profile_prop_flag_values[name] = val

    def load(self, filename):
        with open(filename, 'rt', encoding='utf8') as f:
            data = json.load(f)
            self._dump = data

            self._cr_by_id = {}
            self._cr_by_name = {}
            for cr in self._dump:
                self._cr_by_id[cr['id']] = cr
                self._cr_by_name[cr['name']] = cr

    def _get_prop_type_by_id(self, prop_type_id):
        prop_type = f"{prop_type_id}"
        if prop_type_id in self._game_profile_type_names:
            prop_type = self._game_profile_type_names[prop_type_id]
        elif prop_type_id >= len(self._game_profile_type_names):
            prop_type = 'custom'
        return prop_type
    
    def _get_prop_attr_flag_list(self, attr):
        if attr == 0:
            return 'FLAG_NONE'

        flags = []
        for i in range(16):
            idx = (1 << i)
            if (attr & idx) != 0:
                flag = self._game_profile['property_flags'][idx]
                flags.append(flag)

        return '|'.join(flags)
    
    def generate_class_record_dump_flat_header(self, cr):
        output = ''
        vft_addr = 'None'
        if cr['vftable_va'] is not None:
            vft_addr = f"0x{cr['vftable_va']:X}"
        
        if cr['bad_dti']:
            output += f"// vftable: {vft_addr}\n"
        else:
            output += f"// vftable: {vft_addr}, size:0x{cr['size']:X}, jamcrc:0x{cr['id'] or -1:X}, abstract:{cr['is_abstract']}, hidden:{cr['is_hidden']}\n"

        parent_name = None
        if cr['parent_id'] != 0:
            parent_name = self._cr_by_id[cr['parent_id']]['name']

        if parent_name is not None:
            output += f"class {cr['name']}: public {parent_name}\n"
        else:
            output += f"class {cr['name']}\n"
        return output

    def generate_class_record_dump_flat_vft_body(self, cr):
        output = ''
        output += '{\n'
        output += '\t/* TODO */\n'
        props = cr['properties']

        # Properties are one of:
        #   - accessor: getter/setter and event properties, which hold function pointers.
        #   - field: points within the class object's memory (shown as an offset from the owner object).
        #   - address: points outside the class object (e.g. globals), shown as an absolute address.
        # Bad DTI records use the size of the DTI they share with their parent, while the real class
        # may be larger, so give them 2x the class size of leeway.
        getter_setter_flag = self._game_profile_prop_flag_values['FLAG_0080_GETTER_SETTER']
        object_size = cr['size'] * 2 if cr['bad_dti'] else cr['size']
        def prop_kind(prop):
            if (prop['attr'] & getter_setter_flag) != 0:
                return 'accessor'
            if self._get_prop_type_by_id(prop['type']) in ('event', 'event32', 'event64'):
                return 'accessor'
            if 0 <= prop['get'] - prop['owner'] < object_size:
                return 'field'
            return 'address'

        # Sort fields by offset, then addresses, then accessors (in dump order).
        kind_order = {'field': 0, 'address': 1, 'accessor': 2}
        def sort_key(prop):
            kind = prop_kind(prop)
            if kind == 'field':
                return (kind_order[kind], prop['get'] - prop['owner'])
            if kind == 'address':
                return (kind_order[kind], prop['get'])
            return (kind_order[kind], 0)
        props = sorted(props, key=sort_key)

        for prop in props:
            # Lookup property type name (if available)
            prop_type = self._get_prop_type_by_id(prop['type'])

            attr_flag_list = self._get_prop_attr_flag_list(prop['attr'])
            output += f"\tname: {prop.get('name', '')}, type:{prop_type}, attr:{attr_flag_list}"
            kind = prop_kind(prop)
            if kind == 'field':
                output += f", offset:0x{prop['get'] - prop['owner']:X}"
            elif kind == 'address':
                output += f", address:0x{prop['get']:X}"
            else:
                output += f", get:0x{prop['get']:X}, get_count:0x{prop['get_count']:X}, set:0x{prop['set']:X}, set_count:0x{prop['set_count']:X}"
            output += f"\n"
            
        output += '}\n'

        return output

    def generate_class_record_dump_flat(self, cr):
        output = ''
        output += self.generate_class_record_dump_flat_header(cr)
        output += self.generate_class_record_dump_flat_vft_body(cr)
        output += '\n'
        return output

    def write_flat_dump(self, filepath):
        # # Temp - dump single CR
        # cr = self._cr_by_name['rCnsMatrix']
        # return self.generate_class_record_dump_flat(cr)

        with open(filepath, 'wt', encoding='utf8') as f:
            for i, cr in enumerate(self._dump):
                if i % 100 == 0:
                    print(f"Generating class records [{i}/{len(self._dump)}]")
                f.write(self.generate_class_record_dump_flat(cr))

    # Legacy format (matches the C++ text dump output of the `main` branch).
    def generate_class_record_dump_legacy(self, cr):
        output = ''

        # Bad DTI records share the DTI of their parent record.
        dti_id = cr['parent_id'] if cr['bad_dti'] else cr['id']
        vftable_va = cr['vftable_va'] or 0
        name_crc = jamcrc(cr['name'].encode(self._game_profile['native_encoding'])) & 0x7FFFFFFF
        if cr['bad_dti'] or dti_id == name_crc:
            output += f"// {cr['name']} vftable:0x{vftable_va:X}, Size:0x{cr['size']:X}, CRC32:0x{dti_id:X}\n"
        else:
            output += f"// {cr['name']} vftable:0x{vftable_va:X}, Size:0x{cr['size']:X}, CRC32:0x{dti_id:X}, CalcCRC:0x{name_crc:X}\n"

        # C++ styled inheritance chain comment.
        parent_names = []
        seen = set()
        parent_id = cr['parent_id']
        while parent_id in self._cr_by_id and parent_id not in seen:
            seen.add(parent_id)
            parent = self._cr_by_id[parent_id]
            parent_names.append(parent['name'])
            parent_id = parent['parent_id']
        inheritance = f": {', '.join(parent_names)}" if parent_names else ''
        output += f"class {cr['name']} /*{inheritance}*/ {{\n"

        # The legacy format always includes the parent properties.
        getter_setter_flag = self._game_profile_prop_flag_values['FLAG_0080_GETTER_SETTER']
        array_flag = self._game_profile_prop_flag_values['FLAG_0020_ARRAY']
        def uncapped_offset(prop):
            if (prop['attr'] & getter_setter_flag) != 0:
                return LEGACY_INVALID_OFFSET
            return prop['get'] - prop['owner']

        for prop in sorted(cr['all_properties'], key=uncapped_offset):
            if 'comment' in prop:
                output += f"\t// Comment: {prop['comment']}\n"

            name = prop.get('name', '')
            name_raw = base64.b64decode(prop['name_raw']) if 'name_raw' in prop else b''
            prop_crc = jamcrc(name_raw)
            # Flags are shown as in MHW's packed type/attr field (attr << 12), so they read the same across games.
            flags = prop['attr'] << 12
            type_and_name = f"{self._get_prop_type_by_id(prop['type'])} '{name}'"

            offset = uncapped_offset(prop)
            if offset < 0 or offset > cr['size']:
                offset = LEGACY_INVALID_OFFSET

            is_array = (prop['attr'] & array_flag) != 0
            is_getter_setter = (prop['attr'] & getter_setter_flag) != 0
            if is_array and is_getter_setter:
                # Dynamic array
                var_string = f"{type_and_name}[*]"
                output += f"\t{var_string:<50}; // Offset:0x{offset:X}, DynamicArray, Getter:0x{prop['get']:X}, Setter:0x{prop['set']:X}, GetCount:0x{prop['get_count']:X}, Reallocate:0x{prop['set_count']:X} CRC32:0x{prop_crc:X}, Flags:0x{flags:X}\n"
            elif is_array:
                # Static/fixed-length array
                var_string = f"{type_and_name}[{prop['get_count'] & 0xFFFFFFFF}]"
                output += f"\t{var_string:<50}; // Offset:0x{offset:X}, Array, CRC32:0x{prop_crc:X}, Flags:0x{flags:X}\n"
            elif is_getter_setter:
                output += f"\t{type_and_name:<50}; // Offset:0x{offset:X}, PSEUDO-PROP, Getter:0x{prop['get']:X}, Setter:0x{prop['set']:X}, CRC32:0x{prop_crc:X}, Flags:0x{flags:X}\n"
            else:
                output += f"\t{type_and_name:<50}; // Offset:0x{offset:X}, Var, CRC32:0x{prop_crc:X}, Flags:0x{flags:X}\n"

        output += '};\n\n'
        return output

    def write_legacy_dump(self, filepath):
        image_base = self._dump[0]['image_base'] if self._dump else 0
        date_time = datetime.datetime.now().strftime('%d-%m-%Y %H-%M-%S')

        with open(filepath, 'wt', encoding='utf8') as f:
            f.write(f"// Ando's MTF DTI Dumper log [{date_time}]\n")
            f.write(f"// Image Base: {image_base:#X}\n\n")
            for i, cr in enumerate(self._dump):
                if i % 100 == 0:
                    print(f"Generating class records [{i}/{len(self._dump)}]")
                f.write(self.generate_class_record_dump_legacy(cr))
            f.write("// END OF FILE")