"""Read-only audit of exactly the commit range intended for publication."""
import argparse
import json
from pathlib import Path, PurePosixPath
import subprocess

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--base',default='origin/main')
p.add_argument('--candidate',default='HEAD')
a=p.parse_args()
def git(*args):return subprocess.check_output(['git','-c','safe.directory='+str(Path(__file__).resolve().parents[1]),*args],text=True).strip()
commits=git('rev-list',a.base+'..'+a.candidate).splitlines()
if len(commits)!=1:raise SystemExit('Expected exactly one clean commit above base')
if git('rev-parse',a.candidate+'^')!=git('rev-parse',a.base):raise SystemExit('Candidate must descend directly from base')
for commit in commits:
    if git('log','--format=%H',a.base+'..'+commit,'--','src/code_patches.h'):raise SystemExit('Private header exists in publication range')
    files=git('ls-tree','-r','--name-only',commit).splitlines()
    bad=[]
    roots={'src','tests','tools','scripts','config','third_party','.github','docs'}
    root_files={'README.md','LICENSE','AGENTS.md','CMakeLists.txt','.gitignore','.gitattributes'}
    for name in files:
        path=PurePosixPath(name)
        if path.parts[0] not in roots and name not in root_files:bad.append(name)
        if any(part in {'private','_local','reports','artifacts','assets','save','saves'} or part.startswith('build') for part in path.parts[:-1]):bad.append(name)
        if path.name in {'code.lua','code_patches.h'}:bad.append(name)
        if path.suffix.lower() in {'.sgr','.sgimg','.ogg','.mp3','.wav','.ttf','.otf','.bnk','.sav','.log','.elf','.vpk','.apk','.so','.psp2dmp'}:bad.append(name)
        if path.suffix.lower() in {'.png','.jpg','.jpeg','.webp','.gif'} and name not in {'docs/images/controls_es.png','docs/images/controls_en.png'}:bad.append(name)
    if bad:raise SystemExit('Forbidden paths: '+', '.join(sorted(set(bad))))
print(json.dumps({'base':git('rev-parse',a.base),'candidate':git('rev-parse',a.candidate),'new_commits':commits,'files':len(files),'private_header_in_published_history':False,'path_scan':'passed','note':'Also review source ownership and VPK contents before publishing.'},indent=2))
