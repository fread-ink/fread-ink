#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>	
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "mxcfb.h"

void usage(char* name, FILE *out) {
  fprintf(out, "\n");
  fprintf(out, "%s [on|off|update]\n", name);
  fprintf(out, "\n");
  fprintf(out, "  Operations:\n");
  fprintf(out, "     on or off: Enable or disable epdc auto-update.\n");
  fprintf(out, "        update: Update entire display once.\n");
  fprintf(out, "  Options:\n");
  fprintf(out, "     -d device: Device file to open (default: /dev/fb0)\n");
  fprintf(out, "            -m: Monochrome update (for update operation only)\n");
  fprintf(out, "\n");
}


int update(int fd, int monochrome) {
  int ret;

	struct mxcfb_update_data update_data = {
		0, 0, 800, 600,
		WAVEFORM_MODE_AUTO,
	};
	update_data.update_mode = UPDATE_MODE_PARTIAL;

  if(monochrome) {
    update_data.flags = EPDC_FLAG_FORCE_MONOCHROME;
  }
  
  ret = ioctl(fd, MXCFB_SET_UPDATE_SCHEME, UPDATE_SCHEME_QUEUE);
  if(ret == -1) {
    fprintf(stderr, "Could not set update scheme to UPDATE_SCHEME_QUEUE\n");
    return -1;
  }
  ret = ioctl(fd, MXCFB_SEND_UPDATE, &update_data);
  if(ret == -1) {
    fprintf(stderr, "Could not send full screen update\n");
    return -1;
  }
  
  return 0;
}

int set_autoupdate(int fd, int status) {
  int ret;
  int mode;

  if(status) {
    mode = AUTO_UPDATE_MODE_AUTOMATIC_MODE;
  } else {
    mode = AUTO_UPDATE_MODE_REGION_MODE;
  }

  ret = ioctl(fd, MXCFB_SET_AUTO_UPDATE_MODE, &mode);
  if(ret == -1) {
    fprintf(stderr, "Could not change auto-update mode\n");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {
  
  int c;
  int fd;
  int ret;
  char defaultDevPath[] = "/etc/fb0";
  char* devPath = NULL;
  char* cmd = NULL;
  int monochrome = 0;
  
  while((c = getopt(argc, argv, "d:m")) != -1) {
    switch(c) {
    case 'd':
      devPath = optarg;
      break;
      
    case 'm':
      monochrome = 1;
      break;
    }
  }
  
  if(!devPath) {
    devPath = defaultDevPath;
  }
  
  if(optind < argc) {
    cmd = argv[optind];
  }

  if(!cmd) {
    usage(argv[0], stderr);
    return 1;
  }

	fd = open(devPath, O_RDWR);
  if(fd < 0) {
    fprintf(stderr, "Could not open device: %s\n", devPath);
    return 1;
  }

  if(strcmp(cmd, "on") == 0) {
    ret = set_autoupdate(fd, 1);
  } else if(strcmp(cmd, "off") == 0) {
    ret = set_autoupdate(fd, 0);
  } else if(strcmp(cmd, "update") == 0) {
    ret = update(fd, monochrome);
  } else {
    usage(argv[0], stderr);
    return 1;    
  }

  close(fd);
  if(ret < 0) {
    return 1;
  }
  return 0;
}

