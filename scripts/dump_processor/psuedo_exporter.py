from pprint import pprint
import json
import sys
from .game_profiles import mt_game_profiles

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
        # for prop in vft['properties']:
        #     prop['calc_value_get'] = prop['get']
        #     prop['calc_value_get_count'] = prop['get_count']
        #     prop['calc_value_set'] = prop['set']
        #     prop['calc_value_set_count'] = prop['set_count']

        # Sort properties by offset
        #vft['properties'].sort(key=lambda x: x['calc_offset'])
        
        for prop in cr['properties']:
            # Lookup property type name (if available)
            prop_type = self._get_prop_type_by_id(prop['type'])

            attr_flag_list = self._get_prop_attr_flag_list(prop['attr'])
            output += f"\tname: {prop.get('name', '')}, type:{prop_type}, attr:{attr_flag_list}"
            output += f", raw_base:0x{prop['owner']:X}, raw_get:0x{prop['get']:X}, raw_get_count:0x{prop['get_count']:X}, raw_set:0x{prop['set']:X}, raw_set_count:0x{prop['set_count']:X}"
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