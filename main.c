//--------------------------------------------------------------
//File name:   main.c
//--------------------------------------------------------------
#include "launchelf.h"

extern u8 *iomanx_irx;
extern int size_iomanx_irx;
extern u8 *filexio_irx;
extern int size_filexio_irx;
extern u8 *ps2dev9_irx;
extern int size_ps2dev9_irx;
extern u8 *ps2ip_irx;
extern int size_ps2ip_irx;
extern u8 *ps2smap_irx;
extern int size_ps2smap_irx;
extern u8 *ps2host_irx;
extern int size_ps2host_irx;
extern u8 *ps2ftpd_irx;
extern int size_ps2ftpd_irx;
extern u8 *ps2atad_irx;
extern int size_ps2atad_irx;
extern u8 *ps2hdd_irx;
extern int size_ps2hdd_irx;
extern u8 *ps2fs_irx;
extern int size_ps2fs_irx;
extern u8 *poweroff_irx;
extern int size_poweroff_irx;
extern u8 *loader_elf;
extern int size_loader_elf;
extern u8 *ps2netfs_irx;
extern int size_ps2netfs_irx;
extern u8 *iopmod_irx;
extern int size_iopmod_irx;
extern u8 *usbd_irx;
extern int size_usbd_irx;
extern u8 *usb_mass_irx;
extern int size_usb_mass_irx;
extern u8 *cdvd_irx;
extern int size_cdvd_irx;

//#define DEBUG
#ifdef DEBUG
#define dbgprintf(args...) scr_printf(args)
#define dbginit_scr() init_scr()
#else
#define dbgprintf(args...) do { } while(0)
#define dbginit_scr() do { } while(0)
#endif

enum
{
	BUTTON,
	DPAD
};

void Reset();

int trayopen=FALSE;
int selected=0;
int timeout=0;
int mode=BUTTON;
char LaunchElfDir[MAX_PATH], mainMsg[MAX_PATH];
char CNF[MAX_PATH];
int numCNF=0;
int maxCNF;
int swapKeys;

#define IPCONF_MAX_LEN  (3*16)
char if_conf[IPCONF_MAX_LEN];
int if_conf_len;

char ip[16]      = "192.168.0.10";
char netmask[16] = "255.255.255.0";
char gw[16]      = "192.168.0.1";

char netConfig[IPCONF_MAX_LEN+64];	//Adjust size as needed

//State of module collections
int have_NetModules = 0;
int have_HDD_modules = 0;
//State of sbv_patches
int have_sbv_patches = 0;
//Old State of Checkable Modules (valid header)
int	old_sio2man  = 0;
int	old_mcman    = 0;
int	old_mcserv   = 0;
int	old_padman   = 0;
int old_fakehost = 0;
int old_poweroff = 0;
int	old_iomanx   = 0;
int	old_filexio  = 0;
int	old_ps2dev9  = 0;
int	old_ps2ip    = 0;
int	old_ps2atad  = 0;
int old_ps2hdd   = 0;
int old_ps2fs    = 0;
int old_ps2netfs = 0;
//State of Uncheckable Modules (invalid header)
int	have_cdvd     = 0;
int	have_usbd     = 0;
int	have_usb_mass = 0;
int	have_ps2smap  = 0;
int	have_ps2host  = 0;
int	have_ps2ftpd  = 0;
//State of Checkable Modules (valid header)
int have_urgent   = 0;	//flags presence of urgently needed modules
int	have_sio2man  = 0;
int	have_mcman    = 0;
int	have_mcserv   = 0;
int	have_padman   = 0;
int have_fakehost = 0;
int have_poweroff = 0;
int	have_iomanx   = 0;
int	have_filexio  = 0;
int	have_ps2dev9  = 0;
int	have_ps2ip    = 0;
int	have_ps2atad  = 0;
int have_ps2hdd   = 0;
int have_ps2fs    = 0;
int have_ps2netfs = 0;
//--------------------------------------------------------------
//executable code
//--------------------------------------------------------------
//Function to print a text row to the 'ito' screen
//------------------------------
void	PrintRow(int row, char *text_p)
{	int x = SCREEN_MARGIN + LINE_THICKNESS + FONT_WIDTH + 4;
	int y = (SCREEN_MARGIN + FONT_HEIGHT*2 + LINE_THICKNESS + 12 + FONT_HEIGHT*row)/2;

	printXY(text_p, x, y, setting->color[3], TRUE);
}
//------------------------------
//endfunc PrintRow
//--------------------------------------------------------------
//Function to show a screen with debugging info
//------------------------------
void ShowDebugScreen(void)
{	char TextRow[256];

	clrScr(setting->color[0]);
	sprintf(TextRow, "Urgent = %3d", have_urgent);
	PrintRow(0, TextRow);
	drawScr();
	waitPadReady(0, 0);
	while(1)
	{	if	(readpad())
			if	(new_pad & PAD_CROSS)
				break;
	}
}
//------------------------------
//endfunc ShowDebugScreen
//--------------------------------------------------------------
//Function to check for presence of key modules
//------------------------------
void	CheckModules(void)
{	smod_mod_info_t	mod_t;

	old_sio2man  = (have_sio2man = smod_get_mod_by_name(IOPMOD_NAME_SIO2MAN, &mod_t));
	old_mcman    = (have_mcman = smod_get_mod_by_name(IOPMOD_NAME_MCMAN, &mod_t));
	old_mcserv   = (have_mcserv = smod_get_mod_by_name(IOPMOD_NAME_MCSERV, &mod_t));
	old_padman   = (have_padman = smod_get_mod_by_name(IOPMOD_NAME_PADMAN, &mod_t));
	old_fakehost = (have_fakehost = smod_get_mod_by_name(IOPMOD_NAME_FAKEHOST, &mod_t));
	old_poweroff = (have_poweroff = smod_get_mod_by_name(IOPMOD_NAME_POWEROFF, &mod_t));
	old_iomanx   = (have_iomanx = smod_get_mod_by_name(IOPMOD_NAME_IOMANX, &mod_t));
	old_filexio  = (have_filexio = smod_get_mod_by_name(IOPMOD_NAME_FILEXIO, &mod_t));
	old_ps2dev9  = (have_ps2dev9 = smod_get_mod_by_name(IOPMOD_NAME_PS2DEV9, &mod_t));
	old_ps2ip    = (have_ps2ip = smod_get_mod_by_name(IOPMOD_NAME_PS2IP, &mod_t));
	old_ps2atad  = (have_ps2atad = smod_get_mod_by_name(IOPMOD_NAME_PS2ATAD, &mod_t));
	old_ps2hdd   = (have_ps2hdd = smod_get_mod_by_name(IOPMOD_NAME_PS2HDD, &mod_t));
	old_ps2fs    = (have_ps2fs = smod_get_mod_by_name(IOPMOD_NAME_PS2FS, &mod_t));
	old_ps2netfs = (have_ps2netfs= smod_get_mod_by_name(IOPMOD_NAME_PS2NETFS, &mod_t));
}
//------------------------------
//endfunc CheckModules
//--------------------------------------------------------------
// Parse network configuration from IPCONFIG.DAT
// Now completely rewritten to fix some problems
//------------------------------
static void getIpConfig(void)
{
	int fd;
	int i;
	int len;
	char c;
	char buf[IPCONF_MAX_LEN];

	fd = fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT", O_RDONLY);
	if (fd >= 0) 
	{	bzero(buf, IPCONF_MAX_LEN);
		len = fioRead(fd, buf, IPCONF_MAX_LEN - 1); //Save a byte for termination
		fioClose(fd);
	}

	if	((fd > 0) && (len > 0))
	{	buf[len] = '\0'; //Ensure string termination, regardless of file content
		for	(i=0; ((c = buf[i]) != '\0'); i++) //Clear out spaces and any CR/LF
			if	((c == ' ') || (c == '\r') || (c == '\n'))
				buf[i] = '\0';
		strncpy(ip, buf, 15);
		i = strlen(ip)+1;
		strncpy(netmask, buf+i, 15);
		i += strlen(netmask)+1;
		strncpy(gw, buf+i, 15);
	}

	bzero(if_conf, IPCONF_MAX_LEN);
	strncpy(if_conf, ip, 15);
	i = strlen(ip) + 1;
	strncpy(if_conf+i, netmask, 15);
	i += strlen(netmask) + 1;
	strncpy(if_conf+i, gw, 15);
	i += strlen(gw) + 1;
	if_conf_len = i;
	sprintf(netConfig, "Net config: %s %s %s", ip, netmask, gw);

}
//------------------------------
//endfunc getIpConfig
//--------------------------------------------------------------
int drawMainScreen(void)
{
	int nElfs=0;
	int i;
	int x, y;
	uint64 color;
	char c[MAX_PATH+8], f[MAX_PATH];
	char *p;
	
	strcpy(setting->dirElf[12], "CONFIG");
	if (maxCNF > 1){
		strcpy(setting->dirElf[13], "LOAD CONFIG--");
		strcpy(setting->dirElf[14], "LOAD CONFIG++");
	}
	
	clrScr(setting->color[0]);
	
	// �g�̒�
	x = SCREEN_MARGIN + LINE_THICKNESS + FONT_WIDTH;
	y = SCREEN_MARGIN + FONT_HEIGHT*2 + LINE_THICKNESS + 12;
	if(setting->dirElf[0][0]){
		if(mode==BUTTON)	sprintf(c, "TIMEOUT: %d", timeout/SCANRATE);
		else				sprintf(c, "TIMEOUT: ");
		printXY(c, x, y/2, setting->color[3], TRUE);
		y += FONT_HEIGHT*2;
	}
	for(i=0; i<15; i++){
		if(setting->dirElf[i][0]){
			switch(i){
			case 0:
				strcpy(c,"DEFAULT: ");
				break;
			case 1:
				strcpy(c,"     ��: ");
				break;
			case 2:
				strcpy(c,"     �~: ");
				break;
			case 3:
				strcpy(c,"     ��: ");
				break;
			case 4:
				strcpy(c,"     ��: ");
				break;
			case 5:
				strcpy(c,"     L1: ");
				break;
			case 6:
				strcpy(c,"     R1: ");
				break;
			case 7:
				strcpy(c,"     L2: ");
				break;
			case 8:
				strcpy(c,"     R2: ");
				break;
			case 9:
				strcpy(c,"     L3: ");
				break;
			case 10:
				strcpy(c,"     R3: ");
				break;
			case 11:
				strcpy(c,"  START: ");
				break;
			case 12:
				strcpy(c," SELECT: ");
				break;
			case 13:
				strcpy(c,"   LEFT: ");
				break;
			case 14:
				strcpy(c,"  RIGHT: ");
				break;
			}
			if(setting->filename){
				if((p=strrchr(setting->dirElf[i], '/')))
					strcpy(f, p+1);
				else
					strcpy(f, setting->dirElf[i]);
				if((p=strrchr(f, '.')))
					*p = 0;
			}else{
				strcpy(f, setting->dirElf[i]);
			}
			strcat(c, f);
			if(nElfs++==selected && mode==DPAD)
				color = setting->color[2];
			else
				color = setting->color[3];
			printXY(c, x, y/2, color, TRUE);
			y += FONT_HEIGHT;
		}
	}
	x = SCREEN_MARGIN;
	y = SCREEN_HEIGHT-SCREEN_MARGIN-FONT_HEIGHT;
	if(mode==BUTTON)	sprintf(c, "PUSH ANY BUTTON or D-PAD!");
	else{
		if(swapKeys) 
			sprintf(c, "�~:OK ��:Cancel");
		else
			sprintf(c, "��:OK �~:Cancel");
	}
	
	setScrTmp(mainMsg, c);
	drawScr();
	
	return nElfs;
}
//------------------------------
//endfunc drawMainScreen
//--------------------------------------------------------------
void delay(int count)
{
	int i;
	int ret;
	for (i  = 0; i < count; i++) {
	        ret = 0x01000000;
		while(ret--) asm("nop\nnop\nnop\nnop");
	}
}
//------------------------------
//endfunc delay
//--------------------------------------------------------------
void initsbv_patches(void)
{
	if(!have_sbv_patches)
	{	dbgprintf("Init MrBrown sbv_patches\n");
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		have_sbv_patches = 1;
	}
}
//------------------------------
//endfunc initsbv_patches
//--------------------------------------------------------------
void	load_iomanx(void)
{
	int ret;

	if	(!have_iomanx)
	{	SifExecModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL, &ret);
		have_iomanx = 1;
	}
}
//------------------------------
//endfunc load_iomanx
//--------------------------------------------------------------
void	load_filexio(void)
{
	int ret;

	if	(!have_filexio)
	{	SifExecModuleBuffer(&filexio_irx, size_filexio_irx, 0, NULL, &ret);
		have_filexio = 1;
	}
}
//------------------------------
//endfunc load_filexio
//--------------------------------------------------------------
void	load_ps2dev9(void)
{
	int ret;

	load_iomanx();
	if	(!have_ps2dev9)
	{	SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &ret);
		have_ps2dev9 = 1;
	}
}
//------------------------------
//endfunc load_ps2dev9
//--------------------------------------------------------------
void	load_ps2ip(void)
{
	int ret;

	load_ps2dev9();
	if	(!have_ps2ip)
	{	SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, &ret);
		have_ps2ip = 1;
	}
	if	(!have_ps2smap)
	{	SifExecModuleBuffer(&ps2smap_irx, size_ps2smap_irx,
		                    if_conf_len, &if_conf[0], &ret);
		have_ps2smap = 1;
	}
}
//------------------------------
//endfunc load_ps2ip
//--------------------------------------------------------------
void	load_ps2atad(void)
{
	int ret;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";

	load_ps2dev9();
	if	(!have_ps2atad)
	{	SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, &ret);
		have_ps2atad = 1;
	}
	if	(!have_ps2hdd)
	{	SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &ret);
		have_ps2hdd = 1;
	}
	if	(!have_ps2fs)
	{	SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, sizeof(pfsarg), pfsarg, &ret);
		have_ps2fs = 1;
	}
}
//------------------------------
//endfunc load_ps2atad
//--------------------------------------------------------------
void	load_ps2host(void)
{
	int ret;

	load_ps2ip();
	if	(!have_ps2host)
	{	SifExecModuleBuffer(&ps2host_irx, size_ps2host_irx, 0, NULL, &ret);
		have_ps2host = 1;
	}
}
//------------------------------
//endfunc load_ps2host
//--------------------------------------------------------------
void	load_ps2ftpd(void)
{
	int 	ret;
	int		arglen;
	char* arg_p;

	arg_p = "-anonymous";
	arglen = strlen(arg_p);

	load_ps2ip();
	if	(!have_ps2ftpd)
	{	SifExecModuleBuffer(&ps2ftpd_irx, size_ps2ftpd_irx, arglen, arg_p, &ret);
		have_ps2ftpd = 1;
	}
}
//------------------------------
//endfunc load_ps2ftpd
//--------------------------------------------------------------
void	load_ps2netfs(void)
{
	int ret;

	load_ps2ip();
	if	(!have_ps2netfs)
	{	SifExecModuleBuffer(&ps2netfs_irx, size_ps2netfs_irx, 0, NULL, &ret);
		have_ps2netfs = 1;
	}
}
//------------------------------
//endfunc load_ps2netfs
//--------------------------------------------------------------
void loadBasicModules(void)
{
	if	(!have_sio2man)
	{	SifLoadModule("rom0:SIO2MAN", 0, NULL);
		have_sio2man = 1;
	}
	if	(!have_mcman)
	{	SifLoadModule("rom0:MCMAN", 0, NULL);
		have_mcman = 1;
	}
	if	(!have_mcserv)
	{	SifLoadModule("rom0:MCSERV", 0, NULL);
		have_mcserv = 1;
	}
	if	(!have_padman)
	{	SifLoadModule("rom0:PADMAN", 0, NULL);
		have_padman = 1;
	}
}
//------------------------------
//endfunc loadBasicModules
//--------------------------------------------------------------
void loadCdModules(void)
{
	int ret;
	
	if	(!have_cdvd)
	{	SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, &ret);
		cdInit(CDVD_INIT_INIT);
		CDVD_Init();
		have_cdvd = 1;
	}
}
//------------------------------
//endfunc loadCdModules
//--------------------------------------------------------------
void loadUsbModules(void)
{
	int ret;
	
	if	(!have_usbd)
	{	SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
		have_usbd = 1;
	}
	if	(!have_usb_mass)	
	{	SifExecModuleBuffer(&usb_mass_irx, size_usb_mass_irx, 0, NULL, &ret);
		delay(3);
		ret = usb_mass_bindRpc();
		have_usb_mass = 1;
	}
}
//------------------------------
//endfunc loadUsbModules
//--------------------------------------------------------------
void poweroffHandler(int i)
{
	hddPowerOff();
}
//------------------------------
//endfunc poweroffHandler
//--------------------------------------------------------------
void loadHddModules(void)
{
	int ret;
	int i=0;
	
	if(!have_HDD_modules)
	{	drawMsg("Loading HDD Modules...");
		hddPreparePoweroff();
		hddSetUserPoweroffCallback((void *)poweroffHandler,(void *)i);
		if	(!have_poweroff)
		{	SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);
			have_poweroff = 1;
		}
		load_iomanx();
		load_filexio();
		load_ps2dev9();
		load_ps2atad(); //also loads ps2hdd & ps2fs
		have_HDD_modules = TRUE;
	}
}
//RA NB: I have removed the loading of ps2netfs from loadHddModules as it made
//no sense. That is a networking module, not an HDD module. It needs ps2ip+smap.
//I also consider it a wasted resource to always use up one server thread for a
//server having no convenient clients.
//------------------------------
//endfunc loadHddModules
//--------------------------------------------------------------
// Load Network modules by EP (modified by RA)
//------------------------------
void loadNetModules(void)
{
	if(!have_NetModules){
		loadHddModules();
		drawMsg("Loading FTP Modules...");
		
		// getIpConfig(); //RA NB: I always get that info, early in init
		// Also, my module checking makes some other tests redundant
		load_ps2netfs(); // loads ps2netfs from internal buffer
		load_ps2ftpd();  // loads ps2dftpd from internal buffer
		have_NetModules = 1;
	}
	strcpy(mainMsg, netConfig);
}
//------------------------------
//endfunc loadNetModules
//--------------------------------------------------------------
// SYSTEM.CNF�̓ǂݎ��
int ReadCNF(char *direlf)
{
	char *systemcnf;
	int fd;
	int size;
	int n;
	int i;
	
	/*
	loadCdModules();
	CDVD_FlushCache();
	CDVD_DiskReady(0);
	*/
	i = 0x10000;
	while(i--) asm("nop\nnop\nnop\nnop");
	fd = fioOpen("cdrom0:\\SYSTEM.CNF;1",1);
	if(fd>=0) {
		size = fioLseek(fd,0,SEEK_END);
		fioLseek(fd,0,SEEK_SET);
		systemcnf = (char*)malloc(size+1);
		fioRead(fd, systemcnf, size);
		systemcnf[size+1]=0;
		for(n=0; systemcnf[n]!=0; n++){
			if(!strncmp(&systemcnf[n], "BOOT2", 5)) {
				n+=5;
				break;
			}
		}
		while(systemcnf[n]!=0 && systemcnf[n]==' ') n++;
		if(systemcnf[n]!=0 ) n++; // salta '='
		while(systemcnf[n]!=0 && systemcnf[n]==' ') n++;
		if(systemcnf[n]==0){
			free(systemcnf);
			return 0;
		}
		
		for(i=0; systemcnf[n+i]!=0; i++) {
			direlf[i] = systemcnf[n+i];
			if(i>2)
				if(!strncmp(&direlf[i-1], ";1", 2)) {
					direlf[i+1]=0;
					break;
				}
		}
		fioClose(fd);
		free(systemcnf);
		return 1;
	}
	return 0;
}
//------------------------------
//endfunc ReadCNF
//--------------------------------------------------------------
// ELF�̃e�X�g�Ǝ��s
void RunElf(const char *path)
{
	char tmp[MAX_PATH];
	static char fullpath[MAX_PATH];
	static char party[40];
	char *p;
	
	if(path[0]==0) return;
	
	if(!strncmp(path, "hdd0:/", 6)){
		loadHddModules();
		sprintf(party, "hdd0:%s", path+6);
		p = strchr(party, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
	}else if(!strncmp(path, "mc", 2)){
		party[0] = 0;
		if(path[2]==':'){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkELFheader(fullpath)<0){
				fullpath[2]='1';
				if(checkELFheader(fullpath)<0){
					sprintf(mainMsg, "%s is Not Found.", path);
					return;
				}
			}
		} else {
			strcpy(fullpath, path);
			if(checkELFheader(fullpath)<0){
				sprintf(mainMsg, "%s is Not Found.", path);
				return;
			}
		}
	}else if(!strncmp(path, "mass", 4)){
		loadUsbModules();
		party[0] = 0;
		strcpy(fullpath, "mass:");
		strcat(fullpath, path+6);
		if(checkELFheader(fullpath)<0){
			sprintf(mainMsg, "%s is Not Found.", path);
			return;
		}
	}else if(!strncmp(path, "host:", 5)){
		initHOST();
		party[0] = 0;
		strcpy(fullpath, "host:");
		strcat(fullpath, path+6);
		makeHostPath(fullpath, fullpath);
		if(checkELFheader(fullpath)<0){
			sprintf(mainMsg, "%s is Not Found.", path);
			return;
		}
	}else if(!stricmp(path, "MISC/PS2Disc")){
		drawMsg("Reading SYSTEM.CNF...");
		strcpy(mainMsg, "Failed");
		party[0]=0;
		trayopen=FALSE;
		if(!ReadCNF(fullpath)) return;
		//strcpy(mainMsg, "Success!"); return;
	}else if(!stricmp(path, "MISC/FileBrowser")){
		mainMsg[0] = 0;
		tmp[0] = 0;
		LastDir[0] = 0;
		getFilePath(tmp, FALSE);
		if(tmp[0]) RunElf(tmp);
		else return;
	}else if(!stricmp(path, "MISC/PS2Browser")){
		party[0]=0;
		strcpy(fullpath,"rom0:OSDSYS");
	}else if(!stricmp(path, "MISC/PS2Net")){
		mainMsg[0] = 0;
		loadNetModules();
		return;
//Next two clauses are only for debugging
/*
	}else if(!stricmp(path, "MISC/IOP Reset")){
		mainMsg[0] = 0;
		Reset();
		if(!strncmp(LaunchElfDir, "mass:", 5))
			loadUsbModules();
		padReset();
		setupPad();
		return;
	}else if(!stricmp(path, "MISC/Debug Screen")){
		mainMsg[0] = 0;
		ShowDebugScreen();
		return;
*/
//end of two clauses used only for debugging
	}else if(!strncmp(path, "cdfs", 4)){
		party[0] = 0;
		strcpy(fullpath, path);
		CDVD_FlushCache();
		CDVD_DiskReady(0);
	}else if(!strncmp(path, "rom", 3)){
		party[0] = 0;
		strcpy(fullpath, path);
	}
	
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	free(setting);
	free(elisaFnt);
	padPortClose(0,0);
	RunLoaderElf(fullpath, party);
}
//------------------------------
//endfunc RunElf
//--------------------------------------------------------------
void RunSelectedElf(void)
{
	int n=0;
	int i;
	
	for(i=0; i<12; i++){
		if(setting->dirElf[i][0] && n++==selected){
			RunElf(setting->dirElf[i]);
			break;
		}
	}
}
//------------------------------
//endfunc RunSelectedElf
//--------------------------------------------------------------
// Config Cycle Left  (--) by EP
void decConfig()
{
	if (numCNF > 0)
		numCNF--;
	else
		numCNF = maxCNF-1;
	
	if (numCNF == 0)
		strcpy(CNF, "LAUNCHELF.CNF");
	else
		sprintf(CNF, "LAUNCHELF%i.CNF", numCNF);
	loadConfig(mainMsg, CNF);
	
	timeout = (setting->timeout+1)*SCANRATE;
	if(setting->discControl)
		loadCdModules();
}
//------------------------------
//endfunc decConfig
//--------------------------------------------------------------
// Config Cycle Right (++) by EP
void incConfig()
{
	if (numCNF < maxCNF-1)
		numCNF++;
	else
		numCNF = 0;
	
	if (numCNF == 0)
		strcpy(CNF, "LAUNCHELF.CNF");
	else
		sprintf(CNF, "LAUNCHELF%i.CNF", numCNF);
	loadConfig(mainMsg, CNF);
	
	timeout = (setting->timeout+1)*SCANRATE;
	if(setting->discControl)
		loadCdModules();
}
//------------------------------
//endfunc incConfig
//--------------------------------------------------------------
// reboot IOP (original source by Hermes in BOOT.c - cogswaploader)
void Reset()
{
	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
	while(SifIopSync());
	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();

	SifInitRpc(0);
	FlushCache(0);
	FlushCache(2);

	have_cdvd     = 0;
	have_usbd     = 0;
	have_usb_mass = 0;
	have_ps2smap  = 0;
	have_ps2host  = 0;
	have_ps2ftpd  = 0;
	have_NetModules = 0;
	have_HDD_modules = 0;
	have_sbv_patches = 0;

	CheckModules();
	loadBasicModules();
	mcInit(MC_TYPE_RESET);
	mcInit(MC_TYPE_MC);
}
//------------------------------
//endfunc Reset
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *p;
	int nElfs;
	CdvdDiscType_t cdmode;
	int hdd_booted = 0;
	int host_or_hdd_booted = 0;
	int mass_booted = 0;
	int mc_booted = 0;
	int cdvd_booted = 0;
	int	host_booted = 0;

	SifInitRpc(0);
	while(SifIopSync());
	CheckModules();
	loadBasicModules();
	mcInit(MC_TYPE_MC);

	if	((argc > 0) && argv[0])
	{	if	(!strncmp(argv[0], "mass", 4))
			mass_booted = 1;
		else if	(!strncmp(argv[0], "mc", 2))
			mc_booted = 1;
		else if	(!strncmp(argv[0], "cd", 2))
			cdvd_booted = 1;
		else if	((!strncmp(argv[0], "hdd", 3)) || (!strncmp(argv[0], "pfs", 3)))
			hdd_booted = 1;
		else if	((!strncmp(argv[0], "host", 4)))
		{	host_or_hdd_booted = 1;
			if	(have_fakehost)
				hdd_booted = 1;
			else
			  host_booted = 1;
		}
	}

	if	(host_booted)	//Fix untestable modules for host booting
	{	have_ps2smap = 1;
		have_ps2host	= 1;
	}

	if	(mass_booted)	//Fix untestable module for USB_mass booting
	{	have_usbd = 1;
		//have_usb_mass = 1;
	}

	strcpy(LaunchElfDir, argv[0]);
	if	(	((p=strrchr(LaunchElfDir, '/'))==NULL)
			&&((p=strrchr(LaunchElfDir, '\\'))==NULL)
			)	p=strrchr(LaunchElfDir, ':');
	if	(p!=NULL)
		*(p+1)=0;

	LastDir[0] = 0;

	loadConfig(mainMsg, strcpy(CNF, "LAUNCHELF.CNF"));
	maxCNF = setting->numCNF;
	swapKeys = setting->swapKeys;
	if(setting->resetIOP)
	{	Reset();
		if(!strncmp(LaunchElfDir, "mass:", 5))
		{	initsbv_patches();
			loadUsbModules();
		}
	}
	getIpConfig();
	setupPad();
	initsbv_patches();


	if(setting->discControl)
		loadCdModules();

//Last chance to look at bootup screen, so allow braking here
/*
	if(readpad() && (new_pad && PAD_UP))
	{ scr_printf("________ Boot paused. Press 'Circle' to continue.\n");
		while(1)
		{	if(new_pad & PAD_CIRCLE)
				break;
			while(!readpad());
		}
	}
*/
	setupito();
	
	timeout = (setting->timeout+1)*SCANRATE;
	while(1){
		if(setting->discControl){
			CDVD_Stop();
			cdmode = cdGetDiscType();
			if(cdmode==CDVD_TYPE_NODISK){
				trayopen = TRUE;
				strcpy(mainMsg, "No Disc");
			}else if(cdmode>=0x01 && cdmode<=0x04){
				strcpy(mainMsg, "Detecting Disc");
			}else if(trayopen==TRUE){
				trayopen=FALSE;
				strcpy(mainMsg, "Stop Disc");
			}
		}
		
		timeout--;
		nElfs = drawMainScreen();
		
		waitPadReady(0,0);
		if(readpad()){
			switch(mode){
			case BUTTON:
				if(new_pad & PAD_CIRCLE){
					RunElf(setting->dirElf[1]);
				}else if(new_pad & PAD_CROSS) 
					RunElf(setting->dirElf[2]);
				else if(new_pad & PAD_SQUARE) 
					RunElf(setting->dirElf[3]);
				else if(new_pad & PAD_TRIANGLE) 
					RunElf(setting->dirElf[4]);
				else if(new_pad & PAD_L1) 
					RunElf(setting->dirElf[5]);
				else if(new_pad & PAD_R1) 
					RunElf(setting->dirElf[6]);
				else if(new_pad & PAD_L2) 
					RunElf(setting->dirElf[7]);
				else if(new_pad & PAD_R2) 
					RunElf(setting->dirElf[8]);
				else if(new_pad & PAD_L3)
					RunElf(setting->dirElf[9]);
				else if(new_pad & PAD_R3)
					RunElf(setting->dirElf[10]);
				else if(new_pad & PAD_START)
					RunElf(setting->dirElf[11]);
				else if(new_pad & PAD_SELECT){
					config(mainMsg, CNF);
					timeout = (setting->timeout+1)*SCANRATE;
					if(setting->discControl)
						loadCdModules();
				}else if(maxCNF > 1 && new_pad & PAD_LEFT){
					decConfig();
				}else if(maxCNF > 1 && new_pad & PAD_RIGHT){
					incConfig();
				}else if(new_pad & PAD_UP || new_pad & PAD_DOWN){
					selected=0;
					mode=DPAD;
				}
				break;
			
			case DPAD:
				if(new_pad & PAD_UP){
					selected--;
					if(selected<0)
						selected=nElfs-1;
				}else if(new_pad & PAD_DOWN){
					selected++;
					if(selected>=nElfs)
						selected=0;
				}else if((!swapKeys && new_pad & PAD_CROSS)
				      || (swapKeys && new_pad & PAD_CIRCLE) ){
					mode=BUTTON;
					timeout = (setting->timeout+1)*SCANRATE;
				}else if((swapKeys && new_pad & PAD_CROSS)
				      || (!swapKeys && new_pad & PAD_CIRCLE) ){
					if(maxCNF > 1 && selected==nElfs-1){
						mode=BUTTON;
						incConfig();
					}else if(maxCNF > 1 && selected==nElfs-2){
						mode=BUTTON;
						decConfig();
					}else if((maxCNF > 1 && selected==nElfs-3) || (selected==nElfs-1)){
						mode=BUTTON;
						config(mainMsg, CNF);
						timeout = (setting->timeout+1)*SCANRATE;
						if(setting->discControl)
							loadCdModules();
					}else
						RunSelectedElf();
				}
				break;
			}
		}
		if(timeout/SCANRATE==0 && setting->dirElf[0][0] && mode==BUTTON){
			RunElf(setting->dirElf[0]);
			timeout = (setting->timeout+1)*SCANRATE;
		}
	}
}
//------------------------------
//endfunc main
//--------------------------------------------------------------
//End of file: main.c
//--------------------------------------------------------------