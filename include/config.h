#ifndef __CONFIG_H
#define __CONFIG_H

/* frame configs */
#define NO_GLOBAL_ERROR_BUFFER
#define NO_SILENT_ERROR

/* query configs */
#define ANIDB_ENDPOINT  "api.anidb.net"
#define ANIDB_PORT      "9000"
#define ANIDB_PROTOVER  "3"
#define ANIDB_NREAD     1024
#define ANIDB_AUTHFMT   "AUTH user=%s&pass=%s&clientver=%s&client=%s&protover="ANIDB_PROTOVER
#define ANIDB_ANAMEFMT  "ANIME s=%s&aname=%s&amask=%s"
#define ANIDB_ANIIDFMT  "ANIME s=%s&aid=%d&amask=%s"
#define ANIDB_NSESSION  5
#define MAX_SESSION_LEN 32

#endif