#!/usr/bin/env python3
"""Build a data-inclusive test NSP from locally supplied game files and tools."""
import argparse,hashlib,json,pathlib,re,shutil,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
TITLE='现代战争3:堕落国度'
TITLE_ID='0100F5344D433300'

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--data-dir',type=pathlib.Path,required=True)
 p.add_argument('--keys',type=pathlib.Path,required=True)
 p.add_argument('--packer',type=pathlib.Path,required=True,help='hacbrewpack executable')
 p.add_argument('--nacptool',type=pathlib.Path,required=True)
 p.add_argument('--output',type=pathlib.Path,required=True)
 p.add_argument('--version',default='0.1.0',help='NACP display version (up to 15 ASCII bytes)')
 a=p.parse_args()
 if not re.fullmatch(r'[A-Za-z0-9._-]{1,15}',a.version):raise SystemExit('Invalid display version')
 recipe=json.loads((ROOT/'loader/mc3.eapx.json').read_text())
 assets=[]
 for entry in recipe['extract']:
  name=pathlib.Path(entry['destination']).name
  file=a.data_dir/name
  if not file.is_file():raise SystemExit('Missing game file: '+name)
  v=entry['validate'];h=hashlib.sha256()
  with file.open('rb') as f:
   while b:=f.read(1048576):h.update(b)
  if file.stat().st_size!=v['size'] or h.hexdigest() not in v['sha256']:
   raise SystemExit('Game version/hash mismatch: '+name)
  assets.append(file.resolve())
 exefs=ROOT/'build/exefs'
 for name in ['main','main.npdm']:
  if not(exefs/name).is_file():raise SystemExit('Compile first: missing build/exefs/'+name)
 required={'header_key':64,'key_area_key_application_00':32}
 allowed={**required,'master_key_00':32,'aes_kek_generation_source':32,
          'aes_key_generation_source':32,'key_area_key_application_source':32}
 keys={}
 for line in a.keys.read_text().splitlines():
  m=re.fullmatch(r'\s*(\w+)\s*=\s*([0-9a-fA-F]+)\s*',line)
  if m and m[1] in allowed and len(m[2])==allowed[m[1]]:keys[m[1]]=m[2]
 if not required.keys()<=keys.keys():raise SystemExit('Missing required packaging keys')
 a.output.mkdir(parents=True,exist_ok=True)
 with tempfile.TemporaryDirectory(prefix='mc3-pack-') as tmp:
  work=pathlib.Path(tmp);romfs=work/'romfs';control=work/'control'
  romfs.mkdir();control.mkdir()
  for src in assets:
   dst=romfs/src.name
   try:dst.hardlink_to(src)
   except OSError:shutil.copyfile(src,dst)
  keyfile=work/'packaging.keys';keyfile.write_text(''.join(k+' = '+v+'\n' for k,v in keys.items()));keyfile.chmod(0o600)
  subprocess.run([str(a.nacptool.resolve()),'--create',TITLE,'MC3 Switch Port',a.version,str(control/'control.nacp'),'--titleid='+TITLE_ID],check=True)
  langs=['AmericanEnglish','BritishEnglish','Japanese','French','German','LatinAmericanSpanish','Spanish','Italian','Dutch','CanadianFrench','Portuguese','Russian','Korean','TraditionalChinese','SimplifiedChinese','BrazilianPortuguese']
  for lang in langs:shutil.copyfile(ROOT/'assets/mc3-icon-256.jpg',control/('icon_'+lang+'.dat'))
  cmd=[str(a.packer.resolve()),'-k',str(keyfile),'--exefsdir',str(exefs),'--romfsdir',str(romfs),'--controldir',str(control),'--tempdir',str(work/'temp'),'--ncadir',str(work/'nca'),'--nspdir',str(a.output.resolve()),'--nologo']
  r=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
  if r.returncode:raise SystemExit('Packager failed; exit status '+str(r.returncode))
  result=a.output/(TITLE_ID.lower()+'.nsp')
  if not result.is_file():raise SystemExit('Packager did not produce an NSP')
  print('Created test NSP:',result,'bytes:',result.stat().st_size)
  print('Installation, graphics, audio and gameplay still require hardware testing.')
if __name__=='__main__':main()
