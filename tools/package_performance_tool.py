"""Bundle the offline Windows/CLI performance tool, presets and documentation."""
from pathlib import Path
import zipfile
import argparse
from perf_tool import PRESETS

def package(output):
    root=Path(__file__).resolve().parents[1]
    output=Path(output)
    output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(output,'x',zipfile.ZIP_DEFLATED) as z:
        for name in ('perf_tool.py','stability_audit.py','PerformanceTool.cmd','PerformanceTool.ps1','PERFORMANCE_README.txt'):
            z.write(root/'tools'/name,name)
        for p in (root/'src').glob('*.c'):
            z.write(p,'api-source/src/'+p.name)
        for name,values in PRESETS.items():
            z.writestr('presets/'+name+'.cfg',''.join(f'{k}={v}\n' for k,v in values.items()))
        z.write(root/'docs/PERFORMANCE_TOOL.md','PERFORMANCE_TOOL.md')
        z.write(root/'docs/images/controls-vita.png','controls-vita.png')
    with zipfile.ZipFile(output) as z: assert z.testzip() is None
    return output

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    print(package(parser.parse_args().output))
