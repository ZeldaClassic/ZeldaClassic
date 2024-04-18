#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "zc/maps.h"
#include "zc/zelda.h"
#include "zc/hero.h"
#include "base/colors.h"
#include "base/zsys.h"
#include "base/mapscr.h"
#include "pal.h"
#include "subscr.h"
#include "base/misctypes.h"

extern HeroClass Hero;

int32_t CSET_SIZE = 16;  // this is only changed to 4 in the NES title screen
int32_t CSET_SHFT = 4;   // log2 of CSET_SIZE

bool stayLit = false;

bool usingdrypal = false; //using dried up lake colors
RGB olddrypal; //palette to restore when lake rehydrates

void copy_pal(RGB *src,RGB *dest)
{
    for(int32_t i=0; i<256; i++)
        dest[i]=src[i];
}

void loadfullpal()
{
    for(int32_t i=0; i<240; i++)
        RAMpal[i]=_RGB(colordata+i*3);
        
    for(int32_t i=240; i<255; i++)
        RAMpal[i]=pal_gui[i];
        
    refreshpal=true;
}

/*void loadlvlpal256(int32_t level)
  {
  byte *si = colordata + */

extern PALETTE tempgreypal;
extern PALETTE userPALETTE[256];

void loadlvlpal(int32_t level)
{
	byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;
	
	for(int32_t i=0; i<16*3; i++)
	{
		RAMpal[CSET(2)+i] = _RGB(si);
		tempgreypal[CSET(2)+i] = _RGB(si); //preserve monochrome
		si+=3;
	}
	
	for(int32_t i=0; i<16; i++)
	{
		RAMpal[CSET(9)+i] = _RGB(si);
		tempgreypal[CSET(9)+i] = _RGB(si); //preserve monochrome
		si+=3;
	}
	
	if (get_qr(qr_CSET1_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(1)+i] = _RGB(si);
			tempgreypal[CSET(1)+i] = _RGB(si); //preserve monochrome
			si+=3;
		}
	}
	if (get_qr(qr_CSET5_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+1)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(5)+i] = _RGB(si);
			tempgreypal[CSET(5)+i] = _RGB(si); //preserve monochrome
			si+=3;
		}
	}
	if (get_qr(qr_CSET7_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+2)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(7)+i] = _RGB(si);
			tempgreypal[CSET(7)+i] = _RGB(si); //preserve monochrome
			si+=3;
		}
	}
	if (get_qr(qr_CSET8_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+3)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(8)+i] = _RGB(si);
			tempgreypal[CSET(8)+i] = _RGB(si); //preserve monochrome
			si+=3;
		}
	}
	
	if(!get_qr(qr_NOLEVEL3FIX) && level==3) {
		RAMpal[CSET(6)+2] = NESpal(0x37);
		tempgreypal[CSET(6)+2] = NESpal(0x37);
	}
		
	create_rgb_table(&rgb_table, RAMpal, NULL);
	create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
	memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
	
	for(int32_t q=0; q<PAL_SIZE; q++)
	{
		trans_table2.data[0][q] = q;
		trans_table2.data[q][q] = q;
	}
	
	//! We need to store the new palette into the monochrome scratch palette. 
	//memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
	//! Doing this is bad, because we are also copying over the sprite palettes.
	
	
	/* Old handling for monochrome, before tint features added -V
	if ( isMonochrome () ) {
	//memcpy(tempgreypal, RAMpal, PAL_SIZE*sizeof(RGB));
		
	//Refresh the monochrome palette to avoid gfx glitches from loading the lpal.  
	setMonochrome(false);
	setMonochrome(true);
	}*/
	
	if(isMonochrome()){
		if(lastMonoPreset){
			restoreMonoPreset();
		} else {
			setMonochrome(false);
			setMonochrome(true);
		}
	}
	
	if(isUserTinted()){
		restoreTint();
	}
	
	refreshpal=true;
}

void loadpalset(int32_t cset, int32_t dataset, bool update_tint)
{
	int32_t j = CSET(dataset) * 3;

	for (int32_t i = 0; i < 16; i++, j += 3)
	{
		// if ( isMonochrome() ) tempgreypal[CSET(2)+i] = _RGB(&colordata[j]); //Use monochrome sprites and Hero pal... 
		if (isMonochrome() || isUserTinted()) tempgreypal[CSET(cset) + i] = _RGB(&colordata[j]); //Use monochrome sprites and Hero pal... 
		else
			RAMpal[CSET(cset) + i] = _RGB(&colordata[j]);
	}

	if (update_tint){
		if (isMonochrome()) {
			if (lastMonoPreset) {
				restoreMonoPreset();
			}
			else {
				setMonochrome(false);
				setMonochrome(true);
			}
		}

		if (isUserTinted()) {
			restoreTint();
		}
	}
    
	//If writing cset 6 or 14, record which sprite csets are being referenced
	if(cset==6){
		if (!get_qr(qr_NOLEVEL3FIX) && DMaps[currdmap].color == 3) {
			RAMpal[CSET(6) + 2] = NESpal(0x37);
		}

		if (dataset >= poSPRITE255 && dataset < poSPRITE255 + pdSPRITE) 
			currspal6 = dataset - poSPRITE255;
		else
			currspal6 = -1;
	}
	if (cset == 14)
	{
		if (dataset >= poSPRITE255 && dataset < poSPRITE255 + pdSPRITE)
			currspal14 = dataset - poSPRITE255;
		else
			currspal14 = -1;
	}
	refreshpal=true;
}

void ringcolor(bool forceDefault)
{
    int32_t itemid = current_item_id(itype_ring);
    
    if (!forceDefault && itemid > -1)
        loadpalset(6,itemsbuf[itemid].misc1 ? pSprite(zc_min((pdSPRITE-1),itemsbuf[itemid].misc1)):6);
    else
        loadpalset(6,6);
    
    refreshpal=true;
}

void loadfadepal(int32_t dataset)
{
	byte *si = colordata + CSET(dataset)*3;
    
	for(int32_t i=0; i<pdFADE*16; i++)
	{
		if(isMonochrome() || isUserTinted())tempgreypal[CSET(2)+i] = _RGB(si);
		else RAMpal[CSET(2)+i] = _RGB(si);
		si+=3;
	}
    
	refreshpal=true;
	
	if(isMonochrome()){
		if(lastMonoPreset){
			restoreMonoPreset();
		} else {
			setMonochrome(false);
			setMonochrome(true);
		}
	}
    
	if(isUserTinted()){
		restoreTint();
	}
}

void interpolatedfade()
{
	int32_t dpos = 64;
	int32_t lpos = 32;
	int32_t last = CSET(5)-1;
	
	if(get_qr(qr_FADECS5))
	{
		last += 16;
		if (!get_qr(qr_CSET5_LEVEL)) loadpalset(5,5);
	}
	
	loadlvlpal(DMaps[currdmap].color);
	byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poFADE1)*3;
	
	for(int32_t i=0; i<16; i++)
	{
		int32_t light = si[0]+si[1]+si[2];
		si+=3;
		fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+i,CSET(2)+i);
	}
	
	fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
	if (get_qr(qr_FADECS1))
	{
		if (!get_qr(qr_CSET1_LEVEL)) loadpalset(1,1);
		fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(1),CSET(1)+15);
	}
	if (get_qr(qr_FADECS7))
	{
		if (!get_qr(qr_CSET7_LEVEL)) loadpalset(7,7);
		fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(7),CSET(7)+15);
	}
	if (get_qr(qr_FADECS8))
	{
		if (!get_qr(qr_CSET8_LEVEL)) loadpalset(8,8);
		fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(8),CSET(8)+15);
	}
	if (get_qr(qr_FADECS9))
	{
		fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(9),CSET(9)+15);
	}
	refreshpal=true;
}

void fade(int32_t level,bool blackall,bool fromblack)
{
	int32_t cx = fromblack ? 30 : 0;
	
	for(int32_t i=0; i<=30; i+=(get_qr(qr_FADE))?2:1)
	{
		if(get_qr(qr_FADE))
		{
			int32_t dpos = (cx<<6)/30;
			int32_t lpos = zc_min(dpos,blackall?64:32);
			int32_t last = CSET(5)-1;
			
			if(get_qr(qr_FADECS5))
			{
				last += 16;
				if (!get_qr(qr_CSET5_LEVEL)) loadpalset(5,5);
			}
			
			loadlvlpal(level);
			byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
			
			for(int32_t j=0; j<16; ++j)
			{
				int32_t light = si[0]+si[1]+si[2];
				si+=3;
				fade_interpolate(RAMpal,black_palette,RAMpal,light?lpos:dpos,CSET(2)+j,CSET(2)+j);
			}
			
			fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(3),last);
			if (get_qr(qr_FADECS1))
			{
				if (!get_qr(qr_CSET1_LEVEL)) loadpalset(1,1);
				fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(1),CSET(1)+15);
			}
			if (get_qr(qr_FADECS7))
			{
				if (!get_qr(qr_CSET7_LEVEL)) loadpalset(7,7);
				fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(7),CSET(7)+15);
			}
			if (get_qr(qr_FADECS8))
			{
				if (!get_qr(qr_CSET8_LEVEL)) loadpalset(8,8);
				fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(8),CSET(8)+15);
			}
			if (get_qr(qr_FADECS9))
			{
				fade_interpolate(RAMpal,black_palette,RAMpal,dpos,CSET(9),CSET(9)+15);
			}
			refreshpal=true;
		}
		else
		{
			switch(cx)
			{
			case 0:
				loadlvlpal(level);
				break;
				
			case 10:
				loadfadepal(level*pdLEVEL+poFADE1);
				break;
				
			case 20:
				loadfadepal(level*pdLEVEL+poFADE2);
				break;
				
			case 30:
				if(blackall)
				{
					for(int32_t j=0; j<pdFADE*16; j++)
						RAMpal[CSET(2)+j]=black_palette[0];
						
					refreshpal=true;
				}
				else
					loadfadepal(level*pdLEVEL+poFADE3);
					
				break;
			}
		}
		
		if(!get_qr(qr_NOLEVEL3FIX) && level==3)
			RAMpal[CSET(6)+2] = NESpal(0x37);
			
		//put_passive_subscr(framebuf,0,passive_subscreen_offset,false,false);
		advanceframe(true);
		
		if(Quit)
			break;
			
		fromblack ? --cx : ++cx;
		
		if(get_qr(qr_FADE))
		{
			fromblack ? --cx : ++cx;
		}
	}
	
	// Make sure the palette is set exactly right before returning...
	if(fromblack)
		loadlvlpal(level);
}


// false: change screen lighting to naturaldark
// true: lighten room
void lighting(bool existslight, bool setnaturaldark, int32_t specialstate)
{
	switch(specialstate){
		case pal_litOVERRIDE:
			stayLit=existslight;
			break;
		case pal_litRESET:
			stayLit=false;
			break;
		case pal_litSET:
			stayLit=true;
			break;
		case pal_litRESETONLY:
			stayLit=false;
			return;
	}
	if(stayLit)
	{
		existslight=true;
	}
    bool newstate = !existslight && (setnaturaldark ? ((TheMaps[currmap*MAPSCRS+currscr].flags&fDARK) != 0) : naturaldark);
    if(get_qr(qr_NEW_DARKROOM)) newstate = false;
    if(darkroom != newstate)
    {
		fade((Hero.getSpecialCave()>0) ? (Hero.getSpecialCave()>=GUYCAVE) ? 10 : 11 : DMaps[currdmap].color, false, darkroom);
        darkroom = newstate;
    }
    
    if(setnaturaldark)
        naturaldark = newstate;
}

// Only used during Insta-Warps
void lightingInstant()
{
	stayLit=false;
	bool newstate = ((TheMaps[currmap*MAPSCRS+currscr].flags&fDARK) != 0);
	if(get_qr(qr_NEW_DARKROOM)) newstate = false;
	if(darkroom != newstate)
	{
		int32_t level = (Hero.getSpecialCave()>0) ? (Hero.getSpecialCave()>=GUYCAVE) ? 10 : 11 : DMaps[currdmap].color;

		if(darkroom) // Old room dark, new room lit
		{
			loadlvlpal(level);
			
			if(get_qr(qr_FADECS5) && !get_qr(qr_CSET5_LEVEL)) loadpalset(5,5);
			if(get_qr(qr_FADECS1) && !get_qr(qr_CSET1_LEVEL)) loadpalset(1,1);
			if(get_qr(qr_FADECS7) && !get_qr(qr_CSET7_LEVEL)) loadpalset(7,7);
			if(get_qr(qr_FADECS8) && !get_qr(qr_CSET8_LEVEL)) loadpalset(8,8);
	
		}
		else // Old room lit, new room dark
		{
			if(get_qr(qr_FADE))
			{
				int32_t last = CSET(5)-1;
				int32_t light;
				
				if(get_qr(qr_FADECS5))
				{
					last += 16;
					if (!get_qr(qr_CSET5_LEVEL)) loadpalset(5,5);
				}
				
				byte *si = colordata + CSET(level*pdLEVEL+poFADE1)*3;
				
				for(int32_t j=0; j<16; ++j)
				{
					light = si[0]+si[1]+si[2];
					si+=3;
					fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+j,CSET(2)+j);
				}
				
				fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
				if (get_qr(qr_FADECS1))
				{
					if (!get_qr(qr_CSET1_LEVEL)) loadpalset(1,1);
					fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(1),CSET(1)+15);
				}
				if (get_qr(qr_FADECS7))
				{
					if (!get_qr(qr_CSET7_LEVEL)) loadpalset(7,7);
					fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(7),CSET(7)+15);
				}
				if (get_qr(qr_FADECS8))
				{
					if (!get_qr(qr_CSET8_LEVEL)) loadpalset(8,8);
					fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(8),CSET(8)+15);
				}
				if (get_qr(qr_FADECS9))
				{
					fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(9),CSET(9)+15);
				}
			}
			else // No interpolated fading
				loadfadepal(level*pdLEVEL+poFADE3);
		}
		
		if(!get_qr(qr_NOLEVEL3FIX) && level==3)
			RAMpal[CSET(6)+2] = NESpal(0x37);
			
		create_rgb_table(&rgb_table, RAMpal, NULL);
		create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
		memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
		
		for(int32_t q=0; q<PAL_SIZE; q++)
		{
			trans_table2.data[0][q] = q;
			trans_table2.data[q][q] = q;
		}
		
		darkroom = newstate;
	}
	
	naturaldark = newstate;
}

byte drycolors[11] = {0x12,0x11,0x22,0x21,0x31,0x32,0x33,0x35,0x34,0x36,0x37};

void dryuplake()
{
    if(whistleclk<0 || whistleclk>=88)
        return;
        
    if((++whistleclk)&7)
        return;
        
    if(whistleclk<88)
    {
        if(tmpscr->flags7 & fWHISTLEPAL)
        {
            if(!usingdrypal)
            {
                usingdrypal = true;
                olddrypal = RAMpal[CSET(3)+3];
            }
            
            RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
            refreshpal=true;
            
        }
    }
    else
    {
        if(tmpscr->flags & fWHISTLE)
        {
            if(hiddenstair(0,true))
                sfx(tmpscr->secretsfx);
        }
    }
}

void rehydratelake(bool instant)
{
    if(whistleclk==-1)
        return;
        
    if(instant && usingdrypal)
    {
        usingdrypal = false;
        RAMpal[CSET(3)+3] = olddrypal;
        refreshpal=true;
        return;
    }
    
    whistleclk&=0xF8;
    
    do
    {
        whistleclk-=8;
        
        if(usingdrypal)
        {
            RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
            refreshpal=true;
        }
        
        advanceframe(true);
        
        if(((whistleclk>>3)&3) == 1)
            for(int32_t i=0; i<4 && !Quit; i++)
                advanceframe(true);
    }
    while(whistleclk!=0 && !Quit);
    
    whistleclk=-1;
    
    if(usingdrypal)
    {
        usingdrypal = false;
        RAMpal[CSET(3)+3] = olddrypal;
        refreshpal=true;
    }
}

static int32_t palclk[3];
static int32_t palpos[3];

void reset_pal_cycling()
{
    for(int32_t i=0; i<3; i++)
        palclk[i]=palpos[i]=0;
}

void cycle_palette()
{
    if(!get_qr(qr_FADE) || darkroom)
        return;
        
    int32_t level = (Hero.getSpecialCave()==0) ? DMaps[currdmap].color : (Hero.getSpecialCave()<GUYCAVE ? 11 : 10);
    palcycle cycle_none[1][3];  //create a null palette cycle here. -Z
	memset(cycle_none, 0, sizeof(cycle_none)); 
    for(int32_t i=0; i<3; i++)
    {
        palcycle c = ( level < NUM_PAL_CYCLES ) ? QMisc.cycles[level][i] : cycle_none[0][i]; //Only 0 through 255 have valid data in 2.50.x. -Z
        
        if(c.count&0xF0)
        {
            if(++palclk[i] >= c.speed)
            {
                palclk[i]=0;
                
                if(++palpos[i] >= (c.count>>4))
                    palpos[i]=0;
                    
                byte *si = colordata + CSET(level*pdLEVEL+poFADE1+1+palpos[i])*3;
                si += (c.first&15)*3;
                
                for(int32_t col=c.first&15; col<=(c.count&15); col++)
                {
                    RAMpal[CSET(c.first>>4)+col] = _RGB(si);
                    si+=3;
                }
                
                refreshpal=true;
            }
        }
    }
    
    // No need to do handle refreshpal here; it's done in updatescr().
}

int32_t reverse_NESpal(RGB c)
{
    int32_t dist = 12000;
    int32_t index = 0;
    
    for(int32_t i = 0; (i < 64) && (dist != 0); i++)
    {
        int32_t r = (c.r - NESpal(i).r);
        int32_t g = (c.g - NESpal(i).g);
        int32_t b = (c.b - NESpal(i).b);
        int32_t d = r*r + g*g + b*b;
        
        if(d < dist)
        {
            dist = d;
            index = i;
        }
    }
    
    return index;
}
