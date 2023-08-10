/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stdio.h>
#include <string.h>
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)


extern struct ipc_struct *fs_ipc_struct;

/* You could add new functions or include headers here.*/
/* LAB 5 TODO BEGIN */

extern int alloc_fd();

void int2str(int n, char *str){
	char buf[256];
	int i = 0, tmp = n;

	if(!str){
		return;
	}
	while(tmp){
		buf[i] = (char)(tmp % 10) + '0';
		tmp /= 10;
		i++;
	}
	int len = i;
	str[i] = '\0';
	while(i > 0){
		str[len - i] = buf[i - 1];
		i--;
	}
}

void str2int(char *str, int *n){
	int tmp = 0;
	for(int i = 0; i < strlen(str); i++){
		tmp = tmp * 10 + (str[i] - '0');
	}
	
	*n = tmp;
}

/* LAB 5 TODO END */


FILE *fopen(const char * filename, const char * mode) {

	

	/* LAB 5 TODO BEGIN */
	struct ipc_msg *ipc_msg = ipc_create_msg(fs_ipc_struct, sizeof(struct fs_request) + 2, 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_OPEN;
    strcpy(fr->open.pathname, filename);
	FILE *file = (struct FILE *)malloc(sizeof(*file));
	if(*mode == 'r'){
    	fr->open.flags = O_RDONLY;
		fr->open.mode = 0;
		file->isWrite = 0;
	}
	else{
		fr->open.flags = O_WRONLY;
		fr->open.mode = 1;
		file->isWrite = 1;
	}
	int file_fd = alloc_fd();
    fr->open.new_fd = file_fd;
    int ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    if(ret >= 0) {
		file->fd = file_fd;
        return file;
    }
	else{
		ipc_msg = ipc_create_msg( fs_ipc_struct, sizeof(struct fs_request), 0);
        chcore_assert(ipc_msg);
        fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        fr->req = FS_REQ_CREAT;
        strcpy(fr->creat.pathname, filename);
        ret = ipc_call(fs_ipc_struct, ipc_msg);
        ipc_destroy_msg(fs_ipc_struct, ipc_msg);
        if(ret >= 0) {
			file->fd = file_fd;
			return fopen(filename, mode);
        }
	}
	/* LAB 5 TODO END */
    return NULL;
}

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
 	int cnt = nmemb * size + 1;
	
    struct ipc_msg *ipc_msg = ipc_create_msg(fs_ipc_struct, sizeof(struct fs_request) + cnt + 1, 0);
    chcore_assert(ipc_msg);
	
    struct fs_request* fr =(struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_WRITE;
    fr->write.fd = f->fd;
    fr->write.count = size * nmemb;

	char* buff =(char*)malloc(size * nmemb );
	memcpy(buff,src,size * nmemb);
    ipc_set_msg_data(ipc_msg, buff , sizeof(struct fs_request), cnt + 1);

    ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);

	
	
	/* LAB 5 TODO END */
    return 0;

}

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */
    int cnt = nmemb * size;
	struct ipc_msg *ipc_msg = ipc_create_msg( fs_ipc_struct, sizeof(struct fs_request) + cnt + 2, 0);
    chcore_assert(ipc_msg);



    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
	fr->req = FS_REQ_READ;
    fr->read.fd = f->fd;
    fr->read.count = cnt;
    int ret = ipc_call(fs_ipc_struct, ipc_msg);


    if(ret > 0) {
        memcpy(destv, ipc_get_msg_data(ipc_msg), ret);
    }
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
	/* LAB 5 TODO END */
    return 0;

}

int fclose(FILE *f) {

	/* LAB 5 TODO BEGIN */
	struct ipc_msg *ipc_msg = ipc_create_msg(fs_ipc_struct, sizeof(struct fs_request), 0);
    chcore_assert(ipc_msg);


    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_CLOSE;
    fr->close.fd = f->fd;
    int ret = ipc_call(fs_ipc_struct, ipc_msg);


    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    if(ret < 0)
        return ret;
	/* LAB 5 TODO END */
    return 0;

}

/* Need to support %s and %d. */
int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */
	
	char rbuf[512];
    memset(rbuf, 0x0, sizeof(rbuf));
	fread(rbuf, sizeof(char), sizeof(rbuf), f);

	va_list ap;
    va_start(ap, fmt);
	int start = 0, i = 0, offset = 0;
	while(i < strlen(fmt)){
		if(fmt[i] == '%'){
			i++;
			if(fmt[i] == 's'){
				char *tmp = va_arg(ap, char *);
				offset = start;
				while(offset < strlen(rbuf)){
					if(rbuf[offset] == ' '){
						break;
					}
					offset++;
				}
				memcpy(tmp, rbuf + start, offset - start);
				start = offset + 1;
			}
			else{
				if(fmt[i] == 'd'){
					int *tmp = va_arg(ap, int *);
					offset = start;
					while(offset < strlen(rbuf)){
						if(rbuf[offset] == ' '){
							break;
						}
						offset++;
					}
					char str[256];
					memset(str, '\0', sizeof(str));
					memcpy((char *)str, rbuf + start, offset - start);
					str2int(str, tmp);
					start = offset + 1;
				}
			}
		}
		i++;
	}
 
    va_end(ap);


	/* LAB 5 TODO END */
    return 0;
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */

	char wbuf[512];
    memset(wbuf, 0x0, sizeof(wbuf));

	va_list ap;
    va_start(ap, fmt);
	int start = 0, i = 0;
	int offset = 0;
	while(i < strlen(fmt)){
		if(fmt[i] == '%'){
			memcpy(wbuf + offset, fmt + start, i - start);
			offset += i - start;
			i++;
			start = i + 1;
			if(fmt[i] == 's'){
				char *tmp = va_arg(ap, char *);
				memcpy(wbuf + offset, tmp, strlen(tmp));
				offset += strlen(tmp);
			}
			else{
				if(fmt[i] == 'd'){
					int tmp = va_arg(ap, int);
					char str[256];
					memset(str, '\0', sizeof(str));
					int2str(tmp, str);
					memcpy(wbuf + offset, str, strlen(str));
					offset += strlen(str);
				}
			}
		}
		i++;
	}
	fwrite(wbuf, sizeof(char), sizeof(wbuf), f);
 
    va_end(ap);

	/* LAB 5 TODO END */
    return 0;
}

