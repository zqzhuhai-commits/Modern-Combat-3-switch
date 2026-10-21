#!/usr/bin/env python3
"""Package ELF32 ARM PT_LOAD segments as an uncompressed, hashed NSO0.
Uses the public NSO segment layout. No proprietary tools or game bytes needed.
"""
import argparse,hashlib,pathlib,struct
p=argparse.ArgumentParser();p.add_argument('elf',type=pathlib.Path);p.add_argument('nso',type=pathlib.Path);a=p.parse_args()
d=a.elf.read_bytes()
if d[:6]!=b'\x7fELF\x01\x01' or struct.unpack_from('<H',d,18)[0]!=40:raise SystemExit('Expected ELF32 little-endian ARM')
phoff=struct.unpack_from('<I',d,28)[0];ents,num=struct.unpack_from('<HH',d,42)
segments=[]
for i in range(num):
 v=struct.unpack_from('<8I',d,phoff+i*ents)
 if v[0]==1:segments.append(v)
if len(segments)!=3:raise SystemExit('Expected text, rodata and data PT_LOAD segments')
h=bytearray(256);h[:4]=b'NSO0';struct.pack_into('<I',h,12,0x38)
h[0x40:0x60]=hashlib.sha256(d).digest()
payload=bytearray()
for i,(_,off,addr,_,size,mem,flags,align) in enumerate(segments):
 if off+size>len(d) or mem<size or addr%4096:raise SystemExit('Invalid load segment')
 if flags != (5,4,6)[i]:raise SystemExit('Unexpected load segment permissions')
 b=d[off:off+size]
 struct.pack_into('<4I',h,0x10+i*0x10,256+len(payload),addr,size,mem-size if i==2 else 0)
 struct.pack_into('<I',h,0x60+4*i,size)
 h[0xa0+32*i:0xc0+32*i]=hashlib.sha256(b).digest();payload.extend(b)
a.nso.parent.mkdir(parents=True,exist_ok=True);a.nso.write_bytes(h+payload)
# Exact segment round-trip validates file offsets, sizes and emitted digests.
out=a.nso.read_bytes()
for i,(_,off,addr,_,size,mem,flags,align) in enumerate(segments):
 start=struct.unpack_from('<I',out,0x10+16*i)[0];b=out[start:start+size]
 assert b==d[off:off+size] and hashlib.sha256(b).digest()==out[0xa0+32*i:0xc0+32*i]
print('ELF32 ARM -> NSO0: 3 exact segments; hashes checked')
