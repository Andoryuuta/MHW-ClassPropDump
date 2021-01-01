"""
Requires Python 3.8(x64) and frida (via pip: `py -3 -m pip install frida`).
"""

import frida
import time
import sys
import json

def main():
    executable_name = "MonsterHunterWorld.exe"
    pID = frida.spawn(executable_name)
    session = frida.attach(pID)

    script = session.create_script("""

    var mhwMod = Process.getModuleByName('MonsterHunterWorld.exe');
    var mtDTICtor = Memory.scanSync(mhwMod.base, mhwMod.size, "48 89 5C 24 08 57 48 83 EC 20 81 61 30 00 00 80 1F")[0];
    console.log('mtDTICtor : ' + JSON.stringify(mtDTICtor));

    var crtStaticInitializeDispatcher = Memory.scanSync(mhwMod.base, mhwMod.size, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 33 ED 48 8B FA 48 2B F9 48 8B")[0];
    console.log('crtStaticInitializeDispatcher : ' + JSON.stringify(crtStaticInitializeDispatcher));
    
    Interceptor.attach(mtDTICtor.address, {
        onEnter: function(args) {
            //console.log('Context : ' + JSON.stringify(this.context));

            var parentDTI = this.context.r8;
            var className = this.context.rdx.readCString();
            var classDTI = this.context.rcx;
            var classSize = this.context.r9 & 0xFFFF;

            // There is a "lea rax, [rip + 0x2ea154f]" instruction right after this function which sets up the DTI implmentation.
            var retAddrInsn = Instruction.parse(this.returnAddress);
            var implDTI = retAddrInsn.next.add(retAddrInsn.operands[1].value.disp);

            send({type: 'data', payload: {parent_dti: parentDTI, class_name: className, class_dti: classDTI, class_size: classSize, impl_dti: implDTI}});
        },
    });

    Interceptor.attach(crtStaticInitializeDispatcher.address, {
        onEnter: function(args) {
            send({type: "event", name: "static-init-start"});
        },
        onLeave(retval) {
            send({type: "event", name: "static-init-finish", imageBase: mhwMod.base});
            Interceptor.detachAll();
        }
    });
    
""")


    dti_map = {}

    def on_message(message, data):
        m = message['payload']
        if m['type'] == 'data':
            dti_map[m['payload']['class_dti']] = m['payload']
            print("Got DTIs: ", len(dti_map))
        elif m['type'] == 'event':
            if m['name'] == 'static-init-start':
                print("Static initalizers starting")
            elif m['name'] == 'static-init-finish':
                print("Static initalizers finished. Dumping results to json...")
                with open('dti_dump.json', 'wt') as f:
                    f.write(json.dumps({'image_base':m['imageBase'], 'dti_map': dti_map}, indent=4))

                print('Done!')

    script.on('message', on_message)
    script.load()
    
    frida.resume(pID)

    print("[!] Ctrl+D on UNIX, Ctrl+Z on Windows/cmd.exe to detach from instrumented program.\n\n")
    sys.stdin.read()
    session.detach()

if __name__ == '__main__':
    main()