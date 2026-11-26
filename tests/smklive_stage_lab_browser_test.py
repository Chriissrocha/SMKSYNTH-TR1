from pathlib import Path
import os
from playwright.sync_api import sync_playwright
root=str(Path(__file__).resolve().parents[1]/'web/smklive-tester')
with sync_playwright() as p:
 b=p.chromium.launch(executable_path=os.environ.get('CHROMIUM','/usr/bin/chromium'),args=['--no-sandbox']);page=b.new_page(viewport={'width':1440,'height':1000});page.set_default_timeout(3000);errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 page.evaluate('''Object.defineProperty(window,'isSecureContext',{value:true});window.fakeInput={id:'smk',name:'SMK-37 PRO TEST MOCK',state:'connected',open:async()=>{},close:async()=>{}};window.fakeAccess={inputs:new Map([['smk',fakeInput]]),outputs:new Map(),sysexEnabled:false};Object.defineProperty(navigator,'requestMIDIAccess',{value:async(o)=>fakeAccess});window.emit=(...data)=>fakeInput.onmidimessage?.({data:Uint8Array.from(data)});''')
 page.set_content(open(root+'/index.html').read());page.wait_for_function('window.SMKLIVE !== undefined');page.wait_for_timeout(350)
 print('STATUS',page.locator('#status').inner_text());assert not errors,errors
 # PC SEQ + encoder / GLOBE sticky and mouse
 page.keyboard.down('Shift');page.wait_for_timeout(80);page.locator('#knobPlus').click();page.wait_for_timeout(80);assert page.evaluate('SMKLIVE.engine.sim_live_transpose()')==1;page.keyboard.up('Shift');page.wait_for_timeout(70)
 page.get_by_role('button',name='Segurar SEQ').click();page.locator('#knobPlus').click();page.wait_for_timeout(80);assert page.evaluate('SMKLIVE.engine.sim_live_transpose()')==2;page.get_by_role('button',name='Soltar SEQ').click()
 page.get_by_role('button',name='Segurar GLOBE').click();page.wait_for_timeout(40);assert page.evaluate('downButtons.has(10)');page.locator('[data-knob="0"]').hover();page.mouse.wheel(0,-80);page.wait_for_timeout(50);assert page.evaluate('downButtons.has(10)');page.get_by_role('button',name='Soltar GLOBE').click();assert not page.evaluate('downButtons.has(10)')
 # Connect and test notes, dual ownership and releases
 page.locator('#midiConnect').click();page.wait_for_function('midiInput?.id === "smk"');page.evaluate('emit(0x90,60,95)');page.wait_for_timeout(80);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(60)')>=1;assert page.locator('[data-key="12"]').evaluate('(e)=>e.classList.contains("on")')
 page.keyboard.down('z');page.wait_for_timeout(40);page.evaluate('emit(0x80,60,0)');page.wait_for_timeout(50);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(60)')>=1;page.keyboard.up('z');page.wait_for_timeout(80);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(60)')==0
 # MIDI learn fader; trigger CC; note-on velocity zero
 page.locator('summary').filter(has_text='Mapear knobs').click();page.locator('#midiTarget').select_option('fader:0');page.locator('#midiLearn').click();page.evaluate('emit(0xb0,22,80)');page.evaluate('emit(0xb0,22,44)');assert page.locator('[data-fader="0"]').input_value()=='44';page.evaluate('emit(0x90,64,100)');page.wait_for_timeout(60);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(64)')>=1;page.evaluate('emit(0x90,64,0)');page.wait_for_timeout(70);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(64)')==0
 page.evaluate('emit(0x90,65,100)');page.wait_for_timeout(60);page.locator('#midiDisconnect').click();page.wait_for_timeout(80);assert page.evaluate('SMKLIVE.engine.sim_live_route_count(65)')==0;assert page.locator('#flashGo').is_disabled()
 assert page.evaluate('SMKLIVE.runTests()');print(page.locator('#testoutput').inner_text());print('PASS PC + MOCK MIDI + FLASH gate');assert not errors,errors
 b.close()

