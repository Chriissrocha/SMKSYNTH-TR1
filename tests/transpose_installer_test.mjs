// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {webcrypto} from 'node:crypto';
if(!globalThis.crypto)globalThis.crypto=webcrypto;
const dir=path.resolve(process.argv[2]||'web/install-transpose');
const {PINS,PENDING,chooseOperation,validate,readPending}=await import(pathToFileURL(path.join(dir,'service.js')));
const manifest=JSON.parse(fs.readFileSync(path.join(dir,'manifest.json')));
assert.deepEqual(PINS,manifest);
assert.equal(PENDING,'smksynth.1.3.1-TR1.pro-elite.pending.v1');
const id=text=>({model:text.split('_')[0],text});
const ctrl=fs.readFileSync(path.join(dir,'controller.js'),'utf8');
assert(ctrl.includes("$('connect').disabled=busy||!ready;"));
assert(ctrl.includes("TR1 ainda não compilado. Instalação bloqueada."));
if(process.argv.includes('--built')){
 for(const [model,label] of Object.entries({pro:'Pro',elite:'Elite'})){
  const name='SMK-37 '+label;
  assert.equal(chooseOperation(id(name+'_016')).kind,'release');
  assert.equal(chooseOperation(id(name+'_19668')).kind,'update');
  assert.equal(chooseOperation(id(name+'_19669')).kind,'update');
  for(const kind of ['release','update']){
   const pin=PINS[model][kind];assert.equal(pin.product,name+'_19669');
   const raw=new Uint8Array(fs.readFileSync(path.join(dir,pin.url)));
   assert.equal((await validate(raw,model,kind)).product,pin.product);
   const bad=raw.slice();bad[900]^=1;await assert.rejects(()=>validate(bad,model,kind));
   await assert.rejects(()=>validate(raw,model==='pro'?'elite':'pro',kind));
   const pending={model,kind,source:name+(kind==='release'?'_016':'_19668'),sha:pin.sha};
   assert.equal(chooseOperation(id('ota-'+name+'_900'),pending).mode,'resume');
   assert.equal(readPending({getItem:()=>JSON.stringify(pending)}).sha,pin.sha);
   assert.throws(()=>chooseOperation(id('ota-'+name+'_900'),{...pending,sha:'old'}));
  }
 }
 console.log('PASS: TR1 model-specific firmware, SHA-256, corruption rejection and resumable operation selection.');
}else{
 assert.deepEqual(PINS,{});
 assert.throws(()=>chooseOperation(id('SMK-37 Pro_19668')));
 await assert.rejects(()=>validate(new Uint8Array(),'pro','update'));
 assert(!fs.existsSync(path.join(dir,'firmware')));
 console.log('PASS: preview has no firmware pins or packages and refuses installation.');
}
