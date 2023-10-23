from pprint import pprint
import json
import sys
from .game_profiles import mt_game_profiles

class DumpProcessor():
    def __init__(self):
        self._game_profile = None
        self._game_profile_type_names = {}
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

    def load(self, filename):
        with open(filename, 'rt') as f:
            data = json.load(f)
            self._dump = data

            # Cleanup JSON object (empty arrays with written as null instead of empty array)
            for cr in self._dump:
                if 'properties' not in cr or cr['properties'] is None:
                    cr['properties'] = []
                else:
                    for vft in cr['properties']:
                        if 'properties' not in vft or vft['properties'] is None:
                            vft['properties'] = []

            # Fixup/decode property names which are written as pure byte arrays.
            # This is required due to different encodings across games (shift-jis, utf8, etc),
            # which we don't want to attempt to transcode in the C++ codebase.
            for cr in self._dump:
                for vft in cr['properties']:
                    for p in vft['properties']:
                        p['name'] = bytearray(p['name_bytes']).decode(self._game_profile['native_encoding'])
                        del p['name_bytes']

            # for cr in self._dump:
            #     pprint(cr)

            # Create a map of class record by DTI ID
            cr_by_id = {}
            for cr in self._dump:
                cr_by_id[cr['id']] = cr
            self._cr_by_id = cr_by_id

            # Create a map of class record by DTI name
            cr_by_name = {}
            for cr in self._dump:
                cr_by_name[cr['name']] = cr
            self._cr_by_name = cr_by_name

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

    def generate_class_record_dump_flat_header(self, cr, vft, name):
        output = ''
        vft_addr = 'None'
        if vft is not None:
            vft_addr = f"0x{vft['vftable_va']:X}"
        
        output += f"// vftable: {vft_addr}, size:0x{cr['size']:X}, jamcrc:0x{cr['id']:X}, abstract:{cr['is_abstract']}, hidden:{cr['is_hidden']}\n"

        parent_name = None
        if cr['parent_id'] != 0:
            parent_name = self._cr_by_id[cr['parent_id']]['name']

        if parent_name is not None:
            output += f"class {name}: public {parent_name}\n"
        else:
            output += f"class {name}\n"
        return output

    def generate_class_record_dump_flat_vft_body(self, cr, vft):
        output = ''
        output += '{\n'
        output += '\t/* TODO */\n'
        for prop in vft['properties']:
            calc_value_get = prop['get'] - prop['owner']
            if calc_value_get > cr['size'] or calc_value_get < 0:
                calc_value_get = prop['get']

            calc_value_get_count = prop['get_count'] - prop['owner']
            if calc_value_get_count > cr['size'] or calc_value_get_count < 0:
                calc_value_get_count = prop['get_count']

            calc_value_set = prop['set'] - prop['owner']
            if calc_value_set > cr['size'] or calc_value_set < 0:
                calc_value_set = prop['set']

            calc_value_set_count = prop['set_count'] - prop['owner']
            if calc_value_set_count > cr['size'] or calc_value_set_count < 0:
                calc_value_set_count = prop['set_count']


            prop['calc_value_get'] = calc_value_get
            prop['calc_value_get_count'] = calc_value_get_count
            prop['calc_value_set'] = calc_value_set
            prop['calc_value_set_count'] = calc_value_set_count
        
        # Sort properties by offset
        #vft['properties'].sort(key=lambda x: x['calc_offset'])
        
        for prop in vft['properties']:
            # Lookup property type name (if available)
            prop_type = self._get_prop_type_by_id(prop['type'])

            attr_flag_list = self._get_prop_attr_flag_list(prop['attr'])
            output += f"\tname: {prop['name']}, type:{prop_type}, attr:{attr_flag_list}"
            output += f", get:0x{prop['calc_value_get']:X}, get_count:0x{prop['calc_value_get_count']:X}, set:0x{prop['calc_value_set']:X}, set_count:0x{prop['calc_value_set_count']:X}"
            output += f"\n"
            
        output += '}\n'

        return output

    def generate_class_record_dump_flat(self, cr):
        output = ''
        if len(cr['properties']) == 0:
            vft = None
            output += self.generate_class_record_dump_flat_header(cr, vft, cr['name'])
            output += " { /* TODO - Unable to dump properties */ }"
        elif len(cr['properties']) == 1:
            vft = cr['properties'][0]
            output += self.generate_class_record_dump_flat_header(cr, vft, cr['name'])
            output += self.generate_class_record_dump_flat_vft_body(cr, vft)
        else:
            for i in range(len(cr['properties'])):
                cr_name = f"{cr['name']}_{i}"
                vft = cr['properties'][i]
                output += self.generate_class_record_dump_flat_header(cr, vft, cr_name)
                output += self.generate_class_record_dump_flat_vft_body(cr, vft)
        
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