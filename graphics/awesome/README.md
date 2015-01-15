
# Implementing e-ink display updates

Since it is probably not a good idea to enable automatic updating, we need ways for the e-ink display to update when graphics change.

Assuming that the mxcfb driver is being used alone, the relevant ioctl calls are in mxcfb.h and the especially relevant ones are here:

```
#define MXCFB_SET_AUTO_UPDATE_MODE	_IOW('F', 0x2D, __u32)
#define MXCFB_SET_UPDATE_SCHEME		_IOW('F', 0x32, __u32)
#define MXCFB_SEND_UPDATE		_IOW('F', 0x2E, struct mxcfb_update_data)
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE	_IOW('F', 0x2F, __u32)
```

Example code that uses these functions is available here:

ttps://github.com/CrazyCoder/coolreader-kindle-qt/blob/master/drivers/QKindleFb/qkindlefb.cpp

## Display updates from X

The amazon version of awesome adds a bunch of functionality that extends the lua API to allow receiving xdamage events.

## Display updates from web page

Bind event handler on "new element added to page" which binds an onload handler that uses Element.getBoundingClientRect() to send events to the node.js app which then triggers screen updates to the proper screen areas using ioctls. We should probably also bind the screen updater to onchange events.

### Notes on midori and sokoke

Midori uses JavasScriptCore, the webkit built-in javascript interpreter. It uses it via a small midori-specific library called sokoke. Midori extensions are generally written in vala. Examples of extension are available in the extensions/ dir of midori. The vala api for extensions is available in midori/midori.vapi and it looks like it's not possible to extend sokoke/JavaScriptCore using this api.

Here's some info on extending JavaScriptCore:

  http://rvr.typepad.com/wind/2011/10/webkit-extending-javascript-1.html

and the PhoneGap SDK apparently extends JavaScriptCore to give access to low-level features from javascript.

# Development

## Requirements

sudo aptitude install xserver-xephyr awesome midori

## Running

./xephyr.sh
