#!/usr/bin/env python3
"""Run the real binder against a supplied original ELF's dynamic exports."""
import ctypes
import pathlib
import subprocess
import sys

library, bridge = map(pathlib.Path, sys.argv[1:])
exports = {}
for line in subprocess.check_output(['readelf', '--dyn-syms', '--wide', str(library)], text=True).splitlines():
    cols = line.split()
    if len(cols) >= 8 and cols[0].rstrip(':').isdigit() and cols[6] != 'UND':
        exports[cols[7]] = int(cols[1], 16)
lookup_type = ctypes.CFUNCTYPE(ctypes.c_size_t, ctypes.c_void_p, ctypes.c_char_p)
log_type = ctypes.CFUNCTYPE(None, ctypes.c_char_p)
queried, missing = [], []
@lookup_type
def lookup(_module, name):
    queried.append(name.decode())
    return exports.get(name.decode(), 0)
@log_type
def log(name):
    missing.append(name.decode())
api = (ctypes.c_size_t * 15)()
bridge_lib = ctypes.CDLL(str(bridge.resolve()))
bridge_lib.mc3_bind.argtypes = [ctypes.c_void_p, lookup_type, ctypes.c_void_p, log_type]
result = bridge_lib.mc3_bind(api, lookup, None, log)
if result or len(queried) != 15:
    raise SystemExit(f'Native export binding failed: {missing}')
print(f'Native export binding passed: {len(queried)} symbols, zero missing')
