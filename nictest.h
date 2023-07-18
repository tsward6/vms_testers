

/*
* 
* Tim Ward
* 01/12/2022
* VMS Software, Inc.
* NICTEST.h 
*
* NICTEST 1.0
* This version of NICTEST only supports simple link layer
* LAN device testing. 
*/

#ifndef NT
#define NT

int input_loop();


#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef uint64
typedef unsigned long uint64;
#endif

/* frump LANCP.H */
/* Device name constants */
#define MED			0			/* Medium is 1st character */
#define CNTR			1			/* Controller is 2nd character */
#define CNUM			2			/* Controller number is 3rd char */
#define UNIT			3			/* Unit is 4th character */

typedef struct dev_item
{
	uint16 bufflen;
	uint16 itemcode;
	uint32 buffaddr;
	uint32 retlenaddr;
} DEV_ITEM;

#ifndef NODENAME_LEN
#define NODENAME_LEN		64			/* Length of node name field */
#endif


#ifndef DC$_SCOM            
#define DC$_SCOM 32         /* SYNCHRONOUS COMMUNICATIONS DEVICES */
#endif

#ifndef DVS$_DEVCLASS
#define DVS$_DEVCLASS 1     /* Device class - VALUE - 4 bytes (only one used) */
#endif

#define DEVNAME_LEN		8			/* Length of device name field */

#ifndef NMA$C_PCLI_LINKSTATE
#define NMA$C_PCLI_LINKSTATE	2835
#endif

//#include "lancp_lite.h"

//--includes------------------
#include <stdio.h>
#include <stdlib.h>
#include "nmadef.h"
#include <arpa/inet.h>
#include <descrip>
#include <devdef>
#include <dvidef>
#include <iodef>
#include <ssdef>
#include <syidef.h>
#include <ctype.h>
#include <lib$routines>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <starlet>
#include <stsdef>
//#include <semaphore.h>
#include <pthread.h>
//#include <time.h>

//#include "parm_defs.h"




#define $FAIL(status)		(((status) & STS$M_SUCCESS) != SS$_NORMAL)
#define $SUCCESS(status)	(((status) & STS$M_SUCCESS) == SS$_NORMAL)

//#define EXIT_PROGRAM $SUCCESS(status)

/* IOSB processes IO requests //
 struct _iosb { /* IOSB structure /
    short w_err; // completion status 
    short w_xfer_size; // transfer size
    short w_addl; // additional staus
    short w_misc; // misc.
    unsigned short status;
}; // qio_iosb_arr[N_MAX_ADAPTERS];
*/

#pragma nomember_alignment

struct iosb						/* IOSB structure */
{
	short w_err;					/* Completion status */
	short w_xfer_size;				/* Transfer size */
	short w_addl;					/* Additional status */
	short w_misc;					/* Miscellaneous */
};

#define STR_MIN_MATCH 2

#define BUFFER_LEN 32 * 1024

// variable size upon return (give maximum size) will return var num bytes
typedef __unaligned short * UNALIGNED_SHORTP;
typedef __unaligned int * UNALIGNED_INTP;

#define N_MAX_DEVS 100
    
#define MIN_PACKET_SIZE 46    
#define MIN_IPV4_PACKET_SIZE 64
#define IPV4_HEADER_LEN 20
#define MIN_IPV4_PACKET_OPT_SIZE MIN_IPV4_PACKET_SIZE + (sizeof(int) * 9)
#define MAX_PACKET_SIZE 9000


unsigned char MCA_ADDRS[][6] = { 
                            {0xAB, 0x00, 0x01, 0x00, 0x00, 0x01}, 
                            {0xAB, 0x00, 0x02, 0x00, 0x00, 0x02}, 
                            {0xAB, 0x00, 0x03, 0x00, 0x00, 0x03}, 
                            {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
                            {0xAB, 0x00, 0x04, 0x00, 0x00, 0x04}, 
                            {0xAB, 0x00, 0x05, 0x00, 0x00, 0x05}, 
                            {0xAB, 0x00, 0x06, 0x00, 0x00, 0x06}, 
                            {0xAB, 0x00, 0x07, 0x00, 0x00, 0x07}, 
                            {0xAB, 0x00, 0x08, 0x00, 0x00, 0x08}, 
                            {0xAB, 0x00, 0x09, 0x00, 0x00, 0x09}, 
                            {0xAB, 0x00, 0x0A, 0x00, 0x00, 0x0A}, 
                            {0xAB, 0x00, 0x0B, 0x00, 0x00, 0x0B}, 
                            {0xAB, 0x00, 0x0C, 0x00, 0x00, 0x0C}, 
                            {0xAB, 0x00, 0x0D, 0x00, 0x00, 0x0D}, 
                            {0xAB, 0x00, 0x0E, 0x00, 0x00, 0x0E}, 
                            {0xAB, 0x00, 0x0F, 0x00, 0x00, 0x0F}, 
                            {0xAB, 0x00, 0x10, 0x00, 0x00, 0x10}, 
                            {0xAB, 0x00, 0x11, 0x00, 0x00, 0x11}, 
                            {0xAB, 0x00, 0x12, 0x00, 0x00, 0x12}, 
                            {0xAB, 0x00, 0x13, 0x00, 0x00, 0x13}
             };
#define DEF_MCA MCA_ADDRS[0]
#define N_MCA_ADDRS sizeof(MCA_ADDRS) / sizeof(MCA_ADDRS[0])
unsigned char * CUR_MCA = DEF_MCA;



#define IPV4_FRAME_PCL (unsigned short)0x0800
#define DEF_ETH_FRAME_PCL (unsigned short)0x0801
//#define PCLI_BUS_VAL 9018

int n_sys_dev_names;
char sys_dev_names[100][5];

/* for startup */
struct setparmdsc {
    int parm_len;
    void *parm_buffer;
};

typedef struct {

    char name[100];
    unsigned short channel;
    int id;
    unsigned char hwa[6];
	int line_speed;
	int link_state;
    //int running;
    unsigned short cur_fpcl;

} nic;

enum target_type {
    SYS_TARTYPE, MCA_TARTYPE, USR_ADDED_TARTYPE       
};


#define N_MAX_TARGETS 100
struct target{
    char name[100];
    unsigned char mac[6];
    int type;

} targets[N_MAX_TARGETS];
int n_targets;





struct packet_data_info {
    unsigned char *packet_data;
    nic *n;
    unsigned int packet_pcl;
    int  packet_size;
    char *content_str;
};

/* IOSB processes IO requests */
 struct _iosb { /* IOSB structure */
    short w_err; // completion status 
    short w_xfer_size; // transfer size
    short w_addl; // additional staus
    short w_misc; // misc.
    unsigned short status;
} qio_iosb_arr[N_MAX_DEVS];

/* assignable NICS */
nic * nics[N_MAX_DEVS];
int n_nics;

 struct dev {
    short w_len;
    short w_info;
    char *a_str;
};

struct cmd {
    char name[100];
    struct qual {
        char name[100];
        char *vals[100];
        //void *vals[100];
        int n_vals;
    } quals[100];
    int n_quals;
}; 

const char *il_cmds[] = { "exit", "quit", "help", "echo", "performance",
                                     "show", "send", "test", "burst", "receive", "add", "remove",
                                     "stop" }; 

/* string comparison helper routines */
//int streq(char *full_str, char *str) { return strcmp(full_str, str) == 1; }
//int strneq(char *full_str, char *str) { return !streq(full_str, str); }

#define CMD_STATUS_VALID  1
#define CMD_STATUS_INVALID  0
#define CMD_STATUS_AMBIG  2

#define INVALID_INPUT SS$_ABORT
//#define UNIMPLEMENTED SS$_ABORT

#define PCLI_BUS_VAL 9018



/* P5 receive header buffer */
struct p5_param { 
    unsigned char da[6]; //destination address
    unsigned char sa[6]; // source address
    char misc[20]; // for token ring, etc...
};
/* IOSB processes IO requests */

#define PACKET_PCL_TYPE_LB 0x01
//#define PACKET_PCL_TYPE_LB_TS 0x11


//#define GEN_PCL_TYPE_USER_NO_LB     0x00
//#define GEN_PCL_TYPE_USER_LB        (GEN_PCL_TYPE_USER_NO_LB | PACKET_PCL_TYPE_LB)
//#define GEN_PCL_TYPE_USER           0x00
//#define GEN_PCL_TYPE_USER_LB        (GEN_PCL_TYPE_USER | PACKET_PCL_TYPE_LB)


#define GEN_HEADER_BYTES    0xD0C1B200
//#define TCP_PPCL            0X00000006
//#define UDP_PPCL            0X00000011
#define TCP_PPCL            0X00000600
#define UDP_PPCL            0X00001100
#define GEN_PPCL            GEN_HEADER_BYTES


enum {
    CLUSTER_MODE, ADMIN_MODE, JUNKIE_MODE
} PROG_MODES;
int PROG_MODE;

pthread_t rec_threads[N_MAX_DEVS];
//pthread_t rcv_watcher;
int rthreads_inuse[N_MAX_DEVS] = { 0 };
//int rthreads_tojoin[N_MAX_DEVS] = { 0 };

pthread_mutex_t rtm; //, rtjm; //, exm;
pthread_cond_t rtcv;

struct rt_info {
            nic *n;
            int dur;
            int prm;
            int n_mca;
            char reply_msg[10000];
        };

/* program routines */
void print_mac(unsigned char *mac);
void print_mac_nonl(unsigned char *mac);
void print_time_now();
void print_program_header();
void print_dev_names();
void print_nic(nic *n);
void print_assignable_nics();
int str_mmatch(char *full_str, char *s);
int streq(char *full_str, char *s);
int strneq(char *full_str, char *s);
int is_space(char c);
int modify_input(char *input, char *mod_input);
int input_loop();
int get_assignable_nics();
int get_dev_names();

void show_buffer_bytes(char *description, unsigned char *bytes, int nbytes);


#endif