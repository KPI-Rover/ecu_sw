#!/bin/python3

from pathlib import Path

lines = open(Path(__file__).parent / "../../KPIRover.c").read().split('\n')

decoder_active = False

params = []

def log(text):
    types = {"ERROR": True,
             "WARN": True,
             "INFO": True,
             "DEBUG": True}

    if len(text) == 0 or text[0] != "[":
        print(text)
        return

    msg_type = text.split("]", 1)[0][1:]

    if msg_type not in types:
        print(f'[INTERNAL ERROR] msg_type "{msg_type}" does not exist')
        exit(2)

    if types[msg_type]:
        print(text)

def process_offset(v):
    global params

    offset = int(v)

    if offset != 0:
        log("[WARN] Offsets are overridden by the database and should be kept as 0 in the metadata")
        return False

    params[-1]["offset"] = int(v)

    return True

def process_type(v):
    global params

    if v not in ["INT8", "UINT8", "INT16", "UINT16", "INT32", "UINT32", "FLOAT"]:
        log(f'[ERROR] Invalid type: "{v}"')
        return False

    params[-1]["type"] = v

    return True

def process_persistent(v):
    global params

    if v not in ["true", "false"]:
        log(f"[ERROR] Invalid boolean value: {v}")
        return False

    params[-1]["persistent"] = True if v == "true" else False

    if len(params) > 1:
        if params[-1]["persistent"] and not params[-2]["persistent"]:
            log("[ERROR] Persistent parameters must come before the non-persistent ones")
            return False

    return True

def process_defaultValue(v):
    if params[-1]["type"] == "FLOAT":
        try:
            params[-1]["defaultValue"] = float(v.strip("fF"))
        except:
            log(f"[ERROR] Invalid floating-point value: {v}")
            return False
    else:
        try:
            params[-1]["defaultValue"] = int(v)
        except:
            log(f"[ERROR] Invalid integer value: {v}")
            return False

    ''' unfinished and broken, no reason to fix right now
    elif params[-1]["type"] == "INT32":
        try:
            params[-1]["defaultValue"] = int(v)

            if params[-1]["defaultValue"] > 0x7fffffff:
                print(f"[WARN] defaultValue overflows it's data type: {v} ({hex(params[-1]['defaultValue'])}) > 0x7fffffff")
            elif params[-1]["defaultValue"] < -0x80000000:
                print(f"[WARN] defaultValue overflows it's data type: {v} ({hex(params[-1]['defaultValue'] % 0x100000000)}) < -0x80000000")
        except:
            log(f"[ERROR] Invalid floating-point value: {v}")
            return False
    '''

    return True

for l in lines:
    log("> " + l)
    if 'struct ulDatabase_ParamMetadata ulDatabase_params[]' in l:
        log("[DEBUG] Starting to read metadata")
        decoder_active = True
        continue

    if ('}' in l) and ('{' not in l):
        log("[DEBUG] End of metadata")
        break

    if not decoder_active:
        continue

    metadata = [i.strip() for i in l.split('}')[0].split('{')[1].split(',')]
    name = l.split("//")[1].strip().strip(",")

    params.append({'name': name})

    metadata_item_pool = [
        {
            "name": "offset",
            "func": process_offset
        },
        {
            "name": "type",
            "func": process_type
        },
        {
            "name": "persistent",
            "func": process_persistent
        },
        {
            "name": "defaultValue",
            "func": process_defaultValue
        }
    ]

    for i, v in enumerate(metadata_item_pool):
        v["position"] = i

    for c, t in enumerate(metadata):
        index = -1
        result = False
        value = ""

        if '=' in t:
            item, value = [i.strip() for i in t.split("=")]
            
            try:
                index = [i['name'] for i in metadata_item_pool].index(item)
            except ValueError:
                log(f'[ERROR] {i}: "{item}": no such metadata key')
        else:
            index = 0
            value = t

        if index != -1:
            result = metadata_item_pool.pop(index)['func'](value)

        if not result:
            unwrapped_l = l.replace("\t", " " * 8)
            log(f"[ERROR] > {unwrapped_l}")

            arrow_offset = unwrapped_l.index("{") + 1
            ll = unwrapped_l[arrow_offset:]

            for _ in range(c):
                arrow_offset += ll.index(",") + 1
                ll = unwrapped_l[arrow_offset:]

            while unwrapped_l[arrow_offset] == ' ':
                arrow_offset += 1

            underline_len = len(t) - 1

            while unwrapped_l[arrow_offset + underline_len] == ' ':
                underline_len -= 1

            log("[ERROR] > " + " " * arrow_offset + "^" + "~" * underline_len)
            log("[ERROR] Error detected, not continuing")
            exit(1)

log("[INFO] Result:")

for i in params:
    print("[INFO] " + i["name"])


log("[INFO] Patching ulDatabase.h...")

ulDatabase_h = open(Path(__file__).parent / "../ulDatabase.h").read().split('\n')
while ulDatabase_h[-1] == "":
    del ulDatabase_h[-1]

enum_since = [i.startswith("enum ulDatabase_ParamId {") for i in ulDatabase_h].index(True) + 1
enum_len = [i.startswith("};") for i in ulDatabase_h[enum_since:]].index(True)

log("[DEBUG]: Removing old enum data:")
for i, l in enumerate(ulDatabase_h):
    if enum_since <= i < enum_since + enum_len:
        log("[DEBUG] -" + l)
    else:
        log("[DEBUG]  " + l)

del ulDatabase_h[enum_since:enum_since+enum_len]

ulDatabase_h.insert(enum_since, "\tPARAM_COUNT")
for i in params[::-1]:
    ulDatabase_h.insert(enum_since, f"\t{i['name']},")

log("[DEBUG] Writing new ulDatabase.h...")

with open(Path(__file__).parent / "../ulDatabase.h", "w") as f:
    for i in ulDatabase_h:
        print(i + '\n', end = '')
        f.write(i + '\n')

log("[DEBUG] ulDatabase.h closed")
