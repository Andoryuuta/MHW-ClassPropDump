from pprint import pprint
import json

class DumpProcessor():
    def __init__(self):
        self._dump = None
        self._cr_by_id = None
        self._cr_by_name = None
    
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

            # Fixup/decode property names which are written as pure byte arrays of Shift-JIS text.
            for cr in self._dump:
                for vft in cr['properties']:
                    for p in vft['properties']:
                        p['name'] = bytearray(p['name_bytes']).decode('shiftjis')
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
            prop['calc_offset'] = prop['get'] - prop['owner'] 
        
        vft['properties'].sort(key=lambda x: x['calc_offset'])
        for prop in vft['properties']:
            output += f"\tname: {prop['name']}, type:{prop['type']}, attr:{prop['attr']}, offset:0x{prop['calc_offset']:X}\n"
            
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

    def generate_flat_dump(self):
        # # Temp - dump single CR
        # cr = self._cr_by_name['rCnsMatrix']
        # return self.generate_class_record_dump_flat(cr)

        output = ''
        for cr in self._dump:
            output += self.generate_class_record_dump_flat(cr)
        
        return output




def main():
    dp = DumpProcessor()
    dp.load(r'C:\Program Files (x86)\Steam\steamapps\common\ULTIMATE MARVEL VS. CAPCOM 3\dti_map_with_props.json')
    flat_dump = dp.generate_flat_dump()

    with open('./flat_dump.h', 'wt', encoding='utf8') as f:
        f.write(flat_dump)
    
    print("created flat dump")

    # with open(r'C:\Program Files (x86)\Steam\steamapps\common\ULTIMATE MARVEL VS. CAPCOM 3\dti_map_with_props.json', 'rt') as f:
    #     data = json.load(f)

    #     fix_property_names(data)

    #     for cr in data:
    #         # pprint(cr)
    #         # if 'properties' in cr and cr['properties'] is not None:
    #         #     for vft in cr['properties']:
    #         #         if 'properties' in vft and vft['properties'] is not None:
    #         #             for p in vft['properties']:
    #         #                 print(p)

    #         if not cr['is_abstract'] and len(cr['properties']) > 1:
    #             print(cr['name'])


    #         # parent_name = None
    #         # if cr['']
    #         # if cr['properties'] is None:

                

    #         # for vft in cr['properties']:
    #         # name = cr['name']


if __name__ == '__main__':
    main()