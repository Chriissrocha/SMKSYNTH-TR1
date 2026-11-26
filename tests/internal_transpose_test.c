/* SPDX-License-Identifier: GPL-3.0-only */
#define main obv_host_main
#include "../firmware/src/obervave.c"
#undef main
#include <assert.h>
static int32_t audio[HALF_FRAMES*2];
static void render(void){memset(audio,0,sizeof audio);engine_audio(audio,HALF_FRAMES);}
static unsigned gate(unsigned part,unsigned note){unsigned count=0;for(unsigned i=0;i<NV;i++)count+=vc[i].gate&&vc[i].part==part&&vc[i].note==note;return count;}
static void physical(unsigned on,unsigned note){panel_note_in(on,note,on?100:0);}
static void press(unsigned bit){assert(smk_transpose_buttons(bit,(1u<<BTN_SEL)|bit));}
int main(void){
 const unsigned down=1u<<BTN_OCT_DOWN,up=1u<<BTN_OCT_UP,scale=1u<<BTN_SEL;
 engine_init();pat_init(&pat);pf_init();set.tempo=120;set.m_vol=100;
 for(unsigned t=0;t<NTRK;t++){trk_load(t,0);live_p[t].v[P_MODE]=M_POLY;live_p[t].v[P_ARP_MODE]=A_OFF;patch_keyboard_octave_set(&live_p[t],0);}
 seq_trk=0;page=PG_PERFORM;layer=L_NONE;step_held=-1;rec=REC_OFF;
 assert(panel_transpose==0&&!smk_transpose_buttons(up,up)&&!smk_transpose_buttons(scale,scale|up));
 patch_t before=live_p[0];unsigned dirty=set_dirty;
 press(up);assert(panel_transpose==1&&!strcmp(toast,"TRANSPOSE +1")&&!memcmp(&before,&live_p[0],sizeof before)&&set_dirty==dirty);
 physical(1,60);render();assert(kb_press[60].route[0].note==61&&gate(0,61));
 press(up);assert(panel_transpose==2&&gate(0,61));
 physical(1,62);render();assert(gate(0,64));
 physical(0,60);render();assert(!gate(0,61)&&gate(0,64));
 physical(0,62);render();assert(!gate(0,64));
 for(unsigned i=0;i<20;i++)press(up);assert(panel_transpose==12);
 for(unsigned i=0;i<30;i++)press(down);assert(panel_transpose==-12);
 assert(smk_transpose_buttons(up,scale|up|down)&&!panel_transpose&&!strcmp(toast,"TRANSPOSE 0"));
 press(down);assert(panel_transpose==-1&&!strcmp(toast,"TRANSPOSE -1"));
 for(unsigned pg=0;pg<NPAGE;pg++)if(pg!=PG_PERFORM){page=pg;assert(!smk_transpose_buttons(up,scale|up));}
 page=PG_PERFORM;for(unsigned l=L_SEQ;l<=L_FX;l++){layer=l;assert(!smk_transpose_buttons(up,scale|up));}layer=L_NONE;
 mm_on=1;assert(!smk_transpose_buttons(up,scale|up));mm_on=0;
 preset_ui=1;assert(!smk_transpose_buttons(up,scale|up));preset_ui=0;
 pb_confirm=1;assert(!smk_transpose_buttons(up,scale|up));pb_confirm=0;
 assert(panel_transpose==-1);
 note_in(1,60,100);render();assert(gate(0,60));note_in(0,60,0);render();assert(!gate(0,60));
 patch_keyboard_octave_set(&live_p[0],1);physical(1,60);render();assert(gate(0,71));physical(0,60);render();assert(!gate(0,71));
 /* Layer sums both individual octaves, with one global semitone shift. */
 keyboard_cfg_t k;keyboard_defaults(&k);k.mode=KB_LAYER;k.part[0]=0;k.part[1]=1;k.octave[0]=-1;k.octave[1]=1;keyboard_changed(k);
 physical(1,60);render();assert(kb_press[60].count==2&&gate(0,59)&&gate(1,71));
 press(up);physical(0,60);render();assert(!gate(0,59)&&!gate(1,71));
 /* Split point remains on the physical key and does not shift. */
 k.mode=KB_SPLIT;k.split=60;k.octave[0]=k.octave[1]=0;keyboard_changed(k);patch_keyboard_octave_set(&live_p[0],0);press(up);
 physical(1,59);physical(1,60);render();assert(gate(0,60)&&gate(1,61));physical(0,59);physical(0,60);render();assert(!gate(0,60)&&!gate(1,61));
 /* Drum notes stay unchanged. A note beyond MIDI range is not clamped or stuck. */
 k.mode=KB_SINGLE;keyboard_changed(k);obv_patch_set_engine(&live_p[0],OBV_DRUMS);physical(1,60);assert(kb_press[60].count==1&&kb_press[60].route[0].note==60);physical(0,60);render();
 obv_patch_set_engine(&live_p[0],OBV_ANALOG);physical(1,127);assert(!kb_press[127].count);physical(0,127);
 /* Sustain and releases keep the original routed pitch after changing transpose. */
 ctl_sustain=1;physical(1,64);render();physical(0,64);render();assert(gate(0,65));press(up);ctl_sustain=0;render();assert(!gate(0,65));
 for(unsigned t=0;t<NTRK;t++)for(unsigned n=0;n<128;n++)assert(!kb_refs[t][n]);
 puts("PASS: semitone limits/reset, unchanged controls/storage, held-note release, MIDI input, octave, split/layer, drums, range and sustain.");
}
