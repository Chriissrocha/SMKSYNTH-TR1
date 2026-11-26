/* SPDX-License-Identifier: GPL-3.0-only */
/* Stage controls: no new persistent records, no shared control ownership. */
#pragma once
static void smk_panel_panic(void);
static uint8_t live_send_memory[4][NTRK];
static uint8_t *live_send(unsigned i){return i==0?set.mix_cho:i==1?set.mix_dly:i==2?set.mix_rev:set.mix_drv;}
static void live_fx_toggle(unsigned i){
 unsigned t=seq_trk%NTRK;uint8_t *send=live_send(i);
 if(send[t]){live_send_memory[i][t]=send[t];send[t]=0;}
 else{
  send[t]=live_send_memory[i][t]?live_send_memory[i][t]:80;
  if(i<3){static const uint8_t p[]={P_CHO_MIX,P_DLY_MIX,P_REV_MIX};if(!live_p[0].v[p[i]]){set_param(p[i],48);fx_sync();}}
  else {if(!set.dv_amt)set.dv_amt=35;if(!set.dv_mix)set.dv_mix=90;}
 }
 mix_apply();set_dirty=1;set_t=fm1_ms;smk_controls_epoch++;pf_redraw();
 static const char*const on[]={"CHORUS ON","DELAY ON","REVERB ON","DRIVE ON"};
 static const char*const off[]={"CHORUS OFF","DELAY OFF","REVERB OFF","DRIVE OFF"};say(send[t]?on[i]:off[i]);
}
static void live_preset_step(int dir){unsigned t=seq_trk%NTRK;trk_load(t,(trk_prog[t]+NPROG+dir)%NPROG);pf_active[t]=255;pf_redraw();}
static void live_pad_event(unsigned p,unsigned vel){
 if(!vel)return;
 if(p<4){trk_select(p);return;}
 if(p<8){live_fx_toggle(p-4);return;}
 if(p<11){live_transpose_set(((int)p-9)*12);return;}
 if(p==11){hold^=1u<<(seq_trk%NTRK);ctl_hold=hold;say(hold&(1u<<seq_trk)?"HOLD ON":"HOLD OFF");}
 else if(p==12||p==13)live_preset_step(p==12?-1:1);
 else if(p==14)mix_mute(seq_trk%NTRK);
 else {smk_panel_panic();say("PANIC / ALL NOTES OFF");}
 pf_redraw();
}
static uint16_t live_pad_color(unsigned p){
 unsigned t=seq_trk%NTRK;
 if(p<4)return smk_dim(C_TRKC[p],p==t?240:28);
 if(p<8){static const uint16_t fx[]={RGB(40,190,255),RGB(100,125,255),RGB(90,255,180),RGB(255,155,40)};return smk_dim(fx[p-4],live_send(p-4)[t]?220:22);}
 if(p<11)return smk_dim(RGB(190,255,70),live_transpose==((int)p-9)*12?240:30);
 if(p==11)return smk_dim(C_CREAM,hold&(1u<<t)?230:24);
 if(p==14)return smk_dim(C_RED,set.mix_mute&(1u<<t)?230:30);
 if(p==15)return RGB(135,20,16);
 return smk_dim(C_AMBER,pf_role[p]==8u?220:65);
}
/* SEQ is dedicated to the live profile. Tap toggles pad bank; hold is a modifier.
 * A used modifier never changes bank, and arrows are consumed before their regular handler. */
static uint32_t live_buttons(uint32_t b,uint32_t *rel){
 const uint32_t seq=1u<<BTN_SEQ,arrows=(1u<<BTN_LEFT)|(1u<<BTN_RIGHT);
 if(save_ui_active()||page==PG_INITIALIZE){live_shift=0;live_shift_used=0;*rel&=~seq;return b&~seq;}
 if(b&seq){live_shift=1;live_shift_used=0;live_shift_t=fm1_ms;pf_redraw();}
 if(live_shift&&(b&arrows)){
  unsigned a=b&arrows;live_shift_used=1;
  live_transpose_set((fm1_in.buttons&arrows)==arrows||a==arrows?0:live_transpose+(a&(1u<<BTN_RIGHT)?1:-1));b&=~arrows;
 }
 if(live_shift&&(b&(1u<<BTN_KNOB_BANK))){
  live_shift_used=1;live_zone_learn=0;pf_nav(pf_bank()==5u?0u:5u);b&=~(1u<<BTN_KNOB_BANK);
 }
 if(*rel&seq){
  /* A queued detent from the held gesture belongs to transpose, never cutoff. */
  if(live_shift){int delta=fm1_enc_take(ENC_KNOB1);if(delta){live_shift_used=1;live_transpose_set(live_transpose+clampi(delta,-24,24));}}
  if(live_shift&&!live_shift_used&&fm1_ms-live_shift_t<600u)pf_nav(pf_bank()==4u?0u:4u);
  live_shift=live_shift_used=0;pf_redraw();
 }
 b&=~seq;*rel&=~seq;
 if(b&(1u<<BTN_PLAY)){live_zone_learn=0;smk_panel_panic();say("PANIC / ALL NOTES OFF");b&=~(1u<<BTN_PLAY);}
 if(b&(1u<<BTN_REC)){live_fx_toggle(1);b&=~(1u<<BTN_REC);}
 *rel&=~(1u<<BTN_REC);
 if(pf_bank()==4u&&page!=PG_KEYBOARD&&!layer&&(b&arrows)){
  if((b&arrows)!=arrows)live_preset_step(b&(1u<<BTN_RIGHT)?1:-1);
  b&=~arrows;
 }
 return b;
}
/* Paint only when the strip was touched (including a page change), not per loop.
 * This indicator is kept out of the main theme's animated meter area. */
static void live_badge_poll(void){
 if(!live_badge_dirty)return;
 if(!zm_live&&!live_transpose){live_badge_dirty=0;return;}
 cv_begin(66,18,C_PANEL);
 uint16_t ink=live_transpose?RGB(190,255,70):C_DIMT;
 cv_rect(0,0,66,18,C_SCOPE_BG);cv_rect(0,0,2,18,ink);
 char s[12]="TR ";fmt_s(s+3,live_transpose);
 cv_text(5,1,&FONT_S,s,ink);cv_blit(174,222);live_badge_dirty=0;
}

static int live_zone_editing(void){return pf_bank()==5u;}
static void live_zone_turn(unsigned knob,int delta){
 unsigned t=seq_trk%NTRK;
 if(knob==0){live_zone_low[t]=(uint8_t)clampi(live_zone_low[t]+delta,0,live_zone_high[t]);live_zone_mode=1;live_zone_mask|=1u<<t;}
 else if(knob==1){live_zone_high[t]=(uint8_t)clampi(live_zone_high[t]+delta,live_zone_low[t],127);live_zone_mode=1;live_zone_mask|=1u<<t;}
 else if(knob==2){live_zone_low[t]=0;live_zone_high[t]=127;live_zone_mode=1;live_zone_mask|=1u<<t;}
 else if(knob==3)live_zone_mode=delta>0;
 live_zone_learn=0;pf_redraw();
}
static void live_zone_pad(unsigned p,unsigned vel){
 if(!vel)return;
 unsigned t=p%4u;live_zone_learn=0;trk_select(t);page=PG_KEYBOARD;
 if(p<4u){if(!live_zone_mode){live_zone_mode=1;live_zone_mask=1u<<t;}else live_zone_mask^=1u<<t;}
 else if(p>=8u){live_zone_learn=(uint8_t)(p-7u);}
 pf_redraw();
}
static uint16_t live_zone_color(unsigned p){
 unsigned t=p%4u;
 if(p<4u)return smk_dim(C_TRKC[t],live_zone_mode&&(live_zone_mask&(1u<<t))?230:25);
 if(p<8u)return smk_dim(C_TRKC[t],seq_trk==t?230:60);
 return smk_dim(p<12u?C_AMBER:C_CREAM,live_zone_learn==p-7u?255:45);
}
static void live_zone_draw(void){
 char name[PATCH_NAME+1],low[12],hi[12],v[8],letter[2]={0,0};
 uint16_t accent=zm_live?zm_live_accent():C_AMBER;
 for(unsigned y=0;y<240;y+=120){mui_begin(y);cv_rect(0,0,240,240,C_PANEL);
  zm_text(8,4,"4 PART ZONES",24,accent);zm_text(181,7,live_zone_mode?"ON":"OFF",20,live_zone_mode?accent:C_DIMT);
  for(unsigned t=0;t<NTRK;t++){
   int yy=31+41*(int)t;int on=live_zone_mode&&(live_zone_mask&(1u<<t));uint16_t c=on?C_TRKC[t]:C_DIMT;
   letter[0]='A'+t;zm_name(name,&live_p[t]);fmt_note(low,live_zone_low[t]);fmt_note(hi,live_zone_high[t]);fmt_u(v,set.mix_lvl[t]);
   cv_rect(7,yy,2,38,c);if(seq_trk==t)mui_border(5,yy,230,39,accent);
   zm_text(13,yy+3,letter,24,c);zm_text(35,yy+1,name,20,on?C_CREAM:C_DIMT);zm_text(35,yy+22,low,16,c);zm_text(77,yy+22,"-",16,c);zm_text(90,yy+22,hi,16,c);
   cv_rect(149,yy+27,75,5,C_RULE);cv_rect(149,yy+27,(unsigned)set.mix_lvl[t]*75u/127u,5,c);zm_text(194,yy+6,v,16,c);
  }
  if(live_zone_learn)zm_text(8,199,live_zone_learn<=4u?"PLAY LOW KEY":"PLAY HIGH KEY",20,accent);
  else zm_text(8,199,"K1 LO K2 HI K3 ALL K4 ON",16,C_DIMT);
  zm_text(8,222,smk_bank==2u?"FADERS C: VOL A-D":smk_bank==1u?"FADERS B: FX":"FADERS A: SOUND",16,accent);mui_end(y);
 }
}
