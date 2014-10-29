/* This is a generated file, don't edit */

#define NUM_APPLETS 237

const char applet_names[] ALIGN1 = ""
"[" "\0"
"[[" "\0"
"acpid" "\0"
"addgroup" "\0"
"adduser" "\0"
"adjtimex" "\0"
"ash" "\0"
"awk" "\0"
"base64" "\0"
"basename" "\0"
"beep" "\0"
"blkid" "\0"
"blockdev" "\0"
"bootchartd" "\0"
"cal" "\0"
"cat" "\0"
"catv" "\0"
"chgrp" "\0"
"chmod" "\0"
"chown" "\0"
"chpasswd" "\0"
"chpst" "\0"
"chroot" "\0"
"chrt" "\0"
"chvt" "\0"
"cksum" "\0"
"clear" "\0"
"cmp" "\0"
"comm" "\0"
"cp" "\0"
"cpio" "\0"
"cryptpw" "\0"
"cttyhack" "\0"
"cut" "\0"
"date" "\0"
"dd" "\0"
"deallocvt" "\0"
"delgroup" "\0"
"deluser" "\0"
"depmod" "\0"
"devmem" "\0"
"df" "\0"
"diff" "\0"
"dirname" "\0"
"dmesg" "\0"
"dos2unix" "\0"
"du" "\0"
"dumpkmap" "\0"
"echo" "\0"
"ed" "\0"
"egrep" "\0"
"eject" "\0"
"env" "\0"
"envdir" "\0"
"envuidgid" "\0"
"expand" "\0"
"expr" "\0"
"false" "\0"
"fbset" "\0"
"fbsplash" "\0"
"fdisk" "\0"
"fgconsole" "\0"
"fgrep" "\0"
"findfs" "\0"
"flock" "\0"
"fold" "\0"
"free" "\0"
"freeramdisk" "\0"
"fsck.minix" "\0"
"fstrim" "\0"
"fsync" "\0"
"fuser" "\0"
"getopt" "\0"
"getty" "\0"
"grep" "\0"
"groups" "\0"
"gunzip" "\0"
"gzip" "\0"
"halt" "\0"
"hd" "\0"
"hdparm" "\0"
"head" "\0"
"hexdump" "\0"
"hostid" "\0"
"hwclock" "\0"
"id" "\0"
"init" "\0"
"insmod" "\0"
"install" "\0"
"ionice" "\0"
"ipcrm" "\0"
"ipcs" "\0"
"kbd_mode" "\0"
"kill" "\0"
"killall" "\0"
"killall5" "\0"
"klogd" "\0"
"last" "\0"
"less" "\0"
"linux32" "\0"
"linux64" "\0"
"linuxrc" "\0"
"ln" "\0"
"loadfont" "\0"
"loadkmap" "\0"
"logger" "\0"
"login" "\0"
"logname" "\0"
"logread" "\0"
"losetup" "\0"
"ls" "\0"
"lsmod" "\0"
"lsof" "\0"
"lspci" "\0"
"lsusb" "\0"
"makedevs" "\0"
"md5sum" "\0"
"mdev" "\0"
"mesg" "\0"
"mkdir" "\0"
"mkfifo" "\0"
"mknod" "\0"
"mkpasswd" "\0"
"mkswap" "\0"
"mktemp" "\0"
"modinfo" "\0"
"modprobe" "\0"
"more" "\0"
"mount" "\0"
"mountpoint" "\0"
"mt" "\0"
"mv" "\0"
"nice" "\0"
"nohup" "\0"
"od" "\0"
"openvt" "\0"
"passwd" "\0"
"patch" "\0"
"pivot_root" "\0"
"poweroff" "\0"
"printenv" "\0"
"printf" "\0"
"ps" "\0"
"pstree" "\0"
"pwd" "\0"
"pwdx" "\0"
"rdate" "\0"
"rdev" "\0"
"readahead" "\0"
"readlink" "\0"
"readprofile" "\0"
"realpath" "\0"
"reboot" "\0"
"renice" "\0"
"reset" "\0"
"resize" "\0"
"rev" "\0"
"rm" "\0"
"rmdir" "\0"
"rmmod" "\0"
"rtcwake" "\0"
"runlevel" "\0"
"runsv" "\0"
"runsvdir" "\0"
"script" "\0"
"scriptreplay" "\0"
"sed" "\0"
"seq" "\0"
"setarch" "\0"
"setconsole" "\0"
"setfont" "\0"
"setkeycodes" "\0"
"setlogcons" "\0"
"setserial" "\0"
"setsid" "\0"
"setuidgid" "\0"
"sh" "\0"
"sha1sum" "\0"
"sha256sum" "\0"
"sha3sum" "\0"
"sha512sum" "\0"
"showkey" "\0"
"sleep" "\0"
"smemcap" "\0"
"softlimit" "\0"
"sort" "\0"
"split" "\0"
"stat" "\0"
"stty" "\0"
"su" "\0"
"sulogin" "\0"
"sum" "\0"
"sv" "\0"
"svlogd" "\0"
"swapoff" "\0"
"swapon" "\0"
"switch_root" "\0"
"sync" "\0"
"sysctl" "\0"
"syslogd" "\0"
"tac" "\0"
"tail" "\0"
"tar" "\0"
"tee" "\0"
"test" "\0"
"touch" "\0"
"tr" "\0"
"true" "\0"
"tty" "\0"
"ttysize" "\0"
"ubiattach" "\0"
"ubidetach" "\0"
"ubimkvol" "\0"
"ubirmvol" "\0"
"ubirsvol" "\0"
"ubiupdatevol" "\0"
"umount" "\0"
"uname" "\0"
"unexpand" "\0"
"uniq" "\0"
"unix2dos" "\0"
"uptime" "\0"
"users" "\0"
"usleep" "\0"
"uudecode" "\0"
"uuencode" "\0"
"vi" "\0"
"vlock" "\0"
"volname" "\0"
"wall" "\0"
"watchdog" "\0"
"wc" "\0"
"who" "\0"
"whoami" "\0"
"xargs" "\0"
"yes" "\0"
"zcat" "\0"
;

#ifndef SKIP_applet_main
int (*const applet_main[])(int argc, char **argv) = {
test_main,
test_main,
acpid_main,
addgroup_main,
adduser_main,
adjtimex_main,
ash_main,
awk_main,
base64_main,
basename_main,
beep_main,
blkid_main,
blockdev_main,
bootchartd_main,
cal_main,
cat_main,
catv_main,
chgrp_main,
chmod_main,
chown_main,
chpasswd_main,
chpst_main,
chroot_main,
chrt_main,
chvt_main,
cksum_main,
clear_main,
cmp_main,
comm_main,
cp_main,
cpio_main,
cryptpw_main,
cttyhack_main,
cut_main,
date_main,
dd_main,
deallocvt_main,
deluser_main,
deluser_main,
modprobe_main,
devmem_main,
df_main,
diff_main,
dirname_main,
dmesg_main,
dos2unix_main,
du_main,
dumpkmap_main,
echo_main,
ed_main,
grep_main,
eject_main,
env_main,
chpst_main,
chpst_main,
expand_main,
expr_main,
false_main,
fbset_main,
fbsplash_main,
fdisk_main,
fgconsole_main,
grep_main,
findfs_main,
flock_main,
fold_main,
free_main,
freeramdisk_main,
fsck_minix_main,
fstrim_main,
fsync_main,
fuser_main,
getopt_main,
getty_main,
grep_main,
id_main,
gunzip_main,
gzip_main,
halt_main,
hexdump_main,
hdparm_main,
head_main,
hexdump_main,
hostid_main,
hwclock_main,
id_main,
init_main,
modprobe_main,
install_main,
ionice_main,
ipcrm_main,
ipcs_main,
kbd_mode_main,
kill_main,
kill_main,
kill_main,
klogd_main,
last_main,
less_main,
setarch_main,
setarch_main,
init_main,
ln_main,
loadfont_main,
loadkmap_main,
logger_main,
login_main,
logname_main,
logread_main,
losetup_main,
ls_main,
modprobe_main,
lsof_main,
lspci_main,
lsusb_main,
makedevs_main,
md5_sha1_sum_main,
mdev_main,
mesg_main,
mkdir_main,
mkfifo_main,
mknod_main,
cryptpw_main,
mkswap_main,
mktemp_main,
modinfo_main,
modprobe_main,
more_main,
mount_main,
mountpoint_main,
mt_main,
mv_main,
nice_main,
nohup_main,
od_main,
openvt_main,
passwd_main,
patch_main,
pivot_root_main,
halt_main,
printenv_main,
printf_main,
ps_main,
pstree_main,
pwd_main,
pwdx_main,
rdate_main,
rdev_main,
readahead_main,
readlink_main,
readprofile_main,
realpath_main,
halt_main,
renice_main,
reset_main,
resize_main,
rev_main,
rm_main,
rmdir_main,
modprobe_main,
rtcwake_main,
runlevel_main,
runsv_main,
runsvdir_main,
script_main,
scriptreplay_main,
sed_main,
seq_main,
setarch_main,
setconsole_main,
setfont_main,
setkeycodes_main,
setlogcons_main,
setserial_main,
setsid_main,
chpst_main,
ash_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
showkey_main,
sleep_main,
smemcap_main,
chpst_main,
sort_main,
split_main,
stat_main,
stty_main,
su_main,
sulogin_main,
sum_main,
sv_main,
svlogd_main,
swap_on_off_main,
swap_on_off_main,
switch_root_main,
sync_main,
sysctl_main,
syslogd_main,
tac_main,
tail_main,
tar_main,
tee_main,
test_main,
touch_main,
tr_main,
true_main,
tty_main,
ttysize_main,
ubi_tools_main,
ubi_tools_main,
ubi_tools_main,
ubi_tools_main,
ubi_tools_main,
ubi_tools_main,
umount_main,
uname_main,
expand_main,
uniq_main,
dos2unix_main,
uptime_main,
who_main,
usleep_main,
uudecode_main,
uuencode_main,
vi_main,
vlock_main,
volname_main,
wall_main,
watchdog_main,
wc_main,
who_main,
whoami_main,
xargs_main,
yes_main,
gunzip_main,
};
#endif

const uint16_t applet_nameofs[] ALIGN2 = {
0x0000,
0x0002,
0x0005,
0x000b,
0x0014,
0x001c,
0x0025,
0x0029,
0x002d,
0x0034,
0x003d,
0x0042,
0x0048,
0x0051,
0x005c,
0x0060,
0x0064,
0x0069,
0x006f,
0x0075,
0x007b,
0x0084,
0x008a,
0x0091,
0x0096,
0x009b,
0x00a1,
0x00a7,
0x00ab,
0x00b0,
0x00b3,
0x00b8,
0x00c0,
0x00c9,
0x00cd,
0x00d2,
0x00d5,
0x00df,
0x00e8,
0x00f0,
0x00f7,
0x00fe,
0x0101,
0x0106,
0x010e,
0x0114,
0x011d,
0x0120,
0x0129,
0x012e,
0x0131,
0x0137,
0x013d,
0x0141,
0x0148,
0x0152,
0x0159,
0x015e,
0x0164,
0x016a,
0x0173,
0x0179,
0x0183,
0x4189,
0x0190,
0x0196,
0x019b,
0x01a0,
0x01ac,
0x01b7,
0x01be,
0x01c4,
0x01ca,
0x01d1,
0x01d7,
0x01dc,
0x01e3,
0x01ea,
0x01ef,
0x01f4,
0x01f7,
0x01fe,
0x0203,
0x020b,
0x0212,
0x021a,
0x021d,
0x0222,
0x0229,
0x0231,
0x0238,
0x023e,
0x0243,
0x024c,
0x0251,
0x0259,
0x0262,
0x0268,
0x026d,
0x0272,
0x027a,
0x0282,
0x028a,
0x028d,
0x0296,
0x029f,
0x82a6,
0x02ac,
0x02b4,
0x02bc,
0x02c4,
0x02c7,
0x02cd,
0x02d2,
0x02d8,
0x02de,
0x02e7,
0x02ee,
0x02f3,
0x02f8,
0x02fe,
0x0305,
0x030b,
0x0314,
0x031b,
0x0322,
0x032a,
0x0333,
0x0338,
0x033e,
0x0349,
0x034c,
0x034f,
0x0354,
0x035a,
0x035d,
0x8364,
0x036b,
0x0371,
0x037c,
0x0385,
0x038e,
0x0395,
0x0398,
0x039f,
0x03a3,
0x03a8,
0x03ae,
0x03b3,
0x03bd,
0x03c6,
0x03d2,
0x03db,
0x03e2,
0x03e9,
0x03ef,
0x03f6,
0x03fa,
0x03fd,
0x0403,
0x0409,
0x0411,
0x041a,
0x0420,
0x0429,
0x0430,
0x043d,
0x0441,
0x0445,
0x044d,
0x0458,
0x0460,
0x046c,
0x0477,
0x0481,
0x0488,
0x0492,
0x0495,
0x049d,
0x04a7,
0x04af,
0x04b9,
0x04c1,
0x04c7,
0x04cf,
0x04d9,
0x04de,
0x04e4,
0x04e9,
0x84ee,
0x04f1,
0x04f9,
0x04fd,
0x0500,
0x0507,
0x050f,
0x0516,
0x0522,
0x0527,
0x052e,
0x0536,
0x053a,
0x053f,
0x0543,
0x0547,
0x054c,
0x0552,
0x0555,
0x055a,
0x055e,
0x0566,
0x0570,
0x057a,
0x0583,
0x058c,
0x0595,
0x05a2,
0x05a9,
0x05af,
0x05b8,
0x05bd,
0x05c6,
0x05cd,
0x05d3,
0x05da,
0x05e3,
0x05ec,
0x85ef,
0x05f5,
0x85fd,
0x0602,
0x060b,
0x060e,
0x0612,
0x0619,
0x061f,
0x0623,
};

const uint8_t applet_install_loc[] ALIGN1 = {
0x33,
0x42,
0x24,
0x31,
0x31,
0x23,
0x22,
0x13,
0x11,
0x11,
0x34,
0x34,
0x33,
0x33,
0x13,
0x31,
0x31,
0x11,
0x43,
0x24,
0x12,
0x33,
0x31,
0x13,
0x11,
0x31,
0x33,
0x33,
0x13,
0x24,
0x32,
0x21,
0x33,
0x23,
0x22,
0x31,
0x21,
0x31,
0x11,
0x32,
0x32,
0x33,
0x32,
0x22,
0x13,
0x33,
0x11,
0x43,
0x32,
0x13,
0x01,
0x41,
0x32,
0x31,
0x22,
0x21,
0x33,
0x23,
0x23,
0x13,
0x13,
0x23,
0x21,
0x12,
0x11,
0x11,
0x31,
0x33,
0x33,
0x22,
0x31,
0x31,
0x31,
0x44,
0x34,
0x34,
0x32,
0x33,
0x11,
0x21,
0x24,
0x33,
0x13,
0x31,
0x21,
0x34,
0x14,
0x33,
0x31,
0x33,
0x33,
0x31,
0x33,
0x13,
0x11,
0x32,
0x43,
0x22,
0x12,
0x22,
0x33,
0x31,
0x13,
0x13,
0x33,
0x44,
0x44,
0x44,
0x11,
0x33,
0x33,
0x13,
0x33,
0x31,
0x33,
0x32,
0x33,
0x33,
0x01,
};
