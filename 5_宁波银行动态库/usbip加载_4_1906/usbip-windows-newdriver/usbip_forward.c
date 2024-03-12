#include "usbip_forward.h"
#include <winsock2.h>
#include <winsock.h>
#pragma comment(lib,"Ws2_32.lib")




#define BUFREAD_P(devbuf)	((devbuf)->offp - (devbuf)->offhdr)
#define BUFREADMAX_P(devbuf)	((devbuf)->bufmaxp - (devbuf)->offp)
#define BUFREMAIN_C(devbuf)	((devbuf)->bufmaxc - (devbuf)->offc)
#define BUFHDR_P(devbuf)	((devbuf)->bufp + (devbuf)->offhdr)
#define BUFCUR_P(devbuf)	((devbuf)->bufp + (devbuf)->offp)
#define BUFCUR_C(devbuf)	((devbuf)->bufc + (devbuf)->offc)

typedef struct
{
	int buff_len;
	char buff[10*1024];
}MERGE_BUFF_T;

#define MERGE_BUFF_MAX_NUM 5

typedef struct _devbuf {
	//int sock;//new add
	const char	*desc;
	BOOL	is_req, swap_req;
	BOOL	invalid;
	/* asynchronous read is in progress */
	BOOL	in_reading;
	/* asynchronous write is in progress */
	BOOL	in_writing;
	/* step 1: reading header, 2: reading data */
	int	step_reading;
	HANDLE	hdev;
	char	*bufp, *bufc;	/* bufp: producer生产者, bufc: consumer消费者 */
	DWORD	offhdr;		/* header offset for producer */
	DWORD	offp, offc;	/* offp: producer offset, offc: consumer offset */
	DWORD	bufmaxp, bufmaxc;
	struct _devbuf	*peer;
	OVERLAPPED	ovs[2];
	/* completion event for read or write */
	HANDLE	hEvent;
	int count;
	int start_first;
	int need_auto_reply;
	char last_cmd_data[64];
	int time_out_count;
	int interrupt_flag;
	int end_flag;

	int get_decp_flag;
	int get_decp_len;

	int trans_type;
	MERGE_BUFF_T merge_buff[MERGE_BUFF_MAX_NUM];

} devbuf_t;

/*
* Two devbuf's are shared via hEvent, which indicates read or write completion.
* Such a global variable does not pose a severe limitation.
* Because userspace binaries(usbip.exe, usbipd.exe) have only a single usbip_forward().
*/
//static HANDLE	hEvent;

#ifdef DEBUG_PDU
#undef USING_STDOUT

static void
dbg_to_file(char *fmt, ...)
{
	FILE	*fp;
	va_list ap;

#ifdef USING_STDOUT
	fp = stdout;
#else
	if (fopen_s(&fp, "debug_pdu.log", "a+") != 0)
		return;
#endif
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
#ifndef USING_STDOUT
	fclose(fp);
#endif
}

static const char *
dbg_usbip_hdr_cmd(unsigned int cmd)
{
	switch (cmd) {
	case USBIP_CMD_SUBMIT:
		return "CMD_SUBMIT";
	case USBIP_RET_SUBMIT:
		return "RET_SUBMIT";
	case USBIP_CMD_UNLINK:
		return "CMD_UNLINK";
	case USBIP_RET_UNLINK:
		return "RET_UNLINK";
	default:
		return "UNKNOWN";
	}
}

static void
dump_iso_pkts(struct usbip_header *hdr)
{
	struct usbip_iso_packet_descriptor	*iso_desc;
	int	n_pkts;
	int	i;

	switch (hdr->base.command) {
	case USBIP_CMD_SUBMIT:
		n_pkts = hdr->u.cmd_submit.number_of_packets;
		if (hdr->base.direction)
			iso_desc = (struct usbip_iso_packet_descriptor *)(hdr + 1);
		else
			iso_desc = (struct usbip_iso_packet_descriptor *)((char *)(hdr + 1) + hdr->u.cmd_submit.transfer_buffer_length);
		break;
	case USBIP_RET_SUBMIT:
		n_pkts = hdr->u.ret_submit.number_of_packets;
		iso_desc = (struct usbip_iso_packet_descriptor *)((char *)(hdr + 1) + hdr->u.ret_submit.actual_length);
		break;
	default:
		return;
	}

	for (i = 0; i < n_pkts; i++) {
		dbg_to_file("  o:%d,l:%d,al:%d,st:%d\n", iso_desc->offset, iso_desc->length, iso_desc->actual_length, iso_desc->status);
		iso_desc++;
	}
}

static void
dump_usbip_header(struct usbip_header *hdr)
{
	dbg_to_file("DUMP: %s,seq:%u,devid:%x,dir:%s,ep:%x\n",
		dbg_usbip_hdr_cmd(hdr->base.command), hdr->base.seqnum, hdr->base.devid, hdr->base.direction ? "in" : "out", hdr->base.ep);

	switch (hdr->base.command) {
	case USBIP_CMD_SUBMIT:
		dbg_to_file("  flags:%x,len:%x,sf:%x,#p:%x,intv:%x\n",
			hdr->u.cmd_submit.transfer_flags,
			hdr->u.cmd_submit.transfer_buffer_length,
			hdr->u.cmd_submit.start_frame,
			hdr->u.cmd_submit.number_of_packets,
			hdr->u.cmd_submit.interval);
		dbg_to_file("  setup: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n",
			hdr->u.cmd_submit.setup[0], hdr->u.cmd_submit.setup[1], hdr->u.cmd_submit.setup[2],
			hdr->u.cmd_submit.setup[3], hdr->u.cmd_submit.setup[4], hdr->u.cmd_submit.setup[5],
			hdr->u.cmd_submit.setup[6], hdr->u.cmd_submit.setup[7]);
		dump_iso_pkts(hdr);
		break;
	case USBIP_CMD_UNLINK:
		dbg_to_file("  seq:%x\n", hdr->u.cmd_unlink.seqnum);
		break;
	case USBIP_RET_SUBMIT:
		dbg_to_file("  st:%d,al:%d,sf:%d,#p:%d,ec:%d\n",
			hdr->u.ret_submit.status,
			hdr->u.ret_submit.actual_length,
			hdr->u.ret_submit.start_frame,
			hdr->u.cmd_submit.number_of_packets,
			hdr->u.ret_submit.error_count);
		dump_iso_pkts(hdr);
		break;
	case USBIP_RET_UNLINK:
		dbg_to_file(" st:%d\n", hdr->u.ret_unlink.status);
		break;
	default:
		/* NOT REACHED */
		break;
	}
	dbg_to_file("DUMP DONE-------\n");
}

#define DBGF(fmt, ...)		dbg_to_file(fmt, ## __VA_ARGS__)
#define DBG_USBIP_HEADER(hdr)	dump_usbip_header(hdr)

#else

#define DBGF(fmt, ...)
#define DBG_USBIP_HEADER(hdr)

#endif
#define OUT_Q_LEN 256
static void
swap_usbip_header_base_endian(struct usbip_header_basic *base)
{
	base->command = htonl(base->command);
	base->seqnum = htonl(base->seqnum);
	base->devid = htonl(base->devid);
	base->direction = htonl(base->direction);
	base->ep = htonl(base->ep);
}

static void
swap_cmd_submit_endian(struct usbip_header_cmd_submit *pdu)
{
	pdu->transfer_flags = ntohl(pdu->transfer_flags);
	pdu->transfer_buffer_length = ntohl(pdu->transfer_buffer_length);
	pdu->start_frame = ntohl(pdu->start_frame);
	pdu->number_of_packets = ntohl(pdu->number_of_packets);
	pdu->interval = ntohl(pdu->interval);
}

static void
swap_ret_submit_endian(struct usbip_header_ret_submit *pdu)
{
	pdu->status = ntohl(pdu->status);
	pdu->actual_length = ntohl(pdu->actual_length);
	pdu->start_frame = ntohl(pdu->start_frame);
	pdu->number_of_packets = ntohl(pdu->number_of_packets);
	pdu->error_count = ntohl(pdu->error_count);
}

static void
swap_cmd_unlink_endian(struct usbip_header_cmd_unlink *pdu)
{
	pdu->seqnum = ntohl(pdu->seqnum);
}

static void
swap_ret_unlink_endian(struct usbip_header_ret_unlink *pdu)
{
	pdu->status = ntohl(pdu->status);
}

static void
swap_usbip_header_cmd(unsigned int cmd, struct usbip_header *hdr)
{
	switch (cmd) {
	case USBIP_CMD_SUBMIT:
		swap_cmd_submit_endian(&hdr->u.cmd_submit);
		break;
	case USBIP_RET_SUBMIT:
		swap_ret_submit_endian(&hdr->u.ret_submit);
		break;
	case USBIP_CMD_UNLINK:
		swap_cmd_unlink_endian(&hdr->u.cmd_unlink);
		break;
	case USBIP_RET_UNLINK:
		swap_ret_unlink_endian(&hdr->u.ret_unlink);
		break;
	default:
		/* NOTREACHED */
		err("unknown command in pdu header: %d", cmd);
		break;
	}
}

static void
swap_usbip_header_endian(struct usbip_header *hdr, BOOL from_swapped)
{
	unsigned int	cmd;

	if (from_swapped) {
		swap_usbip_header_base_endian(&hdr->base);
		cmd = hdr->base.command;
	}
	else {
		cmd = hdr->base.command;
		swap_usbip_header_base_endian(&hdr->base);
	}
	swap_usbip_header_cmd(cmd, hdr);
}

static void
swap_iso_descs_endian(char *buf, int num)
{
	struct usbip_iso_packet_descriptor	*ip_desc;
	int i;

	ip_desc = (struct usbip_iso_packet_descriptor *)buf;
	for (i = 0; i < num; i++) {
		ip_desc->offset = ntohl(ip_desc->offset);
		ip_desc->status = ntohl(ip_desc->status);
		ip_desc->length = ntohl(ip_desc->length);
		ip_desc->actual_length = ntohl(ip_desc->actual_length);
		ip_desc++;
	}
}



static BOOL
record_outq_seqnum(unsigned long seqnum, long *out_q_seqnum_array)
{
	int	i;
	//out("record_outq_seqnum addr :%08x\n", out_q_seqnum_array);
	for (i = 0; i < OUT_Q_LEN; i++) {
		int	found_empty_slot;

		/* record_outq_seqnum can be called multiple times.
		* seqnum should be checked if it was already marked.
		*/
		if (out_q_seqnum_array[i] == seqnum)
			return TRUE;
		if (out_q_seqnum_array[i])
			continue;
		found_empty_slot = i;
		for (; i < OUT_Q_LEN; i++) {
			if (out_q_seqnum_array[i] == seqnum)
				return TRUE;
		}
		out_q_seqnum_array[found_empty_slot] = seqnum;
		return TRUE;
	}
	return FALSE;
}

static BOOL
is_outq_seqnum(unsigned long seqnum, long *out_q_seqnum_array)
{
	int	i;
	for (i = 0; i < OUT_Q_LEN; i++) {
		if (out_q_seqnum_array[i] != seqnum)
			continue;
		out_q_seqnum_array[i] = 0;
		return TRUE;
	}
	return FALSE;
}

static int
get_xfer_len(BOOL is_req, struct usbip_header *hdr, long *out_q_seqnum_array)
{
	if (is_req) {
		if (hdr->base.command == USBIP_CMD_UNLINK)
			return 0;
		if (hdr->base.direction)
			return 0;
		if (!record_outq_seqnum(hdr->base.seqnum, out_q_seqnum_array)) {
			err("failed to record. out queue full");
		}
		return hdr->u.cmd_submit.transfer_buffer_length;
	}
	else {
		if (hdr->base.command == USBIP_RET_UNLINK)
			return 0;
		if (is_outq_seqnum(hdr->base.seqnum, out_q_seqnum_array))
			return 0;
		return hdr->u.ret_submit.actual_length;
	}
}

static int
get_iso_len(BOOL is_req, struct usbip_header *hdr)
{
	if (is_req) {
		if (hdr->base.command == USBIP_CMD_UNLINK)
			return 0;
		return hdr->u.cmd_submit.number_of_packets * sizeof(struct usbip_iso_packet_descriptor);
	}
	else {
		if (hdr->base.command == USBIP_RET_UNLINK)
			return 0;
		return hdr->u.ret_submit.number_of_packets * sizeof(struct usbip_iso_packet_descriptor);
	}
}

static BOOL
setup_rw_overlapped(devbuf_t *buff)
{
	int	i;

	for (i = 0; i < 2; i++) {
		memset(&buff->ovs[i], 0, sizeof(OVERLAPPED));
		buff->ovs[i].hEvent = (HANDLE)buff;
	}
	return TRUE;
}

static BOOL
init_devbuf(devbuf_t *buff, const char *desc, BOOL is_req, BOOL swap_req, HANDLE hdev, HANDLE hEvent)
{
	buff->bufp = (char *)malloc(1024);
	if (buff->bufp == NULL)
		return FALSE;
	buff->bufc = buff->bufp;
	buff->desc = desc;
	buff->is_req = is_req;
	buff->swap_req = swap_req;
	buff->in_reading = FALSE;
	buff->in_writing = FALSE;
	buff->invalid = FALSE;
	buff->step_reading = 0;
	buff->offhdr = 0;
	buff->offp = 0;
	buff->offc = 0;
	buff->bufmaxp = 1024;
	buff->bufmaxc = 0;
	buff->hdev = hdev;
	buff->hEvent = hEvent;
	if (!setup_rw_overlapped(buff)) {
		free(buff->bufp);
		return FALSE;
	}
	return TRUE;
}

static void
cleanup_devbuf(devbuf_t *buff)
{
	free(buff->bufp);
	if (buff->bufp != buff->bufc)
		free(buff->bufc);
}

static VOID CALLBACK
read_completion(DWORD errcode, DWORD nread, LPOVERLAPPED lpOverlapped)
{
	devbuf_t	*rbuff;

	rbuff = (devbuf_t *)lpOverlapped->hEvent;
	if (errcode == 0) {
		rbuff->offp += nread;
		if (nread == 0)
			rbuff->invalid = TRUE;
	}
	rbuff->in_reading = FALSE;
	SetEvent(rbuff->hEvent);
}

static BOOL
read_devbuf(devbuf_t *rbuff, DWORD nreq)
{
	if (BUFREADMAX_P(rbuff) < nreq) {
		char	*bufnew;

		if (rbuff->bufp != rbuff->bufc) {
			/* reallocation is allowed only if producer and consumer use their own buffers */
			DWORD	nmore = nreq - BUFREADMAX_P(rbuff);

			bufnew = (char *)realloc(rbuff->bufp, rbuff->bufmaxp + nmore);
			if (bufnew == NULL) {
				err("%s: failed to reallocate buffer: %s", __FUNCTION__, rbuff->desc);
				return FALSE;
			}
			rbuff->bufp = bufnew;
			rbuff->bufmaxp += nmore;
		}
		else {
			DWORD	nexist = BUFREAD_P(rbuff);

			bufnew = (char *)malloc(nreq + nexist);
			if (bufnew == NULL) {
				err("%s: failed to allocate buffer: %s", __FUNCTION__, rbuff->desc);
				return FALSE;
			}
			if (nexist > 0) {
				/* copy from already read usbip header */
				memcpy(bufnew, BUFHDR_P(rbuff), nexist);
			}
			rbuff->bufp = bufnew;
			rbuff->offhdr = 0;
			rbuff->offp = nexist;
			rbuff->bufmaxp = nreq + nexist;
		}
	}

	if (!rbuff->in_reading) {
		if (!ReadFileEx(rbuff->hdev, BUFCUR_P(rbuff), nreq, &rbuff->ovs[0], read_completion)) {
			DWORD error = GetLastError();
			err("%s: failed to read: err: 0x%lx", __FUNCTION__, error);
			if (error == ERROR_NETNAME_DELETED) {
				err("%s: could the client have dropped the connection?", __FUNCTION__);
			}
			return FALSE;
		}
		rbuff->in_reading = TRUE;
	}
	return TRUE;
}

static VOID CALLBACK
write_completion(DWORD errcode, DWORD nwrite, LPOVERLAPPED lpOverlapped)
{
	devbuf_t	*wbuff, *rbuff;

	wbuff = (devbuf_t*)lpOverlapped->hEvent;
	wbuff->in_writing = FALSE;

	SetEvent(wbuff->hEvent);

	if (errcode != 0)
		return;

	if (nwrite == 0) {
		wbuff->invalid = TRUE;
		return;
	}
	rbuff = wbuff->peer;
	rbuff->offc += nwrite;
}

static VOID CALLBACK
my_write_completion(DWORD errcode, DWORD nwrite, LPOVERLAPPED lpOverlapped)
{
	devbuf_t	*wbuff, *rbuff;

	wbuff = (devbuf_t*)lpOverlapped->hEvent;
	wbuff->in_writing = FALSE;

	 SetEvent(wbuff->hEvent);

	if (errcode != 0)
		return;

	if (nwrite == 0) {
		wbuff->invalid = TRUE;
		return;
	}
	// rbuff = wbuff->peer;
	// rbuff->offc += nwrite;
}

//调整usbip的包头
int adjust_usbip_head(char *src,char *dec,int len)
{
	int i = 0,j = 0;
	if(len != 48)
		return -1;
	memcpy(dec,src,48);
	for(i = 0;i<7;++i)
	{
		for(j = 0;j<4;++j)
		{
			dec[i*4 + j] = src[(i*4) + (3 - j)];//*(BUFCUR_C(rbuff) + (i*4) + (3 - j));
		}
	}
	dec[0] = 0x03;
	dec[20] = 0x00;
	return 0;
}

//判断是否是test包
int handle_transform_data(devbuf_t *wbuff,devbuf_t *rbuff,int drection)
{
	char data[128] = "need attached again";
	char heart_data[64] = "this is test heart data";
	char tmp[100*1024] = {0x00},tmp1[32] = {0x00};
	devbuf_t *tmp_buff = NULL;
	int usbip_drection = 0;
	char response_data[100*1024] = {0x00};
			  //数据条数，  数据长度
	int i = 0,j = 0,data_num = 0,data_len = 0;
	// if(BUFREMAIN_C(rbuff) == 79 && drection == 0 && 0 == memcmp((BUFCUR_C(rbuff) + 48),"USBC",4) &&(*(BUFCUR_C(rbuff) + 48 + 12) == 0x00 && *(BUFCUR_C(rbuff) + 48 + 13) == 0x00 && *(BUFCUR_C(rbuff) + 48 + 14) == 0x06 && *(BUFCUR_C(rbuff) + 48 + 15) == 0x00))
	// {
	// 	WriteLogFile("0 == memcmp((BUFCUR_C(rbuff) + 48),\"USBC\",4)\n");
	// 	rbuff->need_auto_reply = 1;
	// 	memset(rbuff->last_cmd_data,0x00,sizeof(rbuff->last_cmd_data));
	// 	memcpy(rbuff->last_cmd_data,BUFCUR_C(rbuff) + 48,13);
	// 	for(i = 0;i<sizeof(rbuff->merge_buff)/sizeof(MERGE_BUFF_T);++i)  //找到可用的缓冲buff
	// 	{
	// 		if(rbuff->merge_buff[i].buff_len <= 0)
	// 			break;
	// 	}
	// 	if(i < sizeof(rbuff->merge_buff)/sizeof(MERGE_BUFF_T))   //往缓冲buff里填充数据
	// 	{
	// 		rbuff->merge_buff[i].buff_len = 79;
	// 		memcpy(rbuff->merge_buff[i].buff,BUFCUR_C(rbuff),rbuff->merge_buff[i].buff_len);
	// 	}
	// 	adjust_usbip_head(BUFCUR_C(rbuff),response_data,48);
	// 	while (rbuff->in_writing)
	// 	{
	// 		WriteLogFile("rbuff->in_writing = %d",rbuff->in_writing);
	// 		Sleep(1000);
	// 	}
	// 	if (!WriteFileEx(rbuff->hdev, response_data, 48, &rbuff->ovs[1], my_write_completion)) {
	// 		err("%s: failed to write sock: err: 0x%lx", __FUNCTION__, GetLastError());
	// 		return FALSE;
	// 	}
	// 	rbuff->in_writing = TRUE;
	// 	wbuff->in_writing = FALSE;
	// 	SetEvent(wbuff->hEvent);
	// 	tmp_buff = wbuff->peer;
	// 	tmp_buff->offc += 79;
	// 	return 1;
	// }
	// else if(rbuff->need_auto_reply == 1 && BUFREMAIN_C(rbuff) == 48 && drection == 0)
	// {
	// 	WriteLogFile("if(rbuff->need_auto_reply == 1 && BUFREMAIN_C(rbuff) == 48 && drection == 0)\n");
	// 	rbuff->need_auto_reply = 0;
	// 	memset(response_data,0x00,sizeof(response_data));
	// 	adjust_usbip_head(BUFCUR_C(rbuff),response_data,48);
	// 	memcpy(&response_data[48],rbuff->last_cmd_data,13);
	// 	response_data[51] = 0x53;
	// 	memset(rbuff->last_cmd_data,0x00,sizeof(rbuff->last_cmd_data));
	// 	while (rbuff->in_writing)
	// 	{
	// 		WriteLogFile("rbuff->in_writing = %d",rbuff->in_writing);
	// 		Sleep(1000);
	// 	}
	// 	if (!WriteFileEx(rbuff->hdev, response_data, 48+13, &rbuff->ovs[1], my_write_completion)) {
	// 		err("%s: failed to write sock: err: 0x%lx", __FUNCTION__, GetLastError());
	// 		return FALSE;
	// 	}
	// 	rbuff->in_writing = TRUE;
	// 	wbuff->in_writing = FALSE;
	// 	SetEvent(wbuff->hEvent);
	// 	tmp_buff = wbuff->peer;
	// 	tmp_buff->offc += 48;
	// 	return 1;
	// }
	// else    //处理普通包
	{
		if(drection == 1)
		{
			for(i = 0;i<BUFREMAIN_C(rbuff);++i)
			{
				// WriteLogFile("%.02x",rbuff->bufp[i]);
				if(memcmp(BUFCUR_C(rbuff) + i,"need attached again",strlen("need attached again")) == 0)
				{
					// WriteLogFile("read_write_dev 1111111111111111111111111111111111111");
					return -2;
				}
			}
		}

#if 1	
		if(drection == 0 && 0 != *(BUFCUR_C(rbuff) + 19) && rbuff->trans_type == 0x02 )//从pc到机柜的包,并且不是传输控制,只有块传输做此处理  && rbuff->trans_type == 0x02
		{
			 //WriteLogFile("enter drection == 0 && 0 != *(BUFCUR_C(rbuff) + 19),BUFREMAIN_C(rbuff) = %d\n",BUFREMAIN_C(rbuff));

			// memset(tmp,0x00,sizeof(tmp));
			// for(i = 0;i<BUFREMAIN_C(rbuff);++i)
			// {
			// 	memset(tmp1,0x00,sizeof(tmp1));
			// 	sprintf(tmp1,"%02x ",*(BUFCUR_C(rbuff) + i));
			// 	strcat(tmp,tmp1);
			// 	// WriteLogFile("%x ",*(BUFCUR_C(rbuff) + i));
			// }
			// WriteLogFile("buff = %s",tmp);

			usbip_drection = *(BUFCUR_C(rbuff) + 15);
			//  WriteLogFile("usbip_drection = %d",usbip_drection);
			if(usbip_drection == 0 && *(BUFCUR_C(rbuff) + 3) == 0x01)  //out方向，先主动回复，将数据写进缓存buff
			{
				for(i = 0;i<MERGE_BUFF_MAX_NUM;++i)  //找到可用的缓冲buff
				{
					if(rbuff->merge_buff[i].buff_len <= 0)
						break;
				}
				// WriteLogFile("i = %d",i);
				if(i < MERGE_BUFF_MAX_NUM)   //往缓冲buff里填充数据
				{
					rbuff->merge_buff[i].buff_len = BUFREMAIN_C(rbuff);
					// WriteLogFile("------------------buff_len = %d",rbuff->merge_buff[i].buff_len);
					memcpy(rbuff->merge_buff[i].buff,BUFCUR_C(rbuff),rbuff->merge_buff[i].buff_len);
				}
				else
				{
					WriteLogFile("------------------i = %d",i);
				}
				memset(response_data,0x00,sizeof(response_data));
				adjust_usbip_head(BUFCUR_C(rbuff),response_data,48);

				// memset(tmp,0x00,sizeof(tmp));
				// for(i = 0;i<48;++i)
				// {
				// 	memset(tmp1,0x00,sizeof(tmp1));
				// 	sprintf(tmp1,"%02x ",response_data[i]);
				// 	strcat(tmp,tmp1);
				// 	// WriteLogFile("%x ",*(BUFCUR_C(rbuff) + i));
				// }
				// WriteLogFile("111111111 response_data = %s",tmp);

				// while (rbuff->in_writing)
				// {
				// 	WriteLogFile("rbuff->in_writing = %d",rbuff->in_writing);
				// 	Sleep(1000);
				// }
				if (!WriteFileEx(rbuff->hdev, response_data, 48, &rbuff->ovs[1], my_write_completion)) {
					err("%s: failed to write sock: err: 0x%lx", __FUNCTION__, GetLastError());
					return FALSE;
				}
				rbuff->in_writing = TRUE;
				wbuff->in_writing = FALSE;
				SetEvent(wbuff->hEvent);
				tmp_buff = wbuff->peer;
				tmp_buff->offc += BUFREMAIN_C(rbuff);
				return 1;
			}
			else					 //in方向，将缓存中的数据一下子发出去
			{
				// WriteLogFile(" BUFREMAIN_C(rbuff) = %d",BUFREMAIN_C(rbuff));
				//if(BUFREMAIN_C(rbuff) == 48)
				{
					memset(response_data,0x00,sizeof(response_data));
					memcpy(response_data,BUFCUR_C(rbuff),BUFREMAIN_C(rbuff));
					data_len = data_len + BUFREMAIN_C(rbuff);
					data_len += 3;  //1个字节代表数据条数，两字节代表数据总长度
					for(i = 0;i<MERGE_BUFF_MAX_NUM;++i)  //找到有数据的缓冲buff
					{
						// WriteLogFile("rbuff->merge_buff[%d].buff_len = %d\n",i,rbuff->merge_buff[i].buff_len);
						if(rbuff->merge_buff[i].buff_len > 0)
						{
							data_num++;
							response_data[data_len++] = (rbuff->merge_buff[i].buff_len>>8) & 0xff;   //这个字节放这条数据的长度
							response_data[data_len++] = rbuff->merge_buff[i].buff_len & 0xff;
							memcpy(&response_data[data_len],rbuff->merge_buff[i].buff,rbuff->merge_buff[i].buff_len);

							// memset(tmp,0x00,sizeof(tmp));
							// for(j = 0;j<data_len + rbuff->merge_buff[i].buff_len;++j)
							// {
							// 	memset(tmp1,0x00,sizeof(tmp1));
							// 	sprintf(tmp1,"%02x ",response_data[j]);
							// 	strcat(tmp,tmp1);
							// 	// WriteLogFile("%x ",*(BUFCUR_C(rbuff) + i));
							// }
							// WriteLogFile("i = %d buff = %s",i,tmp);
							data_len += rbuff->merge_buff[i].buff_len;

							// WriteLogFile("----------- data_len = %d",data_len);
						}
					}
					if(data_num > 0)
					{
						// WriteLogFile("2222222 data_num = %d",data_num);
						memset(rbuff->merge_buff,0x00,MERGE_BUFF_MAX_NUM*sizeof(MERGE_BUFF_T));
						response_data[BUFREMAIN_C(rbuff)] = data_num;
						response_data[BUFREMAIN_C(rbuff) + 1] = (data_len - BUFREMAIN_C(rbuff) - 3)>>8 & 0xff;
						response_data[BUFREMAIN_C(rbuff) + 2] = (data_len - BUFREMAIN_C(rbuff) - 3)& 0xff;
						response_data[3] = response_data[3] | 0x80;
					}
					else
					{
						data_len = data_len - 3;
					}

					

					// memset(tmp,0x00,sizeof(tmp));
					// for(i = 0;i<data_len;++i)
					// {
					// 	memset(tmp1,0x00,sizeof(tmp1));
					// 	sprintf(tmp1,"%02x ",response_data[i]);
					// 	strcat(tmp,tmp1);
					// 	// WriteLogFile("%x ",*(BUFCUR_C(rbuff) + i));
					// }
					// WriteLogFile("2222222 response_data = %s",tmp);

					

					if (!WriteFileEx(wbuff->hdev, response_data, data_len, &wbuff->ovs[1], my_write_completion))
					{
						err("%s: failed to write sock: err: 0x%lx", __FUNCTION__, GetLastError());
						return FALSE;
					}
					tmp_buff = wbuff->peer;
					tmp_buff->offc += BUFREMAIN_C(rbuff);
					wbuff->in_writing = TRUE;
				}
				
			}
		}
		else
#endif
		{
			if(drection == 0 && 0 == *(BUFCUR_C(rbuff) + 19) && BUFREMAIN_C(rbuff) == 48)   //为控制传输  get_decp_flag
			{
				//获取配置描述符
				if(((*(BUFCUR_C(rbuff) + 40))&0xff) == 0x80 && *(BUFCUR_C(rbuff) + 41) == 0x06 && *(BUFCUR_C(rbuff) + 42) == 0x00 && *(BUFCUR_C(rbuff) + 43) == 0x02 && *(BUFCUR_C(rbuff) + 44) == 0x00 && *(BUFCUR_C(rbuff) + 45) == 0x00)
				{
					rbuff->get_decp_flag = 1;
					rbuff->get_decp_len = *(BUFCUR_C(rbuff) + 47);
					rbuff->get_decp_len = rbuff->get_decp_len << 8 | *(BUFCUR_C(rbuff) + 46);
					WriteLogFile("get_decp_len = %d",rbuff->get_decp_len);
				}
			}
			if(drection == 1 && wbuff->get_decp_flag == 1 && (BUFREMAIN_C(rbuff) - 48) == wbuff->get_decp_len && wbuff->get_decp_len > 9)
			{
				wbuff->get_decp_flag = 0;
				for(i = 0;i<wbuff->get_decp_len;++i)
				{
					if(*(BUFCUR_C(rbuff) + 48 + i) == 0x07 && *(BUFCUR_C(rbuff) + 48 + i + 1) == 0x05)
					{
						wbuff->trans_type = *(BUFCUR_C(rbuff) + 48 + i + 3);
						WriteLogFile("i = %d,trans_type = %d",i,wbuff->trans_type);
					}
				}
			}

			
			if(drection == 1 && BUFREMAIN_C(rbuff) == 48 && 0x00 == *(BUFCUR_C(rbuff)+3) && 0x00 == *(BUFCUR_C(rbuff)+1) && 0x00 == *(BUFCUR_C(rbuff)+2)&& 'B' == *(BUFCUR_C(rbuff)+0))  //0 == memcmp(heart_data,BUFCUR_C(rbuff),strlen(heart_data))
			{
				WriteLogFile("this is test heart data \n");
				wbuff->in_writing = FALSE;
				SetEvent(wbuff->hEvent);
				tmp_buff = wbuff->peer;
				tmp_buff->offc += BUFREMAIN_C(rbuff);
				return 1;
			}
			else 
			{
				// memset(tmp,0x00,sizeof(tmp));
				// for(i = 0;i<BUFREMAIN_C(rbuff);++i)
				// {
				// 	memset(tmp1,0x00,sizeof(tmp1));
				// 	sprintf(tmp1,"%02x ",*(BUFCUR_C(rbuff) + i));
				// 	strcat(tmp,tmp1);
				// }
				// WriteLogFile("---------- normal buff = %s",tmp);

				if (!WriteFileEx(wbuff->hdev, BUFCUR_C(rbuff), BUFREMAIN_C(rbuff), &wbuff->ovs[1], write_completion))
				{
					err("%s: failed to write sock: err: 0x%lx", __FUNCTION__, GetLastError());
					return FALSE;
				}
				wbuff->in_writing = TRUE;
			}
		}
	}
	return 1;
}

static BOOL
write_devbuf(devbuf_t *wbuff, devbuf_t *rbuff,int drection)
{
	devbuf_t *tmp_buff = NULL;
	int i = 0,j = 0,result = 0;
	// char tmp[30*1024] = {0x00},tmp1[32] = {0x00},response_data[128] = {0x00};
	if (rbuff->bufp != rbuff->bufc && BUFREMAIN_C(rbuff) == 0) {
		free(rbuff->bufc);
		rbuff->bufc = rbuff->bufp;
		rbuff->offc = 0;
		rbuff->bufmaxc = rbuff->offhdr;
	}

	if (!wbuff->in_writing && BUFREMAIN_C(rbuff) > 0)
	{
		wbuff->time_out_count = 0;
		rbuff->time_out_count = 0;
		result = handle_transform_data(wbuff,rbuff,drection);
		return result;
	}

	return TRUE;
}

static int
read_dev(devbuf_t *rbuff, BOOL swap_req_write, long *out_q_seqnum_array)
{
	struct usbip_header	*hdr;
	unsigned long	xfer_len, iso_len, len_data;
	//printf("read_dev start\n");
	if (BUFREAD_P(rbuff) < sizeof(struct usbip_header)) {//读取头部48字节.此时bufp有48字节数据
		rbuff->step_reading = 1;
		if (!read_devbuf(rbuff, sizeof(struct usbip_header) - BUFREAD_P(rbuff)))
			return -1;
		return 0;
	}
	//printf("read_dev start1\n");
	hdr = (struct usbip_header *)BUFHDR_P(rbuff);
	if (rbuff->step_reading == 1) {
		if (rbuff->swap_req)
			swap_usbip_header_endian(hdr, TRUE);
		rbuff->step_reading = 2;
	}

	xfer_len = get_xfer_len(rbuff->is_req, hdr, out_q_seqnum_array);
	iso_len = get_iso_len(rbuff->is_req, hdr);

	len_data = xfer_len + iso_len;
	if (BUFREAD_P(rbuff) < len_data + sizeof(struct usbip_header)) {
		DWORD	nmore = (DWORD)(len_data + sizeof(struct usbip_header)) - BUFREAD_P(rbuff);

		if (!read_devbuf(rbuff, nmore))//如果出了头部48还有数据域，则继续读取往bufp存入 现有长度为48+nmore
			return -1;
		return 0;
	}

	if (rbuff->swap_req && iso_len > 0)
		swap_iso_descs_endian((char *)(hdr + 1) + xfer_len, hdr->u.ret_submit.number_of_packets);

	DBG_USBIP_HEADER(hdr);

	if (swap_req_write) {
		if (iso_len > 0)
			swap_iso_descs_endian((char *)(hdr + 1) + xfer_len, hdr->u.ret_submit.number_of_packets);
		swap_usbip_header_endian(hdr, FALSE);
	}

	rbuff->offhdr += (sizeof(struct usbip_header) + len_data);
	if (rbuff->bufp == rbuff->bufc)
		rbuff->bufmaxc = rbuff->offp;
	rbuff->step_reading = 0;

	return 1;
}

//drection  0:pc->机柜     1:机柜->pc
static int
read_write_dev(devbuf_t *rbuff, devbuf_t *wbuff, long *out_q_seqnum_array,int drection)
{
	int	res = 0;
	if (!rbuff->in_reading) {
		if ((res = read_dev(rbuff, wbuff->swap_req, out_q_seqnum_array)) < 0)
		{
			WriteLogFile("\n\n -------- read_write_dev ,result= %d\n",res);
			return FALSE;
		}
		if (res == 0)
			return TRUE;
	}
	return write_devbuf(wbuff, rbuff,drection);
}

//static volatile BOOL	interrupted;

static void
signalhandler(int signal)
{
	//interrupted = TRUE;
	//SetEvent(hEvent);
}

typedef struct MY_HEART_DATA_STRUCT
{
	devbuf_t	*buff;
	int 		*interrupted;
}MY_HEART_DATA;

int tcp_is_connected(int fd)
{
   	int optval, optlen = sizeof(int);  
    if (fd <= 0) return 0;
	getsockopt(fd, SOL_SOCKET, SO_ERROR,(char*) &optval, &optlen); 

	if(optval == 0)  //处于连接状态
		return 0;
	return -1;
}


DWORD WINAPI send_test_heart_data(LPVOID lpParam)
{
	char heart_data[64] = {0x00,0x00,0x00,'B'};
	MY_HEART_DATA *tmp_buff = (MY_HEART_DATA *)lpParam;
	tmp_buff->buff->time_out_count = 0;
  
	// WriteLogFile("enter send_test_heart_data");
	while(tmp_buff->buff->interrupt_flag)
	{
		tmp_buff->buff->time_out_count++;
		// WriteLogFile("\n\ntmp_buff->time_out_count = %d\n",tmp_buff->time_out_count);
		if(tmp_buff->buff->time_out_count > 50)
		{
			if(!tmp_buff->buff->in_writing && 0 == BUFREMAIN_C(tmp_buff->buff))
			{
				tmp_buff->buff->time_out_count = 0;
				send(tmp_buff->buff->hdev,heart_data,48,0);
				WriteLogFile("send test heart data");
			}
			// WriteLogFile("send test heart data");
		}
  		if(0 != tcp_is_connected(tmp_buff->buff->hdev))   //此socket已断开连接
		{
			WriteLogFile("--------------------0 != tcp_is_connected---------------------");
			*(tmp_buff->interrupted) = 1;
			tmp_buff->buff->time_out_count = 0;
		}
		Sleep(1000);
	}
	tmp_buff->buff->end_flag = 1;
	WriteLogFile("tmp_buff->end_flag = 1");
}



int
usbip_forward(HANDLE hdev_src, int sockfd, BOOL inbound)//inbound FALSE
{
	int result = -1;
	devbuf_t	buff_src, buff_dst;//src:driver dst:server-ter 
	const char* desc_src, *desc_dst;
	BOOL	is_req_src;
	BOOL	swap_req_src, swap_req_dst;
	static HANDLE	hEvent;
	int interrupted;
	HANDLE hdev_dst;
	time_t time1,time2;
	double diff_t;
	long out_q_seqnum_array[OUT_Q_LEN] = { 0 };
	char heart_data[64] = "this is test heart data";
	MY_HEART_DATA my_heart_data;

	HANDLE send_test_handle = NULL;

	hdev_dst =(void *)sockfd;

	

	WriteLogFile("hdev_src:%p\n",hdev_src);

	test_fd = hdev_dst;
	if (inbound) {
		desc_src = "socket";
		desc_dst = "stub";
		is_req_src = TRUE;
		swap_req_src = TRUE;
		swap_req_dst = FALSE;
	}
	else {
		desc_src = "vhci";
		desc_dst = "socket";
		is_req_src = FALSE;
		swap_req_src = FALSE;
		swap_req_dst = TRUE;
	}
	interrupted = 0;
	time(&time1);
	//printf("in forward fd is:%d\n",*(int*)hdev_dst);
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL) {
		err("failed to create event");
		return;
	}

	if (!init_devbuf(&buff_src, desc_src, TRUE, swap_req_src, hdev_src, hEvent)) {
		CloseHandle(hEvent);
		err("%s: failed to initialize %s buffer", __FUNCTION__, desc_src);
		return;
	}
	if (!init_devbuf(&buff_dst, desc_dst, FALSE, swap_req_dst, hdev_dst, hEvent)) {
		CloseHandle(hEvent);
		err("%s: failed to initialize %s buffer", __FUNCTION__, desc_dst);
		cleanup_devbuf(&buff_src);
		return;
	}


	buff_src.peer = &buff_dst;
	buff_dst.peer = &buff_src;

	buff_src.count = 0;
	buff_dst.count = 0;

	buff_src.start_first = 0;
	buff_dst.start_first = 0;

	buff_src.time_out_count = 0;	
	buff_dst.time_out_count = 0;	

	buff_src.get_decp_flag = 0;
	buff_dst.get_decp_flag = 0;

	buff_src.get_decp_len = 0;
	buff_dst.get_decp_len = 0;

	buff_src.trans_type = 0;
	buff_dst.trans_type = 0;


	buff_dst.end_flag = 0;

	memset(buff_src.merge_buff,0x00,MERGE_BUFF_MAX_NUM*sizeof(MERGE_BUFF_T));
	memset(buff_dst.merge_buff,0x00,MERGE_BUFF_MAX_NUM*sizeof(MERGE_BUFF_T));

	buff_dst.interrupt_flag = 1;
	my_heart_data.buff =  &buff_dst;
	my_heart_data.interrupted = &interrupted;
	send_test_handle = CreateThread(NULL, 0, send_test_heart_data, &my_heart_data, 0, NULL);

	signal(SIGINT, signalhandler);

	//printf("in forwarding!\n");
	while (!interrupted) {
		/*time(&time2);
		diff_t = difftime(time2, time1);
		if(diff_t > 3)
			break;*/
		// buff_src.time_out_count++;	
		// buff_dst.time_out_count++;	

		// WriteLogFile("____buff_src.time_out_count = %d,buff_dst.time_out_count = %d",buff_src.time_out_count,buff_dst.time_out_count);

		result = read_write_dev(&buff_src, &buff_dst, out_q_seqnum_array,0);
		if (result <= 0)
		{
			 WriteLogFile("11111111111,read_write_dev,result = %d",result);
			break;
		}
		//  WriteLogFile("444444444444,buff_src.invalid = %d,buff_dst.invalid = %d",buff_src.invalid,buff_dst.invalid);
		result = read_write_dev(&buff_dst, &buff_src, out_q_seqnum_array,1);	
		if (result <= 0)
		{
			 WriteLogFile("222222222222,read_write_dev,result = %d",result);
			break;
		}
		// WriteLogFile("444444444444 result %d",result);	

		if (buff_src.invalid || buff_dst.invalid)
		{
			 WriteLogFile("333333333333,buff_src.invalid = %d,buff_dst.invalid = %d",buff_src.invalid,buff_dst.invalid);
			break;
		}
		
		if (buff_src.in_reading && buff_dst.in_reading &&
			(buff_src.in_writing || BUFREMAIN_C(&buff_dst) == 0) &&
			(buff_dst.in_writing || BUFREMAIN_C(&buff_src) == 0)) {
			// WriteLogFile("WaitForSingleObjectEx");	
			WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
			ResetEvent(hEvent);
			// WriteLogFile("exit WaitForSingleObjectEx");	
		}
	}

	buff_dst.interrupt_flag = 0;
	while(buff_dst.end_flag == 0)
	{
		Sleep(200);
		WriteLogFile("wait end_flag!!!!!!!!!!!!!!!!");
	}
	

	/*if (interrupted) {
		info("CTRL-C received\n");
	}*/
	signal(SIGINT, SIG_DFL);

	if (buff_src.in_reading)
		CancelIoEx(hdev_src, &buff_src.ovs[0]);
	if (buff_dst.in_reading)
		CancelIoEx(hdev_dst, &buff_dst.ovs[0]);

	// while (buff_src.in_reading || buff_dst.in_reading || buff_src.in_writing || buff_dst.in_writing) {
	// 	WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
	// }

	cleanup_devbuf(&buff_src);
	cleanup_devbuf(&buff_dst);
	CloseHandle(hEvent);

	if(send_test_handle != NULL)
		CloseHandle(send_test_handle);

	WriteLogFile("333333333333333333333333");

	return result;
}
