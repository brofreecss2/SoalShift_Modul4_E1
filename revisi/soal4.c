#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

//commit1 dari cobafuse.c di modul4
static const char *dirpath = "/home/hehe/Downloads";

int endsWith(const char *str, const char *suffix){
	const char *dot = strrchr(str,'.');
	if(strcmp(dot+1,suffix)==0)return 1;
	return 0;
}

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

	if(strcmp(path,"/") == 0){
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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res = 0;
  	int fd = 0 ;

  	if(endsWith(fpath,"copy")){
		int res = chmod(fpath,0000);
		if(res == -1) return -errno;

		char output[1000] = "zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'";
		system(output);

		return -errno;
	}

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	char file_from[1024];
	char file_to[1024];
	char dirdir[1000];

	sprintf(dirdir,"%s/simpanan",dirpath);

	struct stat st;
	memset(&st,0,sizeof(st));
	if(stat(dirdir,&st)==-1){
		mkdir(dirdir,0777);
	}

	sprintf(file_from,"%s%s",dirpath,from);
    sprintf(file_to,"%s%s.copy",dirdir,to);
	res = rename(file_from, file_to);
	
    if(res == -1)
    	return -errno;
 
	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_truncate(const char *path, off_t size)
{
	char fpath[1000];

	sprintf(fpath, "%s%s",dirpath,path);

	int res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

  	char fpath[1000];
	
	sprintf(fpath, "%s%s",dirpath,path);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	res = mknod(fpath, mode, rdev);
	if(res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	if(endsWith(fpath,"copy")){
		int res = chmod(fpath,0000);
		if(res == -1) return -errno;

		char output[1000] = "zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'";
		system(output);

		return -errno;
	}

	int res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.rename 	= xmp_rename,
	.write		= xmp_write,
	.truncate	= xmp_truncate,
	.mknod		= xmp_mknod,
	.open		= xmp_open,
	.utimens 	= xmp_utimens,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}