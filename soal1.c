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

static int xmp_read(const char *path,char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	if(strcmp(path,"/")==0){
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath,"%s%s",dirpath,path);

	int res=0,fd=0;

	(void)fi;

	fd = open(fpath,O_RDONLY);
	if(fd==-1)return -errno;

	res = pread(fd,buf,size,offset);
	if(res == -1)res = -errno;

	close(fd);
	return res;
}

int endsWith(const char *str, const char *suffix){
	if(!str || !suffix)return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if(lensuffix > lenstr)return 0;
	return strncmp(str + lenstr - lensuffix,suffix,lensuffix);
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

	char source_file[1000],dest_file[1000],temp[1025];
	FILE *source,*dest;

	sprintf(source_file,"%s",fpath);
	source = fopen(source_file,"r");

	int del=0;


	char output[1000] = "zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'";
	if(endsWith(fpath,".pdf")==0){
		system(output);
		del=1;
		sprintf(dest_file,"%s.ditandai",fpath);
	}
	else if(endsWith(fpath,".txt")==0){
		system(output);
		del=1;
		sprintf(dest_file,"%s.ditandai",fpath);
	}
	else if(endsWith(fpath,".doc")==0){
		system(output);
		del=1;
		sprintf(dest_file,"%s.ditandai",fpath);
	}
	else{
		sprintf(dest_file,"%s",fpath);
	}

	int exist = access(dest_file,F_OK);
	if(!exist) remove(dest_file);

	dest = fopen(dest_file,"w");
	while(fgets(temp,sizeof(temp),source)!=NULL){
		fprintf(dest,"%s",temp);
	}

	fclose(source);
	fclose(dest);


	int res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	if(del){
		sprintf(temp,"rm %s",source_file);
		system(temp);
		return 1;
	}
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.open		= xmp_open
};

int main(int argc, char *argv[]){
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}