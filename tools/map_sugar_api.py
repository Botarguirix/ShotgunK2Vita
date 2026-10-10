"""Heuristically compare APK Lua calls to a locally supplied SUGAR manual."""
import argparse
import collections
import json
import re
import zipfile
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('apk', type=Path)
parser.add_argument('manual', type=Path)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
manual = args.manual.read_text(encoding='utf-8', errors='replace')
documented = set(re.findall(r'^([A-Za-z_]\w*)\s*\(', manual, re.M))
calls = collections.Counter()
files = collections.defaultdict(set)
with zipfile.ZipFile(args.apk) as archive:
    scripts = [name for name in archive.namelist() if name.startswith('assets/') and name.endswith('.lua')]
    for name in scripts:
        text = archive.read(name).decode('utf-8', errors='replace')
        text = re.sub(r'--\[\[.*?\]\]', '', text, flags=re.S)
        text = re.sub(r'--[^\r\n]*', '', text)
        for call in re.findall(r'(?<![\w.:])([A-Za-z_]\w*)\s*\(', text):
            if call in documented:
                calls[call] += 1
                files[call].add(name)
report = {'method':'Text heuristic; may include definitions and strings, miss aliases and dynamic calls; not a compatibility verdict', 'scripts':len(scripts), 'documented_names':len(documented), 'matched_names':len(calls), 'functions':[{'name':name, 'text_occurrences':count, 'files':sorted(files[name])} for name,count in calls.most_common()]}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps({key:report[key] for key in ('scripts','documented_names','matched_names')}))
print('Most frequent:', ', '.join(name for name,count in calls.most_common(25)))
