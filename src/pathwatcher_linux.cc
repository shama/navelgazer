/*
Copyright (c) 2013 GitHub Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/inotify.h>
#include <linux/limits.h>
#include <unistd.h>

#include <algorithm>

#include "common.h"

static int g_inotify;

void PlatformInit() {
  g_inotify = inotify_init();
  if (g_inotify == -1) {
    return;
  }

  WakeupNewThread();
}

void PlatformThread() {
  // Needs to be large enough for sizeof(inotify_event) + strlen(filename).
  char buf[4096];

  while (true) {
    int size;
    do {
      size = read(g_inotify, buf, sizeof(buf));
    } while (size == -1 && errno == EINTR);

    if (size == -1) {
      break;
    } else if (size == 0) {
      break;
    }

    inotify_event* e;
    for (char* p = buf; p < buf + size; p += sizeof(*e) + e->len) {
      e = reinterpret_cast<inotify_event*>(p);

      int fd = e->wd;
      EVENT_TYPE type;
      std::vector<char> path;
      std::string newpath = e->name;

      if (e->mask & (IN_MOVED_TO)) {
        // A file has moved into a watched dir
        type = EVENT_LINUX_RENAMEIN;
        std::vector<char> new_path_charvect(newpath.begin(), newpath.end());
        PostEventAndWait(type, fd, path, new_path_charvect);
      }

      if (e->mask & (IN_MOVE_SELF)) {
        // Watched file has moved somewhere
        type = EVENT_LINUX_RENAMEOUT;
      } else if (e->mask & (IN_DELETE | IN_DELETE_SELF)) {
        type = EVENT_DELETE;
      } else if (e->mask & (IN_ATTRIB | IN_CREATE | IN_MODIFY)) {
        type = EVENT_CHANGE;
      } else {
        continue;
      }

      PostEventAndWait(type, fd, path);
    }
  }
}

WatcherHandle PlatformWatch(const char* path) {
  int fd = inotify_add_watch(g_inotify, path, IN_ATTRIB | IN_CREATE |
      IN_DELETE | IN_MODIFY | IN_MOVED_TO | IN_MOVE_SELF | IN_DELETE_SELF);
  return fd;
}

void PlatformUnwatch(WatcherHandle fd) {
  inotify_rm_watch(g_inotify, fd);
}

bool PlatformIsHandleValid(WatcherHandle handle) {
  return handle >= 0;
}

bool PlatformIsEMFILE(WatcherHandle handle) {
  return handle == -24;
}
