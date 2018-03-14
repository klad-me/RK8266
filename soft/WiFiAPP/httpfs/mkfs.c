#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>


#define MAX_FILES	63


struct file
{
    char *name;
    unsigned char *data;
    int size;
    int offs;
};

struct file files[MAX_FILES];
int n_files=0;


void scan_dir(char *parent)
{
    char path[1024];
    sprintf(path, "fs%s", parent);
    DIR *d=opendir(path);
    if (!d) return;
    
    struct dirent *e;
    while ( (e=readdir(d)) )
    {
	if (e->d_name[0]=='.') continue;
	
	char fname[1024];
	sprintf(fname, "fs%s/%s", parent, e->d_name);
	
	struct stat st;
	if (stat(fname, &st)!=0) continue;
	
	if (S_ISDIR(st.st_mode))
	{
	    printf("dir %s\n", fname+2);
	    scan_dir(fname+2);
	} else
	if (S_ISREG(st.st_mode))
	{
	    printf("file %s\n", fname+2);
	    
	    FILE *f=fopen(fname, "rb");
	    if (f)
	    {
		files[n_files].name=strdup(fname+2);
		char *type;
		if (!strcasecmp(fname+strlen(fname)-5, ".html")) type="text/html; charset=utf-8"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".htm")) type="text/html; charset=utf-8"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".txt")) type="text/plain; charset=utf-8"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".xml")) type="text/xml; charset=utf-8"; else
		if (!strcasecmp(fname+strlen(fname)-3, ".js")) type="text/javascript; charset=utf-8"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".jpg")) type="image/jpeg"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".gif")) type="image/gif"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".png")) type="image/png"; else
		if (!strcasecmp(fname+strlen(fname)-4, ".css")) type="text/css"; else
		    type="text/plain";
		char header[1024];
		sprintf(header,
		    "HTTP/1.0 200 Ok\r\n"
		    "Content-Type: %s\r\n"
		    "Content-Length: %d\r\n"
		    "Connection: close\r\n"
		    "\r\n",
		    type, (int)st.st_size );
		files[n_files].size=strlen(header)+st.st_size;
		files[n_files].data=(unsigned char*)malloc(files[n_files].size);
		memcpy(files[n_files].data, header, strlen(header));
		fread(files[n_files].data+strlen(header), 1, st.st_size, f);
		fclose(f);
		n_files++;
	    }
	}
    }
    
    closedir(d);
}


int main()
{
    scan_dir("");
    printf("files: %d\n", n_files);
    
    struct ent
    {
	char fname[24];
	int len;
	int pos;
    };
    FILE *out=fopen("httpfs.bin", "wb");
    
    int offset=sizeof(struct ent)*(n_files+1);	// позиция первых данных
    int i;
    
    // Записываем каталог файлов
    for (i=0; i<n_files; i++)
    {
	struct ent e;
	memset(&e, 0x00, sizeof(e));
	strncpy(e.fname, files[i].name, sizeof(e.fname)-1);
	e.len=files[i].size;
	e.pos=offset;
	printf("%s len=%d pos=%d\n", e.fname, e.len, e.pos);
	fwrite(&e, 1, sizeof(e), out);
	files[i].offs=offset;
	offset+=(e.len+3) & ~3;
    }
    
    // Записываем последнюю пустую запись
    {
	struct ent e;
	memset(&e, 0x00, sizeof(e));
	fwrite(&e, 1, sizeof(e), out);
    }
    
    // Записываем данные файлов
    for (i=0; i<n_files; i++)
    {
	fseek(out, files[i].offs, SEEK_SET);
	fwrite(files[i].data, 1, files[i].size, out);
    }
    
    fclose(out);
    
    return 0;
}