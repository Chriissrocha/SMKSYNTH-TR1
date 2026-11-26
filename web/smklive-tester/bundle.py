from pathlib import Path
from base64 import b64encode
root=Path(__file__).resolve().parents[2]
p=root/'web/smklive-tester'
s=(p/'template.html').read_text()
wasm=(root/'build/smklive-tester/smklive.wasm').read_bytes()
protocol=(root/'web/install/protocol.js').read_text(); package=(root/'web/install/package.js').read_text()
# Original license headers and code preserved. Strip only ES module export statement for standalone use.
import re
protocol=re.sub(r'^export \{[^\n]*\};\s*$', '',protocol,flags=re.M)
package=re.sub(r'^export \{[^\n]*\};\s*$', '',package,flags=re.M)
install='const initLiveInstaller=(()=>{\n'+protocol+'\n'+package+'\n'+(p/'install-live.js').read_text()+'\nreturn initLiveInstaller;})();'
s=s.replace('__WASM_BASE64__',b64encode(wasm).decode('ascii')).replace('__PC_MIDI_JS__',(p/'midi-pc.js').read_text()).replace('__INSTALL_JS__',install)
(p/'index.html').write_text(s)
