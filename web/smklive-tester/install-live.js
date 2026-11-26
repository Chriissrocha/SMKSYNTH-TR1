// SPDX-License-Identifier: GPL-3.0-only
// Uses the original protocol.js and package.js from SMKSYNTH (bundled at build).
// Update only. Manifest must come from the matching trusted build artifact.
const LIVE_PENDING='smklive.live2.update.pending.v1';
const LIVE_TARGETS={pro:'SMK-37 Pro_19701',elite:'SMK-37 Elite_19702'};
const LIVE_ORIGINS={pro:[19668,19701],elite:[19668,19702]};
function liveModel(text){return Object.keys(LIVE_TARGETS).find(m=>new RegExp('^(ota-)?'+LIVE_TARGETS[m].split('_')[0]+'_[0-9]{3,5}$').test(text));}
async function checkLivePackage(raw,manifest){
 const m=manifest?.model,p=LIVE_TARGETS[m];
 if(!p||manifest.schema!=='smklive-update-v1'||manifest.release!=='1.3.1-live2'||manifest.operation!=='update'||manifest.product!==p||!Number.isInteger(manifest.size)||manifest.size!==raw.length||raw.length>4000000||! /^[a-f0-9]{64}$/.test(manifest.sha256))throw Error('Manifesto incompatível: use o manifesto do build SMKLIVE V2.');
 const sha=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',raw)),b=>b.toString(16).padStart(2,'0')).join('');
 if(sha!==manifest.sha256||productOf(raw)!==p)throw Error('SHA-256 ou identidade do pacote não confere.');
 const image=logicalImage(raw);if(!filesOk(image))throw Error('CRC interno do pacote inválido.');
 return {model:m,product:p,sha,image};
}
function liveSelection(text,pkg,pending){
 const m=liveModel(text);if(!m||m!==pkg.model)throw Error('Modelo do teclado não corresponde ao pacote.');
 const name=pkg.product.split('_')[0];
 if(text==='ota-'+name+'_900'){
   if(pending?.model===m&&pending.sha===pkg.sha&&LIVE_ORIGINS[m].some(v=>pending.source===name+'_'+v))return {mode:'resume',source:pending.source,identity:text};
   throw Error('Loader sem atualização correspondente. Retome com o instalador e pacote originais.');
 }
 if(!LIVE_ORIGINS[m].some(v=>text===name+'_'+v))throw Error('Origem não habilitada: '+text+'. Este instalador aceita SMKSYNTH 1.3.1 (19668) ou SMKLIVE V2 do mesmo modelo.');
 if(pending&&(pending.model!==m||pending.sha!==pkg.sha||pending.source!==text))throw Error('Existe uma operação pendente diferente. Use o pacote original.');
 return {mode:'update',source:text,identity:text};
}
async function liveExecute({access,storage,pkg,selection,log,step,UpdaterType=Updater}){
 const name=pkg.product.split('_')[0];
 const guard=(phase,id)=>{if(phase==='start'?id.text!==selection.identity:id.text!=='ota-'+name+'_900')throw Error('Identidade alterada / loader incorreto. Gravação recusada.');};
 const up=new UpdaterType(access,log,{guard});
 // Reject ambiguous setups; the user must disconnect other MIDI equipment before installing.
 const d=await up.find();if(!d)throw Error('Teclado não encontrado.');d.link.close();
 const pending=JSON.parse(storage.getItem(LIVE_PENDING)||'null');
 const current=liveSelection(d.id.text,pkg,pending);
 if(current.identity!==selection.identity||current.source!==selection.source||current.mode!==selection.mode)throw Error('Identidade mudou. Identifique novamente.');
 storage.setItem(LIVE_PENDING,JSON.stringify({model:pkg.model,sha:pkg.sha,source:current.source}));
 if(current.mode==='resume'){if(!await up.resume(pkg.image,step,{product:pkg.product}))throw Error('Retomada incompleta. Guarde o log.');}
 else await up.install(pkg.image,pkg.product,step,false);
 storage.removeItem(LIVE_PENDING);return pkg.product;
}
function initLiveInstaller(){
 const $i=id=>document.getElementById(id);let access=null,device=null,pkg=null,selection=null,working=false,scanVersion=0,lines=[];
 const log=s=>{lines.push(new Date().toISOString()+' '+s);$i('flashLog').textContent=lines.join('\n');};
 const pending=()=>JSON.parse(localStorage.getItem(LIVE_PENDING)||'null');
 function render(){for(const id of ['flashFw','flashManifest','flashIdentify','flashAck','flashBackup'])$i(id).disabled=working;$i('flashGo').disabled=working||!selection||!pkg||!$i('flashAck').checked||!$i('flashBackup').checked;$i('midiConnect').disabled=working;}
 function assess(){selection=null;if(pkg&&device)try{selection=liveSelection(device.text,pkg,pending());$i('flashState').textContent=(selection.mode==='resume'?'Retomar':'Atualizar')+': '+selection.source+' → '+pkg.product;}catch(e){$i('flashState').textContent=e.message;}render();}
 async function load(){const v=++scanVersion;pkg=null;selection=null;render();try{const fw=$i('flashFw').files[0],mf=$i('flashManifest').files[0];if(!fw||!mf){$i('flashPkg').textContent='Selecione os dois arquivos do build.';return;}if(fw.size>4000000||mf.size>16000)throw Error('Arquivos excedem o limite.');const checked=await checkLivePackage(new Uint8Array(await fw.arrayBuffer()),JSON.parse(await mf.text()));if(v!==scanVersion)return;pkg=checked;$i('flashPkg').textContent='Identidade e integridade conferidas: '+pkg.product+' · SHA-256 '+pkg.sha;assess();}catch(e){if(v===scanVersion)$i('flashPkg').textContent=e.message;}finally{render();}}
 $i('flashFw').onchange=load;$i('flashManifest').onchange=load;$i('flashAck').onchange=render;$i('flashBackup').onchange=render;
 async function exclusive(fn){if(!isSecureContext||!navigator.requestMIDIAccess||!navigator.locks?.request)throw Error('Abra em HTTPS ou localhost, no Chrome/Edge atualizado.');await navigator.locks.request('obervave-smk37-firmware-update',{ifAvailable:true},async lock=>{if(!lock)throw Error('Outra aba está utilizando o instalador.');await fn();});}
 async function stopPlay(){await disconnectMidi();panic();if(ctx?.state==='running'){await ctx.suspend();$('audio').textContent='Ativar áudio';} }
 $i('flashIdentify').onclick=async()=>{working=true;device=selection=null;render();try{await stopPlay();await exclusive(async()=>{access=await navigator.requestMIDIAccess({sysex:true});if(!access.sysexEnabled)throw Error('Permissão SysEx não concedida.');const up=new Updater(access,log);const found=await up.find();if(!found)throw Error('Nenhuma identidade reconhecida. Confira USB.');found.link.close();device=found.id;$i('flashDevice').textContent=device.text;access.onstatechange=()=>{if(!working){device=selection=null;$i('flashDevice').textContent='USB mudou; identifique novamente.';render();}};});assess();}catch(e){$i('flashDevice').textContent=e.message;log(e.message);}finally{working=false;render();}};
 $i('flashGo').onclick=async()=>{if($i('flashGo').disabled)return;if(!confirm('Atualização EXPERIMENTAL: '+selection.identity+' → '+pkg.product+'\n\nConfirma backup e cabo USB estável? Não desconecte nem feche a página.'))return;working=true;render();$i('flashProgress').removeAttribute('value');try{await stopPlay();await exclusive(async()=>{if(!access?.sysexEnabled)throw Error('Identifique o teclado novamente.');await liveExecute({access,storage:localStorage,pkg,selection,log,step:(phase,n)=>{$i('flashState').textContent=phase+' '+(n??'');if(!['write','verify'].includes(phase)||Number(n)%100===0)log(phase+' '+(n??''));}});});$i('flashProgress').value=1;$i('flashState').textContent='Concluído. Identidade final confirmada: '+pkg.product;device=selection=null;}catch(e){$i('flashState').textContent='Interrompido: '+e.message+' · guarde o log e retome com o mesmo pacote.';log(e.message);device=selection=null;}finally{working=false;render();}};
 $i('flashDownloadLog').onclick=()=>{const u=URL.createObjectURL(new Blob([lines.join('\n')],{type:'text/plain'}));const a=document.createElement('a');a.href=u;a.download='SMKLIVE-update-log.txt';a.click();setTimeout(()=>URL.revokeObjectURL(u),1000);};
 window.addEventListener('beforeunload',e=>{if(working){e.preventDefault();e.returnValue='';}});render();
}
