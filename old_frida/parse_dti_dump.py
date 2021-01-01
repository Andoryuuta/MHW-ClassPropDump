import json

def get_parents(dti_map, dti):
    if dti == '0x0' or dti not in dti_map:
        return ''
    else:
        cdata = dti_map[dti]
        parent = get_parents(dti_map, cdata['parent_dti'])
        if parent == '':
            return cdata['class_name']
        else:
            return cdata['class_name'] + ', ' + parent

with open('dti_dump.json', 'rt') as f:
    d = json.loads(f.read())

    with open('dti_class_dump.h', 'wt') as outfile:
        for dti, cdata in d['dti_map'].items():
            if cdata['class_dti'] == cdata['parent_dti']:
                inheirit_str = ''
            else:
                inheirit_str = get_parents(d['dti_map'], cdata['parent_dti'])


            dti_rva = int(cdata['class_dti'], 16) - int(d['image_base'], 16)
            dti_factory_new_rva = (int(cdata['impl_dti'], 16) + 8*4) - int(d['image_base'], 16)# 4th method in DTI vftable.

            output  = f""
            output += f"// {cdata['class_name']}::DTI RVA: {hex(dti_rva)}\n"
            output += f"// {cdata['class_name']}::Factory::NewInstance RVA: {hex(dti_factory_new_rva)}\n"
            output += f"class {cdata['class_name']}: {inheirit_str} {{\n"
            output += f"\t// Size: {hex(cdata['class_size'])}\n"
            output += '}\n\n'

            outfile.write(output)

print("Done!")

