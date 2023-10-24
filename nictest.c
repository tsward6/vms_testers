
#include "NICTEST.h"

/*
* 
* Tim Ward
* 01/12/2022
* VMS Software, Inc.
* NICTEST.c
*
* NICTEST 1.0
* This version of NICTEST only supports simple link layer
* LAN device testing. 
*/



int get_mac(unsigned char *mac, char * target_str)
{
    
    const int MAC_ADDR_LEN = 6;
    int status;
    
    memset(mac, 0, MAC_ADDR_LEN);

    /* assume invalid input and parse for validity */
    status = INVALID_INPUT;
    if(strlen(target_str) == 17) {

        if(target_str[2] == '-' && target_str[5] == '-' && target_str[8] == '-' &&
           target_str[11] == '-' && target_str[14] == '-') {
            
            int tmp_offset = 0;
            int n_segs = 0;
            for(int i = 0; i < MAC_ADDR_LEN; i++) {
                char *tstr_ptr;
                if(i == 0) 
                    tstr_ptr = target_str;
                else 
                    tstr_ptr = target_str + ((i*2) + tmp_offset++ + 1);
                if(isxdigit(tstr_ptr[0]) && isxdigit(tstr_ptr[1])) {
                    char tmp_str[3];
                    memset(tmp_str, 0, 3);
                    tmp_str[0] = tstr_ptr[0];
                    tmp_str[1] = tstr_ptr[1];
                    int tmp_int;
                    sscanf(tmp_str,"%02x",&tmp_int);
                    mac[n_segs++] = (unsigned char)(tmp_int & 0xff);
                } else 
                    break;
            }
            if(n_segs == MAC_ADDR_LEN) 
                status = SS$_NORMAL;
            else 
                memset(mac, 0, MAC_ADDR_LEN);   
        }
    }

    return status;

}

int assign_nic(nic *n)
{
    struct dev {
        short w_len;
        short w_info;
        char *a_str;
    }d;
    d.w_len = 4;
    d.w_info = 0;
    d.a_str = n->name; 
    //--assign a channel (open a port) 
    int status = sys$assign(
        &d, // devname - device number 
        &n->channel,   // chan - channel number 
        0,       // access mode(0=kernal,...)
        0        // mailbox name(0=default,...)
    ); 
    if($FAIL(status)) {
        //print_status(status);
        printf("Assigning channel %x for device %s FAILED\n", n->channel, n->name);
        n->channel = 0;
    } 
    //printf("8+++++~> CHANNELING BONERS: %s %d %x\n", n->name, n->channel, status);
    return status;
}

int deassign_nic(nic *n)
{
    return SYS$DASSGN(
            //&d, // devname - device number 
            //&a->channel //,   // chan - channel number 
            //0,       // access mode(0=kernal,...)
            //0        // mailbox name(0=default,...)
            //); 
            n->channel);
}

int nic_startup_802e(nic *n, int prm, int n_mca)
{
    if(n_mca > 1) {
        printf("MCA > 1 CURRENTLY UNIMPLEMENTED.\n");
        return SS$_ABORT;
    }
    else if(n_mca) {
        //printf("USING MCA ADDR: ");
        //print_mac(MCA_ADDRS[0]);
    }

    void * parm_ptr;


    printf("FINGER BLASTING JUNKIES~~~~%s\n", n->name);

    if(n_mca) {

        struct _802E_data {
        short pcli_fmt; 
        int fmt_value;
        short pcli_prm; 
        int prm_value;
        short pcli_bus;
        int bus_value;
        short pcli_pid; // protocol id - 08-00-2B-90-00 // Loopback protocol
        short pid_len;
        char pid_val[5];
        short pcli_mca;  
        short pcli_mca_len;  
        short pcli_mca_modifier; 
        unsigned char mca_vals[1*6];
    } _802E_DATA_MCA = {
                    NMA$C_PCLI_FMT, NMA$C_LINFM_802E, 
                    //NMA$C_PCLI_PRM, NMA$C_STATE_OFF, 
                    NMA$C_PCLI_PRM, prm, 
                    NMA$C_PCLI_BUS, PCLI_BUS_VAL, 
                    //NMA$C_PCLI_PAD, NMA$C_STATE_OFF, 
                    //NMA$C_PCLI_PTY, n->cur_pcl
                    //NMA$C_PCLI_PTY, 0x0080
                    //NMA$C_PCLI_PTY, (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8),
                    //NMA$C_PCLI_MCA, 2 + (6*n_mca),  NMA$C_LINMC_SET, 
                    NMA$C_PCLI_PID, 0x05, 0x08, 0x00, 0X2B, 
                        (n->cur_fpcl & 0xff) << 8, ((n->cur_fpcl & 0xff00) >> 8),
                    NMA$C_PCLI_MCA, 2 + (6*n_mca),  NMA$C_LINMC_SET
                
        };

        int mca_index = 0;
        for(int r = 0; r < n_mca; r++) { 
            for(int c = 0; c < 6; c++) { 
                _802E_DATA_MCA.mca_vals[mca_index++] = MCA_ADDRS[r][c]; 
                //printf("CRAWLING JUNKIES[%d][%d]: %x %x ~~~\n",
                //r, c, ETH_DATA.mca_vals[mca_index-1], MCA_ADDRS[r][c]); 
            } 
        }

        if(prm == NMA$C_STATE_ON) 
            _802E_DATA_MCA.prm_value= NMA$C_STATE_ON;
        
        struct setparmdsc _802E_PARM = {sizeof(_802E_DATA_MCA), &_802E_DATA_MCA};
        parm_ptr = &_802E_PARM;


    }
    else {
    struct _802E_data {
        short pcli_fmt; 
        int fmt_value;
        short pcli_prm; 
        int prm_value;
        short pcli_bus;
        int bus_value;
        short pcli_pid; // protocol id - 08-00-2B-90-00 // Loopback protocol
        short pid_len;
        char pid_val[5];
    } _802E_DATA = {
                    NMA$C_PCLI_FMT, NMA$C_LINFM_802E, 
                    //NMA$C_PCLI_PRM, NMA$C_STATE_OFF, 
                    NMA$C_PCLI_PRM, prm, 
                    NMA$C_PCLI_BUS, PCLI_BUS_VAL, 
                    //NMA$C_PCLI_PAD, NMA$C_STATE_OFF, 
                    //NMA$C_PCLI_PTY, n->cur_pcl
                    //NMA$C_PCLI_PTY, 0x0080
                    //NMA$C_PCLI_PTY, (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8),
                    //NMA$C_PCLI_MCA, 2 + (6*n_mca),  NMA$C_LINMC_SET, 
                    NMA$C_PCLI_PID, 0x05, 0x08, 0x00, 0X2B, 
                        (n->cur_fpcl & 0xff) << 8, ((n->cur_fpcl & 0xff00) >> 8)
                
        };

        if(prm == NMA$C_STATE_ON) 
            _802E_DATA.prm_value= NMA$C_STATE_ON;
        
        struct setparmdsc _802E_PARM = {sizeof(_802E_DATA), &_802E_DATA};
        parm_ptr = &_802E_PARM;

    }

    //{NMA$C_PCLI_FMT, NMA$C_LINFM_802E,

    
        int status = sys$qiow(
                        0,n->channel,
                        IO$_SETMODE|IO$M_CTRL|IO$M_STARTUP,
                        &qio_iosb_arr[n->id], 0, 0, 0,
                        //(__int64)&lb_parm,
                        (__int64)parm_ptr,
                        0, 0, 0, 0
                    ); 
    
    if ($SUCCESS(status)) 
        status = qio_iosb_arr[n->id].w_err; 
    if ($FAIL(status)) { 
        printf("Fail status = %04X %04X\n", qio_iosb_arr[n->id].w_addl, qio_iosb_arr[n->id].w_misc); 
        //return qio_iosb_arr[n->id].w_addl; 
        status = qio_iosb_arr[n->id].w_addl; 
    }

    return status;

}

int nic_startup_eth(nic *n, int prm, int n_mca)
{
    if(n_mca > 1) {
        printf("MCA > 1 CURRENTLY UNIMPLEMENTED.\n");
        return SS$_ABORT;
    }
    else if(n_mca) {
        //printf("USING MCA ADDR: ");
        //print_mac(MCA_ADDRS[0]);
    }

    unsigned int huge_boner = (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8); //(n->cur_fpcl >> 8) | ((n->cur_fpcl & 0xff) >> 8);
    //printf("DANCING BONERS: %x %hx %d %d\n", huge_boner, n->cur_fpcl, 12, (n->cur_fpcl & 0xff00) >> 8);
    void * parm_ptr;
    if(n_mca) {
        //const int n_emm_see_aye = (n_mca > 0) ? n_mca : 1;
        
        #pragma nomember_alignment
        struct ethernet_data {
                    short pcli_fmt; 
                    int fmt_value;
                    short pcli_prm; 
                    int prm_value;
                    short pcli_bus;
                    int bus_value;
                    short pcli_pad;
                    int pad_value;
                    short pcli_pid; 
                    int pty_val;
                    short pcli_mca;  
                    short pcli_mca_len;  
                    short pcli_mca_modifier; 
                    unsigned char mca_vals[1*6];
        } ETH_DATA = {
                    NMA$C_PCLI_FMT, NMA$C_LINFM_ETH, 
                    NMA$C_PCLI_PRM, prm, 
                    NMA$C_PCLI_BUS, PCLI_BUS_VAL, 
                    NMA$C_PCLI_PAD, NMA$C_STATE_OFF, 
                    //NMA$C_PCLI_PTY, (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8),
                    NMA$C_PCLI_PTY, huge_boner,
                    NMA$C_PCLI_MCA, 2 + (6*n_mca),  NMA$C_LINMC_SET, 
        };
        //memset(ETH_DATA.mca_vals, 0, n_mca*6);
        int mca_index = 0;
        for(int r = 0; r < n_mca; r++) { 
            for(int c = 0; c < 6; c++) { 
                ETH_DATA.mca_vals[mca_index++] = MCA_ADDRS[r][c]; 
                //printf("CRAWLING JUNKIES[%d][%d]: %x %x ~~~\n",
                //r, c, ETH_DATA.mca_vals[mca_index-1], MCA_ADDRS[r][c]); 
            } 
        }
        if(prm == NMA$C_STATE_ON) 
            ETH_DATA.prm_value= NMA$C_STATE_ON;
        
      
        //exit(9);
    
        struct setparmdsc ETH_PARM = {sizeof(ETH_DATA), &ETH_DATA};
        parm_ptr = &ETH_PARM;

    } else {

        #pragma nomember_alignment
        struct ethernet_data {
                    short pcli_fmt; 
                    int fmt_value;
                    short pcli_prm; 
                    int prm_value;
                    short pcli_bus;
                    int bus_value;
                    short pcli_pad;
                    int pad_value;
                    short pcli_pid; 
                    int pty_val;
                    //
        } ETH_DATA = {
                    NMA$C_PCLI_FMT, NMA$C_LINFM_ETH, 
                    //NMA$C_PCLI_PRM, NMA$C_STATE_OFF, 
                    NMA$C_PCLI_PRM, prm, 
                    NMA$C_PCLI_BUS, PCLI_BUS_VAL, 
                    NMA$C_PCLI_PAD, NMA$C_STATE_OFF, 
                    //NMA$C_PCLI_PTY, n->cur_pcl
                    //NMA$C_PCLI_PTY, 0x0080
                    NMA$C_PCLI_PTY, (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8),
                    //NMA$C_PCLI_MCA, 2 + (6*n_mca),  NMA$C_LINMC_SET, 
        };

        if(prm == NMA$C_STATE_ON) 
            ETH_DATA.prm_value= NMA$C_STATE_ON;
        
        struct setparmdsc ETH_PARM = {sizeof(ETH_DATA), &ETH_DATA};
        parm_ptr = &ETH_PARM;

    }

    //if(prm == NMA$C_STATE_ON)  
      //  printf("GRANDPA'S GETTIN' PROMISCUOUS TONIGHT!!!\n");
 
    //printf("GRABBING JUNKIES: %d %x\n", n_mca, (n->cur_fpcl & 0xff) << 8 | ((n->cur_fpcl & 0xff00) >> 8));

   //if(n->cur_fpcl != DEF_ETH_FRAME_PCL) {
        //printf("WHINING BONERS: %hx %hx\n", n->cur_fpcl, DEF_ETH_FRAME_PCL);
        //exit(9);
    //}

    


    //struct setparmdsc lb_parm = {sizeof(ed), &ed};
    //parm = &lb_parm;
    //return SS$_ABORT;
    int status = sys$qiow(
                        0,n->channel,
                        IO$_SETMODE|IO$M_CTRL|IO$M_STARTUP,
                        &qio_iosb_arr[n->id], 0, 0, 0,
                        //(__int64)&lb_parm,
                        (__int64)parm_ptr,
                        0, 0, 0, 0
                    ); 
    
    if ($SUCCESS(status)) 
        status = qio_iosb_arr[n->id].w_err; 
    if ($FAIL(status)) { 
        printf("Fail status = %04X %04X\n", qio_iosb_arr[n->id].w_addl, qio_iosb_arr[n->id].w_misc); 
        //return qio_iosb_arr[n->id].w_addl; 
        status = qio_iosb_arr[n->id].w_addl; 
    }
    
    
    /* set the promiscuous mode back to OFF */
    //ETH_DATA.prm_value = NMA$C_STATE_OFF;
    //for(int i = 0; i < 100; i++)
    //printf("COAGULATING JUNKIES: %x\n", status);
    return status;

}


#if 0
enum GEN_PCL_TYPES {
    GEN_PCL_TYPE_USER_NO_LB = 0x00, GEN_PCL_TYPE_USER_LB = 0x01, 
    GEN_PCL_TYPE_TEST = 0x11, 
    GEN_PCL_TYPE_PERF = 0x21, 
};
#define GEN_PCL_TYPE_LB 0x01 
#endif

int nic_startup(nic *n, int prm, int n_mca)
{

    int status;

    //printf("FRAMING WEINERS: %x %x\n", (n->cur_fpcl & 0xff00) >> 8, n->cur_fpcl);

    switch((n->cur_fpcl & 0xff00) >> 8) {
    //switch(n->cur_fpcl & 0xff) {

    case 0X08:
    //case 0X08:
        //printf("ETHERNET FRAME PCL 08-XX\n");
        status = nic_startup_eth(n, prm, n_mca);
        break;
    
    case 0X09:
    case 0X19:
        printf("DADDY'S PACKIN A BIG WEENIE %x ~~~", n->cur_fpcl);
        status = nic_startup_802e(n, prm, n_mca);
        break;

    default:
        printf("%hx is an unsupported FRAME PROTOCOL.\n", n->cur_fpcl);
        status = SS$_ABORT;
        break;

    }
    return status;

}


int get_packet_data(struct packet_data_info *pdi)
{
    int status = SS$_ABORT;
    unsigned char *pdata = pdi->packet_data;
    nic *pd_nic = pdi->n;
    unsigned int packet_pcl = pdi->packet_pcl;
    int packet_size = pdi->packet_size;
    char *content_str = pdi->content_str;
    int pindex = 0;
    //int add_opts = 0;

    switch(packet_pcl & 0xffffff00) {
    case UDP_PPCL:
    case TCP_PPCL:

        //if(packet_pcl & GEN_PCL_TYPE_USER_LB) {
            // TODO: complicated, have to add OPTIONS section to header, increase header size...
            //printf("\t\tOVULATING NANNIES @@ %x %x @@ ", pd_nic->cur_fpcl, packet_pcl);
            //printf("+++++==COPULATING JABRONIES: %x %d %d\n", packet_pcl, packet_size, MIN_IPV4_PACKET_OPT_SIZE);
            //exit(9);
            //if(packet_size < MIN_IPV4_PACKET_OPT_SIZE) {
              //  printf("OPTING BONERS~~~");
                //packet_size  = MIN_IPV4_PACKET_OPT_SIZE;
                
            //}
        //}

        //int huge_frump_size = 
        
        pdata[pindex++] = 0x45;
        if(packet_pcl & PACKET_PCL_TYPE_LB) 
            pdata[pindex++] = 0x01; // OPTION TYPE? (No operation used for debugging???)
        else
            pdata[pindex++] = 0;
        pdata[pindex++] = packet_size >> 8;
        pdata[pindex++] = packet_size & 0xFF;
        pdata[pindex++] = 0; //packet->header.id >> 8;
        pdata[pindex++] = 0; //packet->header.id & 0xFF;
        pdata[pindex++] = ((unsigned short)0x4000) >> 8;
        pdata[pindex++] = ((unsigned short)0x4000) & 0xFF;
        pdata[pindex++] = 0x40;
        //pdata[pindex++] = packet_pcl;
        //unsigned char packet_pcl_char = (packet_pcl & 0x0000ff00) >> 8;
        //printf("SHARING WEINERS: %x~~~\n", (packet_pcl & 0x0000ff00) >> 8);
        pdata[pindex++] = (packet_pcl & 0x0000ff00) >> 8;;
        pdata[pindex++] = 0; //chksum 0
        pdata[pindex++] = 0; // ...
        //pdata[index++] = sa_ip_addr[0];
        //pdata[index++] = sa_ip_addr[1];
        //pdata[index++] = sa_ip_addr[2];
        //pdata[index++] = sa_ip_addr[3];
        pindex += 4;
        if(!0) {
            pindex += 4;
        } else {
            //pdata[index++] = xmt_ip_addr[0];
            //pdata[index++] = xmt_ip_addr[1];
            //pdata[index++] = xmt_ip_addr[2];
            //pdata[index++] = xmt_ip_addr[3];
        }

        // OPTIONS
        
        if(packet_pcl & PACKET_PCL_TYPE_LB) {
            //int first_byte = 1;
            //for(; pindex < packet_size; pindex++) {
                //printf("COPULATING JABRONIES[%d]~~~", pdata[j]);
              //  if(first_byte) {
                    pdata[pindex++] = 0x01; // TODO: make enum
                    //printf("COPULATING JABRONIES[%d]~~~", pdata[pindex-1]);
                //    first_byte = 0;
                //}
            //}
            
            //exit(9);
            
        }


        #if 0
                    if(packet_pcl == UDP_PCL) {
                        pkt_block[index++] = sp >> 8;
                        pkt_block[index++] = sp & 0xff;
                        pkt_block[index++] = dp >> 8;
                        pkt_block[index++] = dp & 0xff;
                        pkt_block[index++] =(packet_size - IPV4_HEADER_LEN) >> 8;
                        pkt_block[index++] =(packet_size - IPV4_HEADER_LEN) & 0xFF;
                        pkt_block[index++] = 0; //calc_udp_header_chksm(h, sa, da, app_data, 0);
                        pkt_block[index++] = 0;
                    } else {
                        pkt_block[index++] = sp >> 8;
                        pkt_block[index++] = sp & 0xFF;
                        pkt_block[index++] = dp >> 8;
                        pkt_block[index++] = dp & 0xff;
                        index += sizeof(int)*2; // seq + ack num
                        pkt_block[index++] = 0x05 << 4 | 0; // first nibble is TCP header len in 32 bit longwords
                        pkt_block[index++] = 0x00;
                        pkt_block[index++] = 0; // window size
                        pkt_block[index++] = 0;
                        pkt_block[index++] = 0; // checksum
                        pkt_block[index++] = 0;
                        pkt_block[index++] = 0; //urg_ptr
                        pkt_block[index++] = 0;
                    }
        #endif
        if(content_str) {
            for(int i = 0; i < strlen(content_str); i++) {
                //printf("GRIPING WEINERS~~~\n");
                pdata[pindex++] = content_str[i];
                if(pindex == packet_size) // only write content as far as you can
                    break;
            }
        }
        //show_buffer_bytes("SMACKING WEINERS", pdata, packet_size);
          //  printf("+++++==PACKING WEINERS: %x %d %d\n", packet_pcl, packet_size, MIN_IPV4_PACKET_OPT_SIZE);
            //exit(9);
        break;
    case GEN_PPCL:

        //if(packet_pcl & GEN_PCL_TYPE_USER_LB) {

            //printf("\t\tOVULATING TRANNIES @@ %x %x @@\n", pd_nic->cur_fpcl, packet_pcl);

        //}

        //while(1) printf("BETTE MIDDLER SHARTED: %x %x ~~~  ", packet_pcl, GEN_PCL_TYPE_USER_LB);


        pdata[pindex++] = packet_pcl >> 24;
        pdata[pindex++] = (packet_pcl >> 16) & 0xff;
        pdata[pindex++] = (packet_pcl >> 8) & 0xff;
        pdata[pindex++] = packet_pcl & 0xff; 
        if(content_str) {
            for(int i = 0; content_str[i] && (i + pindex) < packet_size; i++)
                pdata[pindex++] = content_str[i];
        }
       
        break;
    default: 
        while(1) printf("BARRY MANILOW SHARTED\n");
        break;
    }

    //printf("%%%%%%%%-JIGGLING BONERS BIG TIME-%%%%%%%%\n");

    return status;
}


//== Display (print) routines ===============================

/**
 * PRINT_MAC_NONL:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display a mac address with NO NEWLINE CHAR. (XX-XX-XX-XX-XX-XX)
 *
 * INPUTS:
 *	mac - 6 byte unsigned char address
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */
void print_mac_nonl(unsigned char *mac)
{
    for(int j = 0; j < 6; j++) {
        if(j < 5)
            printf("%x-", mac[j]);
        else
            printf("%x", mac[j]);
    }
}


/**
 * PRINT_MAC:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display a mac address. (XX-XX-XX-XX-XX-XX)
 *
 * INPUTS:
 *	mac - 6 byte unsigned char address
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */

void print_mac(unsigned char *mac)
{
    print_mac_nonl(mac);
    printf("\n");
}

/**
 * PRINT_TIME_NOW:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display the current timestamp. (NOW)
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */

void print_time_now()
{
    /* get a current timestamp */
    time_t now;
    time(&now);
    printf("%s", ctime(&now));
}

/**
 * PRINT_PROGRAM_HEADER:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display the program header.
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */

void print_program_header()
{
    printf("--NIC-TEST v. 1.0--\n");
    printf("-Tim Ward, VSI inc. 01/2022\n");
    printf("-");
    print_time_now();
    printf("-------------------\n");
    
}


/**
 * PRINT_DEV_NAMES:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display ALL devices (not only assignable) returned by LANCP.
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	main (, various)
 */

void print_dev_names()
{
    printf("-%d devices:\n", n_sys_dev_names);
    for(int i = 0; i < n_sys_dev_names; i++) {
        printf("%s", sys_dev_names[i]);
        if(i < n_sys_dev_names-1)
            printf(", ");
        else
            printf("\n");
    }
}


void print_frame_pcl(unsigned short frame_pcl) {
    
    printf("--Frame Protocol: [");
                            //print_frame_pcl(frame_pcl);
                            if((frame_pcl >> 8) < 10)
                                printf("0");
                            printf("%x]-[", frame_pcl >> 8);
                            if((frame_pcl & 0xff) < 10)
                                printf("0");
                            printf("%x], ", frame_pcl & 0xff); 
                            /*
                            unsigned int ppcl = (packet_pcl & 0xffffff00) >> 8;
                            switch(packet_pcl >> 8) {
                            case UDP_PPCL >> 8:
                                printf("Packet protocol: UDP (%x)\n", ppcl);
                                break;
                            case TCP_PPCL >> 8:
                                printf("Packet protocol: TCP (%x)\n", ppcl);
                                break;
                            case GEN_PPCL >> 8:
                                printf("Packet protocol: GENERIC (%x)\n", ppcl);
                                break;
                            default:
                                break;
                            }
                            */
}

/**
 * PRINT_NIC:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display a NIC data structure.
 *
 * INPUTS:
 *	NIC struct
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */

void print_nic(nic *n)
{
    printf("-+-\"%s\"[%d] %dMB-+-\n", n->name, n->id, n->line_speed);
    print_mac(n->hwa);
    printf("Link=");
    if(n->link_state == NMA$C_STATE_ON) 
        printf("UP\n");
    else
        printf("DOWN\n");
    if(n->cur_fpcl) 
        printf("-Frame PCL: %x\n", n->cur_fpcl);
    int receiving; // = 0;
    pthread_mutex_lock(&rtm);
    receiving = rthreads_inuse[n->id];
    pthread_mutex_unlock(&rtm);
    if(receiving)
        printf("--currently in RECEIVE mode.\n");


    printf("-+--+--+--+--+-\n");
}


void print_target(struct target *t)
{
    printf("--<<<< target %s >>>>--\n", t->name);
    printf("  ");
    print_mac_nonl(t->mac);
    switch(t->type) {
    case SYS_TARTYPE:
        printf(" SYSTEM node\n");
        break;
    case MCA_TARTYPE:
        printf(" MCA node\n");
        break;
    case USR_ADDED_TARTYPE:
        printf(" USER ADDED (outside) node\n");
        break;
    default:
        while(1) printf("DUNKING BONERS~~~");
    }
    //printf("--<<<<------------->>>>\n");
}

/**
 * PRINT_ASSIGNABLE_NICS:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Display ONLY assignable NICS. (that can be used in the program)
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	Various
 */

void print_assignable_nics()
{
    printf("=--= Assignable NICS:\n");
    for(int i = 0; nics[i]; print_nic(nics[i++])); 
}

//== Comparison routines ===============================

/**
 * STR_MMATCH:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	This routine checks to see if a string abbreviaiton matches
 *  to a comparison string. 
 *
 * INPUTS:
 *	full_str - STRING, s2 - STRING
 *
 * OUTPUTS:
 *	Index
 *
 * CALLER:
 *	Various
 */
 
int str_mmatch(char *full_str, char *s) 
{
    int index;
    for(index = 0; index < strlen(s) && (full_str[index] == s[index]); index++);
    //return index >= STR_MIN_MATCH && index == strlen(s);
    return index == strlen(s);
}

/**
 * STREQ:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	String comparison EQUALS helper routine. 
 *
 * INPUTS:
 *	full_str - STRING, s2 - STRING
 *
 * OUTPUTS:
 *	Index
 *
 * CALLER:
 *	Various
 */

int streq(char *s1, char *s2)
{
    int index;
    for(index = 0; index < strlen(s2) && (s1[index] == s2[index]); index++);
    //return index >= STR_MIN_MATCH && index == strlen(s);
    return index == strlen(s2);
}


/**
 * STRNEQ:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	String comparison NOT EQUALS helper routine. 
 *
 * INPUTS:
 *	full_str - STRING, s2 - STRING
 *
 * OUTPUTS:
 *	Index
 *
 * CALLER:
 *	various
 */

int strneq(char *full_str, char *s)
{
    return !streq(full_str, s);
}


/**
 * STRLOWER:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Set the string to all lowercase values. (tolower()) 
 *
 * INPUTS:
 *	lower - STRING, s - STRING, size - INT
 *
 * OUTPUTS:
 *	None
 *
 * CALLER:
 *	various
 */

void strlower(char *lower, char *s, int size)
{
    memset(lower, 0, size);
    for(int i = 0; s[i]; i++) 
        lower[i] = tolower(s[i]);
}


/**
 * IS_SPACE:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Determine if a CHAR is a space. (' ', \t or \n) 
 *
 * INPUTS:
 *	c - CHAR
 *
 * OUTPUTS:
 *	Boolean
 *
 * CALLER:
 *	various
 */
int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

/**
 * MODIFY_INPUT:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Modify the input for parsing commands and qualifiers. 
 *
 * INPUTS:
 *	input - STRING, mod_input - STRING
 *
 * OUTPUTS:
 *	status
 *
 * CALLER:
 *	input_loop
 */


int modify_input(char *input, char *mod_input)
{
    int mindex = 0;
    char c;
    int i;
    int in_quotes = 0, paren_level = 0;
    for(i = 0; input[i]; i++) {
        c = input[i];
        if(is_space(c) && strlen(mod_input) == 0) 
            continue;
        if( (!in_quotes && i < strlen(input)-1) &&
            (is_space(c) && is_space(input[i+1])) ) {
            continue;
        }

        if(!in_quotes && c == '(') {
            //while(1)
            //paren_level++;
            while(1)
            printf("BABYING JABRONIES BIG TIME ~~~> ");
            //exit(9);
            //paren_level++;
            //mod_input[mindex++] = c;
            //continue;
        }

        if(c == '"') {
            if(!in_quotes) 
                in_quotes = 1;
            else {
                // ...
                in_quotes = 0;
            }
        } else if( (c == '=' || c == '/') && (mindex && !is_space(mod_input[mindex-1])) ) 
            mod_input[mindex++] = ' ';
        while( (i < strlen(input)-1) && (c == '/' && is_space(input[i+1])) ) 
            i++;
        if(c == '\t')
            c = ' ';
        mod_input[mindex++] = c;
        if( ( i < strlen(input)-1 ) && (c == '=' && !is_space(input[i+1]) ) ) 
            mod_input[mindex++] = ' ';

    }
    
    while(is_space(mod_input[strlen(mod_input)-1])) 
        mod_input[strlen(mod_input)-1] = 0;

    if(strlen(mod_input) == 0 || in_quotes) {
        if(in_quotes) {
            printf("open quote with no closing quote found.\n");
        }
        return SS$_ABORT;
    }
    return SS$_NORMAL;
}




void add_system_targets()
{
    for(int i = 0; i < n_nics; i++) {
        char nic_name_lower[100];
        strlower(nic_name_lower, nics[i]->name, 100);
        int add_sys_target = 1;
        for(int i = 0; i < n_targets; i++) {
            char target_name_lower[100];
            strlower(target_name_lower, targets[i].name, 100);
            if(streq(nic_name_lower, target_name_lower)) {
                printf("system target \"%s\" already exists.\n", target_name_lower);
                add_sys_target = 0;
                break;
            }
        }
        if(add_sys_target) {
            if(n_targets == N_MAX_TARGETS) {
                printf("~~~STU STU STAMMERING JABRONIES 2~~~\"%d\"\n", n_targets);
                return; // INVALID_INPUT;
            }
            targets[n_targets].type = SYS_TARTYPE;
            strcpy(targets[n_targets].name, nics[i]->name);
            for(int j = 0; j < 6; j++) {
                targets[n_targets].mac[j] = nics[i]->hwa[j];
                //printf("HUGE WINKING TRANNIES: %x\n", targets[n_targets].mac[i]);
            }
            n_targets++;

        }
    }
}

void add_mca_targets()
{
    int n_mca_addrs = sizeof(MCA_ADDRS) / sizeof(MCA_ADDRS[0]);
    for(int i = 0; i < n_mca_addrs; i++) {
        char tname[100];
        memset(tname, 0, 100);
        int tn_index = 0;
        tname[tn_index++] = 'M';
        char str[100];
        sprintf(str, "%d", i+1);
        for(int i = 0; str[i]; i++)
            tname[tn_index++] = str[i];
        printf("JUGGLING JUNKIES BIG TIME: \"%s\"\n", tname);
        //exit(9);

        //char nic_name_lower[100];
        //strlower(nic_name_lower, nics[i]->name, 100);
        int add_mca_target = 1;
        for(int i = 0; i < n_targets; i++) {
            char target_name_lower[100];
            strlower(target_name_lower, targets[i].name, 100);
            if(streq(tname, target_name_lower)) {
                printf("system target \"%s\" already exists.\n", target_name_lower);
                add_mca_target = 0;
                break;
            }
        }
        if(add_mca_target) {
            if(n_targets == N_MAX_TARGETS) {
                printf("~~~STU STU STAMMERING JABRONIES 2~~~\"%d\"\n", n_targets);
                return; // INVALID_INPUT;
            }
            targets[n_targets].type = MCA_TARTYPE;
            strcpy(targets[n_targets].name, tname);
            for(int j = 0; j < 6; j++) {
                targets[n_targets].mac[j] = MCA_ADDRS[i][j];
                printf("HUGE WINKING TRANNIES: %x\n", targets[n_targets].mac[j]);
            }
            n_targets++;

        }
    }
}



void show_packet_data(char *description, unsigned char *bytes, int nbytes) {
    show_buffer_bytes(description, bytes, nbytes);
}

/**
 * BURST_CMD:
 * -------------
 */
int burst_cmd(struct cmd *cmds[], int n_cmds)
{
    
    int status = SS$_NORMAL;
    int rate = 1000;
    double duration = 0.0, burst_duration = 0.0;
    int duration_stop_at = 10;
    int burst_duration_stop_at = 1;
    int packet_size = MIN_PACKET_SIZE;
    int duration_set = 0, burst_duration_set = 0;
    unsigned short frame_pcl = DEF_ETH_FRAME_PCL;
    unsigned int packet_pcl = GEN_PPCL;
    char content_str[1000];
    memset(content_str, 0, 1000);
    
    //printf("=====BURST COMMAND================\n");
    for(int i = 0; i < n_cmds; i++) {
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            char qlr[100];
            strlower(qlr, q.name, 100);
            if(str_mmatch("rate", qlr)) {

            } else if(str_mmatch("duration", qlr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"BURST /DURATION=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    for(int i = 0; q_val[i]; i++) {
                        if(!isdigit(q_val[i])) {
                            printf("%s is an invalid DURATION. (>= 1) (1)\n", q_val);
                            return INVALID_INPUT;
                        }
                    }
                    duration_stop_at = atoi(q_val);
                    if(duration_stop_at < 1) { // || n_packets > MAX_N_PACKETS???) {
                        printf("%s is an invalid DURATION. (>= 1) (2)\n", q_val);
                        status = INVALID_INPUT;
                    } 
                    if(duration_set++) {
                        printf("DURATION duplicate qualifier.\n");
                        return INVALID_INPUT;
                    } 
                }
                
            } else if(str_mmatch("burst_duration", qlr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"BURST /BURST_DURATION=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    for(int i = 0; q_val[i]; i++) {
                        if(!isdigit(q_val[i])) {
                            printf("%s is an invalid BURST DURATION. (>= 1) (1)\n", q_val);
                            return INVALID_INPUT;
                        }
                    }
                    burst_duration_stop_at = atoi(q_val);
                    if(burst_duration_stop_at < 1) { // || n_packets > MAX_N_PACKETS???) {
                        printf("%s is an invalid BURST DURATION. (>= 1) (2)\n", q_val);
                        status = INVALID_INPUT;
                    } 
                    if(burst_duration_set++) {
                        printf("BURST DURATION duplicate qualifier.\n");
                        return INVALID_INPUT;
                    } 
                }
                
            }else if(str_mmatch("size", qlr)) {
                
            } else if(str_mmatch("frame_pcl", qlr)) {
                
            } else if(str_mmatch("packet_pcl", qlr)) {
                
            } else if(str_mmatch("content", qlr)) {
                
            }
            for(int k = 0; k < q.n_vals; k++) {
                char *q_val = q.vals[k];
                printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            }
        }
    }
    printf("=================================\n\n");

    struct timespec start_time, prev_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    clock_gettime(CLOCK_REALTIME, &prev_time);

    printf("~~~BURSTING BONERS~~~\n");
    

    int n_sent = 0;

    while(1) {

        struct packet_data_info pdi;
        memset(&pdi, 0, sizeof(struct packet_data_info));
        unsigned char packet_data[packet_size];
        memset(packet_data, 0, packet_size);
        pdi.packet_data = &packet_data[0];
        pdi.n = 0;
        
        pdi.packet_pcl = packet_pcl;
        pdi.packet_size = packet_size;
        if(strlen(content_str) > 0) 
            pdi.content_str = content_str;
        get_packet_data(&pdi);

        //show_buffer_bytes("TWERKING JUNKIES", packet_data, packet_size);

        struct p5_param xmt_param;
        //get_xmt_param(sender->hwa, target_mac);
        

        // CODED OUT BELOW FOR XG52 BUILD...
        #if 0
        for(int i = 0; i < 6; i++) {
            xmt_param.sa[i] = sender->hwa[i];
            xmt_param.da[i] = target_mac[i];
            //printf("HICCUPING JABRONIES: %X %X\n", xmt_param.sa[i], xmt_param.da[i]);
        }

        //printf(" FRAMING JABRONIES WE ARE[%d]\n", eth_frame_pcl);
        //exit(9);
        show_buffer_bytes("GULPING BONERS", packet_data, packet_size);


        status = sys$qiow (
                                0,sender->channel,
                                IO$_WRITEVBLK, 
                                &qio_iosb_arr[sender->id],0,0,
                                packet_data, 
                                packet_size,0,0,
                                (__int64)&xmt_param,0 
        ); 

        if ($FAIL(status)) {
            fprintf(stderr, "qiow failed -69 %d\n", status);
            //return FAILURE;
            break;
        }
        #endif

        printf("GURGLING BONERS: %lf %lf %d\n", duration, burst_duration, burst_duration_stop_at);
            

        
        struct timespec cur_time;
        CHECK_LOOP_END:

        clock_gettime(CLOCK_REALTIME, &cur_time);
        duration = (cur_time.tv_sec - start_time.tv_sec) +
				    (cur_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

        
        if(duration >= duration_stop_at) {
            printf("DRIBBLING WEINERS~~~\n");
            break;
        }

        

        //printf("BUBBLING JUNKIES: %lf\n", duration);
        printf("@@~~~BURSTING BONERS~~~>> %d %d @@=> ", n_sent, rate);
        if(n_sent == rate) {
            //struct timespec boner_time;
            //clock_gettime(CLOCK_REALTIME, &boner_time);
            printf("@@~~~BURSTING BONERS~~~>> %lf %lf %d  @@=> ", duration, burst_duration, burst_duration_stop_at);
            //while()
            burst_duration = (cur_time.tv_sec - prev_time.tv_sec) +
				    (cur_time.tv_nsec - prev_time.tv_nsec) / 1000000000.0;
            
            if(burst_duration < burst_duration_stop_at) {
                //printf("BURSTING WEINERS: %lf %lf %d\n", duration, burst_duration, burst_duration_stop_at);
            
                goto CHECK_LOOP_END; 
            }

            

            printf("NIBBLING WEINERS~~~\n");
            clock_gettime(CLOCK_REALTIME, &prev_time);
            n_sent = 0;
        } else
            n_sent++;

    }

    return status;
}






int parse_frame_pcl(char *str, unsigned char *tmp_ret)
{
    int FAILURE = SS$_ABORT;

    //while(1)
    printf("~~~~NIBBLING JUNKIES ~~~~\"%s\"~~~~\n", str);
    
    //unsigned char tmp_ret[2] = {0,0};
    if(str_mmatch("ipv4", str)) {
        //tmp_ret[0] = 0x08;
        //tmp_ret[1] = 0x00;
        
        tmp_ret[0] = IPV4_FRAME_PCL & 0xff;
        tmp_ret[1] = (IPV4_FRAME_PCL & 0xff00) >> 8;
        //ret[0]
        //printf("NIBBLING WEINERS: %x %x\n", tmp_ret[0], tmp_ret[1]);
        //exit(9);
    } else if(str_mmatch("ipv6", str)) {
        printf("NIBBLING BONERS: %x %x\n", tmp_ret[0], tmp_ret[1]);
        exit(9);
        tmp_ret[0] = 0x86;
        tmp_ret[1] = 0xDD;
    } else if(str_mmatch("arp", str)) {
        //tmp_ret[0] = 0x08;
        //tmp_ret[1] = 0x06;
        tmp_ret[0] = 0x06;
        tmp_ret[1] = 0x08;
        printf("DRIBBLING JUNKIES: %x %x\n", tmp_ret[0], tmp_ret[1]);
        exit(9);
    } else if(streq(str, "decnet")) {
        while(1)printf("CORNPOP WAS FOLDING BONERS~~~\n");
        tmp_ret[0] = 0x06;
        tmp_ret[1] = 0x03;
    //} //else if(streq(str, "802")) {
        //while(1)printf("CORNPOP WAS HOLDING WEINERS \"%s\"~~~\n",str);
        //tmp_ret[0] = 0x81;
        //tmp_ret[1] = 0x00;
        //strcpy(str, "81-00");
        //goto _802;
    } else if(str_mmatch("802e", str)) {
        //while(1)printf("CORNPOP WAS HOLDING BONERS~~~\n");
        //tmp_ret[0] = 0x91;
        //tmp_ret[1] = 0x00;
        unsigned short short_weiner = 0x1900;

        tmp_ret[0] = short_weiner & 0xff;
        tmp_ret[1] = (short_weiner & 0xff00) >> 8;
    }else {
        //ps(str);
        //printf("CORNPOP WAS JUGGLING BONERS~~~\"%s\"\n", str);
        if(strlen(str) >=3 && str[2] == '-') {
            char tmp[3];
            tmp[0] = str[0];
            tmp[1] = str[1];
            tmp[2] = '\0';
            //printf("CRYING JUNKIES: \"%s\"\n", tmp);
            if(streq("81", tmp)) {

                _802:
                printf("CORNPOP WAS HOLDING BONERS~~~\"%s\"\n", str);
                
                tmp_ret[0] = 0x81;
                if(strlen(str) == 5) {

                    

                    char tmp[3];
                    tmp[0] = str[3];
                    tmp[1] = str[4];
                    tmp[2] = '\0';
                    int eth_pcl = (int)strtol(tmp, NULL, 16); //atoi(tmp);
                    
                    //pd(eth_pcl);
                    if(eth_pcl == 0) {
                        if(tmp[0] != '0' || tmp[1] != '0') {
                            //printf("invalid input. (TODO: go to usage)\n");
                            return FAILURE;
                        }
                    }
                    tmp_ret[1] = eth_pcl;
                    printf("EXPOSING GRANNIES: %x %x\n", tmp_ret[0], tmp_ret[1]);
                   
                } else {
                    
                    printf("invalid input. (TODO: go to usage) 5\n");
                    return FAILURE;
                }
            }
            else if(streq("08", tmp) || streq("68", tmp)) {

                //printf("EXPOSING GRANNIES I WAS: %x %x ~~~*~~~ ", tmp_ret[0], tmp_ret[1]);

                if(streq("08", tmp))
                    tmp_ret[1] = 0x08;
                else
                    tmp_ret[1] = 0x86;
                
                //frame_pcl_str[0] = ??;
                if(strlen(str) == 5) {
                    char tmp[3];
                    tmp[0] = str[3];
                    tmp[1] = str[4];
                    tmp[2] = '\0';
                    int eth_pcl = (int)strtol(tmp, NULL, 16); //atoi(tmp);
                    
                    //pd(eth_pcl);
                    if(eth_pcl == 0) {
                        if(tmp[0] != '0' || tmp[1] != '0') {
                            //printf("invalid input. (TODO: go to usage)\n");
                            return FAILURE;
                        }
                    }
                    tmp_ret[0] = eth_pcl;
                } else {
                    //printf("invalid input. (TODO: go to usage)\n");
                    return FAILURE;
                }
            } else if(streq("06", tmp)) {
                tmp_ret[0] = 0x06;
                if(strlen(str) == 5) {
                    char tmp[3];
                    tmp[0] = str[3];
                    tmp[1] = str[4];
                    tmp[2] = '\0';
                    int dec_pcl = (int)strtol(tmp, NULL, 16); //atoi(tmp);
                    if(dec_pcl == 0) {
                        if(tmp[0] != '0' || tmp[1] != '0') {
                            printf("invalid input. (TODO: go to usage) 1\n");
                            return FAILURE;
                        }
                    }
                    if(dec_pcl < 2 || dec_pcl > 4) {
                        //printf("invalid DECNET PCL nibble (2-4)\n");
                        //return FAILURE;
                    }
                    //pd(dec_pcl);
                    tmp_ret[1] = dec_pcl;
                } else {
                    printf("invalid input. (TODO: go to usage) 2\n");
                    return FAILURE;
                }
            }else if(tmp[1] == '9') {
                //ps(str);
                //ps(tmp);
                
                if(strlen(str) != 5) {
                    printf("DROPPING BONERS 1");
                    exit(9);
                }
                if(!isdigit(str[1])) {
                    printf("DROPPING BONERS 1+1");
                    exit(9);
                }
                if(!isdigit(str[3])) {
                    printf("DROPPING BONERS 1+2");
                    exit(9);
                }
                if(!isdigit(str[4])) {
                    printf("DROPPING BONERS 1+3");
                    exit(9);
                }
                //tmp_ret[0] = 0x08;
                int _802e_pcl_b1 = (int)strtol(tmp, NULL, 16);
                //printf("BUMPING BONERS: %c %d\n", tmp[1], _802e_pcl_b1);
                //e("wowz");
                //if(_802e_pcl_b1 == 0) {
                    //if(str[0] != '0' || str[1] != '0') {
                      //  printf("invalid input 34-1. (TODO: go to usage)\n");
                        //return FAILURE;
                    //}
                    //e("CRYING BONERS 1");
                //}

                //e("CRYING BONERS  2");
                char byte2[3];
                byte2[0] = str[3];
                byte2[1] = str[4];
                byte2[2] = '\0';
                //tmp_ret[0] = tmp[0];
                //tmp_ret[1] = tmp[1];
                //int _802e_pcl_b2 = atoh(byte2);
                int _802e_pcl_b2 = (int)strtol(byte2, NULL, 16); 
                //printf("DUMPING BONERS: %c %d\n", byte2[1], _802e_pcl_b2);
                //e("wowz");
                //if(_802e_pcl_b2 == 0) {
                  //  if(byte2[0] != '0' || byte2[1] != '0') {
                    //    printf("invalid input 34. (TODO: go to usage)\n");
                      //  return FAILURE;
                    //}
                //}
                //e("TODO: 802E protocol (NEW INPLEMENTATION INCOMPLETE)");
                tmp_ret[0] = _802e_pcl_b1;
                tmp_ret[1] = _802e_pcl_b2;
            } else if(streq(str, "60-05")) {
                printf("EXPLODING BONERS");
                tmp_ret[0] = 0x60;
                //tmp_ret[0] = 0x06;
                tmp_ret[1] = 0x05;
            } 
            else {
                printf("invalid input. (TODO: go to usage) 3\n");
                return FAILURE;
            }
        } else {
            printf("invalid input. (TODO: go to usage) 4\n");
            return FAILURE;
        }
        //p("STU STU STAMMERING BONERS");
    }
    //unsigned char *ret = malloc(2);
    //ret[0] = tmp_ret[0];
    //ret[1] = tmp_ret[1];
    //printf("EXPLODING BONERS BIG TIME: %x %x\n", tmp_ret[0], tmp_ret[1]);
    //e("whaa");
    //return ret;
    return SS$_NORMAL;
}


/* display buffer bytes (to show like wireshark) */
void show_buffer_bytes(char *description, unsigned char *bytes, int nbytes)
{
    printf("=====(%d bytes)\n", nbytes);
    if(strneq("", description))
        printf("%s\n", description);
    int row_len = 16;
    for(int i = 0; i < nbytes; i++) {
        if((i > 0) && (i % (row_len / 2) == 0))
            printf("  ");
        if((i > 0) && (i % row_len) == 0) 
            printf("| (%d)\n", i);
        unsigned char byte = bytes[i];
        if(byte <= 0xf)
            printf("0");
        printf("%x ", byte);
    }
    printf("\n=====\n");
}

int mac_eq(unsigned char *m1, unsigned char *m2)
{
    for(int i = 0; i < 6; i++) {
        if(m1[i] != m2[i])
            return 0;
    }
    return 1;
}

/**
 * TEST_CMD:
 * -------------
 */

int test_cmd(struct cmd *cmds[], int n_cmds)
{
    int status;
    int n_mca = 1; // TODO WANT ALL ENABLED

    /* only do testing with GENERIC PPCL */
    const unsigned int packet_pcl = GEN_PPCL | PACKET_PCL_TYPE_LB;

    if(n_cmds < 3) {
        printf("Invalid TEST COMMAND input. (1)\n");
        return INVALID_INPUT;
    }

    nic *testers[N_MAX_DEVS];
    memset(testers, 0, sizeof(nic*)*N_MAX_DEVS);
    int n_testers;

    char *dname = cmds[1]->name;

    char dname_lower[100];
    strlower(dname_lower, dname, 100);

    if(!str_mmatch("all", dname_lower) && strlen(dname) < 3) {
        printf("Invalid TEST COMMAND input. (2)\n");
        return INVALID_INPUT;
    }


    int prm = NMA$C_STATE_OFF;
    
    if(str_mmatch("all", dname_lower)) {
        n_testers = 0;
        for(int i = 0; i < n_nics; i++) 
            testers[n_testers++] = nics[i];
    } else {
    
        for(int i = 0; !testers[0] && nics[i]; i++) {
            char *nic_name = nics[i]->name;
            char nic_name_lower[100];
            strlower(nic_name_lower, nic_name, 100);
            if(str_mmatch(nic_name_lower, dname_lower)) {
                testers[0] = nics[i];
                //print_nic(testers[0]);
                //exit(9);
            }
        }   
        n_testers = 1;
    }

    /* only proceed if the TESTER is recognized in the system */
    if(!testers[0]) {
        printf("\"%s\" is an unknown device name.\n", dname);
        return INVALID_INPUT;
    }
    //print_nic(testers[0]);

    unsigned short frame_pcl = DEF_ETH_FRAME_PCL;
    unsigned char loop_back = 0, LB_TYPE_WAIT = 0X10, LB_TYPE_NOWAIT = 0X20;
    loop_back = LB_TYPE_NOWAIT;

    unsigned int packet_size = MIN_PACKET_SIZE;

    int read_type;
    if(loop_back == LB_TYPE_NOWAIT) 
        read_type = IO$_READVBLK|IO$M_NOW;
    else 
        read_type = IO$_READVBLK;
                                
    int test_to_mca = 0;
    int n_target_macs;
    char *target = cmds[2]->name;
            char target_lr[100];
            strlower(target_lr, target, 100);
            unsigned char target_macs[n_nics][6];
            memset(target_macs, 0, n_nics*6);
            char target_name[100];
            memset(target_name, 0, 100);
            
            if(str_mmatch("all", target_lr)) {
                n_target_macs = n_nics;
                for(int i = 0; i < n_target_macs; i++) {
                    for(int j = 0; j < 6; j++)
                        target_macs[i][j] = nics[i]->hwa[j];
                }
            } else if(str_mmatch("mca", target_lr)) {
                for(int i = 0; i < 6; i++) 
                    target_macs[0][i] = MCA_ADDRS[0][i];
                n_target_macs = 1;
                test_to_mca = 1;
            } else {
                status = get_mac(target_macs[0], target);
                if($FAIL(status)) {
                    /* check to see if the target is being referenced by name */
                    char usr_req_target_name_lower[100];
                    strlower(usr_req_target_name_lower, target, 100);
                    for(int i = 0; i < n_targets; i++) {
                        char target_name_lower[100];
                        strlower(target_name_lower, targets[i].name, 100);
                        if(streq(usr_req_target_name_lower, target_name_lower)) {
                            for(int j = 0; j < 6; j++)
                                target_macs[0][j] = targets[i].mac[j];
                            status = SS$_NORMAL;
                            n_target_macs = 1;
                            strcpy(target_name, targets[i].name);
                            break;
                        }
                    }
                    if($FAIL(status)) {
                        printf("\"%s\" is an invalid target mac address. (XX-XX-XX-XX-XX-XX)\n", target);
                        return INVALID_INPUT;
                    }
                } else  
                    n_target_macs = 1;
            }

    /* default test duration is 5 seconds */
    double test_duration = 5.0; 

    char content_str[10000];
    memset(content_str, 0, 10000);


    printf("=====TEST COMMAND================%d\n", n_cmds);
    //int n_cmds = 0;
    for(int i = 0; i < n_cmds; i++) {
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            char qnlr[100];
            strlower(qnlr, q.name, 100);
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            for(int k = 0; k < q.n_vals; k++) {
                char *q_val = q.vals[k];
                printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            }
            
            if(str_mmatch("frame_pcl", qnlr)) {
                if(q.n_vals != 1) {
                            printf("Invalid \"TEST /FRAME_PROTOCOL=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            char qv_lr[100];
                            strlower(qv_lr, q_val, 100);
                            unsigned char tmp_fpcl[2];
                            memset(tmp_fpcl, 0, 2);
                            status = parse_frame_pcl(qv_lr, tmp_fpcl);
                            if($FAIL(status)) {
                                printf("\"%s\" is an invalid FRAME PROTOCOL.\n", q_val);
                                return INVALID_INPUT;
                            }
                            frame_pcl = (tmp_fpcl[1] << 8) | tmp_fpcl[0];
                        }
            } else if(streq("prm", qnlr)) {
                if(q.n_vals > 0) while(1) printf("CRYING JAH JAH JUNKIES~~");
                prm = NMA$C_STATE_ON;
            } else if(str_mmatch("content", qnlr)) {
                //if(q.n_vals > 0) while(1) printf("CRYING JAH JAH JUNKIES~~");
                //prm = NMA$C_STATE_ON;
                if(q.n_vals != 1) {
                    printf("Invalid \"TEST /CONTENT=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    q_val++;
                    q_val[strlen(q_val)-1] = 0;
                    strcpy(content_str, q_val);

                }

            } else {
                printf("\"%s\" is an invalid TEST COMMAND QUALIFIER.\n", q.name);
                return INVALID_INPUT;
            }
        }
    }
    printf("====================================\n\n");


    for(int i = 0; i < n_testers; i++) {

        nic *tester = testers[i];
        print_nic(tester);

        /*
        int dev_receiving = 0;
        pthread_mutex_lock(&rtm);
        if(rthreads_inuse[tester->id])
            dev_receiving = 1;
        pthread_mutex_unlock(&rtm);

        if(dev_receiving) {
            printf("device %s is receiving (cannot perform test)\n", tester->name);
            continue;
        }
        */

        
        //print_nic(tester);

        printf("CRASHING JUNKIES 3\n");

        status = assign_nic(tester);
        if($FAIL(status)) {
                                            printf("assign nic for device %s failed\n", tester->name);
                                            //return NULL;
                                            //printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                                              //      tester->name, status);
                        return INVALID_INPUT;
        }
                    //if($SUCCESS(status)) {
                        tester->cur_fpcl = frame_pcl;
                        //printf("CRASHING JUNKIES 1\n");
                        status = nic_startup(tester, prm, n_mca);
        if($FAIL(status)) {
                                            printf("nic startup for device %s failed\n", tester->name);
                                            //return NULL;
                                            //printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                                              //      tester->name, status);
                        return INVALID_INPUT;
        }
                double duration = 0.0;           
                                //printf("Testing device %s to target mac: ")
        for(int j = 0; j < n_target_macs; j++ ) {

                                //printf("CRASHING JUNKIES 2\n");

                                

                               unsigned char *target_mac = target_macs[j];

                               int eq = 1  ;
                            for(int i = 0; eq && i < 6; i++) {
                                if(tester->hwa[i] != target_mac[i]) {
                                    eq = 0;
                                    //while(1) printf("SLAPPING MONKIES: %x\n", packet_pcl);
                                }
                            }
                            if(eq) {
                                printf("CLAPPING MONKIES: %x\n", packet_pcl);
                                printf("Device %s cannot test to itself.\n", tester->name);
                                tester->cur_fpcl = 0;
                                status = deassign_nic(tester);
                                continue;
                            }

                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
                            printf("~~~Testing from device %s to ", tester->name);
                            if(n_cmds == 2)
                                printf("(SELF) ");
                            if(strlen(target_name) > 0) 
                                printf("(%s) ", target_name);
                            printf("target Mac Address: ");
                            for(int i = 0; i < 6; i++) {
                                if(i < 5)
                                    printf("%x-", target_mac[i]);
                                else
                                    printf("%x\n", target_mac[i]);
                            }
                            printf("--Frame Protocol: [");
                            if((tester->cur_fpcl >> 8) < 10)
                                printf("0");
                            printf("%x]-[", tester->cur_fpcl >> 8);
                            if((tester->cur_fpcl & 0xff) < 10)
                                printf("0");
                            printf("%x], ", tester->cur_fpcl & 0xff); 
                            unsigned int ppcl = (packet_pcl & 0xffffff00) >> 8;
                            switch(packet_pcl >> 8) {
                            case UDP_PPCL >> 8:
                                printf("Packet protocol: UDP (%x)\n", ppcl);
                                break;
                            case TCP_PPCL >> 8:
                                printf("Packet protocol: TCP (%x)\n", ppcl);
                                break;
                            case GEN_PPCL >> 8:
                                printf("Packet protocol: GENERIC (%x)\n", ppcl);
                                break;
                            default:
                                break;
                            }

                            //if(packet_pcl & loop_back) {
                              //  printf("----(loopback requested)\n");
                            //}

                            if(test_to_mca)
                                printf("----MCA target address test\n");

                            //printf("--NUM PACKETS: %d\n", n_packets);
                            //printf("--PACKET SIZE: %d\n", packet_size);
                            
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

                            struct timespec start_time;
                            clock_gettime(CLOCK_REALTIME, &start_time);

                            //for(int packet_num = 0; packet_num < n_packets; packet_num++) {
                            #if !0
                            int packet_num = 0;
                            int bytes_rec = 0;
                            unsigned char *pdata_prt;

                            int rec_pkt = 0;

                            struct rcv_packet {
                                nic *rcv_nic;
                                time_t ts;
                                //time(&now);
                                //printf("-%s", ctime(&now));
                                unsigned char mac[6];
                                unsigned int pcl;
                                char msg[10000];
                            } rcv_packets[1000];

                            memset(&rcv_packets, 0, sizeof(struct rcv_packet)*1000);
                            int rpindex = 0;



                            unsigned char sa_addrs[100][6];
                            memset(sa_addrs, 0, 6*100);
                            int sa_addrs_index = 0;

                            
            while(1) {

                                struct packet_data_info pdi;
                                memset(&pdi, 0, sizeof(struct packet_data_info));
                                unsigned char packet_data[packet_size];
                                memset(packet_data, 0, packet_size);
                                pdata_prt = packet_data;
                                pdi.packet_data = &packet_data[0];
                                pdi.n = tester;

                                pdi.packet_pcl = packet_pcl;
                                //int old_packet_size = packet_size;
                                pdi.packet_size = packet_size;
                                if(strlen(content_str) > 0) {
                                    //printf("GURGLING JUNKIES~~~ \"%s\"\n", content_str);
                                    pdi.content_str = content_str;
                                }
                                get_packet_data(&pdi);
                                /*
                                if(*pdi.packet_size > old_packet_size) {
                                    printf("moldy frump status: %d\n", packet_size);
                                    //exit(9);
                                    unsigned char new_packet_data[packet_size];
                                    memset(new_packet_data, 0, packet_size);
                                    memcpy(new_packet_data, packet_data, sizeof(packet_data));
                                    show_buffer_bytes("WHINING JUNKIES BIG TIME", new_packet_data, packet_size);
                                    printf("moldy jabroni status: %d\n", packet_size);
                                    //exit(9);

                                }
                                */
                                struct p5_param xmt_param;
                                for(int i = 0; i < 6; i++) {
                                    xmt_param.sa[i] = tester->hwa[i];
                                    xmt_param.da[i] = target_mac[i];
                                }
                                //if(packet_num == 0 && show_first_packet)
                                    //show_buffer_bytes("Packet Data", packet_data, packet_size);

                                status = sys$qiow (
                                    0,tester->channel,
                                    IO$_WRITEVBLK, 
                                    &qio_iosb_arr[tester->id],0,0,
                                    packet_data, 
                                    packet_size,0,0,
                                    (__int64)&xmt_param,0 
                                ); 

                                if ($FAIL(status)) {
                                    fprintf(stderr, "qiow failed%d\n", status);
                                    break;
                                }

                                packet_num++;
                                #endif



                                    
                                    //time_t lb_start_time = time(0);
                                    //while(!rec_pkt && (time(0) - lb_start_time) < test_duration) {

                                        unsigned char rcv_data[MAX_PACKET_SIZE];
                                        memset(rcv_data, 0, MAX_PACKET_SIZE);
                                        struct p5_param rcv_param;
                                        memset(&rcv_param, 0, sizeof(struct p5_param));

                                        status = sys$qiow (
                                            0,tester->channel,
                                            //IO$_READVBLK|IO$M_NOW, 
                                            read_type,
                                            &qio_iosb_arr[tester->id],0,0,
                                            rcv_data, 
                                            MAX_PACKET_SIZE,0,0,
                                            (__int64)&rcv_param,0 
                                        );
            
                                        if($FAIL(status)) {
                                            fprintf(stderr, "qiow failed uh ohs whaa\n");
                                            //return NULL;
                                            printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                                                    tester->name, status);
                                        }
                                        else 
                                            status = qio_iosb_arr[tester->id].w_err; // checking whether received packet or not

                                        if($FAIL(status)) {
                                            //free(rcv_buffer);
                                            //free(rcv_param);
                                            //printf("????????: %d\n", qio_iosb_arr[receiver->id].w_xfer_size);
                                        } else {

                                            bytes_rec = qio_iosb_arr[tester->id].w_xfer_size;
                
                                            if(!bytes_rec) {
                                                printf("No packet received.\n");
                                            } else {
                                                //show_buffer_bytes("WHEN GRANDPA GETS ANAL EVERYBODY WINS!!!", rcv_data, bytes_rec);
                                                //printf("Loopback for device %s received %d bytes of data.\n", tester->name, bytes_rec);
                                                //char *content_str = (char*)rcv_data
                                                
                                                if( rcv_data[0] == ((GEN_HEADER_BYTES >> 24) & 0xff) &&
                                                    rcv_data[1] == ((GEN_HEADER_BYTES >> 16) & 0xff) &&
                                                    rcv_data[2] == ((GEN_HEADER_BYTES >> 8) & 0xff) ) {
                                                    
                                                    int add_packet = 1;
                                                    if(test_to_mca) {
                                                        for(int i = 0; i < rpindex; i++) {
                                                            if(mac_eq(rcv_packets[i].mac, rcv_param.sa)) {
                                                                //printf("JINGLING BONERS ~~~");
                                                                add_packet = 0;
                                                                break;
                                                            }
                                                        }
                                                    }

                                                    if(add_packet) {
                                                        rec_pkt = 1;
                                                        struct rcv_packet *rp = &rcv_packets[rpindex];
                                                        rp->rcv_nic = tester;
                                                        for(int i = 0; i < 6; i++)
                                                            rp->mac[i] = rcv_param.sa[i];
                                                        rp->pcl = GEN_HEADER_BYTES;
                                                        time(&rp->ts);
                                                        rpindex++;

                                                        char *msg_received = ((char*)rcv_data) + sizeof(GEN_HEADER_BYTES);
                                                        if(strlen(msg_received) > 0) {
                                                            printf("-~- Blumpkin Received: \"%s\" -~-\n", msg_received);
                                                            strcpy(rp->msg, msg_received);
                                                        }

                                                        printf("GRABBING WEINERS (@;@)~~~>");
                                                    }
                                                    
                                                    
                                                    int sa_exists = 0;
                                                    for(int i = 0; !sa_exists && i < sa_addrs_index; i++) {
                                                        unsigned char *mac = sa_addrs[i];
                                                        if(mac_eq(mac, rcv_param.sa)) 
                                                            sa_exists = 1;
                                                    }
                                                    if(!sa_exists) {
                                                        for(int i = 0; i < 6; i++)
                                                            sa_addrs[sa_addrs_index][i] = rcv_param.sa[i];
                                                        sa_addrs_index++;
                                                        
                                                        //show_buffer_bytes("THE ONLY REASON HE DIDN'T HAVE HIS DICK IN HIS ASS IS BECAUSE THEY BOTH HAD PANTS ON!!!",
                                                        //pdata_prt, bytes_rec);
                                                        if(!test_to_mca) {
                                                            printf("Loopback for device %s received %d bytes of data.\n", tester->name, bytes_rec);
                                                            printf("From MAC address: ");
                                                            print_mac(rcv_param.sa);
                                                            //printf("Received packet back!!!\n");
                                                        }
                                                    }
                                                
                                                } 
                                            }
                                            
                                        }
                                    
                                struct timespec cur_time;
                                clock_gettime(CLOCK_REALTIME, &cur_time);
                                duration = (cur_time.tv_sec - start_time.tv_sec) +
				            		(cur_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
                                
                                
                               if((!test_to_mca && rec_pkt) || duration >= test_duration)  
                                    break;
                            


                                    
                                
            } // while not rec_pkt     
            if(test_to_mca) {



                for(int i = 0; i < rpindex; i++) {
                    struct rcv_packet rp = rcv_packets[i];
                    printf("*MCA RCV Entry[%d]: NIC: %s, packet_pcl: %x\n-rcv address: ", 
                            i, rp.rcv_nic->name, rp.pcl);
                    print_mac_nonl(rp.mac);
                    //printf(" ");
                    printf(" -%s", ctime(&rp.ts));
                    if(strlen(rp.msg) > 0) {
                        printf("--Message received: \"%s\"\n", rp.msg);
                    }

                }



            } else {
                if(!rec_pkt)
                    printf("Done waiting %lf seconds for loopback...(no packet received) ~#~# @#@#@-\n", test_duration);
                else {
                    printf("received packet back in %lf seconds...\n", duration);
                }   
            }      
                            
        } // for target_macs
        status = deassign_nic(tester);
    } // for testers
                                


    return SS$_NORMAL;
}


/**
 * SEND_CMD:
 * -------------
 */
int send_cmd(struct cmd *cmds[], int n_cmds)
{

    const unsigned char junk_mac[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    unsigned int packet_pcl = GEN_PPCL;
    char content_str[1000];
    char * sender_name = cmds[1]->name;
    unsigned short frame_pcl = DEF_ETH_FRAME_PCL;
    int status = SS$_NORMAL;
    int send_to_self = (n_cmds == 2), packet_size = MIN_PACKET_SIZE;
    int n_packets = 1, duration_stop_at = 0, interval = 0, uinterval = 0;
    int show_first_packet = 0;
    int lb_timeout = 5;
    int n_senders;
    int quiet = 0;
    int prm = NMA$C_STATE_OFF;
    unsigned char loop_back = 0, LB_TYPE_WAIT = 0X10, LB_TYPE_NOWAIT = 0X20;
    double duration = 0.0;
    nic *senders[n_nics]; 
    
    
    memset(content_str, 0, 1000);
    memset(senders, 0, sizeof(nic*) * n_nics);

    /* determine that there is a valid SUB COMMAND */
    if(!send_to_self && n_cmds != 3) {
        printf("Invalid SEND command input. (1)\n");
        return INVALID_INPUT;
    }

    if(str_mmatch("all", sender_name)) {
        n_senders = 0;
        for(int i = 0; i < n_nics; i++) {
            if(nics[i]->link_state == NMA$C_STATE_ON) 
                senders[n_senders++] = nics[i];
        }
    } else {

        if(strlen(sender_name) < 3) {
            printf("Invalid SEND command input. (1.5)\n");
            return INVALID_INPUT;
        }

        char sender_name_lower[100];
        strlower(sender_name_lower, sender_name, 100);
        nic *sender = 0;
        for(int i = 0; !sender && nics[i]; i++) {
            char *nic_name = nics[i]->name;
            char nic_name_lower[100];
            strlower(nic_name_lower, nic_name, 100);
            if(str_mmatch(nic_name_lower, sender_name_lower))
                sender = nics[i];
        }

        /* only proceed if the SENDER is recognized in the system */
        if(!sender) {
            printf("\"%s\" is an unknown device name.\n", sender_name);
            status = INVALID_INPUT;
        } else if(sender->link_state != NMA$C_STATE_ON) {
            printf("\"%s\" is currently LINK DOWN.\n", sender_name);
            status = INVALID_INPUT;
        } else {
            senders[0] = sender;
            n_senders = 1;
        }
    }
    
    if($SUCCESS(status)) {

        int n_target_macs;
        if(send_to_self) 
            n_target_macs = 1;
        unsigned char target_macs[n_nics][6];
        memset(target_macs, 0, n_nics*6);
        char target_name[100];
        memset(target_name, 0, 100);
        if(n_cmds == 3) {
            char *target = cmds[2]->name;
            char target_lr[100];
            strlower(target_lr, target, 100);
            if(str_mmatch("junk", target_lr)) {
                for(int i = 0; i < 6; i++) 
                    target_macs[0][i] = junk_mac[i];
                n_target_macs = 1;
            } else if(str_mmatch("all", target_lr)) {
                n_target_macs = n_nics;
                for(int i = 0; i < n_target_macs; i++) {
                    for(int j = 0; j < 6; j++)
                        target_macs[i][j] = nics[i]->hwa[j];
                }
            } else if(str_mmatch("mca", target_lr)) {
                for(int i = 0; i < 6; i++) 
                    target_macs[0][i] = MCA_ADDRS[0][i];
                n_target_macs = 1;
            } else {
                status = get_mac(target_macs[0], target);
                if($FAIL(status)) {
                    /* check to see if the target is being referenced by name */
                    char usr_req_target_name_lower[100];
                    strlower(usr_req_target_name_lower, target, 100);
                    for(int i = 0; i < n_targets; i++) {
                        char target_name_lower[100];
                        strlower(target_name_lower, targets[i].name, 100);
                        if(streq(usr_req_target_name_lower, target_name_lower)) {
                            for(int j = 0; j < 6; j++)
                                target_macs[0][j] = targets[i].mac[j];
                            status = SS$_NORMAL;
                            n_target_macs = 1;
                            strcpy(target_name, targets[i].name);
                            break;
                        }
                    }
                    if($FAIL(status))
                        printf("\"%s\" is an invalid target mac address. (XX-XX-XX-XX-XX-XX)\n", target);
                } else  
                    n_target_macs = 1;
            }
        } 

        
        if($SUCCESS(status)) {

            const char * send_cmd_quals[] = { "number", "duration", "size", "content", 
                                                "frame_pcl", "packet_pcl", "loopback",
                                              "interval", "uinterval", "mca", "show", "prm", "quiet" }; 

            /* Now that we have established that the command components 
               are valid, check the validity of any supplied qualifiers */
            int packet_size_set = 0, num_packets_set = 0, duration_set = 0;
            int int_set = 0, uint_set = 0;
            int n_mca = 0;
            for(int i = 0; i < n_cmds; i++) {
                char *cmd_name = cmds[i]->name;
                int n_quals = cmds[i]->n_quals;
                for(int j = 0; j < n_quals; j++) {
                    struct qual q = cmds[i]->quals[j];
                    char q_name_lower[100];
                    strlower(q_name_lower, q.name, 100);
                    char *qual_name = 0;
                    for(int i = 0; i < sizeof(send_cmd_quals)/sizeof(send_cmd_quals[0]); i++) {
                        if(str_mmatch((char*)send_cmd_quals[i], q_name_lower)) {
                            if(qual_name) {
                                printf("\"%s\" is an ambiguous SEND COMMAND QUALIFIER.\n", q.name);
                                return INVALID_INPUT;
                            }
                            qual_name = (char*)send_cmd_quals[i];
                        }
                    }
                    if(!qual_name) {
                        printf("\"%s\" is an invalid SEND COMMAND QUALIFIER.\n", q.name);
                        return INVALID_INPUT;
                    }
                    if(streq("prm", qual_name)) {
                        if(q.n_vals > 0) while(1) printf("CRYING JAH JAH JUNKIES~~");
                        prm = NMA$C_STATE_ON;
                    } else if(streq("quiet", qual_name)) {
                        if(q.n_vals > 0) while(1) printf("OVULATING JABRONIES~~");
                        //prm = NMA$C_STATE_ON;
                        printf("OVULATING JUNKIES~~");
                        quiet = 1;
                    } else if(streq("loopback", qual_name)) {
                        //printf("INHALING JUNKIES\n");
                        //exit(9);
                        if(q.n_vals > 1) {
                            printf("Invalid \"SEND /LOOPBACK=\" qualifier input. (only 0,1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else if(q.n_vals == 1) {
                            char *q_val = q.vals[0];
                            /*
                            for(int i = 0; q_val[i]; i++) {
                                if(!isdigit(q_val[i])) {
                                    printf("%s is an invalid NUM MCA ADDRESSES. [0, 20] (1)\n", q_val);
                                    return INVALID_INPUT;
                                }
                            }
                            n_mca = atoi(q_val);
                            if(n_mca < 0 || n_mca > 20) {
                                printf("%s is an invalid NUM MCA ADDRESSES. [0, 20] (2)\n", q_val);
                                return INVALID_INPUT;
                            }
                            */
                           while(1) printf("GOOFING BONERS: \"%s\"~~~ ", q_val);
                        } else {
                            //n_mca = 1;
                            loop_back = LB_TYPE_NOWAIT;
                            //printf("GOOFING BONERS: \"%d\"~~~ ", loop_back);
                        }


                    } else if(streq("mca", qual_name)) {
                        if(q.n_vals > 1) {
                            printf("Invalid \"SEND /MCA=\" qualifier input. (only 0,1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else if(q.n_vals == 1) {
                            char *q_val = q.vals[0];
                            for(int i = 0; q_val[i]; i++) {
                                if(!isdigit(q_val[i])) {
                                    printf("%s is an invalid NUM MCA ADDRESSES. [0, 20] (1)\n", q_val);
                                    return INVALID_INPUT;
                                }
                            }
                            n_mca = atoi(q_val);
                            if(n_mca < 0 || n_mca > 20) {
                                printf("%s is an invalid NUM MCA ADDRESSES. [0, 20] (2)\n", q_val);
                                return INVALID_INPUT;
                            }
                        } else 
                            n_mca = 1;
                    } else if(streq("number", qual_name)) {
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /NUMBER=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            for(int i = 0; q_val[i]; i++) {
                                if(!isdigit(q_val[i])) {
                                    printf("%s is an invalid NUMBER of packets. (>= 1) (1)\n", q_val);
                                    return INVALID_INPUT;
                                }
                            }
                            n_packets = atoi(q_val);
                            if(n_packets < 1) { // || n_packets > MAX_N_PACKETS???) {
                                printf("%s is an invalid NUMBER of packets. (>= 1) (2)\n", q_val);
                                status = INVALID_INPUT;
                            } 
                            if(num_packets_set++) {
                                printf("NUM PACKETS duplicate qualifier.\n");
                                return INVALID_INPUT;
                            } 
                        }
                    } else if(streq("duration", qual_name)) {
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /DURATION=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            for(int i = 0; q_val[i]; i++) {
                                if(!isdigit(q_val[i])) {
                                    printf("%s is an invalid DURATION. (>= 1) (1)\n", q_val);
                                    return INVALID_INPUT;
                                }
                            }
                            duration_stop_at = atoi(q_val);
                            if(duration_stop_at < 1) { // || n_packets > MAX_N_PACKETS???) {
                                printf("%s is an invalid DURATION. (>= 1) (2)\n", q_val);
                                status = INVALID_INPUT;
                            } 
                            if(duration_set++) {
                                printf("DURATION duplicate qualifier.\n");
                                return INVALID_INPUT;
                            } 
                        }
                    }else if(streq("size", qual_name)) {
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /SIZE=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            for(int i = 0; q_val[i]; i++) {
                                if(!isdigit(q_val[i])) {
                                    printf("%s is an invalid PACKET SIZE. [46, 9000] (1)\n", q_val);
                                    return INVALID_INPUT;
                                }
                            }
                            packet_size = atoi(q_val);
                            if(packet_size < MIN_PACKET_SIZE || packet_size > MAX_PACKET_SIZE) {
                                printf("%s is an invalid PACKET SIZE. [46, 9000] (2)\n", q_val);
                                return INVALID_INPUT;
                            }
                            if(packet_size_set++) {
                                printf("PACKET SIZE duplicate qualifier.\n");
                                return INVALID_INPUT;
                            } 
                        }
                    } else if(streq("interval", qual_name)) {
                        
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /INTERVAL=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            
                            interval = atoi(q_val);
                            if(interval < 1 || interval > 600) {
                                printf("%s is an invalid INTERVAL. [1, 600]\n", q_val);
                                return INVALID_INPUT;
                            }
                            if(int_set++) {
                                printf("INTERVAL duplicate qualifier.\n");
                                return INVALID_INPUT;
                            } 
                        }
                    
                    } else if(streq("uinterval", qual_name)) {
                
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /UINTERVAL=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            uinterval = atoi(q_val);
                            if(uinterval < 1 || uinterval > 999999) {
                                printf("%s is an invalid UINTERVAL. [1, 999999]\n", q_val);
                                return INVALID_INPUT;
                            }
                            if(uint_set++) {
                                printf("UINTERVAL duplicate qualifier.\n");
                                return INVALID_INPUT;
                            } 
                        }
                    
                    } else if(streq("content", qual_name)) {
                        
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /CONTENT=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            q_val++;
                            q_val[strlen(q_val)-1] = 0;
                            strcpy(content_str, q_val);
                        }
                    } else if(streq("frame_pcl", qual_name)) {
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /FRAME_PROTOCOL=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            char qv_lr[100];
                            strlower(qv_lr, q_val, 100);
                            unsigned char tmp_fpcl[2];
                            memset(tmp_fpcl, 0, 2);
                            status = parse_frame_pcl(qv_lr, tmp_fpcl);
                            if($FAIL(status)) {
                                printf("\"%s\" is an invalid FRAME PROTOCOL.\n", q_val);
                                return INVALID_INPUT;
                            }
                            frame_pcl = (tmp_fpcl[1] << 8) | tmp_fpcl[0];
                        }

                    } else if(streq("packet_pcl", qual_name)) {
                        if(q.n_vals != 1) {
                            printf("Invalid \"SEND /PACKET_PROTOCOL=\" qualifier input. (only 1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            char *q_val = q.vals[0];
                            char q_val_lower[1000];
                            strlower(q_val_lower, q_val, 1000);
                            if(str_mmatch("generic", q_val_lower)) 
                                printf("(generic): \"%s\" \"%s\"\n", q_val, q_val_lower);
                            else if(str_mmatch("udp", q_val_lower)) 
                                packet_pcl = UDP_PPCL;
                            else if(str_mmatch("tcp", q_val_lower)) 
                                packet_pcl = TCP_PPCL;
                            else {
                                printf("invalid input quive: \"%s\" \"%s\"\n", q_val, q_val_lower);
                                return INVALID_INPUT;
                            }
                        }

                    } else if(streq("show", qual_name)) {
                        
                        if(q.n_vals != 0) {
                            printf("Invalid \"SEND /SHOW=\" qualifier input. (no values allowed)\n");    
                            return INVALID_INPUT;
                        }
                        show_first_packet = 1;
                    }
                }
            }

            switch(packet_pcl) {
            case UDP_PPCL:
            case TCP_PPCL:
                if(packet_size < MIN_IPV4_PACKET_SIZE) {
                    if(packet_size_set) {
                        printf("Packet size is too small (< 64): %d\n", packet_size);
                        return INVALID_INPUT;
                    } else if(loop_back) 
                        packet_size = MIN_IPV4_PACKET_OPT_SIZE;
                    else 
                        packet_size = MIN_IPV4_PACKET_SIZE;
                } 
            }

            /* by default only send one packet */
            if(!duration_stop_at && !num_packets_set) 
                num_packets_set = 1;

            /* if loopback was requested, enable multicast addressing */
            if(loop_back && n_mca == 0) {
                n_mca = N_MCA_ADDRS; // TODO: want this
                n_mca = 1;
            }

            for(int i = 0; i < n_senders; i++) {
                nic *sender = senders[i];
                for(int j = 0; j < n_target_macs; j++) {
                    unsigned char target_mac[6]; 
                    if(send_to_self) {
                        for(int i = 0; i < 6; i++) 
                            target_mac[i] = sender->hwa[i];
                        printf("\n");
                    } else {
                        for(int i = 0; i < 6; i++) 
                            target_mac[i] = target_macs[j][i];
                        printf("\n");
                    }

                    status = assign_nic(sender);
                    if($SUCCESS(status)) {
                        sender->cur_fpcl = frame_pcl;
                        status = nic_startup(sender, prm, n_mca);
                        if($SUCCESS(status)) {
                            

                            //printf("CLAPPING MONKIES: %x\n", packet_pcl);
                            packet_pcl |= loop_back;
                            if(loop_back)
                                packet_pcl |= PACKET_PCL_TYPE_LB;

                        
                            //printf("CLAPPING JUNKIES: %x\n", packet_pcl);
                            //exit(9);

                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
                            printf("~~~Sending from device %s to ", sender->name);
                            if(n_cmds == 2)
                                printf("(SELF) ");
                            int is_junk = 1;
                            for(int i = 0; is_junk && i < 6; i++) {
                                if(target_mac[i] != junk_mac[i])
                                    is_junk = 0;
                            }
                            if(is_junk)
                                printf("(JUNK) ");
                            if(strlen(target_name) > 0) 
                                printf("(%s) ", target_name);
                            printf("target Mac Address: ");
                            for(int i = 0; i < 6; i++) {
                                if(i < 5)
                                    printf("%x-", target_mac[i]);
                                else
                                    printf("%x\n", target_mac[i]);
                            }
                            //printf("--Frame Protocol: [");
                            print_frame_pcl(sender->cur_fpcl);
                            /*
                            if((sender->cur_fpcl >> 8) < 10)
                                printf("0");
                            printf("%x]-[", sender->cur_fpcl >> 8);
                            if((sender->cur_fpcl & 0xff) < 10)
                                printf("0");
                            printf("%x], ", sender->cur_fpcl & 0xff); 
                            */
                            unsigned int ppcl = (packet_pcl & 0xffffff00) >> 8;
                            
                            switch(packet_pcl >> 8) {
                            case UDP_PPCL >> 8:
                                printf("Packet protocol: UDP (%x)\n", ppcl);
                                break;
                            case TCP_PPCL >> 8:
                                printf("Packet protocol: TCP (%x)\n", ppcl);
                                break;
                            case GEN_PPCL >> 8:
                                printf("Packet protocol: GENERIC (%x)\n", ppcl);
                                break;
                            default:
                                break;
                            }
                            

                            if(packet_pcl & loop_back) {
                                printf("----(loopback requested)\n");
                            }

                            printf("--NUM PACKETS: %d\n", n_packets);
                            printf("--PACKET SIZE: %d\n", packet_size);
                            printf("--CONTENT STR: \"%s\"\n", content_str);
                            printf("--INTERVAL: "); 
                            if(interval)
                                printf("%d", interval);
                            else
                                printf("(none)");
                            printf(", UINTERVAL: ");
                            if(uinterval)
                                printf("%d ", uinterval);
                            else
                                printf("(none) ");
                            printf("\n");
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

                            struct timespec start_time;
                            clock_gettime(CLOCK_REALTIME, &start_time);

                            //for(int packet_num = 0; packet_num < n_packets; packet_num++) {
                            int packet_num = 0;
                            int n_rec = 0; // for loopback
                            while(1) {

                                struct packet_data_info pdi;
                                memset(&pdi, 0, sizeof(struct packet_data_info));
                                unsigned char packet_data[packet_size];
                                memset(packet_data, 0, packet_size);
                                pdi.packet_data = &packet_data[0];
                                pdi.n = sender;

                                pdi.packet_pcl = packet_pcl;
                                int old_packet_size = packet_size;
                                pdi.packet_size = packet_size;
                                if(strlen(content_str) > 0) 
                                    pdi.content_str = content_str;
                                get_packet_data(&pdi);
                                /*
                                if(*pdi.packet_size > old_packet_size) {
                                    printf("moldy frump status: %d\n", packet_size);
                                    //exit(9);
                                    unsigned char new_packet_data[packet_size];
                                    memset(new_packet_data, 0, packet_size);
                                    memcpy(new_packet_data, packet_data, sizeof(packet_data));
                                    show_buffer_bytes("WHINING JUNKIES BIG TIME", new_packet_data, packet_size);
                                    printf("moldy jabroni status: %d\n", packet_size);
                                    //exit(9);

                                }
                                */
                                struct p5_param xmt_param;
                                for(int i = 0; i < 6; i++) {
                                    xmt_param.sa[i] = sender->hwa[i];
                                    xmt_param.da[i] = target_mac[i];
                                }
                                if(packet_num == 0 && show_first_packet)
                                    show_buffer_bytes("Packet Data", packet_data, packet_size);

                                status = sys$qiow (
                                    0,sender->channel,
                                    IO$_WRITEVBLK, 
                                    &qio_iosb_arr[sender->id],0,0,
                                    packet_data, 
                                    packet_size,0,0,
                                    (__int64)&xmt_param,0 
                                ); 

                                if ($FAIL(status)) {
                                    fprintf(stderr, "qiow failed%d\n", status);
                                    break;
                                }

                                packet_num++;

                                if(loop_back) {

                                    int read_type;
                                    if(loop_back == LB_TYPE_NOWAIT) 
                                        read_type = IO$_READVBLK|IO$M_NOW;
                                    else 
                                        read_type = IO$_READVBLK;
                                    if(!quiet)
                                        printf("Waiting %d seconds for loopback...~#~# @#@#@-\n", lb_timeout);
                                    int rec_pkt = 0;
                                    time_t lb_start_time = time(0);
                                    while(!rec_pkt && (time(0) - lb_start_time) < lb_timeout) {

                                        unsigned char rcv_data[MAX_PACKET_SIZE];
                                        memset(rcv_data, 0, MAX_PACKET_SIZE);
                                        struct p5_param rcv_param;
                                        memset(&rcv_param, 0, sizeof(struct p5_param));

                                        status = sys$qiow (
                                            0,sender->channel,
                                            //IO$_READVBLK|IO$M_NOW, 
                                            read_type,
                                            &qio_iosb_arr[sender->id],0,0,
                                            rcv_data, 
                                            MAX_PACKET_SIZE,0,0,
                                            (__int64)&rcv_param,0 
                                        );
            
                                        if($FAIL(status)) {
                                            fprintf(stderr, "qiow failed uh ohs whaa\n");
                                            //return NULL;
                                            printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                                                    sender->name, status);
                                        }
                                        else 
                                            status = qio_iosb_arr[sender->id].w_err; // checking whether received packet or not

                                        if($FAIL(status)) {
                                            //free(rcv_buffer);
                                            //free(rcv_param);
                                            //printf("????????: %d\n", qio_iosb_arr[receiver->id].w_xfer_size);
                                        } else {

                                            int bytes_rec = qio_iosb_arr[sender->id].w_xfer_size;
                
                                            if(!bytes_rec) {
                                                printf("No packet received.\n");
                                            } else {

                                                n_rec++;
                                                //show_buffer_bytes("WHEN GRANDPA GETS ANAL EVERYBODY WINS!!!", rcv_data, bytes_rec);
                                                if(!quiet)
                                                    printf("Loopback for device %s received %d bytes of data.\n", sender->name, bytes_rec);
                                                //char *content_str = (char*)rcv_data
                                                
                                                if( rcv_data[0] == ((GEN_HEADER_BYTES >> 24) & 0xff) &&
                                                    rcv_data[1] == ((GEN_HEADER_BYTES >> 16) & 0xff) &&
                                                    rcv_data[2] == ((GEN_HEADER_BYTES >> 8) & 0xff) ) { 
                                                    //(rcv_data[3] & 0xf0) == (GEN_PCL_TYPE_USER & 0xf0) ) { //} || packet_data[3] == GEN_PCL_TYPE_USER_NO_LB) ) {
                    
                                                    //show_buffer_bytes("~*~*- SLAPPING MONKIES -*~*~", rcv_data, bytes_rec);
                                                    char *msg_received = ((char*)rcv_data) + sizeof(GEN_HEADER_BYTES);
                                                    if(!quiet)
                                                        printf("-~- Message Received: \"%s\" -~-\n", msg_received);
                                                } else if(rcv_data[0] == 0x45) {
                                                    char *msg_received;
                                                    int opts_included = rcv_data[1];
                                                    if(opts_included)
                                                        msg_received = ((char*)rcv_data) + IPV4_HEADER_LEN+1;
                                                    else
                                                        msg_received = ((char*)rcv_data) + IPV4_HEADER_LEN;
                                                    //while(1) {
                                                    if(!quiet)
                                                        printf("-~- Message Received: \"%s\" -~-\n", msg_received);
                                                        //show_buffer_bytes("GRUNTING WEINERS", rcv_data, bytes_rec);
                                                    //}
                                                    
                                                    
                                                }
                                            }
                                            rec_pkt = 1;
                                        }
                                    }
                                    if(!quiet && !rec_pkt)
                                        printf("Done waiting %d seconds for loopback...(no packet received) ~#~# @#@#@-\n", lb_timeout);
                                    
                                }

                                /* check to see if we are done sending packets */
                                struct timespec cur_time;
                                clock_gettime(CLOCK_REALTIME, &cur_time);
                                duration = (cur_time.tv_sec - start_time.tv_sec) +
				            		(cur_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
                                if(duration_stop_at && (duration >= duration_stop_at) ||
                                  num_packets_set && packet_num == n_packets) 
                                    break;

                                /* sleep for intervals, SECOND and MICROSECOND */
                                if(interval) 
                                    sleep(interval);
                                if(uinterval) 
                                    usleep(uinterval);
                                
                            } 

                            /* show the user we are done sending packet(s), deassign the NIC
                               and clear frame protocol  */
                            if(packet_num == 1)
                                printf("%s is done sending packet.\n", sender->name);
                            else
                                printf("%s is done sending %d packets in %lf seconds.\n", sender->name, packet_num, duration);
                            
                            if(loop_back) // && quiet???
                                printf("%s received %d packets back in %lf seconds.\n", sender->name, n_rec, duration);
                            
                            sender->cur_fpcl = 0;
                            status = deassign_nic(sender);

                        } else 
                            printf("NIC Startup failed for device \"%s\".\n", sender->name);

                    } else 
                        printf("Assign NIC failed for device \"%s\".\n", sender->name);
                } 
            } 
        }
    }

    return status;
}

void print_prog_prompt();

void * rcv_thread(void *v)
{


    
    struct rt_info *rti = (struct rt_info*)v;
    nic *receiver = rti->n;
    int duration = rti->dur;
    int prm = rti->prm;
    int n_mca = rti->n_mca;
    


    unsigned short frame_pcl = receiver->cur_fpcl;
    
    //print_nic(receiver);

    //printf("<~~~@-FRAMING BONERS[%s]: %hx %d-@~~~>\n", receiver->name, frame_pcl, n_mca);

    int cnt = 0;
    int receiving; 
    int status;

    status = assign_nic(receiver);
    if($FAIL(status)) {
        printf("Assign nic FAILED.\n");
        goto EXIT_THREAD;
    }

    
    //if(receiver->cur_fpcl == 0) {

            receiver->cur_fpcl = frame_pcl;
            //print_nic(receiver);
            //exit(9);
        //} else {
            //printf("???????\n");
            //exit(9);
        //}
//while(prm == NMA$C_STATE_ON) printf("crying boners");
    status = nic_startup(receiver, prm, n_mca);
    if($FAIL(status)) {
        printf("Nic statup FAILED.\n");
        goto EXIT_THREAD;
    }


    
    time_t start_time = time(0);
    int do_print = 1;
    do {
        if(duration && (time(0) - start_time) >= duration) {
            pthread_mutex_lock(&rtm);
            rthreads_inuse[receiver->id] = 0;
            pthread_mutex_unlock(&rtm);
            //break;
        } else {

            


    //if(content_str)
    //printf("1 COME ON GRANDPA, ENOUGH WITH THE COCK RINGS... \"%s\"\n", content_str);
    //else {
      //  printf("1 COME ON GRANDMA, ENOUGH WITH THE DILDOS... \"%s\"\n", content_str);
    //}

            unsigned char packet_data[MAX_PACKET_SIZE];
            memset(packet_data, 0, MAX_PACKET_SIZE);
            struct p5_param rcv_param;

            status = sys$qiow (
                0,receiver->channel,
                IO$_READVBLK|IO$M_NOW, 
                &qio_iosb_arr[receiver->id],0,0,
                packet_data, 
                MAX_PACKET_SIZE,0,0,
                (__int64)&rcv_param,0 
            );
            
            if($FAIL(status)) {
                fprintf(stderr, "qiow failed uh ohs whaa\n");
                //return NULL;
                printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                        receiver->name, status);
            }
            else 
                status = qio_iosb_arr[receiver->id].w_err; // checking whether received packet or not


            

            if($FAIL(status)) {
                //free(rcv_buffer);
                //free(rcv_param);
                //printf("????????: %d\n", qio_iosb_arr[receiver->id].w_xfer_size);
            } else {

                int bytes_rec = qio_iosb_arr[receiver->id].w_xfer_size;
                
                if(!bytes_rec) {
                    printf("No packet received.\n");
                } else {

                    
                    
                    if( packet_data[0] == ((GEN_HEADER_BYTES >> 24) & 0xff) &&
                        packet_data[1] == ((GEN_HEADER_BYTES >> 16) & 0xff) &&
                        packet_data[2] == ((GEN_HEADER_BYTES >> 8) & 0xff) ) { //&&
                        //(packet_data[3] & 0xf0) == (GEN_PCL_TYPE_USER & 0xf0) ) { 

                        /* if loopback requested */
                        if(packet_data[3] & PACKET_PCL_TYPE_LB) {

                            int reply_data_size = bytes_rec;
                            struct packet_data_info pdi;
                            memset(&pdi, 0, sizeof(struct packet_data_info));
                            //unsigned char packet_data[packet_size];
                            //memset(packet_data, 0, packet_size);
                            unsigned char reply_data[reply_data_size];
                            memset(reply_data, 0, reply_data_size);
                            pdi.packet_data = &reply_data[0];
                            pdi.n = receiver;

                            


                            //if(packet_data[3] & 0xf0 == GEN_PCL_TYPE_USER & 0xf0) {
                              //  show_buffer_bytes("JIGGLING JUNKIES THEY WERE", packet_data, bytes_rec);
                                pdi.packet_pcl = GEN_PPCL; // | GEN_PCL_TYPE_USER;
                            //} else {
                              //  show_buffer_bytes("WIGGLING JUNKIES THEY WERE", packet_data, bytes_rec);
                                //pdi.packet_pcl = GEN_PPCL | GEN_PCL_TYPE_TEST;
                            //}
                            pdi.packet_size = reply_data_size;

                            
                            //char *prev_content_str = content_str;

                            char *content_str = rti->reply_msg;
                            int reply_msg_supplied = strlen(content_str) > 0;
                            if(reply_msg_supplied) {

                                //printf("POOPY BONERS: %d \"%s\"\n", reply_msg_supplied, content_str);
                                pdi.content_str = content_str;

                            } else {

                                //printf("POOPY WEINERS: %d \"%s\"\n", reply_msg_supplied, content_str);
                            

                                //if(!content_str)
                                content_str = ((char*)packet_data) + sizeof(GEN_HEADER_BYTES);
                            
                                //printf("DROOPY WEINERS: \"%s\"\n", content_str);

                                if(strlen(content_str) > 0) 
                                    pdi.content_str = content_str;
                                
                            }


                            

                            get_packet_data(&pdi);

                            //show_buffer_bytes("CLAPPING WEINERS", reply_data, reply_data_size);
                            struct p5_param xmt_param;
                            for(int i = 0; i < 6; i++) {
                                xmt_param.sa[i] = receiver->hwa[i];
                                xmt_param.da[i] = rcv_param.sa[i];
                                //printf("HICCUPING JUNKIES: %X %X\n", xmt_param.sa[i], xmt_param.da[i]);
                            }

                            //if(packet_num == 0 && show_first_packet)
                                    //show_buffer_bytes("Packet Data", packet_data, packet_size);

                            status = sys$qiow (
                                    0,receiver->channel,
                                    IO$_WRITEVBLK, 
                                    &qio_iosb_arr[receiver->id],0,0,
                                    reply_data, 
                                    reply_data_size,0,0,
                                    (__int64)&xmt_param,0 
                            ); 

                            if ($FAIL(status)) {
                                fprintf(stderr, "qiow failed BIG TIMES %x\n", status);
                                break;
                            }
                            

                            //content_str = prev_content_str;
                            
                        }
                    
                    } else if(packet_data[0] == 0x45) {
                        //show_buffer_bytes("EYE PEE VEE FOUR", packet_data, bytes_rec);
                        if(packet_data[1]) {
                            //printf("COPULATING JUNKIES~~~");
                            int reply_data_size = bytes_rec;
                            struct packet_data_info pdi;
                            memset(&pdi, 0, sizeof(struct packet_data_info));
                            //unsigned char packet_data[packet_size];
                            //memset(packet_data, 0, packet_size);
                            unsigned char reply_data[reply_data_size];
                            memset(reply_data, 0, reply_data_size);
                            pdi.packet_data = &reply_data[0];
                            pdi.n = receiver;

                            if(packet_data[9] == 0x11) 
                                pdi.packet_pcl = UDP_PPCL;
                            else if(packet_data[9] == 0x06) 
                                pdi.packet_pcl = TCP_PPCL;
                            else {
                                while(1)printf("COPULATING JUNKIES~~~");
                            }
                            //printf("COPULATING JUNKIES 2~~~");

                            //pdi.packet_pcl = ?; //GEN_PPCL | GEN_PCL_TYPE_USER;
                            pdi.packet_size = reply_data_size;

                            //show_buffer_bytes("COPULATING MONKIES", packet_data, reply_data_size);

                            //char * content_str = ((char*)packet_data) + (sizeof(IPV4_HEADER_LEN) + 1) ;
                            
                            char *content_str = rti->reply_msg; // = content_str;

                            printf("JIGGLING JUNKIES THEY ARE INDEED: \"%s\"\n", content_str);


                            if(!content_str)
                                content_str = ((char*)packet_data) + (IPV4_HEADER_LEN + 1) ;

                            printf("WIGGLING JUNKIES THEY ARE INDEED: \"%s\"\n", content_str);

                            
                            if(strlen(content_str) > 0) 
                                pdi.content_str = content_str;

                            get_packet_data(&pdi);

                            //show_buffer_bytes("CLAPPING WEINERS", reply_data, reply_data_size);
                            struct p5_param xmt_param;
                            for(int i = 0; i < 6; i++) {
                                xmt_param.sa[i] = receiver->hwa[i];
                                xmt_param.da[i] = rcv_param.sa[i];
                                //printf("HICCUPING MONKIES: %X %X\n", xmt_param.sa[i], xmt_param.da[i]);
                            }

                            //if(packet_num == 0 && show_first_packet)
                                    //show_buffer_bytes("Packet Data", packet_data, packet_size);

                            status = sys$qiow (
                                    0,receiver->channel,
                                    IO$_WRITEVBLK, 
                                    &qio_iosb_arr[receiver->id],0,0,
                                    reply_data, 
                                    reply_data_size,0,0,
                                    (__int64)&xmt_param,0 
                            ); 

                            if ($FAIL(status)) {
                                fprintf(stderr, "qiow failed BIG TIMES %x\n", status);
                                break;
                            }
                        }
                    }
                }
            }
            


        //}
        }

        if(((time(0) - start_time) % 30 == 0)) {
            if( do_print ) {
                //printf("(TWERKING JUNKIES \"%s\"~~)\n", receiver->name);
                //printf("<--{%s(%d)} I LOVE to wrap my DICK in beef patties and SCREAM,\n  \
  ~~~COME TO QUIZNO'S!!!~~~>\n", receiver->name, time(0) - start_time);
                //print_prog_prompt();
            }
            //else
            do_print = 0;
        } else if(!do_print)
            do_print = 1;
            
        pthread_mutex_lock(&rtm);
        receiving = rthreads_inuse[receiver->id];
        pthread_mutex_unlock(&rtm);
        
    } while(receiving);
    //

    #if 0
    int sleep_for = 10;
    printf("8~~====+~~> SLEEPING JUNKIES shhhhh.... (%d) 8~~====+~~>\n", sleep_for);
    sleep(sleep_for);
   
    
    printf("EXIT RCV LOOP BYE\n");
    //pthread_exit(NULL);

    pthread_mutex_lock(&rtm);
    rthreads_inuse[receiver->id] = 0;
    pthread_mutex_unlock(&rtm);
    #endif

    EXIT_THREAD:
    //pthread_mutex_lock(&rtm);
    //rthreads_inuse[receiver->id] = 0;
    //if(pthread_join(rec_threads[receiver->id], NULL)) {
      //  printf("JOINING JUNKIES BIG TIMES: %d", receiver->id);
        //exit(9);
    //}
    //pthread_mutex_unlock(&rtm);

    // cv signal all here??

    printf("NIC %s EXITING RCV LOOP BYE\n", receiver->name);
    print_prog_prompt();

    //pthread_cond_signal(&rtcv);
    status = deassign_nic(receiver);
    receiver->cur_fpcl = 0;
    if($FAIL(status)) {
        while(1) printf("WATCHING BONERS~~~\n");
        //return INVALID_INPUT;
    }


    return v;
}


int receive_cmd(struct cmd *cmds[], int n_cmds)
{

    int duration = 0;
    int prm = NMA$C_STATE_OFF;
    int n_mca = 0;

    int status = SS$_NORMAL;

    char *content_str = 0;

    unsigned short frame_pcl = DEF_ETH_FRAME_PCL;
    
    //printf("=====RECEIVE COMMAND================%d\n", n_cmds);
    //int n_cmds = 0;
    for(int i = 0; i < n_cmds; i++) {
        
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        //printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            //printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            char q_lr[100];
            strlower(q_lr, q.name, 100);
            if(str_mmatch("duration", q_lr)) {
                if(q.n_vals != 1) {while(1) printf("@~~~EXPLODING JUNKIES~~~@ ");}
                duration = atoi(q.vals[0]);
                
                //exit(9);
                if(!duration) {
                    while(1) printf("@~~~EXPLODING WEINERS~~~@ ");
                }
                //printf("@~~~CRYING WEINERS: %d~~~@\n", duration);
            } else if(str_mmatch("promiscuous", q_lr) || str_mmatch("prm", q_lr)) {
                if(q.n_vals > 1) {while(1) printf("@~~~EXPLODING MONKEYS~~~@ ");}
                if(q.n_vals > 0) {
                    while(1) printf("@~~~EXPLODING JUNKIES~~~@");
                    prm = atoi(q.vals[q.n_vals]);
                }
                else 
                    prm = NMA$C_STATE_ON;
            } else if(str_mmatch("mca", q_lr)) {
                if(q.n_vals > 1) {while(1) printf("@~~~EXPLODING TRANNIES~~~@ ");}
                if(q.n_vals > 0) {
                    printf("@~~~EXPLODING NANNIES~~~@\n");
                    n_mca = atoi(q.vals[0]);
                    if(n_mca < 1 || n_mca > 20) {
                        printf("\"%s\" is an invalid number of multicast addresses. [1, 20]\n", q.name);
                        return INVALID_INPUT;
                    }
                    //printf("@~~~EXPLODING GRANNIES~~~@ %d\n", n_mca);
                }
                else 
                    n_mca = 1;
            } else if(streq("frame_pcl", q_lr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"RECEIVE /FRAME_PROTOCOL=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    char qv_lr[100];
                    strlower(qv_lr, q_val, 100);
                    unsigned char tmp_fpcl[2];
                    memset(tmp_fpcl, 0, 2);
                    status = parse_frame_pcl(qv_lr, tmp_fpcl);
                    if($FAIL(status)) {
                        printf("\"%s\" is an invalid FRAME PROTOCOL.\n", q_val);
                        return INVALID_INPUT;
                    }
                    frame_pcl = (tmp_fpcl[1] << 8) | tmp_fpcl[0];
                }

            } else if(streq("reply_msg", q_lr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"RECEIVE /REPLY_MSG=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    //char qv_lr[100];
                    //strlower(qv_lr, q_val, 100);
                    printf("FOLDING BONERS TO THE LEFT: %s\n", q_val);

                    q_val++;
                    q_val[strlen(q_val)-1] = 0;
                    //strcpy(content_str, q_val);

                    
                    char tmp_content_str[10000];
                    memset(tmp_content_str, 0, 10000);
                    strcpy(tmp_content_str, q_val);
                    printf("AND THEN TO THE RIGHT: %s\n", tmp_content_str);
                    content_str = tmp_content_str;

                    printf("NOW STRAIGHTEN THEM OUT... %s\n", tmp_content_str);
                    
                    //exit(9);

                    /*
                    unsigned char tmp_fpcl[2];
                    memset(tmp_fpcl, 0, 2);
                    status = parse_frame_pcl(qv_lr, tmp_fpcl);
                    if($FAIL(status)) {
                        printf("\"%s\" is an invalid FRAME PROTOCOL.\n", q_val);
                        return INVALID_INPUT;
                    }
                    frame_pcl = (tmp_fpcl[1] << 8) | tmp_fpcl[0];
                    */
                }
            }else {
                printf("\"%s\" is an invalid RECEIVE COMMAND QUALIFIER.\n", q.name);
                return INVALID_INPUT;
            }
            //for(int k = 0; k < q.n_vals; k++) {
              //  char *q_val = q.vals[k];
                //printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            //}
        }
    }
    //printf("====================================\n\n");
    
    //int status; // = SS$_ABORT;
    

    char *dev_name = cmds[1]->name;
    

    if(!dev_name) {
        printf("Invalid RECEIVE command input. (1)\n");
        return INVALID_INPUT;
    }

    
    //int all_chosen; // = 0;
    char dname_lower[100];
    strlower(dname_lower, dev_name, 100);
    
    int all_chosen = str_mmatch("all", dname_lower);
    int n_recs;
    if(all_chosen) 
        n_recs = n_nics;
    else
        n_recs = 1;
    
    nic *recs[n_recs];
    memset(recs, 0, sizeof(nic*)*n_recs);
    if(all_chosen)
        for(int i = 0; i < n_recs; i++)
            recs[i] = nics[i];
    
    
    
    //printf("~~@~~EXPLODING TRANNIES IN X86~~@~~: \"%s\" %d\n", dname_lower, n_mca);
    //exit(9);

    
    //double duration = 0.0;
    

    if(!all_chosen && strlen(dev_name) < 3) {
        
        printf("\"%s\" is an invalid device name. (1)\n", dev_name);
        return INVALID_INPUT;
    }

    
    

    
    

    //nic *receiver = 0;
    //if(all_chosen)
       // goto REC_ALL;
    for(int i = 0; !recs[0] && i < n_nics; i++) {
        nic *tmp_nic = nics[i];
        char tmp_nic_name_lower[strlen(tmp_nic->name)+1];
        memset(tmp_nic_name_lower, 0, sizeof(tmp_nic_name_lower));
        for(int i = 0; i < strlen(tmp_nic->name); i++)
            tmp_nic_name_lower[i] = tolower(tmp_nic->name[i]);
        if(str_mmatch(tmp_nic_name_lower, dname_lower)) 
            recs[0] = tmp_nic;
    }

    if(!recs[0]) {
        printf("\"%s\" is an unknown device name.\n", dev_name);
        return INVALID_INPUT;
    }
    

    if(n_cmds > 2) {
        printf("Invalid RECEIVE command input. (2)\n");
        return INVALID_INPUT;
    } else {
        for(int i = 0; i < n_recs; i++) {
            nic *receiver = recs[i];

            receiver->cur_fpcl = frame_pcl;
        
            //print_nic(receiver);

            pthread_mutex_lock(&rtm);
            if(rthreads_inuse[receiver->id]) {
                printf("NIC %s is already receiving.\n", receiver->name);
                pthread_mutex_unlock(&rtm);
                return INVALID_INPUT;
            };
            pthread_mutex_unlock(&rtm);

            struct rt_info *rti = malloc(sizeof(struct rt_info));
            memset(rti, 0, sizeof(struct rt_info));
            rti->n = receiver;
            rti->dur = duration;
            rti->prm = prm;
            rti->n_mca = n_mca;
            //rti->reply_msg = 0; //content_str;
            memset(rti->reply_msg, 0, 10000);
            printf("ouch 1\n");
            if(content_str) {
                strcpy(rti->reply_msg, content_str);
            }

            if(pthread_create(&rec_threads[receiver->id], NULL, rcv_thread, rti)) {
                printf("pthread create failed (nics[%d])\n", receiver->id);
                return SS$_ABORT;
            }

            pthread_mutex_lock(&rtm);
            rthreads_inuse[receiver->id] = 1;
            //rthreads_tojoin[i] = 1;
            pthread_mutex_unlock(&rtm);

            //pthread_mutex_lock(&rtjm);
            //rthreads_tojoin[i] = 1;
            //pthread_mutex_unlock(&rtjm);

            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
            printf("~~~Receiving on device %s\n", receiver->name);
                            
            printf("--Frame Protocol: [");
            if((receiver->cur_fpcl >> 8) < 10)
                printf("0");
            printf("%x]-[", receiver->cur_fpcl >> 8);
            if((receiver->cur_fpcl & 0xff) < 10)
                printf("0");
            printf("%x]\n", receiver->cur_fpcl & 0xff); 
            if(n_mca > 1)
                printf("--%d Multicast Addresses enabled.\n", n_mca);  
            else if(n_mca)
                printf("--%d Multicast Address enabled.\n", n_mca);         
            //printf("--NUM PACKETS: %d\n", n_packets);
            
            if(content_str) 
                printf("--Using REPLY TEXT: \"%s\".\n", content_str);
            if(duration)
                printf("--DURATION: %d\n", duration);
            else
                printf("--DURATION=FOREVER\n");

            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");


        //} //else {
            //REC_ALL:
            //printf("CRAWLING JUNKIES[%d]~~~~\n", i);
        }

    }

    return SS$_NORMAL;
}
 


int echo_cmd(struct cmd *cmds[], int n_cmds)
{

    int status;
#define DEF_ECHO_DUR 10
    double total_duration = (double)DEF_ECHO_DUR; //, duration_left = total_duration;

    unsigned short frame_pcl = DEF_ETH_FRAME_PCL;
    int n_mca = 0;
    int echo_full = 0;
    int prm = NMA$C_STATE_OFF;
    int n_echo_nics;

    printf("=====ECHO COMMAND================%d\n", n_cmds);
    //int n_cmds = 0;
    for(int i = 0; i < n_cmds; i++) {
        //printf("ECHOING JUNKIES: %d\n", i);
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            char qlr[100];
            strlower(qlr, q.name, 100);

            char *echo_cmds[] = {"duration", "frame_pcl", "full", "mca", "promiscuous", "prm"};
            int n_matches = 0;
            for(int i = 0; i < sizeof(echo_cmds) / sizeof(echo_cmds[0]); i++) {
                if(str_mmatch(echo_cmds[i], qlr)) {
                    if(n_matches++) {
                        printf("\"%s\" is an ambiguous ECHO command.\n", q.name);
                        return INVALID_INPUT;
                    }
                }
            }

            if(str_mmatch("duration", qlr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"ECHO /DURATION=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    char q_val_lr[100];
                    strlower(q_val_lr, q_val, 100);
                    if(str_mmatch("forever", q_val_lr)) {
                        //printf("GRANDPA'S GETTIN' ANAL FOREVER, YA DIG?\n");
                        total_duration = 0.0;
                    } else {
                        for(int i = 0; q_val[i]; i++) {
                            if(!isdigit(q_val[i])) {
                                printf("%s is an invalid DURATION. (>= 1) (1)\n", q_val);
                                return INVALID_INPUT;
                            }
                        }
                        total_duration = atoi(q_val);
                        if(total_duration < 1) { // || n_packets > MAX_N_PACKETS???) {
                            printf("%s is an invalid DURATION. (>= 1) (2)\n", q_val);
                            return INVALID_INPUT;
                        } 
                    }
                    //if(duration_set++) {
                      //  printf("DURATION duplicate qualifier.\n");
                        //return INVALID_INPUT;
                    //} 
                }
                
            } else if(str_mmatch("promiscuous", qlr) || str_mmatch("prm", qlr)) {
                if(q.n_vals > 1) {while(1) printf("@~~~EXPLODING MONKEYS~~~@ ");}
                if(q.n_vals > 0) {
                    while(1) printf("@~~~EXPLODING JUNKIES~~~@");
                    prm = atoi(q.vals[q.n_vals]);
                }
                else 
                    prm = NMA$C_STATE_ON;
            } else if(str_mmatch("mca", qlr)) {
                if(q.n_vals > 1) {while(1) printf("@~~~EXPLODING TRANNIES~~~@ ");}
                if(q.n_vals > 0) {
                    printf("@~~~EXPLODING NANNIES~~~@\n");
                    n_mca = atoi(q.vals[0]);
                    if(n_mca < 1 || n_mca > 20) {
                        printf("\"%s\" is an invalid number of multicast addresses. [1, 20]\n", q.name);
                        return INVALID_INPUT;
                    }
                    printf("@~~~EXPLODING GRANNIES~~~@ %d\n", n_mca);
                }
                else 
                    n_mca = 1;
            } else if(streq("frame_pcl", qlr)) {
                if(q.n_vals != 1) {
                    printf("Invalid \"ECHO /FRAME_PROTOCOL=\" qualifier input. (only 1 value allowed)\n");    
                    return INVALID_INPUT;
                } else {
                    char *q_val = q.vals[0];
                    char qv_lr[100];
                    strlower(qv_lr, q_val, 100);
                    unsigned char tmp_fpcl[2];
                    memset(tmp_fpcl, 0, 2);
                    status = parse_frame_pcl(qv_lr, tmp_fpcl);
                    if($FAIL(status)) {
                        printf("\"%s\" is an invalid FRAME PROTOCOL.\n", q_val);
                        return INVALID_INPUT;
                    }
                    frame_pcl = (tmp_fpcl[1] << 8) | tmp_fpcl[0];
                }

            } else if(str_mmatch("full", qlr)) {
                if(q.n_vals != 0) {while(1) printf("@~~~EXPLODING SHLONGS~~~@ ");}
                echo_full = 1;
            }else {
                printf("\"%s\" is an invalid ECHO command QUALIFIER.\n", q.name);
                return INVALID_INPUT;
            }
            //for(int k = 0; k < q.n_vals; k++) {
              //  char *q_val = q.vals[k];
                //printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            //}
        }
    }
    printf("====================================\n\n");

    if(n_cmds < 2) {
        printf("invalid ECHO command input. (1)\n");
        return INVALID_INPUT;
    }

    char * enic_name = cmds[1]->name;

    //nic *echo_nic = nics[0];
    char enic_name_lower[100];
    strlower(enic_name_lower, enic_name, 100);
    nic *echo_nics[N_MAX_DEVS]; // = 0;
    memset(echo_nics, 0, sizeof(nic*)*N_MAX_DEVS);

    if(str_mmatch("all", enic_name_lower)) {
        n_echo_nics = n_nics;
        for(int i = 0; i < n_nics; i++) {
            echo_nics[i] = nics[i];
        }
    } else {

        for(int i = 0; !echo_nics[0] && nics[i]; i++) {
            char *nic_name = nics[i]->name;
            char nic_name_lower[100];
            strlower(nic_name_lower, nic_name, 100);
            if(str_mmatch(nic_name_lower, enic_name_lower))
                echo_nics[0] = nics[i];
            n_echo_nics = 1;
        }
    }

    if(!echo_nics[0]) {
        printf("\"%s\" is not a valid ECHO device.\n", enic_name);
        return INVALID_INPUT;
    }

    for(int i = 0; i < n_echo_nics; i++) {

        nic *echo_nic = echo_nics[i];

        status = assign_nic(echo_nic);
        if($FAIL(status)) {
            printf("INHALING MONKIES 1\n");
            return INVALID_INPUT;
        }

    echo_nic->cur_fpcl = frame_pcl;

    status = nic_startup(echo_nic, prm, n_mca);
    if($FAIL(status)) {
        printf("INHALING MONKIES 2\n");
        return INVALID_INPUT;
    }

    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
    printf("~~~Echoing on device %s\n", echo_nic->name);
    if(prm == NMA$C_STATE_ON)
        printf("--PROMISCUOUS MODE ENABLED\n");              
    printf("--Frame Protocol: [");
    if((echo_nic->cur_fpcl >> 8) < 10)
        printf("0");
    printf("%x]-[", echo_nic->cur_fpcl >> 8);
    if((echo_nic->cur_fpcl & 0xff) < 10)
        printf("0");
    printf("%x]\n", echo_nic->cur_fpcl & 0xff);                                       
    //printf("--NUM PACKETS: %d\n", n_packets);
    if(total_duration)
        printf("--DURATION: %d\n", (int)total_duration);
    else
        printf("--DURATION=FOREVER\n");
    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

    
    struct timespec start_time; 
    clock_gettime(CLOCK_REALTIME, &start_time);

    while(1) {


        unsigned char packet_data[MAX_PACKET_SIZE];
        memset(packet_data, 0, MAX_PACKET_SIZE);
        struct p5_param rcv_param;

        status = sys$qiow (
                0,echo_nic->channel,
                IO$_READVBLK|IO$M_NOW, 
                &qio_iosb_arr[echo_nic->id],0,0,
                packet_data, 
                MAX_PACKET_SIZE,0,0,
                (__int64)&rcv_param,0 
        );
            
        if($FAIL(status)) {
            fprintf(stderr, "qiow failed uh ohs whaa whaaaas\n");
                //return NULL;
            printf("~! sys$qio(READVBLK) for device %s failed\n$FAIL(%04X)\n", 
                    echo_nic->name, status);
        }
        else 
            status = qio_iosb_arr[echo_nic->id].w_err; // checking whether received packet or not

            if($FAIL(status)) {
                //free(rcv_buffer);
                //free(rcv_param);
                //printf("????????: %d\n", qio_iosb_arr[receiver->id].w_xfer_size);
                //printf("GRABBING BONERS~~~");
            } else {

                int bytes_rec = qio_iosb_arr[echo_nic->id].w_xfer_size;
                
                if(!bytes_rec) {
                    printf("No packet received.\n");
                } else {

                    unsigned int packet_pcl = 0;
                    const unsigned int tmp_arp_pcl = 0x00000010;
                    unsigned char reply_req_char = 0;

                    if( packet_data[0] == ((GEN_HEADER_BYTES >> 24) & 0xff) &&
                        packet_data[1] == ((GEN_HEADER_BYTES >> 16) & 0xff) &&
                        packet_data[2] == ((GEN_HEADER_BYTES >> 8) & 0xff) ) {
                        //(packet_data[3] & 0xf0) == (GEN_PCL_TYPE_USER & 0xf0) ) {     
                        packet_pcl = GEN_PPCL;
                    } else if(packet_data[0] == 0x00 && packet_data[1] == 0x01 && 
                              bytes_rec == MIN_PACKET_SIZE) {
                        reply_req_char = packet_data[7];
                        packet_pcl = tmp_arp_pcl;
                    } else if(packet_data[0] == 0x45) 
                        packet_pcl = packet_data[9];
                    //else  ??

                    time_t now;
                    time(&now);
                    printf("[-%s - %s/%d bytes/", ctime(&now), echo_nic->name, bytes_rec);
                
                    printf("PPCL"); 
                    if(packet_pcl && !reply_req_char)
                        printf("(%x): " , packet_pcl);
                    else
                        printf(": ");
                    switch(packet_pcl) {
                    case GEN_PPCL:
                        printf("GENERIC]\n");
                        break;
                    case UDP_PPCL >> 8:
                        printf("IPV4/UDP]\n");
                        break;
                    case TCP_PPCL >> 8:
                        printf("IPV4/TCP]\n");
                        break;
                    case tmp_arp_pcl:
                        printf("ARP ");
                        switch(reply_req_char) {
                            case 1:
                                printf("REQUEST]\n");
                                break;
                            case 2:
                                printf("REPLY]\n");
                                break;
                            default:
                                while(1) printf("DROOPING JUNKIES~~~");
                        }
                        break;
                    default:
                        printf("UNKNOWN]\n");
                    }

                    /* if SHOW FULL option chosen, display the packet data (up to 250 bytes) */
                    if(echo_full)
                        show_packet_data("Packet Data", packet_data, bytes_rec < 250 ? bytes_rec : 250);
                
                }
            }
                    
            struct timespec cur_time; 
            clock_gettime(CLOCK_REALTIME, &cur_time);
            if(total_duration && (total_duration - (double)((cur_time.tv_sec - start_time.tv_sec) +
				    (cur_time.tv_nsec - start_time.tv_nsec) / 1000000000.0) <= 0) ) {
                //printf("BREAKING WEENIES\n");
                break;
            }
        }
        status = deassign_nic(echo_nic);
        if($FAIL(status)) {
            printf("INHALING MONKIES 1+2\n");
            return INVALID_INPUT;
        }
    }


 
    return SS$_NORMAL;
}




int stop_cmd(struct cmd *cmds[], int n_cmds)
{
/*
    printf("=====STOP COMMAND================%d\n", n_cmds);
    //int n_cmds = 0;
    for(int i = 0; i < n_cmds; i++) {
        printf("WHINING JUNKIES: %d\n", i);
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            for(int k = 0; k < q.n_vals; k++) {
                char *q_val = q.vals[k];
                printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            }
        }
    }
    printf("====================================\n\n");
*/
    char *stop_cmd_sub = cmds[1]->name;
    //printf("ARMWRESTLING JUNKIES: \"%s\"\n", stop_cmd_sub);
    char stop_cmd_sub_lr[100];
    strlower(stop_cmd_sub_lr, stop_cmd_sub, 100);

    char *stop_cmd_sub_sub = cmds[1]->name;
    //printf("ARMWRESTLING JUNKIES: \"%s\"\n", stop_cmd_sub);
    char stop_cmd_sub_sub_lr[100];
    strlower(stop_cmd_sub_sub_lr, cmds[2]->name, 100);
    printf("ARMWRESTLING JUNKIES: \"%s\"\n", stop_cmd_sub_sub_lr);
    int n_stop_nics;
    if(n_cmds < 3) {
        printf("invalid stop command input. (1)\n");
        return SS$_ABORT;
    } else if(!str_mmatch("all", stop_cmd_sub_sub_lr) && strlen(cmds[2]->name) < 3) {
        printf("\"%s\" is an invalid nic name. (1)\n", cmds[2]->name);
        return SS$_ABORT;
    } 

    

    nic *stop_nics[N_MAX_DEVS];
    memset(stop_nics, 0, sizeof(nic*)*N_MAX_DEVS);

    if(str_mmatch("all", stop_cmd_sub_sub_lr)) {
        n_stop_nics = n_nics;
        for(int i = 0; i < n_nics; i++) {
            stop_nics[i] = nics[i]; 
        }
    } else {
        
        n_stop_nics = 1;
        char nic_name_lr[100];
        strlower(nic_name_lr, cmds[2]->name, 100);
        for(int i = 0; i < n_nics && !stop_nics[0]; i++) {
            //printf("WHINING BONERS BIG TIMES[%d] \"%s\" \"%s\"\n", 
                    //i, nics[i]->name, cmds[2]->name);
            char sys_nic_lr[100];
            strlower(sys_nic_lr, nics[i]->name, 100);
            if(str_mmatch(sys_nic_lr, nic_name_lr)) {
                //printf("GRUNTING WEINERS: \"%s\"\n", nics[i]->name);
                //exit(9);
                stop_nics[0] = nics[i];
            }
        }
        if(!stop_nics[0]) {
            printf("\"%s\" is an invalid nic name. (2)\n", cmds[2]->name);
            return SS$_ABORT;
        }
    }

    

    
    if(str_mmatch("receive", stop_cmd_sub_lr)) {
        //printf("ARM-WRASSLIN JUNKIES: \"%s\"\n", stop_cmd_sub_lr);
        for(int i = 0; i < n_stop_nics; i++) {
        
            nic *stop_nic = stop_nics[i];
        //print_nic(stop_nic);
        //printf("GRUNTING BONERS: \"%s\"\n", stop_nic->name);
        //exit(9);

        printf("Stopping NIC %s from receiving...\n", stop_nic->name);
        int receiving = 0;
        pthread_mutex_lock(&rtm);
        receiving = rthreads_inuse[stop_nic->id];
        if(receiving) 
            rthreads_inuse[stop_nic->id] = 0;
        pthread_mutex_unlock(&rtm);
        if(!receiving)
            printf("  (device %s was not receiving)\n", stop_nic->name);
        }

        
    } else {
        printf("\"%s\" is an invalid stop sub command.\n", stop_cmd_sub);
        return SS$_ABORT;
    }


    return SS$_NORMAL;

}

/**
 * ADD_CMD:
 * -------------
 */
int add_cmd(struct cmd *cmds[], int n_cmds)
{
    #if 0
    printf("=====ADD COMMAND================\n");
    for(int i = 0; i < n_cmds; i++) {
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            for(int k = 0; k < q.n_vals; k++) {
                char *q_val = q.vals[k];
                printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            }
        }
    }
    printf("=====ADD COMMAND COMPLETE=========\n");
    #endif


    int status = SS$_NORMAL;

    //int n_cmds;
    //for(n_cmds = 0; cmds[n_cmds]; n_cmds++);

    if(n_cmds == 1) {
        //printf("HUGE BLUMPKIN ERROR\n");
        return INVALID_INPUT;
    }

    char *cmd_name = cmds[1]->name;
    //printf("~~~JAMMING JUNKIES~~~\"%s\"\n", cmd_name);
    char cmd_name_lr[100];
    strlower(cmd_name_lr, cmd_name, 100);
    if(str_mmatch("target", cmd_name_lr)) {
        //printf("~~~JAMMING BONERS~~~\"%s\"\n", cmd_name_lr);
        if(n_cmds != 4) {
            
            if(n_cmds == 3) {
                char frump_lower[100];
                strlower(frump_lower, cmds[2]->name, 100);
                if(str_mmatch("system", frump_lower)) {
                    //printf("OVERGROWING FRUMPS %s\n", frump_lower);
                    //exit(9);
                    add_system_targets();
                    return SS$_NORMAL;
                } else if(str_mmatch("mca", frump_lower)) {
                    //printf("OVERGROWING JUNKIES %s\n", frump_lower);
                    //exit(9);
                    add_mca_targets();
                    return SS$_NORMAL;
                }
            }
            
            //printf("HUGE BLUMPKIN ERROR 2\n");
            return INVALID_INPUT;
        }
        char *usr_added_target_name = cmds[2]->name;
        //printf("~~~JA JA JAMMING BONERS~~~\"%s\"\n", usr_added_target_name);
        char *uatn_mac = cmds[3]->name;
        unsigned char huge_frump_dumpster[6] = {0};
        status = get_mac(huge_frump_dumpster, uatn_mac);
        if($FAIL(status)) {
            printf("%s is an invalid target mac address (XX-XX-XX-XX-XX-XX)\n", cmds[3]->name);
            return INVALID_INPUT;
        }
        //printf("~~~RA RA RAMMING BONERS~~~\"%d\"\n", status);
        if(n_targets == N_MAX_TARGETS) {
            printf("~~~STU STU STAMMERING BONERS 1~~~\"%d\"\n", n_targets);
            return INVALID_INPUT;
        }
        int in_sys = 0;
        for(int i = 0; i < n_nics; i++) {
            unsigned char *n_mac = nics[i]->hwa;
            int j;
            for(j = 0; j < 6; j++) {
                if(n_mac[j] != huge_frump_dumpster[j])
                    break;
            }
            if(j == 6) {
                in_sys = 1;
                break;
            }
        }

        /* make sure that there isn't already a target with the same name */
        char usr_added_target_name_lower[100];
        strlower(usr_added_target_name_lower, usr_added_target_name, 100);
        //printf("SHOWING BONERS BIG TIME: \"%s\"\n", usr_added_target_name_lower);
        //exit(9);

        for(int i = 0; i < n_targets; i++) {
            char target_name_lower[100];
            strlower(target_name_lower, targets[i].name, 100);
            
            if(streq(usr_added_target_name_lower, target_name_lower)) {
                printf("target \"%s\" already exists.\n", usr_added_target_name);
                return INVALID_INPUT;
            }
        }

        targets[n_targets].type = USR_ADDED_TARTYPE;
        strcpy(targets[n_targets].name, usr_added_target_name);
        for(int i = 0; i < 6; i++) {
            targets[n_targets].mac[i] = huge_frump_dumpster[i];
            //printf("HUGE WINKING GRANNIES: %x\n", targets[n_targets].mac[i]);
        }
        n_targets++;
    } else {
        printf("\"%s\" is an invalid ADD SUBCOMMAND.\n", cmd_name);
        return INVALID_INPUT;
    }


    return status;
}


int help_cmd(struct cmd *cmds[], int n_cmds)
{
    printf("=====HELP COMMAND================\n");
    for(int i = 0; i < n_cmds; i++) {
        char *cmd_name = cmds[i]->name;
        int n_quals = cmds[i]->n_quals;
        printf("COMMAND (CMD VAL)[%d]: \"%s\" %d\n", i, cmd_name, n_quals);
        for(int j = 0; j < n_quals; j++) {
            struct qual q = cmds[i]->quals[j];
            printf("\tQUALIFIER: (N VALS) \"%s\" %d\n", q.name, q.n_vals);
            for(int k = 0; k < q.n_vals; k++) {
                char *q_val = q.vals[k];
                printf("\t\tQUALIFIER VALUE[%d]: \"%s\"\n", k, q_val);
            }
        }
    }
    printf("=================================\n\n");

    //for(int i = 0; i < 10; i++)
      //  printf("^^^^^^^-=- GULPING BONERS -=-^^^^^^^\n");


    int n_il_cmds = sizeof(il_cmds) / sizeof(il_cmds[0]);
    printf("-Program Commands: ");
    for(int i = 0; i < n_il_cmds; i++) {
        printf("%s", il_cmds[i]);
        if(i < n_il_cmds-1)
            printf(", ");
    }
    printf("\n--rerun HELP command with any of the above available commands\n  (ex. HELP SEND ...)\n");


    return SS$_NORMAL;

}



/**
 * SHOW_CMD:
 * -------------
 */
int show_cmd(struct cmd *cmds[], int n_cmds)
{
   
    char *show_cmd_val;
    int status = SS$_NORMAL;


    /* determine that there is a valid SUB COMMAND */
    if(n_cmds < 2) {
        printf("Invalid SHOW command input. (1)\n");
        return INVALID_INPUT;
    }
    show_cmd_val = cmds[1]->name;
    char show_cmd_val_lower[100];
    memset(show_cmd_val_lower, 0, 100);
    for(int i = 0; show_cmd_val[i]; i++)
        show_cmd_val_lower[i] = tolower(show_cmd_val[i]);

    char *valid_show_cmds[] = { "system", "device", "targets", "mca" }; //, "devices" };
    int valid_show_cmd = 0;
    int n_matches = 0;
    char *show_cmd_val_full;
    for(int i = 0; i < sizeof(valid_show_cmds)/sizeof(valid_show_cmds[0]); i++) {
        if(str_mmatch(valid_show_cmds[i], show_cmd_val_lower)) {
            n_matches++;
            show_cmd_val_full = valid_show_cmds[i];
        }
    }
    
    if(!n_matches) {
        printf("\"%s\" is an invalid SHOW SUB COMMAND.\n", show_cmd_val);
        status = INVALID_INPUT;
    } else if(n_matches > 1) {
        printf("\"%s\" is an ambiguous SHOW SUB COMMAND.\n", show_cmd_val);
        status = INVALID_INPUT;
    }

    if($SUCCESS(status)) {
        if(streq("mca", show_cmd_val_full)) {
            printf("SHOW MCA command currently UNIMPLEMENTED, using MCA ADDR[0].\n");
            print_mac(MCA_ADDRS[0]);
        }

        /* SHOW SYSTEM COMMAND */
        else if(streq("system", show_cmd_val_full)) {

            if(n_cmds != 2) {
                printf("Invalid SHOW SYSTEM COMMAND input. (1)\n");
                status = INVALID_INPUT;
            } else {
                /* parse qualifier subtree (at this point) */
                int full_mode = 0;
                for(int i = 0; i < 2; i++) {
                    char *cmd_name = cmds[i]->name;
                    int n_quals = cmds[i]->n_quals;
                    for(int j = 0; j < n_quals; j++) {
                        struct qual q = cmds[i]->quals[j];
                        char qname_lower[100];
                        memset(qname_lower, 0, 100);
                        for(int i = 0; q.name[i]; i++)
                            qname_lower[i] = tolower(q.name[i]);
                        if(str_mmatch("full", qname_lower)) {
                            if(full_mode) { /* if super verbose, printout ... */ }
                            full_mode = 1;
                        } else {
                            printf("\"%s\" is an invalid SHOW SYSTEM COMMAND QUALIFIER.\n", qname_lower);
                            status = INVALID_INPUT;
                            break;
                        }
                    }
                }
                if($SUCCESS(status)) {
                    /* get a current timestamp */
                    time_t now;
                    time(&now);
                    printf("-%s", ctime(&now));
                    if(!full_mode) {
                        print_dev_names();
                        print_assignable_nics();
                    }
                    else {
                        printf("SHOW SYSTEM /FULL option currently UNIMPLEMENTED: \'%d\"\n", n_sys_dev_names);
                        //status = UNIMPLEMENTED;
                        print_dev_names();
                        print_assignable_nics();
                    }
                }

            }
        
        /* SHOW DEVICE COMMAND */
        } else if(streq("device", show_cmd_val_full)) {
            if(n_cmds != 3) {
                printf("Invalid SHOW DEVICE COMMAND input. (1)\n");
                status = INVALID_INPUT;
            } else {
                /* parse qualifier subtree (at this point) */
                int full_mode = 0;
                for(int i = 0; i < 3; i++) {
                    char *cmd_name = cmds[i]->name;
                    int n_quals = cmds[i]->n_quals;
                    for(int j = 0; j < n_quals; j++) {
                        struct qual q = cmds[i]->quals[j];
                        char qname_lower[100];
                        memset(qname_lower, 0, 100);
                        for(int i = 0; q.name[i]; i++)
                            qname_lower[i] = tolower(q.name[i]);
                        if(str_mmatch("full", qname_lower)) {
                            if(full_mode) { /* if super verbose, printout ... */ }
                            full_mode = 1;
                            printf("SHOW DEVICE /FULL option currently UNIMPLEMENTED: \'%d\"\n", n_sys_dev_names);
                            //status = UNIMPLEMENTED;
                            //break;
                        } else {
                            printf("\"%s\" is an invalid SHOW DEVICE COMMAND QUALIFIER.\n", qname_lower);
                            status = INVALID_INPUT;
                            break;
                        }
                    }
                }
                if($SUCCESS(status)) {
                    char *dev_name = cmds[2]->name;
                    char dname_lower[100];
                    strlower(dname_lower, dev_name, 100);
                    if(str_mmatch("list", dname_lower)) 
                        print_dev_names();
                    else if(str_mmatch("receiving", dname_lower)) {

                        for(int i = 0; i < n_nics; i++) {
                            int receiving;
                            pthread_mutex_lock(&rtm);
                            receiving = rthreads_inuse[i];
                            pthread_mutex_unlock(&rtm);
                            if(receiving)
                                printf("Device %s/%hx is currently receiving.\n", nics[i]->name, nics[i]->cur_fpcl);
                        }
                    
                    } else {
                        int found_dev = 0;
                        for(int i = 0; i < n_nics; i++) {
                            nic *tmp_nic = nics[i];
                            char *tmp_nic_name = tmp_nic->name;
                            if(strlen(dname_lower) == 1) {
                                if(dname_lower[0] == tolower(tmp_nic_name[0])) {
                                    found_dev = 1;
                                    print_nic(tmp_nic);
                                }
                            } else {
                                char tmp_nic_name_lower[strlen(tmp_nic_name)+1];
                                memset(tmp_nic_name_lower, 0, sizeof(tmp_nic_name_lower));
                                for(int i = 0; i < strlen(tmp_nic_name); i++)
                                    tmp_nic_name_lower[i] = tolower(tmp_nic_name[i]);
                                if(str_mmatch(tmp_nic_name_lower, dname_lower)) {
                                    print_nic(tmp_nic);
                                    found_dev = 1;
                                }
                            }
                        }
                        if(!found_dev) 
                            printf("No devices matching \"%s\" found in the system.\n", dev_name);
                    }
                }
            }
        /* SHOW TARGET(S) COMMAND */
        } else if(streq("targets", show_cmd_val_full)) {
            int show_sys_only = 0, show_no_sys = 0;

            for(int i = 0; i < n_cmds; i++) {
                char *cmd_name = cmds[i]->name;
                int n_quals = cmds[i]->n_quals;
                for(int j = 0; j < n_quals; j++) {
                    struct qual q = cmds[i]->quals[j];
                    char qname_lower[100];
                    memset(qname_lower, 0, 100);
                    for(int i = 0; q.name[i]; i++)
                        qname_lower[i] = tolower(q.name[i]);
                    if(str_mmatch("system", qname_lower)) {
                        if(q.n_vals > 1) {
                            printf("Invalid \"SHOW /TARGET(S)=\" qualifier input. (only 0/1 value allowed)\n");    
                            return INVALID_INPUT;
                        } else {
                            if(q.n_vals == 0) 
                                show_sys_only = 1;
                            else {
                                char *q_val = q.vals[0];
                                char q_val_lower[100];
                                strlower(q_val_lower, q_val, 100);
                                if(str_mmatch("yes", q_val_lower)) 
                                    show_sys_only = 1;
                                else if(str_mmatch("no", q_val_lower)) 
                                    show_no_sys = 1;
                                else {
                                    printf("\"%s\" is an invalid SHOW TARGET(S) QUALIFIER.\n", q_val);
                                    return INVALID_INPUT;
                                }   
                            }
                        }
                    } else {
                        printf("\"%s\" is an invalid SHOW TARGET(S) COMMAND QUALIFIER.\n", qname_lower);
                        return INVALID_INPUT;
                    }
                }
            }

            char usr_tname_lower[100];
            switch(n_cmds) {
                /* show all targets */
                case 2:
                    if(n_targets == 0)
                        printf("No targets exist.\n");
                    //else
                      //  printf("%d Targets:\n", n_targets);
                    for(int i = 0; i < n_targets; i++) {
                        int show = 1;
                        if(show_sys_only && targets[i].type != SYS_TARTYPE)
                            show = 0;
                        else if(show_no_sys && targets[i].type == SYS_TARTYPE)
                            show = 0;
                        if(show)
                            print_target(&targets[i]);
                    }
                    break;
                /* show specified target */
                case 3:
                    strlower(usr_tname_lower, cmds[2]->name, 100);
                    int found_target = 0;
                    struct target *t;
                    for(int i = 0; i < n_targets; i++) {
                        char tar_name_lower[100];
                        strlower(tar_name_lower, targets[i].name, 100);
                        if(streq(usr_tname_lower, tar_name_lower)) {
                            int show = 1;
                            if((show_sys_only && targets[i].type != SYS_TARTYPE) ||
                               (show_no_sys && targets[i].type == SYS_TARTYPE))
                                show = 0;
                            if(show) 
                                found_target = 1;
                            t = &targets[i];
                            break;
                        }
                    }
                    if(!found_target) 
                        printf("target \"%s\" doesn't exist.\n", cmds[2]->name);
                    else
                        print_target(t);
                    break;
                default:
                    printf("invalid SHOW TARGET(S) input. (1)\n");
                    return INVALID_INPUT;
            } 
       
            }
        }
 
    return status;
}

void print_prog_prompt() {
    char *prompt = "NIC-TEST> ";
    printf("%s", prompt);
}

void stop_rec_threads() {
    for(int i = 0; i < n_nics; i++) {
        pthread_mutex_lock(&rtm);
        if(rthreads_inuse[i]) 
            rthreads_inuse[i] = 0;
        pthread_mutex_unlock(&rtm);
    }
}


/**
 * INPUT_LOOP:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Parse commands from the user until QUIT, EXIT or CTRL/C. 
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	Status
 *
 * CALLER:
 *	main
 */
int input_loop()
{
    
    int status = SS$_NORMAL;
    const int max_input_len = 1000;
    char input[max_input_len];
    char modified_input[max_input_len];
    char *input_ptr; /* omit potential space prefix */
    int cmds_index; 
    struct cmd *cmds[100];

    if(pthread_mutex_init(&rtm, NULL)) {
        printf("pthread_mutex_init FAILED\n");
        status = SS$_ABORT;
        return status;
    }

    if(pthread_cond_init(&rtcv, NULL)) {
        printf("pthread_cond_init FAILED\n");
        status = SS$_ABORT;
        return status;
    }

    while(1) { 

        LOOP_START:

        memset(&cmds, 0, sizeof(cmds));
        memset(input, 0, max_input_len);
        memset(modified_input, 0, max_input_len);
        cmds_index = 0;

        print_prog_prompt();
        
        fgets(input, max_input_len, stdin);
        input[strlen(input)-1] = '\0';
        status = modify_input(input, modified_input);
        if($FAIL(status)) 
            continue;

        /* Only proceed if there is parsable text from the user. */ 
        if(strlen(modified_input) > 0) {
            /* Allow DCL commands with '$' prefix. */
            if(modified_input[0] == '$') {
                system(modified_input + 1);
                continue;
            }

            int tmp_str_index = 0;
            char tmp_str[1000];
            memset(tmp_str, 0, 1000);

            for(int i = 0; modified_input[i]; i++) {
                char c = modified_input[i];
                if(is_space(c)) 
                    goto GATHER_TOKEN;
                else 
                    tmp_str[tmp_str_index++] = c;
                if(i == strlen(modified_input)-1) {
                    GATHER_TOKEN:
                    if(tmp_str[0] == '=') {
                        int open_paren = 0, open_quote = 0;;
                        /* remove the leading '=' from the qualifier value */
                        tmp_str_index--;
                        for(i++; modified_input[i]; i++) {
                            char c = modified_input[i];
                            if(c == '(') {
                                while(1)
                                printf("GURGLING JABRONIES~~~\"%s\" \"%s\" ", modified_input, modified_input + i);
                                exit(9);
                            } else if(c == '"') {
                                if(!open_quote)
                                    open_quote = 1;
                                else
                                    open_quote = 0;
                            } else if(!open_quote && (is_space(c) || i == strlen(modified_input)-1)) {
                                if(!is_space(c))
                                    tmp_str[tmp_str_index++] = c;
                                break;
                            }
                            tmp_str[tmp_str_index++] = c;
                        }

                        if(streq(tmp_str, "=")) {
                            printf("need to provide value for qualifier. (%s)\n", tmp_str);
                            goto LOOP_START;
                        }
                        
                        struct cmd *tmp_command = cmds[cmds_index-1];
                        if(!tmp_command) {
                            printf("\"%s\" is an invalid command.\n", tmp_str);
                            goto LOOP_START;
                        }
                        struct qual *q = &tmp_command->quals[tmp_command->n_quals-1];
                        q->vals[q->n_vals] = malloc(1000);
                        memset(q->vals[q->n_vals], 0, 1000);
                        strcpy(q->vals[q->n_vals++], tmp_str);
                        memset(tmp_str, 0, 1000);
                        tmp_str_index = 0;
                        continue;
                        
                    }

                    if(tmp_str[0] == '(') {
                        int qindex = 0;
                        char qual_val[1000];
                        memset(qual_val, 0, 1000);
                        int paren_level = 0;
                        for(; tmp_str[i]; i++) {
                            c = tmp_str[i];
                            while(1)
                                printf("GURGLING JABRONIES TOO~~~\"%c\" \"%c\" ", c, c);    
                            exit(9);
                            switch(c) {
                            case '(': 
                                paren_level++;
                                break;
                            case ')': 
                                if(--paren_level == 0) {

                                } else {}
                                break;
                            case ',':
                                while(1)
                                printf("GURGLING JABRONIES TOO PLUS WON~~~ ");
                                exit(9);
                                break;
                            default:
                                qual_val[qindex++] = c;
                                break;
                            }
                
                        }
                       
                    }
                    
                    if(tmp_str[0] == '/') {

                        if(cmds_index == 0) {
                            printf("Invalid qualifier input.\n");
                            goto LOOP_START;
                        }

                        struct cmd * tmp_command = cmds[cmds_index-1];

                        /* remove trailing '/' from qualifier */
                        for(int i = 0; tmp_str[i]; i++) {
                            if(i < strlen(tmp_str)-1)
                                tmp_str[i] = tmp_str[i+1];
                            else
                                tmp_str[i] = 0;
                        }
                        
                        strcpy(tmp_command->quals[tmp_command->n_quals++].name, tmp_str);
                        memset(tmp_str, 0, 1000);
                        tmp_str_index = 0;
                        //cmds_index++; ???
                        
                    } else {

                        cmds[cmds_index] = malloc(sizeof(struct cmd));
                        strcpy(cmds[cmds_index]->name, tmp_str);
                        memset(tmp_str, 0, 1000);
                        tmp_str_index = 0;
                        cmds_index++;
                    }
                }
            }

            
            char first_cmd_tolower[100]; 
            strlower(first_cmd_tolower, cmds[0]->name, 100);
            
            /* now process the command */
            char *vcmd = NULL; 
            int valid = 0;
            for(int i = 0; i < sizeof(il_cmds)/sizeof(il_cmds[0]); i++) {
                char *arr_cmd = (char*)il_cmds[i];
                if(str_mmatch((char*)arr_cmd, first_cmd_tolower)) {
                    if(valid++) {
                        printf("\"%s\" is an ambiguous command.\n", cmds[0]->name);
                        goto LOOP_START;
                    }
                    vcmd = arr_cmd;
                }
            }

            if(!vcmd) 
                printf("\"%s\" is an invalid command.\n", cmds[0]->name);
            else {

                if(str_mmatch("exit", first_cmd_tolower) || str_mmatch("quit", first_cmd_tolower)) { 
                    printf("Exiting, good bye.\n");
                    status = 1;
                    break;
                }
                else if(str_mmatch("help", first_cmd_tolower)) 
                    status = help_cmd(cmds, cmds_index); 
                else if(str_mmatch("show", first_cmd_tolower)) 
                    status = show_cmd(cmds, cmds_index); 
                else if(str_mmatch("add", first_cmd_tolower)) 
                    status = add_cmd(cmds, cmds_index); 
                else if(str_mmatch("remove", first_cmd_tolower)) {
                    while(1) printf("WIGGLING BONERS 8-@-@-@=> ");
                } 
                else if(str_mmatch("send", first_cmd_tolower)) 
                    status = send_cmd(cmds, cmds_index); 
                else if(str_mmatch("burst", first_cmd_tolower)) 
                    status = burst_cmd(cmds, cmds_index); 
                else if(str_mmatch("test", first_cmd_tolower)) 
                    status = test_cmd(cmds, cmds_index); 
                else if(str_mmatch("performance", first_cmd_tolower)) {
                    while(1) printf("GIGGLING BONERS 8-@-@-@=> ");
                }   
                else if(str_mmatch("echo", first_cmd_tolower)) 
                    status = echo_cmd(cmds, cmds_index);
                else if(str_mmatch("receive", first_cmd_tolower)) 
                    status = receive_cmd(cmds, cmds_index);
                else if(str_mmatch("stop", first_cmd_tolower)) 
                    status = stop_cmd(cmds, cmds_index); 
    
                if($FAIL(status)) 
                    printf("  (ERROR status=%x)\n", status);

            }
        }
    }

    stop_rec_threads();

    printf("!#!- EXIT INPUT LOOP -!#!\n");

    return status;
}

int process_args(int argc, char *argv[])
{

    printf("PROCESS ARGS UNIMPLEMENTED.\n");
    return SS$_ABORT;

    enum {
        PROC_FLAG = -1, PROC_VAL, PROC_DONE
    } PARSE_STATE;
    int parse_state; 
    int status; 
    int nic_set = 0, target_set = 0, rcv_set = 0, num_set = 0;
    int n_packets = 1;
    const char *n_pkt_str_def_val = "1";
    const char *target_mac_all_str = "all";
    const char *rcv_mode = "RECEIVE_MODE";
    unsigned char target_mac[6] = { 0 };
    char *flag;
    char *nic_name = 0;
    char *n_packets_str = (char*)n_pkt_str_def_val;
    char *target_mac_str = 0; 
    

    status = SS$_NORMAL;
    parse_state = PROC_FLAG;
    int rec_nic_index = -1;
    
    for(int i = 1; (i < argc) && $SUCCESS(status); i++) {
        char *arg = argv[i];

        /* if parsing the flag (-<t(oken)>) */
        if(parse_state) {
            if(arg[0] != '-') {
                printf("invalid arguments. (TODO: usage 1)\n");
                status = SS$_ABORT;
            } else {
                parse_state = PROC_FLAG;
                flag = arg + 1;
                char flag_lower[100];
                strlower(flag_lower, flag, 100);
                char *flag_values[] = { "nics", "number", "transmit", "receive" };
                int n_matches = 0;
                for(int i = 0; $SUCCESS(status) && i < sizeof(flag_values)/sizeof(flag_values[0]); i++) {
                    if(str_mmatch(flag_values[i], flag_lower) && n_matches++) {
                        printf("SNUGGLING JUNKIES ALL DAY: \"%s\" \"%s\"\n", flag_values[i], flag);
                        printf("\"%s\" is an ambiguous program argument flag.\n", flag);
                        status = SS$_ABORT;
                    }
                }

                if($SUCCESS(status)) {
                    if(str_mmatch("transmit", flag)) {
                        printf("==TRANSMIT==\n");
                        if(target_set++) {
                            printf("invalid arguments. (TODO: usage 2)\n");
                            status = SS$_ABORT;
                        } else if(rcv_set++) {
                            printf("invalid arguments. (TODO: usage 2.1)\n");
                            status = SS$_ABORT;
                        } else 
                            parse_state = PROC_VAL;
                    } else if(str_mmatch("nic", flag)) {
                        printf("==NIC==\n");
                        if(nic_set++) {
                            printf("invalid arguments. (TODO: usage 3)\n");
                            status = SS$_ABORT;
                        } else 
                            parse_state = PROC_VAL;
                    } else if(str_mmatch("number", flag)) {
                        printf("==NUMBER (packets)==\n");
                        if(num_set++) {
                            printf("invalid arguments. (TODO: usage 4)\n");
                            status = SS$_ABORT;
                        } else 
                            parse_state = PROC_VAL;
                    } else if(str_mmatch("receive", flag)) {
                        printf("==RECEIVE==\n");
                        if(rcv_set++) {
                            printf("invalid arguments. (TODO: usage 4.1)\n");
                            status = SS$_ABORT;
                        } else if(target_set){
                            printf("invalid arguments. (TODO: usage 4.2)\n");
                            status = SS$_ABORT;
                        } else {
                            // ...
                            parse_state = PROC_VAL;
                        }
                    } else {
                        printf("invalid arguments. (TODO: usage 5)\n");
                        status = SS$_ABORT;
                    }
                }
            }

        /* else if parsing the argument value */
        } else {

            char flag_lower[100];
            strlower(flag_lower, flag, 100);
            if(str_mmatch("nic", flag_lower)) {
                char arg_lower[100];
                strlower(arg_lower, arg, 100);
                if(strlen(arg_lower) < 3) {
                    if(str_mmatch("all", arg_lower)) {
                        nic_name = arg_lower;
                        parse_state = PROC_DONE;
                    } else {
                        printf("\"%s\" is an invalid NIC name (1)\n", arg);
                        status = SS$_ABORT;
                    }
                } else {
                    
                    for(int i = 0; i < n_nics && !nic_name; i++) {
                        char nic_name_lower[100];
                        strlower(nic_name_lower, nics[i]->name, 100);
                        if(str_mmatch(nic_name_lower, arg_lower)) 
                            nic_name = nics[i]->name;
                    }
                    if(!nic_name) {
                        if(str_mmatch("all", arg_lower)) {
                            nic_name = arg_lower;
                            parse_state = PROC_DONE;
                        } else {
                            printf("\"%s\" is an invalid NIC name (2)\n", arg);
                            status = SS$_ABORT;
                        }
                    } else
                        parse_state = PROC_DONE;
                }

            } else if(str_mmatch("target", flag_lower)) {
                status = get_mac(target_mac, arg);
                if($FAIL(status)) {
                    char arg_lower[100];
                    strlower(arg_lower, arg, 100);
                    if(str_mmatch("all", arg_lower)) {
                        target_mac_str = (char*)target_mac_all_str;
                        parse_state = PROC_DONE;
                        status = SS$_NORMAL;
                    } else {
                        printf("\"%s\" is an invalid MAC address. (XX-XX-XX-XX-XX-XX)\n", arg);
                        status = SS$_ABORT;
                    }
                } else {
                    print_mac(target_mac);
                    target_mac_str = arg;
                    parse_state = PROC_DONE;
                }
                
            } else if(str_mmatch("number", flag_lower)) {
                
                n_packets = atoi(arg);
                n_packets_str = arg;
                if(n_packets < 1 || n_packets > 2400000000) {
                    printf("\"%s\" is an invalid NUM PACKETS value.", arg);
                    status = SS$_ABORT;
                } else 
                    parse_state = PROC_DONE;
                
            } else if(str_mmatch("receive", flag_lower)) {
                char arg_lower[100];
                strlower(arg_lower, arg, 100);
                for(int i = 0; i < n_nics && !nic_name; i++) {
                    char nic_name_lower[100];
                    strlower(nic_name_lower, nics[i]->name, 100);
                    if(str_mmatch(nic_name_lower, arg_lower)) {
                        nic_name = nics[i]->name;
                        rec_nic_index = i;
                    }
                }
                if(!nic_name) {
                    if(str_mmatch("all", arg_lower)) {
                        printf("GROVELING JABRONIES 22\n");
                        exit(9);
                        nic_name = arg_lower;
                        parse_state = PROC_DONE;
                        target_mac_str = (char*)rcv_mode;
                    } else {
                        printf("\"%s\" is an invalid NIC name (2)\n", arg);
                        status = SS$_ABORT;
                    }
                } else {
                    target_mac_str = (char*)rcv_mode;
                    parse_state = PROC_DONE;
                }

            }
        }
    }

    if($SUCCESS(status) && parse_state != PROC_DONE) {
        printf("invalid arguments. (TODO: usage 6)\n");
        status = SS$_ABORT;
    } else if(!nic_name || !target_mac_str) {
        printf("invalid arguments. (TODO: usage 7)\n");
        status = SS$_ABORT;
    } else if($SUCCESS(status)) {

        printf("--nic name %s\n", nic_name);

        if(streq((char*)rcv_mode, target_mac_str)) {
            printf("---RECEIVE MODE\n");
            // printf("--num packets: %d\n", n_packets); ????
            // ...
            struct cmd cmd1;
            memset(&cmd1, 0, sizeof(struct cmd));
            strcpy(cmd1.name, "receive");

            struct cmd cmd2;
            memset(&cmd2, 0, sizeof(struct cmd));
            char nic_name_lower[100];
            strlower(nic_name_lower, nic_name, 100);
            strcpy(cmd2.name, nic_name_lower);

            struct cmd * cmds[3];
            cmds[0] = &cmd1;
            cmds[1] = &cmd2;
            
            status = receive_cmd(cmds, 2);

            if(pthread_join(rec_threads[rec_nic_index], NULL)) {
                printf("pthread join FAILED: (nics[%d])\n", rec_nic_index);
                status = SS$_ABORT;
            }
            

        } else {
            printf("---TRANSMIT MODE\n");
            printf("--target mac: %s\n", target_mac_str);
            printf("--num packets: %d\n", n_packets);

            struct cmd cmd1;
            memset(&cmd1, 0, sizeof(struct cmd));
            strcpy(cmd1.name, "send");

            struct cmd cmd2;
            memset(&cmd2, 0, sizeof(struct cmd));
            char nic_name_lower[100];
            strlower(nic_name_lower, nic_name, 100);
            strcpy(cmd2.name, nic_name_lower);

            struct cmd cmd3;
            memset(&cmd3, 0, sizeof(struct cmd));
            strcpy(cmd3.name, target_mac_str);
            strcpy(cmd3.quals[cmd3.n_quals].name, "number"); 
            cmd3.quals[cmd3.n_quals].vals[0] = malloc(1000);
            strcpy(cmd3.quals[0].vals[0], n_packets_str); 
            cmd3.quals[0].n_vals++;
            cmd3.n_quals++;
            struct cmd * cmds[3];
            cmds[0] = &cmd1;
            cmds[1] = &cmd2;
            cmds[2] = &cmd3;
            status = send_cmd(cmds, 3);
        }
    }

    return status;
}       


/**
 * GET_ASSIGNABLE_NICS:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Find assignable NICs in the system that can be configured. This routine
 *  contains code and notation directly from [LAN]LANCP*.*. 
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	status
 *
 * CALLER:
 *	main
 */

int get_assignable_nics() {

    int status;
    char tmp_buffer[BUFFER_LEN];
    memset(tmp_buffer, 0, BUFFER_LEN);

    for(int dnum = 0; dnum < n_sys_dev_names; dnum++) { 

        int qio_chan; 
        int param;
        int plen;
        int i;
        
        int qio_desc[2];
        struct iosb qio_iosb;  
        
        nic * next_nic; 
        char *dname = sys_dev_names[dnum];
        struct dev d = { 4, 0, dname };

        memset(&qio_iosb, 0, sizeof(qio_iosb));

        status = sys$assign(
                            &d, // devname - device number 
                            (unsigned short int *)&qio_chan,   // chan - channel number 
                            0,       // access mode(0=kernal,...)
                            0        // mailbox name(0=default,...)
                        ); 
	    if ($FAIL(status)) {
            printf("(ERROR:status= %x)\n", status);
            continue;
        }
        
        i = 0;
	    qio_desc[0] = sizeof(tmp_buffer);			/* Reset buffer length */
	    qio_desc[1] = (int)&tmp_buffer[0];			/* Set buffer address */
	    status = sys$qiow(0, (unsigned short int)qio_chan,	/* Channel */
			  IO$_SENSEMODE | IO$M_CTRL,		/* Function */
			  (struct _iosb *)&qio_iosb, 0, 0, 0,	/* IOSB */
			  (__int64)&qio_desc, 0, 0, 0, 0);	/* Sensemode buffer descriptor */
	    if ($SUCCESS(status))
		    status = qio_iosb.w_err;
	    if ($SUCCESS(status)) {
		
            while ((i < qio_iosb.w_xfer_size)) {

		        param = *(UNALIGNED_SHORTP)&tmp_buffer[i];	/* Get next parameter in buffer */
		        i += 2;						/* Skip past the parameter */

		        /*
		        * Determine the length of the value of the parameter in the buffer.  Bit 12 tells
		        * us if it's a string parameter (bit 12 set) or not (bit 12 clear).  The size of
		        * non-string parameters is 4 bytes.  The size of string parameters is in the buffer
		        * in the word following the parameter.
		        */
		        if (param & 0x1000) {
			        plen = *(UNALIGNED_SHORTP)&tmp_buffer[i]; /* Get string length */
			        i += 2;					/* Skip past the length */
                } else 
			        plen = 4;				/* Integer, length is 4 */
            
                switch(param & 0xfff) {
            
                case NMA$C_PCLI_HWA:
                next_nic = malloc(sizeof(nic));
                next_nic->id = n_nics;
                strcpy(next_nic->name, dname);
                nics[n_nics++] = next_nic;

                memcpy(&next_nic->hwa, &tmp_buffer[i], 6);
                for(int i = 0; i < 6; i++) {

#if VERBOSE_FULL_DEBUG_MODE
                    print_mac(next_nic->hwa);
#endif
                
                }
                break;
                }
			    i += plen;
            }

            i = 0;
            memset(tmp_buffer, 0, sizeof(tmp_buffer));
            status = SYS$QIOW(0, (unsigned short int)qio_chan,	/* Channel */
		                        IO$_SENSEMODE | IO$M_CTRL | IO$M_SENSE_MAC,	/* Function */
			                    (struct _iosb *)&qio_iosb, 0, 0, 0,	/* IOSB */
			                    (__int64)&qio_desc, 0, 0, 0, 0);	/* Sensemode buffer descriptor */
	        if ($SUCCESS(status))
		        status = qio_iosb.w_err;
	        if ($SUCCESS(status)) {
                while ((i < qio_iosb.w_xfer_size)) {
			        param = *(UNALIGNED_SHORTP)&tmp_buffer[i]; /* Get next parameter in buffer */
			        i += 2;					/* Skip past the parameter */
			        /*
			        * Determine the length of the value of the parameter in the buffer.  Bit 12 tells
			        * us if it's a string parameter (bit 12 set) or not (bit 12 clear).  The size of
			        * non-string parameters is 4 bytes.  The size of string parameters is in the buffer
			        * in the word following the parameter.
			        */
			        if (param & 0x1000) {
				        plen = *(UNALIGNED_SHORTP)&tmp_buffer[i]; /* Get string length */
				        i += 2;					/* Skip past the length */
			        } else
				        plen = 4;				/* Integer, length is 4 */
			        if ((param & 0xFFF) == NMA$C_PCLI_LINESPEED) 
                        next_nic->line_speed = *(UNALIGNED_INTP)&tmp_buffer[i];
			        else if ((param & 0xFFF) == NMA$C_PCLI_LINKSTATE) 
                        next_nic->link_state = *(UNALIGNED_INTP)&tmp_buffer[i];
                    i += plen;
		        }
            } else 
                printf("(ERROR:status 1=%x %s)\n", status, dname);  
        } else 
            printf("(ERROR:status 2=%x %s)\n", status, dname);
        sys$dassgn(qio_chan);
    }
    return status;
}


/**
 * GET_DEV_NAMES:
 * -------------
 *
 * FUNCTIONAL DESCRIPTION:
 *	Communicate with LANCP to get NIC info from the system.
 *
 * INPUTS:
 *	None
 *
 * OUTPUTS:
 *	status
 *
 * CALLER:
 *	main
 */

int get_dev_names() 
{
    int status;
    int req_name_len = 0; 
    char req_name[NODENAME_LEN];				/* Requested device/node name string */
    char devscan_name[DEVNAME_LEN];				/* Device scan search name */
    char dev_devnam[DEVNAME_LEN];	
 
    uint64 devscan_context;					/* Device scan context */
    uint16 len = 4;	// ??				/* Device name length */
    uint32 dev_class = DC$_SCOM;			/* Search for all SCOM devices */
	DEV_ITEM dev_items[] = {			/* Item list describing device type to search for */
		{4, DVS$_DEVCLASS, (uint32)&dev_class, 0},
		{0, 0, 0, 0}
	};

    memset(devscan_name, 0, DEVNAME_LEN);
    memset(dev_devnam, 0, DEVNAME_LEN);
    memset(req_name, 0, NODENAME_LEN);
	
    struct dsc$descriptor_s search_name = {6, DSC$K_DTYPE_T, DSC$K_CLASS_S, (char *)&devscan_name[0]};
	struct dsc$descriptor_s ret_desc = {DEVNAME_LEN, DSC$K_DTYPE_T, DSC$K_CLASS_S, (char *)&dev_devnam[0]};

	dev_devnam[MED] = '%';
	dev_devnam[CNTR] = '%';
	dev_devnam[CNUM] = '%';
	dev_devnam[UNIT] = '0';

	memcpy(&dev_devnam[0], &req_name[0], req_name_len);
    memcpy(&devscan_name[1], dev_devnam, 4);
    devscan_name[0] = '_';
	devscan_name[5] = ':';
	devscan_context = 0;

    while(1) { 

        status = sys$device_scan(&ret_desc, (unsigned short int *)&len, &search_name, &dev_items,
					 (struct _generic_64 *)&devscan_context);
        
        if ($FAIL(status)) {
		    if (status == SS$_NOMOREDEV) 
                break;
            printf("ERROR: status=%x\n", status);
		    return SS$_ABORT; 
	    }

        /* Massage the device name, to zero terminate and strip off leading _ and trailing : */
		dev_devnam[len] = 0;
		if (dev_devnam[len - 1] == ':')
			dev_devnam[--len] = 0;
		if (dev_devnam[0] == '_')
			memcpy(dev_devnam, dev_devnam + 1, len--);
            
        strcpy(sys_dev_names[n_sys_dev_names++], dev_devnam);
    }

    return SS$_NORMAL;
}


/**
 * MAIN:
 * -------------
 */

int main(int argc, char*argv[]) 
{
    
    int status;
    int verbose_mode = 1;

    n_sys_dev_names = 0;
    memset(sys_dev_names, 0, N_MAX_DEVS*5);
    memset(&nics, 0, sizeof(nics));
    memset(&targets, 0, sizeof(targets));
    n_targets = 0;

    /* collect the device names so we can gather info on each one */
    status = get_dev_names();
    if ($FAIL(status)) {
        printf("ERROR: status=%x\n", status);
		return status;
	}

    /* establish assignable nics and extract relevant NIC info */
    status = get_assignable_nics();
    if ($FAIL(status) && n_nics == 0) {
        print_dev_names();
        printf("No assignable NICS found.\n");
		return status;
	}

    /* process arguments (if any) */
    if(argc == 1) {

        /* display info about the NICS within the system */
        print_program_header();
        if(verbose_mode) {
            printf("\n");
            print_dev_names();
            //printf("\n");
            //print_assignable_nics();
        }
    
        /* enter the input loop to receive commands from the user */
        status = input_loop();

    } else 
        status = process_args(argc, argv);

    printf("---END NIC-TEST MAIN\n");
    return status;

}

