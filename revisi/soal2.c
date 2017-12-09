#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

// JANGAN LUPA DIGANTI !!!
static const char *dirpath = "/home/hehe/Documents/test";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path,void *buf,fuse_fill_dir_t filler, off_t offset,struct fuse_file_info *fi)
{
	char fpath[1000];

	filler(buf,".",NULL,0);
	filler(buf,"..",NULL,0);

	if(strcmp(path,"/")==0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath,"%s%s",dirpath,path);

	int res=0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if(dp == NULL) return -errno;

	while((de = readdir(dp)) != NULL){
		struct stat st;
		memset(&st,0,sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		res = (filler(buf,de->d_name,&st,0));
		if(res!=0)break;
	}

	closedir(dp);
	return 0;
}

int endsWith(const char *str, const char *suffix){
	const char *dot = strrchr(str,'.');
	if(strcmp(dot+1,suffix)==0)return 1;
	return 0;
}

static int xmp_read(const char *path,char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	if(strcmp(path,"/")==0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath,"%s%s",dirpath,path);

	int res=0,fd=0;

	if(endsWith(fpath,"pdf") || endsWith(fpath,"doc") || endsWith(fpath,"txt")){
		char source_file[1000],dest_file[1000],dirdir[1000];
		sprintf(source_file,"%s",fpath);
		sprintf(dest_file,"%s/rahasia/%s.ditandai",dirpath,path);
		sprintf(dirdir,"%s/rahasia",dirpath);

		struct stat st;
		memset(&st,0,sizeof(st));
		if(stat(dirdir,&st)==-1){
			mkdir(dirdir,0777);
		}

		res = rename(source_file,dest_file);
		if(res == -1) return -errno;

		res = chmod(dest_file,0000);
		if(res == -1) return -errno;

		char output[1000] = "zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'";
		system(output);
		return -errno;

	}
	else{
		(void)fi;

		fd = open(fpath,O_RDONLY);
		if(fd==-1)return -errno;

		res = pread(fd,buf,size,offset);
		if(res == -1)res = -errno;

		close(fd);
		return res;
	}
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read
};

int main(int argc, char *argv[]){
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}