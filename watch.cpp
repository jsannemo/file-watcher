#include <sys/inotify.h>
#include <cstring>
#include <map>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <linux/limits.h>
#include <cerrno>
#include <cstdio>

using namespace std;

constexpr int BUF_SIZE = 2 * (sizeof(struct inotify_event) + NAME_MAX + 1);

int inotifyFd;

char buf[BUF_SIZE];

map<int, string> watchDescriptors;

void addDescriptor(const string& fileName){
	int watchDescriptor = inotify_add_watch(inotifyFd, fileName.c_str(), IN_MODIFY);
	if(watchDescriptor == -1){
		perror("Could not initialize watch descriptor");
	}
	watchDescriptors[watchDescriptor] = fileName;
}

// If many eventes happen rapidly, we don't really care about it.
bool readEvent(int fd){
	int readBytes = read(fd, buf, BUF_SIZE);
	if(readBytes == -1){
		perror("Read event failed");
		return false;
	}
	if(readBytes == 0){
		return false;
	}
	int idx = 0;
	while(idx < readBytes){
		inotify_event* pevent = (struct inotify_event *) &buf[idx];
		if( (pevent->mask)&IN_IGNORED ){
			addDescriptor(watchDescriptors[pevent->wd]);
		}
		idx += sizeof(pevent) + strlen(pevent->name) + 1;
	}
	return true;
}

int main(int argc, const char** argv){
	if(argc < 3){
		printf("Usage: ./watch file1 [file2 file3 ...] command\n");
		return 1;
	}
	const char* command = argv[argc - 1];

	inotifyFd = inotify_init();
	if(inotifyFd == -1){
		perror("Could not initialize inotify");
		return 2;
	}
	for(int fileIdx = 1; fileIdx < argc - 1; fileIdx++){
		string fileName(argv[fileIdx]);
		addDescriptor(fileName);
	}
	while(readEvent(inotifyFd)){
		system(command);
	}
}
