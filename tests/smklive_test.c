/* SPDX-License-Identifier: GPL-3.0-only */
#define main obv_host_main
#include "../firmware/src/obervave.c"
#undef main
#include <assert.h>
#ifndef FM1_OCT_BOTH
#define FM1_BTN_OCT_DOWN 4
#define FM1_OCT_BOTH ((1u<<4)|(1u<<5))
#endif
#include "../firmware/src/bootguard.h"
static void key(unsigned b){uint32_t r=0;fm1_in.buttons|=1u<<b;assert(!(live_buttons(1u<<b,&r)&(1u<<b)));}
static void up(unsigned b){uint32_t r=1u<<b;fm1_in.buttons&=~r;live_buttons(0,&r);}
static void enc(int delta){host_enc[ENC_KNOB1]=delta;panel_encoders();}
static void pad(unsigned i){uint32_t d=0;host_qput(0,(i<<8)|100);smk_pads(0,&d);host_qput(0,i<<8);smk_pads(0,&d);}
static void render(void){int32_t audio[HALF_FRAMES*2]={0};engine_audio(audio,HALF_FRAMES);}
static void snapshot(const char *path){draw_page();live_badge_poll();write_ppm_as(path);}
int main(void){
 engine_init();pf_init();pat_init(&pat);mm_boot();zm_boot();assert(zm_theme_index()==2);
 set.tempo=120;set.m_vol=100;set.m_dj=64;set.m_ok=M_OK;set.dv_ok=DV_OK;
 for(unsigned t=0;t<NTRK;t++){trk_load(t,0);set.mix_lvl[t]=100;patch_keyboard_octave_set(&live_p[t],0);live_p[t].v[P_ARP_MODE]=A_OFF;}
 seq_trk=0;page=PG_AENV;layer=0;mix_apply();fm1_ms=1000;
 unsigned val=live_p[0].v[P_AA];key(BTN_SEQ);key(BTN_RIGHT);up(BTN_RIGHT);assert(live_transpose==1);enc(1);assert(live_transpose==2&&live_p[0].v[P_AA]==val);
 enc(-1);assert(live_transpose==1);enc(-1);assert(live_transpose==0);enc(-30);assert(live_transpose==-12);enc(30);assert(live_transpose==12);up(BTN_SEQ);assert(pf_bank()==0);
 enc(val>125?-1:1);assert(live_p[0].v[P_AA]!=val);
 unsigned changed=live_p[0].v[P_AA];key(BTN_SEQ);host_enc[ENC_KNOB1]=-3;up(BTN_SEQ);assert(live_transpose==9&&live_p[0].v[P_AA]==changed);live_transpose_set(-12);
 key(BTN_SEQ);for(int i=-11;i<=12;i++){key(BTN_RIGHT);up(BTN_RIGHT);assert(live_transpose==i);}key(BTN_LEFT);key(BTN_RIGHT);assert(live_transpose==0);up(BTN_LEFT);up(BTN_RIGHT);up(BTN_SEQ);assert(pf_bank()==0);
 panel_note_in(1,60,100);render();assert(kb_press[60].count==1&&kb_press[60].route[0].note==60);
 live_transpose_set(12);panel_note_in(1,61,100);render();assert(kb_press[61].route[0].note==73&&kb_refs[0][60]&&kb_refs[0][73]);
 panel_note_in(0,60,0);panel_note_in(0,61,0);render();assert(!kb_refs[0][60]&&!kb_refs[0][73]&&!kb_press[60].count&&!kb_press[61].count);
 live_transpose_set(-12);keyboard_cfg_t k;keyboard_defaults(&k);k.mode=KB_LAYER;k.octave[1]=1;pat.keyboard=k;
 panel_note_in(1,60,100);render();assert(kb_press[60].count==2&&kb_press[60].route[0].note==48&&kb_press[60].route[1].note==60);live_transpose_set(12);panel_note_in(0,60,0);render();assert(!kb_refs[0][48]&&!kb_refs[1][60]);
 k.mode=KB_SPLIT;k.split=60;k.octave[1]=0;pat.keyboard=k;panel_note_in(1,59,100);render();assert(kb_press[59].count==1&&kb_press[59].route[0].part==0&&kb_press[59].route[0].note==71);panel_note_in(0,59,0);render();pat_init(&pat);
 set.midi_ch=1;live_transpose_set(12);mq[mq_w%64][0]=0x91;mq[mq_w%64][1]=60;mq[mq_w%64][2]=100;mq_w++;midi_in(0);assert(live_midi_note[1][60]==73);
 live_transpose_set(0);mq[mq_w%64][0]=0x81;mq[mq_w%64][1]=60;mq[mq_w%64][2]=0;mq_w++;midi_in(0);assert(!live_midi_note[1][60]);set.midi_ch=0;
 live_transpose_set(12);keyboard_play_note(1,127,100);assert(!kb_press[127].count);keyboard_play_note(0,127,0);
 key(BTN_SEQ);up(BTN_SEQ);assert(pf_bank()==4);page=PG_PERFORM;pad(1);assert(seq_trk==1);
 set.mix_cho[1]=45;pad(4);assert(set.mix_cho[1]==0);pad(4);assert(set.mix_cho[1]==45);
 pad(5);assert(set.mix_dly[1]&&live_p[0].v[P_DLY_MIX]);pad(5);assert(!set.mix_dly[1]);
 pad(6);assert(set.mix_rev[1]);pad(7);assert(set.mix_drv[1]&&set.dv_mix);
 pad(8);assert(live_transpose==-12);pad(9);assert(!live_transpose);pad(10);assert(live_transpose==12);
 pad(11);assert(hold&2);pad(11);assert(!(hold&2));unsigned prog=trk_prog[1];pad(13);assert(trk_prog[1]==(prog+1)%NPROG);pad(12);assert(trk_prog[1]==prog);
 pad(14);assert(set.mix_mute&2);pad(14);assert(!(set.mix_mute&2));
 panel_note_in(1,60,100);pad(15);assert(ctl_panic&&!kb_press[60].count);render();assert(!ctl_panic);
 key(BTN_PLAY);assert(ctl_panic);up(BTN_PLAY);render();key(BTN_REC);up(BTN_REC);assert(set.mix_dly[1]);
 /* Four parts, independent ranges, release routing and fader pickup. */
 live_transpose_set(0);seq_trk=0;key(BTN_SEQ);key(BTN_KNOB_BANK);up(BTN_KNOB_BANK);up(BTN_SEQ);assert(pf_bank()==5);
 for(unsigned i=0;i<4;i++)pad(i);assert(live_zone_mode&&live_zone_mask==15);
 panel_note_in(1,60,100);assert(kb_press[60].count==4);for(unsigned i=0;i<4;i++)assert(kb_refs[i][60]);
 live_transpose_set(5);pad(0);panel_note_in(0,60,0);for(unsigned i=0;i<4;i++)assert(!kb_refs[i][60]);render();
 pad(0);live_transpose_set(0);pad(12);panel_note_in(1,59,100);assert(live_zone_high[0]==59&&!kb_press[59].count);panel_note_in(0,59,0);
 pad(9);panel_note_in(1,60,100);panel_note_in(0,60,0);assert(live_zone_low[1]==60);
 live_zone_high[1]=71;live_zone_low[2]=72;live_zone_high[2]=83;live_zone_low[3]=84;
 for(unsigned i=0;i<4;i++){unsigned note=48+12*i;panel_note_in(1,note,100);assert(kb_press[note].count==1&&kb_press[note].route[0].part==i);panel_note_in(0,note,0);}
 /* mixed split/layer */ live_zone_low[2]=60;live_zone_high[2]=71;panel_note_in(1,64,100);assert(kb_press[64].count==2);panel_note_in(0,64,0);render();
 smk_bank=0;uint32_t dirt=0;smk_buttons(1u<<BTN_FADER_BANK,&dirt);assert(smk_bank==1);smk_buttons(1u<<BTN_FADER_BANK,&dirt);assert(smk_bank==2);
 fm1_in.frames=300;for(unsigned i=0;i<4;i++)fm1_in.fader[i]=100;smk_faders(&dirt,&dirt,&dirt);for(unsigned i=0;i<4;i++)fm1_in.fader[i]=20+i*15;smk_faders(&dirt,&dirt,&dirt);for(unsigned i=0;i<4;i++)assert(set.mix_lvl[i]==20+i*15&&ctl_mix_lvl[i]==20+i*15);
 smk_buttons(1u<<BTN_FADER_BANK,&dirt);assert(smk_bank==0);live_zone_mode=0;pf_nav(0);render();
 ctl_seq_run=ctl_seq_restart=1;seq_tick();assert(!ctl_seq_run&&!ctl_seq_restart);
 assert(!obv_engines[OBV_DRUMS]);patch_t drum;obv_drums_preset(&drum,0);patch_t original=drum;assert(obv_patch_sanitize(&drum)&&obv_patch_engine(&drum)==OBV_ANALOG&&obv_patch_engine(&original)==OBV_DRUMS);
 assert(sizeof(patch_t)==80&&sizeof(pattern_t)==14476&&sizeof(psnd_t)==380);
 assert(bootguard_manual(1u<<FM1_BTN_OCT_DOWN));assert(!bootguard_manual(FM1_OCT_BOTH));assert(!bootguard_manual(0));
 bootguard_t guard={0};assert(bootguard_begin(&guard)==BOOT_NORMAL);assert(bootguard_begin(&guard)==BOOT_NORMAL);assert(bootguard_begin(&guard)==BOOT_RECOVERY);assert(bootguard_begin(&guard)==BOOT_ROM);
 /* The old theme setting remains valid and the MIDI map record schema stays intact. */
 mm_data.reserved[0]=0x5A;mm_data.reserved[1]=3;zm_boot();assert(zm_theme_index()==1&&ctl_pattern_tempo);zm_theme_set(2);assert(zm_theme_index()==2&&zm_theme_bits()==7);
 mm_data.reserved[1]=0;zm_boot();assert(zm_theme_index()==0);live_transpose_set(12);
 for(unsigned theme=0;theme<3;theme++){zm_theme_set(theme);for(unsigned pg=0;pg<NPAGE;pg++){page=pg;if(pg==PG_SEQ||pg==PG_DSEQ||pg==PG_DKIT||pg==PG_DSOUND)continue;layer=0;draw_page();live_badge_poll();assert(fb[222*240+174]==RGB(190,255,70));}}
 zm_theme_set(2);seq_trk=0;page=PG_PERFORM;layer=0;memset(live_p[0].v+NPARAM,' ',PATCH_NAME);memcpy(live_p[0].v+NPARAM,"WARM PAD",8);ui_vlevel[0]=23000;ui_vlevel[2]=16000;ui_vlevel[4]=30000;ui_vlevel[5]=10000;
 snapshot("build/smklive/perform.ppm");page=PG_MIXER;snapshot("build/smklive/mixer.ppm");page=PG_KEYBOARD;snapshot("build/smklive/keyboard.ppm");page=PG_THEME;snapshot("build/smklive/theme.ppm");
 puts("PASS: SMKLIVE modifier and K1, held-note routing, split/layer, 16 pads, FX, panic, retired engines/transport, ABI, recovery predicate, themes and indicator.");
 return 0;
}
