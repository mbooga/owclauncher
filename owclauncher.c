#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <windows.h>

#define PROGRAM_NAME "OW_C_launcher"
#define VERSION "0.1"
#define PYVERSION "7.1b2" //version of based open_with_windows.py file
#define GITURL "https://github.com/mbooga/owclauncher"

#define BUFFER_SIZE 4096 //in bytes
#define MAXEVLEN 512 

char* programpath;
extern char **environ;

//unset inherited MOZ_* environment variables
void removeMOZ() {
    char moz[]= "MOZ_";
    char sbuffer[MAXEVLEN];
	char *s;
    int i = 0, j, envlen = 0;
    //backward iteration of the environemnt variable list
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

//A simple function to process the json message sent by the browser
//as of v7.1b2 of open with, passing (") (Double quote) character in command text is buggy
//it is basically ignored !
//the launcher will try to determine if it was used in the command text
int processJson(char* buffer, char* command) {
    int i =0, j = 0, latestquotesi =0, latestquotesj = 0;
    int insidequotes = 0, usedoublequotes =0;
    char c = buffer[0];
    while (c!='\0') {
        if ((c=='[') || (c ==']')) {
            if (insidequotes) {
                command[j] = c;
                i += 1;
                j += 1;
            } else {
                i += 1;
            }
        } else if (c=='\"') {
            if (!insidequotes) {
                latestquotesi = i;
                latestquotesj = j;
                insidequotes = 1;
                usedoublequotes = 0;
                i += 1;
            } else {
                if (usedoublequotes) {
                    command[j] = c;
                    j += 1;
                }
                i += 1;
                insidequotes = 0;
                usedoublequotes =0;
            }
        } else if (c == ',') {
            if (insidequotes) {
                command[j] = c;
            } else {
                command[j] = ' ';
            }
            i += 1;
            j += 1;
        } else if (c == ' ') {
            if (insidequotes) {
                if (usedoublequotes) {
                    command[j] = c;
                    i += 1;
                    j += 1;
                } else {
                    i = latestquotesi;
                    j = latestquotesj;
                    usedoublequotes = 1;
                    command[j] = '\"';
                    i += 1;
                    j += 1;
                }
            } else {
                command[j] = c;
                i += 1;
                j += 1;
            }
        } else if (c=='\\'){
            if ((i<BUFFER_SIZE-1) && (buffer[i+1] == '\\')) {
                command[j] = '\\';
                i += 2;
                j += 1;
            } else {
                i += 1;
            }
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
                 "{\"version\": \"%s_%s_%s\", \"file\": \"", PYVERSION, PROGRAM_NAME, VERSION);
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
    int i =0, j =0;
    const char* allowed[] = {
            "openwith@darktrojan.net",
            "chrome-extension://cogjlncmljjnjpbgppagklanlcbchlno/",
            "chrome-extension://fbmcaggceafhobjkhnaakhgfmdaadhhg/",};

    if (argc >1) {
        for (i=0; i<3; i++) {
            if (strcmp(allowed[i], argv[1])==0) {
                j = 1;
                break;
            }
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
        printf("%s v%s\n\nA Native Messaging Host (aka Launcher) for \"open with\""
               " add-on, based on v%s of open-with-windows.py\n",
               PROGRAM_NAME, VERSION, PYVERSION);
        printf("Source code & Instructions at:\n%s\n", GITURL);
        getchar();
        return -1;
    }
    programpath = argv[0];
    listenToBrowser();
    return 0;
}
