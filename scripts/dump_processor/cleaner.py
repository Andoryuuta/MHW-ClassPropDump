import base64
import copy
from pprint import pprint
import json
import sys
from .game_profiles import mt_game_profiles

class DumpCleaner():
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

    def clean(self, input_filename, output_filename):
        raw_data = None
        with open(input_filename, 'rt') as f:
            raw_data = json.load(f)
            # self._clean_data = copy.deepcopy(data)
        
        # Clean the json
        # Do not reorder this, each cleaning pass may modify the schema which later passes may rely on.
        raw_data = self._clean_json_array_encoding(raw_data)
        raw_data = self._clean_json_string_bytes(raw_data)
        raw_data = self._clean_json_bad_dti(raw_data)
        raw_data = self._clean_json_flattened_properties(raw_data)

        with open(output_filename, 'wt', encoding='utf8') as f:
            json.dump(raw_data, f, ensure_ascii=False, indent=4)
    
    def _clean_json_array_encoding(self, data):
        # Cleanup JSON object (empty arrays with written as null instead of empty array)
        for cr in data:
            if 'processed_vftables' not in cr or cr['processed_vftables'] is None:
                cr['processed_vftables'] = []
            else:
                for vft in cr['processed_vftables']:
                    if 'properties' not in vft or vft['properties'] is None:
                        vft['properties'] = []
        return data
    
    def _clean_json_string_bytes(self, data):
        # Fixup/decode property names which are written as pure byte arrays.
        # This is required due to different encodings across games (shift-jis, utf8, etc),
        # which we don't want to attempt to transcode in the C++ codebase.
        for cr in data:
            for vft in cr['processed_vftables']:
                for p in vft['properties']:
                    if 'name_bytes' in p:
                        if len(p['name_bytes']) != 0:
                            p['name'] = bytearray(p['name_bytes']).decode(self._game_profile['native_encoding'])
                            p['name_raw'] = base64.b64encode(bytes(p['name_bytes'])).decode()
                        del p['name_bytes']

                    if 'comment_bytes' in p:
                        if len(p['comment_bytes']) != 0:
                            p['comment'] = bytearray(p['comment_bytes']).decode(self._game_profile['native_encoding'])
                            p['comment_raw'] = base64.b64encode(bytes(p['comment_bytes'])).decode()
                        del p['comment_bytes']
        return data

    def _clean_json_bad_dti(self, data):
        # Correct bad DTI inheiritance.
        output_data = []
        bad_dti_count = 0
        for cr in data:
            processed_vftables = cr['processed_vftables']
            del cr['processed_vftables']
            del cr['class_vftable_vas']

            if len(processed_vftables) == 0:
                # No vftable?
                o = copy.deepcopy(cr)
                o['vftable_va'] = None
                o['properties'] = []
                o['properties_processed'] = False
                o['bad_dti'] = False
                output_data.append(o)
            elif len(processed_vftables) == 1:
                o = copy.deepcopy(cr)
                vft = processed_vftables[0]
                o['vftable_va'] = vft['vftable_va']
                o['properties'] = vft['properties']
                o['properties_processed'] = vft['properties_processed']
                o['bad_dti'] = False
                output_data.append(o)
            else:
                # Bad DTI

                if cr['name'] not in self._game_profile['real_vftables']:
                    err = f"ERROR! No bad dti vftable fixup in profile ('real_vftables') for class: {cr['name']} (is_abstract:{cr['is_abstract']})\n"
                    for (idx, vft) in enumerate(processed_vftables):
                        err += f"\tvftable #{idx}: {hex(vft['vftable_va'])}\n"
                    print(err)

                    # # Append original solely for debugging.
                    # o = copy.deepcopy(cr)
                    # o['processed_vftables'] = processed_vftables
                    # output_data.append(o)
                    continue
                
                # Verify that the fixup vftable is a valid option (either None, or one of the processed vftable va's)
                real_vft_va = self._game_profile['real_vftables'][cr['name']]
                processed_vft_vas = [vft['vftable_va'] for vft in processed_vftables]
                if real_vft_va is not None and real_vft_va not in processed_vft_vas:
                    print(f"ERROR! Invalid DTI vftable fixup in profile ('real_vftables') for class: {cr['name']} (is_abstract:{cr['is_abstract']})")
                    continue

                if real_vft_va is None:
                    # Abstract class with no vftable generated by compiler.
                    o = copy.deepcopy(cr)
                    o['vftable_va'] = vft['vftable_va']
                    o['properties'] = vft['properties']
                    o['properties_processed'] = vft['properties_processed']
                    o['bad_dti'] = False
                    output_data.append(o)
                else:
                    for vft in processed_vftables:
                        if vft['vftable_va'] == real_vft_va:
                            # Real class for this DTI:
                            o = copy.deepcopy(cr)
                            o['vftable_va'] = vft['vftable_va']
                            o['properties'] = vft['properties']
                            o['properties_processed'] = vft['properties_processed']
                            o['bad_dti'] = False
                            output_data.append(o)
                        else:
                            # A bad-DTI class
                            o = copy.deepcopy(cr)
                            o['name'] = f"BadDTI_{bad_dti_count}"
                            o['parent_id'] = o['id'] 
                            o['id'] = None

                            o['vftable_va'] = vft['vftable_va']
                            o['properties'] = vft['properties']
                            o['properties_processed'] = vft['properties_processed']
                            o['bad_dti'] = True
                            output_data.append(o)

                            bad_dti_count += 1


 
        return output_data

    def _clean_json_flattened_properties(self, data):
        cr_by_id = {cr['id']: copy.deepcopy(cr) for cr in data}

        def get_parent_tree_props(parent_id):
            props = []
            seen = [] # Fix for bad DTI references....
            while parent_id in cr_by_id and parent_id not in seen:
                seen.append(parent_id)
                parent = cr_by_id[parent_id]
                props.extend(parent['properties'])
                parent_id = parent['parent_id']
                # if parent_id in seen:
                #     print("Recursive DTI dumb dumb:", cr_by_id[parent_id])
            return props
        
        def prop_equal(p0, p1):
            # print(p0, p1)
            # Special check for name and comments as they are conditional.
            name_match = False
            if 'name' in p0 and 'name' in p1 and p0['name'] == p1['name']:
                name_match = True
            elif 'name' not in p0 and 'name' not in p1:
                name_match = True

            comment_match = False
            if 'comment' in p0 and 'comment' in p1 and p0['comment'] == p1['comment']:
                comment_match = True
            elif 'comment' not in p0 and 'comment' not in p1:
                comment_match = True
            
            p0_offset_get = p0['get']-p0['owner']
            p1_offset_get = p1['get']-p1['owner']
            p0_offset_set = p0['set']-p0['owner']
            p1_offset_set = p1['set']-p1['owner']

            return (comment_match and
                    name_match and
                    p0['attr'] == p1['attr'] and
                    p0['type'] == p1['type'] and
                    p0['index'] == p1['index'] and
                    p0['get'] == p1['get'] or p0_offset_get == p1_offset_get and 
                    p0['set'] == p1['set'] or p0_offset_set == p1_offset_set)

        for cr in data:
            parent_id = cr['parent_id']
            if parent_id == 0:
                continue

            cr_parent = cr_by_id[parent_id]

            parent_props = get_parent_tree_props(parent_id)
            unflatted_props = []
            for prop in cr['properties']:
                is_parent_prop = False 
                for parent_prop in parent_props:
                    if prop_equal(prop, parent_prop):
                        is_parent_prop = True
                        break

                if cr['name'] == 'sMhWwiseManager':
                    print(is_parent_prop, prop)
                if not is_parent_prop:
                    unflatted_props.append(prop)

            cr['properties'] = unflatted_props

        return data

    # def _clean_bad_dti(self):
    #     # Correct bad DTI inheiritance.
    #     bad_dti_crs = [cr for cr in self._raw_data if len(cr['processed'])] 


    def write(self, filename):
        with open(filename, 'wt', encoding='utf8') as f:
            json.dump(self._raw_data, f, ensure_ascii=False, indent=4)