"""Bundle the offline Windows/CLI performance tool, presets and documentation."""
from pathlib import Path
import zipfile
from perf_tool import PRESETS

root=Path(__file__).resolve().parents[1]
output=root/'artifacts/ShotgunK2Vita-PerformanceTool.zip'
output.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED) as z:
    for name in ('perf_tool.py','PerformanceTool.cmd','PerformanceTool.ps1','PERFORMANCE_README.txt'):
        z.write(root/'tools'/name,name)
    for name,values in PRESETS.items():
        z.writestr('presets/'+name+'.cfg',''.join(f'{k}={v}\n' for k,v in values.items()))
    z.write(root/'docs/PERFORMANCE_TOOL.md','PERFORMANCE_TOOL.md')
    z.write(root/'docs/images/controls-vita.png','controls-vita.png')
with zipfile.ZipFile(output) as z: assert z.testzip() is None
print(output)
