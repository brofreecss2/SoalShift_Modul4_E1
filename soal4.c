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

// int file_is_modified(const char *path, time_t oldTime){
// 	struct stat file_stat;
// 	int err=stat(path,&file_stat);
// 	if(err!=0){
// 		perror("[file_is_modified] stat");
// 		exit(errno);
// 	}
// 	return file_stat.st_mtime>oldTime;
// }


char readPath[1024];

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
	strcpy(readPath,path);
	int res = 0;
  	int fd = 0 ;

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

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

  	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res,res1;

	char fpath[1000],temp[1024];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	sprintf(temp,"%s%s",dirpath,readPath);
	res1 = chmod(temp,0000);
	if(res1 ==  1)res1=-errno;

	close(fd);
	return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];

	if(strcmp(path,"/") == 0)
	{
		char *path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	char path2[1000],command[1000];
	system("mkdir -p /home/hehe/Downloads/simpanan");
	sprintf(path2,"%s%s","/home/hehe/Downloads/simpanan",path);


	int res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;
	close(res);
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	char fpath[1000];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res;

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	char file_from[1024],file_to[1024];

	sprintf(file_from,"%s%s",dirpath,from);
	sprintf(file_to,"%s%s",dirpath,to);


	res = rename(file_from, file_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	char fpath[1000];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	char fpath[1000];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res;

	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	char file_from[1024],file_to[1024];
	char *direct = "/home/hehe/Downloads/simpanan";

	system("mkdir -p /home/hehe/Downloads/simpanan");

	sprintf(file_from,"%s%s",dirpath,from);
	sprintf(file_to,"%s%s",direct,to);


	res = rename(file_from, file_to);
	if (res == -1)
		return -errno;
	

	return 0;
}


static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.open		= xmp_open,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.write		= xmp_write,
	.truncate	= xmp_truncate,
	.chmod		= xmp_chmod,
	.rename 	= xmp_rename,
	.mknod		= xmp_mknod,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

