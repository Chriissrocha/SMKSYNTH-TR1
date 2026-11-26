// SPDX-License-Identifier: GPL-3.0-only
// Installer state and integrity tests only; no physical USB, no real write.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {webcrypto} from 'node:crypto';
import {productOf,logicalImage,filesOk} from '../web/install/package.js';
import {Updater} from '../web/install/protocol.js';
const code=fs.readFileSync(new URL('../web/smklive-tester/install-live.js',import.meta.url),'utf8');
const api=new Function('productOf','logicalImage','filesOk','Updater','crypto',code+'\nreturn {checkLivePackage,liveSelection,liveExecute,LIVE_PENDING};')(productOf,logicalImage,filesOk,Updater,webcrypto);
const raw=new Uint8Array(fs.readFileSync(new URL('../web/install/firmware/SMKSYNTH-1.3.1-update-SMK37-Pro.fwsc',import.meta.url)));
const product='SMK-37 Pro_19701', sha=async b=>Buffer.from(await webcrypto.subtle.digest('SHA-256',b)).toString('hex');
const manifest={schema:'smklive-update-v1',release:'1.3.1-live2',operation:'update',model:'pro',product,size:raw.length,sha256:await sha(raw)};
await assert.rejects(api.checkLivePackage(raw,manifest),/identidade/);
// A fixture made by modifying identity marker bytes IN MEMORY, only to exercise the validator; never emitted or flashed.
const fixture=raw.slice();for(let i=0;i<20;i++)fixture[i*48+47]=i<product.length?(product.charCodeAt(i)+i+1)&255:125;
manifest.sha256=await sha(fixture);const pkg=await api.checkLivePackage(fixture,manifest);assert.equal(pkg.product,product);
const bad=fixture.slice();bad[2000]^=1;await assert.rejects(api.checkLivePackage(bad,manifest),/SHA/);
for(const wrong of ['SMK-37 Elite_19668','SMK-37 Pro_016','SMK-37 Pro_19667','ota-SMK-37 Pro_900','FM-1_904'])assert.throws(()=>api.liveSelection(wrong,pkg,null));
let source='SMK-37 Pro_19668',writes=0,fail=false,wrongLoader=false;
class FakeUpdater{constructor(a,l,o){this.o=o;}async find(){return {id:{text:source},link:{close(){}}};}async install(image,target){this.o.guard('start',{text:source});this.o.guard('write',{text:wrongLoader?'ota-SMK-37 Elite_900':'ota-SMK-37 Pro_900'});writes++;if(fail){source='ota-SMK-37 Pro_900';throw Error('disconnect');}source=target;}async resume(image,step,{product}){this.o.guard('write',{text:source});writes++;source=product;return true;}}
const data=new Map(),storage={getItem:k=>data.get(k),setItem:(k,v)=>data.set(k,v),removeItem:k=>data.delete(k)};
const execute=selection=>api.liveExecute({access:{},storage,pkg,selection,log:()=>{},step:()=>{},UpdaterType:FakeUpdater});
let selection=api.liveSelection(source,pkg,null);wrongLoader=true;await assert.rejects(execute(selection),/loader/);assert.equal(writes,0);wrongLoader=false;fail=true;await assert.rejects(execute(selection),/disconnect/);assert(data.has(api.LIVE_PENDING));fail=false;selection=api.liveSelection(source,pkg,JSON.parse(data.get(api.LIVE_PENDING)));assert.equal(selection.mode,'resume');assert.equal(await execute(selection),product);assert(!data.has(api.LIVE_PENDING));assert.equal(writes,2);
console.log('PASS: historic binary rejected, SHA/identity/CRC gate, exact model and origin, foreign loader rejection, interrupted update pending, same-package resume (mock).');
