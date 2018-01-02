#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <windows.h>

#define PROGRAMNAME "OW_C_launcher"
#define VERSION "0.1"
#define PSVERSION "7.1b2" //version of based open_with_windows.py file

#define BUFFER_SIZE 2048 //in bytes
#define MAXEVLEN 512 

char* programpath;
extern char **environ;

//unset inherited MOZ_* environment variables
void removeMOZ() {
    char moz[]= "MOZ_";
    char sbuffer[MAXEVLEN];
	char *s;
    int i = 0, j, envlen = 0;
	//reverse iteration of the environemnt variable list
    //to properly unset (delete) unwanted elements
    while (environ[i]) {
        i++;
    }
    envlen = --i;
    for (; i >=0; i--) {
        s = environ[i];
        if (strncmp(moz, s, 4) == 0) {
            j = 0;
            while ((j<MAXEVLEN -1)) {
                if ((s[j] == '\0') || (s[j] == '=') ) {
                    sbuffer[j] = '=';
                    sbuffer[j+1] = '\0';
                    putenv(sbuffer);
                    break;
                } else {
                    sbuffer[j]= s[j];
                    j +=1;
                }
            }
        }
    }
}

int processJson(char* buffer, char* command) {
    int i =0,j = 0;
    char c = buffer[0];
    while (c!='\0') {
        if ((c=='\"') || (c=='[') || (c ==']')) {
            i += 1;
        } else if (c=='\\'){
            if ((i<BUFFER_SIZE-1) && (buffer[i+1] == '\\')) {
                command[j] = '\\';
                i += 2;
                j += 1;
            } else {
                i += 1;
            }
        } else if (c == ','){
            command[j] = ' ';
            i += 1;
            j += 1;
        } else {
            command[j] = c;
            i += 1;
            j += 1;
        }

        if (i < BUFFER_SIZE) {
            c = buffer[i];
        } else {
            if (j < BUFFER_SIZE) {
				command[j] = '\0';
            } else {
                command[BUFFER_SIZE-1] = '\0';
            }
            return 0;
        }
    }
    command[j] ='\0';
    return 1;
}

int sendMessage(char* buffer, int32_t len) {
    char lenbuffer[4];
    lenbuffer[3] = (len>>24) & 0xFF;
    lenbuffer[2] = (len>>16) & 0xFF;
    lenbuffer[1] = (len>>8) & 0xFF;
    lenbuffer[0] = len & 0xFF;
    int i;
    i = write(STDOUT_FILENO, lenbuffer, 4);
    if (i!=4) {
        return 0;
    }
    if (len >0 && buffer != NULL) {
        i = write(STDOUT_FILENO, buffer, len);
        if (i!=len) {
            return 0;
        }
    }
    fflush(stdout);
    return 1;
}

int execute(char* buffer) {
    char command[BUFFER_SIZE];
    if (!processJson(buffer, command)) {
        return 0;
    }
    removeMOZ();
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si) ;
    if(CreateProcess(NULL, command, NULL,NULL,FALSE,
                     CREATE_BREAKAWAY_FROM_JOB|CREATE_NEW_CONSOLE,
                     NULL,NULL,&si,&pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        sendMessage(NULL, 0);
        return 1;
    } else {
        sendMessage(NULL, 0);
        return 0;
    }
}

int sendPing(char* buffer) {
    int32_t msgLen;
    msgLen = snprintf(buffer, BUFFER_SIZE,
                 "{\"version\": \"%s_%s_%s\", \"file\": \"", PSVERSION, PROGRAMNAME, VERSION);
    int i=0;
    while (msgLen<BUFFER_SIZE-3 && (programpath[i]!='\0')) {
        if (programpath[i] == '\\') {
            buffer[msgLen] = '\\';
            buffer[msgLen+1] = '\\';
            msgLen += 2;
            i += 1;
        } else {
            buffer[msgLen] = programpath[i];
            msgLen += 1;
            i += 1;
        }
    }
    msgLen += snprintf(buffer+msgLen, BUFFER_SIZE-msgLen, "\"}");

    if (msgLen>= BUFFER_SIZE) {
        return 0;
    }
    return sendMessage(buffer, msgLen);
}


int sendFind(char* buffer) {
    //not yet implemented !!
    //todo: find other browsers and send the list !
    return sendMessage(NULL, 0);
}

int getMessage(char* buffer ) {
    char lenbuffer[4];
    int32_t msglen, i;
    msglen = read(STDIN_FILENO, lenbuffer, 4);
    if (msglen ==4) {
        msglen = *(int32_t *)lenbuffer; //reading the first 4 bytes as int
        if (msglen>0 && msglen < BUFFER_SIZE) {
            i = read(STDIN_FILENO, buffer, msglen);
            if (i==msglen){
                buffer[msglen] = 0;
                return msglen;
            }
        }
    }
    return 0; //function failed
}

int listenToBrowser() {
    char buffer[BUFFER_SIZE];
    int i;
    if (!getMessage(buffer)) {
        return 0;
    }
    char ping[] = "\"ping\"";
    char find[] = "\"find\"";
    if (strncmp(ping, buffer, 6) ==0) {
        return sendPing(buffer);
    } else if (strncmp(find, buffer, 6) ==0) {
        return sendFind(buffer);
    } else {
        return execute(buffer);
    }
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		return -1;
	}

    int i =0, j =0;
    const char* allowed[] = {
            "openwith@darktrojan.net",
            "chrome-extension://cogjlncmljjnjpbgppagklanlcbchlno/",
            "chrome-extension://fbmcaggceafhobjkhnaakhgfmdaadhhg/",};

	for (i=0; i<3; i++) {
		if (strcmp(allowed[i], argv[1])==0) {
			j = 1;
			break;
		}
	}

    if ((j==0) && argc>2) {
        for (i=0; i<3; i++) {
            if (strcmp(allowed[i], argv[2])==0) {
                j = 1;
                break;
            }
        }
    }

    if (j ==0) { //invalid extension !
        return -1;
    }
    programpath = argv[0];
    listenToBrowser();
    return 0;
}
