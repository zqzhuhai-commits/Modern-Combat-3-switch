#!/usr/bin/env python3
# Build-only dependency restoration. OCI layers are SHA-256 pinned.
import pathlib,json,urllib.request,hashlib,concurrent.futures,tarfile,zipfile,shutil
root=pathlib.Path(__file__).resolve().parents[2]/'toolchain';root.mkdir(exist_ok=True)
repo='vita2hos/devcontainer/vita2hos'
with urllib.request.urlopen('https://ghcr.io/token?service=ghcr.io&scope=repository:'+repo+':pull',timeout=30) as r:token=json.load(r)['token']
digests=['71062409b3e0bb8208c02d46d2b23a83942eda85b5646d65e036d5b4593ec8a0','66a024ca62469027d7e4be67f4137617286836751adfff705c2781b46d1dc33c','357ae71f279e530e21578800991e445e522aa090842038050e91eadcf4a6364b','8a996956a651d66cd87a810b903f71e3a57eba534282bc4087a8b8779052abe8','347df04ff89cfd807114afbd7e050e8b174709f96db3e06bcd01ff779e906aec']
def layer(digest):
 p=root/(digest+'.tar.gz');h=hashlib.sha256()
 req=urllib.request.Request('https://ghcr.io/v2/'+repo+'/blobs/sha256:'+digest,headers={'Authorization':'Bearer '+token})
 with urllib.request.urlopen(req,timeout=45) as r,p.open('wb') as f:
  while b:=r.read(1048576):h.update(b);f.write(b)
 if h.hexdigest()!=digest:raise ValueError('Layer hash mismatch')
 print('Verified layer',digest[:12],p.stat().st_size,flush=True)
 return p
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:layers=list(pool.map(layer,digests))
dest=(root/'v2h-root').resolve();dest.mkdir(exist_ok=True)
for p in layers:
 with tarfile.open(p) as t:t.extractall(dest,members=[m for m in t.getmembers() if m.name.startswith('opt/devkitpro/')],filter='data')
for name,url in [('libnx32','https://github.com/aks796/libnx32/releases/download/4.12.0/libnx32.zip'),('mesa32','https://github.com/aks796/mesa32/releases/download/release/mesa32.zip')]:
 p=root/(name+'.zip')
 with urllib.request.urlopen(url,timeout=45) as r,p.open('wb') as f:
  while b:=r.read(1048576):f.write(b)
 with zipfile.ZipFile(p) as z:z.extractall(root/name)
 print('Restored',name,flush=True)
for sub in ['include','lib']:
 shutil.copytree(root/'libnx32/libnx32'/sub,dest/'opt/devkitpro/libnx32'/sub,dirs_exist_ok=True)
print('Toolchain restored',flush=True)
