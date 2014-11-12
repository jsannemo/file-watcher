#include <sys/inotify.h>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <linux/limits.h>
#include <cerrno>
#include <cstdio>

using namespace std;

constexpr int BUF_SIZE = sizeof(struct inotify_event) + NAME_MAX + 1;

char buf[BUF_SIZE];

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
	return true;
}

int main(int argc, const char** argv){
	if(argc < 3){
		printf("Usage: ./watch file1 [file2 file3 ...] command\n");
		return 1;
	}
	const char* command = argv[argc - 1];

	int inotifyFd = inotify_init();
	if(inotifyFd == -1){
		perror("Could not initialize inotify");
		return 2;
	}
	vector<int> watchDescriptors;
	for(int fileIdx = 1; fileIdx < argc - 1; fileIdx++){
		const char* fileName = argv[fileIdx];
		int watchDescriptor = inotify_add_watch(inotifyFd, fileName, IN_MODIFY);
		if(watchDescriptor == -1){
			perror("Could not initialize watch descriptor");
		}
	}
	while(readEvent(inotifyFd)){
		system(command);
	}
}
