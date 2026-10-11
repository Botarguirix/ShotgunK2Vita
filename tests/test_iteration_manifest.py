import hashlib
import json
import tempfile
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from iteration_manifest import finalize

root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(dir=root/'reports') as temp:
    base=Path(temp)
    (base/'reports/iterations/00.99/input').mkdir(parents=True)
    (base/'reports/iterations/00.99/input/runtime.log').write_text('received evidence')
    (base/'docs').mkdir()
    (base/'docs/STATUS_ITERATION99.md').write_text('status')
    (base/'docs/INFORME_ROADMAP_ITERACION99.md').write_text('roadmap')
    out=base/'delivery'
    out.mkdir()
    (out/'test.vpk').write_bytes(b'fixture')
    result=finalize(base,out,'00.99')
    assert result['hardware_validation']=='pending'
    assert result['files']['test.vpk']['sha256']==hashlib.sha256(b'fixture').hexdigest()
    assert (out/'evidence/input/runtime.log').read_text()=='received evidence'
    assert 'INFORME_ROADMAP_ITERACION99.md' in result['files']
    original=(out/'delivery-audit.json').read_bytes()
    try:
        finalize(base,out,'00.99')
    except FileExistsError:
        pass
    else:
        raise AssertionError('overwrite was accepted')
    assert (out/'delivery-audit.json').read_bytes()==original
    assert json.loads(original)['version']=='00.99'
print('PASS delivery audit, evidence, hashes and overwrite refusal')

