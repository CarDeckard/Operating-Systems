#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

#define BITS_PER_FAT 16

int fd;
char* image;
char* fileName;

int totFiles;
int totRootDirFiles;
int totNumDirs;
int totCapacity;
int totAllocatedSize;
int totFileSize;
int unusedSpace;
int unallocatedSpace;
int largestSize;
char* largestPath;
char* cookiePath;
int cookieCluster;
int maxDirLvl;
char* oldestPath;
int oldestYear;
int oldestMonth;
int oldestDay;
int oldestHour;
int oldestMin;
int oldestSec;

void testMMAP();
void testBS();
void testDE();
void testFC();
void testFNorFC();
void printEntryDetails();
void printEntryContent();
void printClusterContent();
void milestone4();
void searchCluster(void*, void*, int, unsigned short, int, int, char*);
int isOldest();
int isCookieFile();
int checkClusterForCookie();
void testWriteFS();

void main(int argc, char **argv){
    ///// CLEAR ALL VARIABLES //////
    int testmmapFlag = 0;
    int testBSFlag = 0;
    int testDEFlag = 0;
    int testFCFlag = 0;
    int testFNorFCFlag = 0;
    int displayContentsFlag = 0;
    int DirEntry = 0;
    int StartCluster = 0;
    int m4Flag = 0;
    int numEFlag = 0;
    int spaceUseFlag = 0;
    int largeFileFlag = 0;
    int cookieFlag = 0;
    int numDirLvlFlag = 0;
    int oldFileFlag = 0;
    int allDataFlag = 0;
    int writeFSFlag = 0;
    fd = 0;
    ////////////////////////////////

    int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"image",               required_argument,  NULL,  'i'},
            {"test-directory-entry",required_argument,  NULL,  'd'},
            {"test-file-clusters",  required_argument,  NULL,  'c'},
            {"test-file-name",      required_argument,  NULL,  'n'},
            {"test-file-contents",  required_argument,  NULL,  'a'},
            {"test-mmap",           no_argument,        NULL,  't'},
            {"test-boot-sector",    no_argument,        NULL,  'b'},
            {"test-num-entries",    no_argument,        NULL,  'e'},
            {"test-space-usage",    no_argument,        NULL,  's'},
            {"test-largest-file",   no_argument,        NULL,  'l'},
            {"test-cookie",         no_argument,        NULL,  'k'},
            {"test-num-dir-levels", no_argument,        NULL,  'q'},
            {"test-oldest-file",    no_argument,        NULL,  'o'},
            {"output-fs-data",      no_argument,        NULL,  'f'},
            {"write-fs-data",       no_argument,        NULL,  'w'},
            {"invalid-image",       no_argument,        NULL,  'z'},
            {0,                     0,                  0,       0},
        };

    int opt;
    while(1){
        opt = getopt_long(argc, argv, "i:t", long_options, &option_index);
        if(opt == -1)
            break;

        switch(opt){
            case 'i':
                image = optarg;
                break;
            case 't':
                testmmapFlag = 1;
                break;
            case 'b':
                testBSFlag = 1;
                break;
            case 'd':
                DirEntry = atoi(optarg);
                testDEFlag = 1;
                break;
            case 'c':
                StartCluster = atoi(optarg);
                testFCFlag = 1;
                break;
            case 'a':
                displayContentsFlag = 1;
            case 'n':
                testFNorFCFlag = 1;
                fileName = optarg;
                break;
            case 'e':
                m4Flag = 1;
                numEFlag = 1;
                break;
            case 's':
                m4Flag = 1;
                spaceUseFlag = 1;
                break;
            case 'l':
                m4Flag = 1;
                largeFileFlag = 1;
                break;
            case 'k':
                m4Flag = 1;
                cookieFlag = 1;
                break;
            case 'q':
                m4Flag = 1;
                numDirLvlFlag = 1;
                break;
            case 'o':
                m4Flag = 1;
                oldFileFlag = 1;
                break;
            case 'f':
                m4Flag = 1;
                allDataFlag = 1;
                break;
            case 'w':
                writeFSFlag = 1;
                break;
            case 'z':
                break;
            case '?':
                printf("Unknown option inputted: %s\n", optarg);
                return;
            default:
                exit(-1);
        }
    }

    fd = open(image, O_RDWR);

    struct stat fs;
    fstat(fd, &fs);
    size_t size = fs.st_size;

    void* addr = mmap(NULL,size, PROT_READ | PROT_WRITE ,MAP_SHARED,fd,0);

    if(addr == MAP_FAILED){
        printf("Map Failed (line: %d)\n", __LINE__);
        return;
    }

    if(testmmapFlag) 
        testMMAP(addr);

    if(testBSFlag) 
        testBS(addr);

    if(testDEFlag){
        DirEntry = DirEntry * 32;
        testDE(DirEntry, addr);
    }

    if(testFCFlag)
        testFC(StartCluster, addr);

    if(testFNorFCFlag)
        testFNorFC(fileName, addr, displayContentsFlag);
    
    if(m4Flag)
        milestone4(addr, numEFlag, spaceUseFlag, largeFileFlag, cookieFlag, numDirLvlFlag, oldFileFlag, allDataFlag);

    if(writeFSFlag)
        testWriteFS(addr);

}

void testMMAP(void* addr){
    size_t bufferSize = 0;
    char csize;
    int location;
    char* instruc;

    while(!feof(stdin)){
        int returnValue = scanf("\n%c %d", &csize, &location);
        if(returnValue != EOF){
            if(csize == 'c'){
                char* ptr = addr + location;
                printf("%c\n", *ptr);
            }else if(csize == 'b'){
                unsigned char* ptr = addr + location;
                printf("%x\n", *ptr);
            }else if(csize == 's'){
                short* ptr = addr + location;
                printf("%i\n", *ptr);
            }else if(csize == 'w'){
                unsigned short* ptr = addr + location;
                printf("%x\n", *ptr);
            }else if(csize == 'i'){
                int* ptr = addr + location;
                printf("%d\n", *ptr);
            }else if(csize == 'u'){
                unsigned int* ptr = addr + location;
                printf("%x\n", *ptr);
            }
        }
    }
}

void testBS(void* addr){
    printf("OEM: ");
    for(int i = 3; i < 11; i++){
        char* OEM = addr + i;
        printf("%c", *OEM);
    }
    printf("\n");

    short* BPS = addr + 0x00B;
    char* SPC= addr + 0x00D;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* maxRootDirEnts = addr + 0x011;
    unsigned short* numLogS = addr + 0x013;
    unsigned char* mediaDesc = addr + 0x015;
    short* SPFAT = addr + 0x016;
    int intNumLogS = *numLogS;
   
    if(*numLogS == 0){
        intNumLogS = *(int*)(addr + 0x020);
    }

    printf("Bytes per sector: %d\n", *BPS);
    printf("Sectors per cluster: %d\n", (int)*SPC);
    printf("Reserved sectors: %d\n", *RS);
    printf("Num FATs: %d\n", (int)*numFATs);
    printf("Max root directory entries: %d\n", *maxRootDirEnts);
    printf("Num logical sectors: %d\n", intNumLogS);
    printf("Media Descriptor: %x\n", (unsigned)*mediaDesc);
    printf("Sectors per FAT: %d\n", *SPFAT);

}

void testDE(int DirEntry, void* addr){

    short* BPS = addr + 0x00B;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* SPFAT = addr + 0x016;
    int FATRegionSize = (int)(*numFATs) * (int)(*SPFAT) * (int)(*BPS);
    int ReservedRegionSize = (int)(*RS) * (int)(*BPS);

    int offset = FATRegionSize + ReservedRegionSize;

    void* rootDir = addr + offset;
    void* entryStart = rootDir + DirEntry;

    char filename[13];
    memset(&filename, 0, 13);
    char name[38];
    memset(&name, 0, 38);
    char* tempChar;
    char* specialByte = entryStart;

    printEntryDetails(entryStart);

}

void testFC(int startC, void* addr){
    if(startC == 0 || startC == 1){
        printf("EOF\n");
        return;
    }

    short* BPS = addr + 0x00B;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* SPFAT = addr + 0x016;
    int ReservedRegionSize = (int)(*RS) * (int)(*BPS);

    unsigned short* clusterZero = addr + ReservedRegionSize;
    unsigned short* currentCptr = clusterZero + startC;

    printf("%d -> ", startC);

    while(*currentCptr != 0xFFFF && *currentCptr != 0){
        printf("%d -> ", *currentCptr);
        currentCptr = clusterZero + *currentCptr;
    }
    printf("EOF\n");
}

void testFNorFC(char* fileName, void* addr, int displayContentsFlag){
    short* BPS = addr + 0x00B;
    char* SPC= addr + 0x00D;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* maxRootDirEnts = addr + 0x011;
    short* SPFAT = addr + 0x016;
    int FATRegionSize = (int)(*numFATs) * (int)(*SPFAT) * (int)(*BPS);
    int ReservedRegionSize = (int)(*RS) * (int)(*BPS);
    int RootDirectorySize = ((int)(*maxRootDirEnts) * 32);
    int clusterSize = (int)(*BPS) * (int)(*SPC);
    int DirPerCluster = clusterSize / 32;

    int FATOffset  = ReservedRegionSize;
    int rootOffset = FATRegionSize + ReservedRegionSize;
    int dataOffset = FATRegionSize + ReservedRegionSize + RootDirectorySize;

    void* rootDir   = addr + rootOffset;
    void* dataStart = addr + dataOffset;
    void* FATStart  = addr + FATOffset;
    
    int clNum = 0;

    int i = 1;
    int bufferCur = 0;
    unsigned short* start;
    int isRD = 1;
    char buffer[12];
    char entryName[12];
    int found = 0;
    int nextFound = 0;
    int fIsSubDir = 0;
    int eIsSubDir = 0;
    void* dirEntry = rootDir;
    void* foundAddr;
    void* clusterStart;
    void* entryPTR;
    unsigned short* clusterNum; //declaration needed for compile, will be set later
    char* tempChar = fileName+1;
    char* etempChar;

    while(!found){
        //Fill Buffer
        memset(buffer, ' ', 11);
        buffer[11] = '\0';
        bufferCur = 0;
        fIsSubDir = 0;
        nextFound = 0;
        while(*tempChar != '/' && *tempChar != '.' && i < strlen(fileName)){
            buffer[bufferCur] = *tempChar;
            tempChar++;
            i++;
            bufferCur++;
        }
        if(*tempChar == '.'){
            //i++ to get around the dot
            i++;

            for(int a = 0; a < 3; a++){
                tempChar++;
                buffer[8+a] = *tempChar;
                i++;
            }
            tempChar++;
        }else if(*tempChar == '/'){
            i++;
            tempChar++;
            fIsSubDir = 1;
        }
        
        //Search Cluster
        if(!isRD){
            while(!nextFound){
                int clusterOffset = ((int)*clusterNum -2) * clusterSize;
                clusterStart = dataStart + clusterOffset;

                for(int entryNum = 0; entryNum < DirPerCluster; entryNum++){
                    eIsSubDir = 0;
                    entryPTR = clusterStart + (entryNum * 32);
                    memset(entryName, ' ', 11);
                    entryName[11] = '\0';
                    etempChar = entryPTR;
                    for(int a = 0; a < 11; a++){
                        entryName[a] = *etempChar;
                        etempChar++;
                    }

                    char* fAttr = entryPTR + 0x0B;
                    if(*fAttr & 0x10){
                        eIsSubDir = 1;
                    }  
                    
                    if(!(strcmp(entryName, buffer)) && (eIsSubDir == fIsSubDir)){
                        clusterNum = entryPTR + 0x1A;
                        if(fIsSubDir == 0){
                            foundAddr = entryPTR;
                            found = 1;
                        }
                        nextFound = 1;
                        break;
                    }
                }

                if(!nextFound){
                    start = FATStart;
                    clusterNum = start + *clusterNum;
                }

            }
    
        }

        //Search RD
        if(isRD){
            while(1){
                eIsSubDir = 0;
                char entryName[12];
                memset(entryName, ' ', 11);
                entryName[11] = '\0';
                for(int a = 0; a < 11; a++){
                    char* tempChar = dirEntry + a;
                    entryName[a] = *tempChar;
                }

                char* fAttr = dirEntry + 0x0B;
                if(*fAttr & 0x10){
                    eIsSubDir = 1;
                }   

                if(!(strcmp(entryName, buffer)) && (eIsSubDir == fIsSubDir)){
                    if(fIsSubDir == 0){
                        foundAddr = dirEntry;
                        found = 1;
                    }
                    clusterNum = dirEntry + 0x1A;
                    break;
                }
                dirEntry += 32;
            }
            isRD = 0;
        }
    
    }

    if(!displayContentsFlag)
        printEntryDetails(foundAddr);
    if(displayContentsFlag)
        printEntryContent(dataStart, FATStart, foundAddr, clusterSize);

}

void printEntryDetails(void* entryStart){
    char filename[13];
    memset(&filename, 0, 13);
    char name[38];
    memset(&name, 0, 38);
    char* tempChar;
    char* specialByte = entryStart;

    if(*specialByte == 0x00){
        printf("Empty entry\n");
        return;
    }else if(*specialByte == (char)0xE5){
        printf("Previously erased entry\n");
        printf("Name: ?");

        for(int i = 1; i < 8; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf(".");
        for(int i = 8; i < 11; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf("\n");

    }else if(*specialByte == 0x05){
        printf("Name: %c", 0xE5);
        for(int i = 1; i < 8; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf(".");
        for(int i = 8; i < 11; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf("\n");

    }else{
        printf("Name: ");
        for(int i = 0; i < 8; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf(".");
        for(int i = 8; i < 11; i++){
            char* tempChar = entryStart + (i*sizeof(char));
            printf("%c", *tempChar);
        }
        printf("\n");
    }

    char* fAttr = entryStart + 0x0B;

    char fAttrStr[100];
    memset(&fAttrStr, 0, 100);

    if(*fAttr & 0x01){
        strcat(fAttrStr, "RO ");
    }
    if(*fAttr & 0x02){
        strcat(fAttrStr, "Hidden ");
    }
    if(*fAttr & 0x04){
        strcat(fAttrStr, "Sys ");
    }
    if(*fAttr & 0x08){
        strcat(fAttrStr, "Vol. label ");
    }
    if(*fAttr & 0x10){
        strcat(fAttrStr, "subdir ");
    }
    if(*fAttr & 0x20){
        strcat(fAttrStr, "archive ");
    }
    if(*fAttr & 0x40){
        strcat(fAttrStr, "device ");
    }
    if(*fAttr & 0x80){
        strcat(fAttrStr, "reserved ");
    }    

    unsigned char* cTimeMilli = entryStart + 0x0D;

    unsigned short* cTime = entryStart + 0x0E;
    short cTimeHour = (short)(*cTime) >> 11;
    short cTimeMin = (*cTime >> 5) & 0b111111;
    short cTimeSec = (*cTime & 0b11111) * 2;
    short cTimeMil = *cTimeMilli * 10;
    if(cTimeMil >= 1000){
        cTimeSec++;
        cTimeMil -= 1000;
    }

    unsigned short* cDate = entryStart + 0x10;
    short cDateYear = (*cDate >> 9) + 1980;
    short cDateMonth = (*cDate >> 5) & 0b1111;
    short cDateDay = *cDate & 0b11111;

    unsigned short* aDate = entryStart + 0x12;
    short aDateYear = (*aDate >> 9) + 1980;
    short aDateMonth = (*aDate >> 5) & 0b1111;
    short aDateDay = *aDate & 0b11111;

    short* eAttr = entryStart + 0x14;

    unsigned short* mTime = entryStart + 0x16;
    short mTimeHour = *mTime >> 11;
    short mTimeMin = (*mTime >> 5) & 0b111111;
    short mTimeSec = (*mTime & 0b11111) * 2;

    unsigned short* mDate = entryStart + 0x18;
    short mDateYear = (*mDate >> 9) + 1980;
    short mDateMonth = (*mDate >> 5) & 0b1111;
    short mDateDay = *mDate & 0b11111;

    unsigned short* cluster = entryStart + 0x1A;
    unsigned int* bytes = entryStart + 0x1C;

    printf("File Attributes: %s\n", fAttrStr);
    printf("Create time: %04i/%02i/%02i %02i:%02i:%02i.%03i\n", cDateYear, cDateMonth, cDateDay, cTimeHour, cTimeMin, cTimeSec, cTimeMil);
    printf("Access date: %04i/%02i/%02i\n", aDateYear, aDateMonth, aDateDay);
    printf("Extended attributes: %i\n", *eAttr);
    printf("Modify time: %04i/%02i/%02i %02i:%02i:%02i.000\n", mDateYear, mDateMonth, mDateDay, mTimeHour, mTimeMin, mTimeSec);
    printf("Start cluster: %i\n", *cluster);
    printf("Bytes: %i\n", *bytes);
}

void printEntryContent(void* dataStart, void* FATStart, void* foundAddr, int clusterSize){

    unsigned short* clusterNum = foundAddr + 0x1A;
    unsigned int* contentLength = foundAddr + 0x1c;
    short* start = FATStart;

    void* contentStart = dataStart + (((int)*clusterNum - 2) * clusterSize);

    int partialClusterSize;
    int numClusters = *contentLength / clusterSize;
    if((*contentLength % clusterSize) > 0){
        partialClusterSize = *contentLength % clusterSize;
    }
    
    int numPrinted = 0;
    while(numPrinted < *contentLength){

        if(numClusters != 0){
            printClusterContent(contentStart, clusterSize);
            numClusters--;
            numPrinted += clusterSize;
            clusterNum = start + *clusterNum;
            contentStart = dataStart + (((int)*clusterNum - 2) * clusterSize);
        }else if(partialClusterSize != 0){
            printClusterContent(contentStart, partialClusterSize);
            numPrinted += partialClusterSize;
        }

    }

}

void printClusterContent(void* clusterStart, int contentLength){
    char* tempChar = clusterStart;

    for(int i = 0; i < contentLength; i++){
        printf("%c", *tempChar);
        tempChar++;
    }

}

void milestone4(void* addr, int numEFlag, int spaceUseFlag, int largeFileFlag, int cookieFlag, int numDirLvlFlag, int oldFileFlag, int allDataFlag){
    totNumDirs= 0;
    totRootDirFiles = 0;
    totFiles = 0;
    maxDirLvl = 1;
    largestSize = 0;
    totCapacity = 0;
    totFileSize = 0;
    unusedSpace = 0;
    oldestYear = 4040;
    oldestMonth = 13;
    oldestDay = 32;
    oldestHour = 25;
    oldestMin = 61;
    oldestSec = 61;
    cookieCluster = 0;
    char* path = malloc(1024);
    largestPath = malloc(1024);
    oldestPath = malloc(1024);
    cookiePath = malloc(1024);

    short* BPS = addr + 0x00B;
    char* SPC= addr + 0x00D;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* maxRootDirEnts = addr + 0x011;
    unsigned short* numLogS = addr + 0x013;
    unsigned char* mediaDesc = addr + 0x015;
    short* SPFAT = addr + 0x016;
    int intNumLogS = (int)*numLogS;
   
    if(*numLogS == 0){
        intNumLogS = *(int*)(addr + 0x020);
    }
    
    int FATRegionSize = (int)(*numFATs) * (int)(*SPFAT) * (int)(*BPS);
    int ReservedRegionSize = (int)(*RS) * (int)(*BPS);
    int RootDirectorySize = ((int)(*maxRootDirEnts) * 32);
    int clusterSize = (int)(*BPS) * (int)(*SPC);
    int DirPerCluster = clusterSize / (32);

    int FATOffset  = ReservedRegionSize;
    int rootOffset = FATRegionSize + ReservedRegionSize;
    int dataOffset = FATRegionSize + ReservedRegionSize + RootDirectorySize;

    void* rootDir   = addr + rootOffset;
    void* dataStart = addr + dataOffset;
    void* FATStart  = addr + FATOffset;

    void* entryStart;
    char* subDirAttr;
    char* specialByte;
    unsigned short* clusterNum;
    for(int i = 0; i < *maxRootDirEnts; i++){
        entryStart = rootDir + (i*32);
        specialByte = entryStart;
        subDirAttr = entryStart + 0x0B;
        if(*specialByte != (char)0x00 && *specialByte != (char)0xE5 && *specialByte != (char)0x2E){
            if(*subDirAttr & 0x10){
                totNumDirs++;
                unusedSpace += 2048;
                clusterNum = entryStart + 0x1A;
                char pathName[10];
                memset(pathName, 0, 10);
                pathName[0] = '/';
                int a = 1;
                for(int i = 0; i < 8; i++){
                    char* tempChar = entryStart + i;
                    if(*tempChar != ' '){
                        pathName[a] = *tempChar;
                        a++;
                    }
                }
                strcpy(path, pathName);
                searchCluster(dataStart, FATStart, clusterSize, *clusterNum, DirPerCluster, 2, path);
            }else{
                totFiles++;
                totRootDirFiles++;
                unsigned int* SizeBytes = entryStart + 0x1C;
                if( *SizeBytes > largestSize){
                    largestSize = *SizeBytes;
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    strcpy(largestPath, fileName);
                }
                if(isOldest(entryStart, oldestYear, oldestMonth, oldestDay, oldestHour, oldestMin, oldestSec) == 1){
                    unsigned short* ADate = entryStart + 0x18;
                    short ADateYear = (*ADate >> 9) + 1980;
                    short ADateMonth = (*ADate >> 5) & 0b1111;
                    short ADateDay = *ADate & 0b11111;
                    unsigned short* ATime = entryStart + 0x16;
                    short ATimeHour = *ATime >> 11;
                    short ATimeMin = (*ATime >> 5) & 0b111111;
                    short ATimeSec = (*ATime & 0b11111) * 2;

                    oldestYear = ADateYear;
                    oldestMonth = ADateMonth;
                    oldestDay = ADateDay;
                    oldestHour = ATimeHour;
                    oldestMin = ATimeMin;
                    oldestSec = ATimeSec;

                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(oldestPath, path);
                    *pathEnd = '\0';
                }
                unsigned short* contentCluster = entryStart + 0x1A;
                if(isCookieFile(dataStart, FATStart, entryStart, clusterSize) == 1){
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(cookiePath, path);
                    *pathEnd = '\0';
                    cookieCluster = (int)*contentCluster;
                }
                totFileSize += *SizeBytes;
            }
        }
        if(*specialByte == (char)0x00){
            break;
        }
    }

    
    if(numEFlag || allDataFlag){
        printf("Number of files in root directory: %d\n", totRootDirFiles);
        printf("Number of files in the file system: %d\n", totFiles);
        printf("Number of directories in the file system: %d\n", totNumDirs);
    }
    if(spaceUseFlag || allDataFlag){
        totCapacity = intNumLogS * (int)(*BPS);
        int numAllocatedEntries = 0;

        int numFATEntries = (FATRegionSize / 2) / (int)*numFATs;
        for(int i = 2; i < numFATEntries; i++){
            unsigned short* currentFATEntry = FATStart + (i*2);
            if(*currentFATEntry != 0x00){
                numAllocatedEntries++;
            }
        }
        totAllocatedSize = numAllocatedEntries * clusterSize;
        unallocatedSpace = totCapacity - totAllocatedSize;
        unusedSpace = totAllocatedSize - totFileSize;

        printf("Total capacity of the file system: %d\n", totCapacity);
        printf("Total allocated space: %d\n", totAllocatedSize);
        printf("Total size of files: %d\n", totFileSize);
        printf("Unused, but allocated, space (for files): %d\n", unusedSpace);
        printf("Unallocated space: %d\n", unallocatedSpace);
    }
    if(largeFileFlag || allDataFlag){
        printf("Largest file (%d bytes): %s\n", largestSize, largestPath);
    }
    if(cookieFlag || allDataFlag){
        printf("Path to file with cookie: %s\n", cookiePath);
        printf("Starting cluster for file with cookie: %d\n", cookieCluster);
    }
    if(numDirLvlFlag || allDataFlag){
        printf("Directory hierarchy levels: %d\n", maxDirLvl);
    }
    if(oldFileFlag || allDataFlag){
        printf("Oldest file: %s\n", oldestPath);
    }
}

void searchCluster(void* dataStart, void* FATStart, int clusterSize, unsigned short clusterNum, int DirPerCluster, int dirlvl, char* path){
    void* entryStart;
    char* specialByte;
    char* subDirAttr;
    unsigned short* nextCluster;
    char pathName[10];

    int clusterOffset = (clusterNum-2) * clusterSize;
    void* clusterStart = dataStart + clusterOffset;
    for(int i = 0; i < DirPerCluster; i++){
        entryStart = clusterStart + (i*32);
        specialByte = entryStart;
        subDirAttr = entryStart + 0x0B;
        if(*specialByte != (char)0x00 && *specialByte != (char)0xE5 && *specialByte != (char)0x2E){
            if(*subDirAttr & 0x10){
                totNumDirs++;
                memset(pathName, 0, 10);
                pathName[0] = '/';
                int a = 1;
                for(int i = 0; i < 8; i++){
                    char* tempChar = entryStart + i;
                    if(*tempChar != ' '){
                        pathName[a] = *tempChar;
                        a++;
                    }
                }
                char* pathEnd = path + strlen(path);
                strcat(path, pathName);
                nextCluster = entryStart + 0x1A;
                searchCluster(dataStart, FATStart, clusterSize, *nextCluster, DirPerCluster, dirlvl+1, path);
                *pathEnd = '\0';
            }else{
                totFiles++;
                unsigned int* SizeBytes = entryStart + 0x1C;
                if( *SizeBytes > largestSize){
                    largestSize = *SizeBytes;
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(largestPath, path);
                    *pathEnd = '\0';
                }
                if(isOldest(entryStart, oldestYear, oldestMonth, oldestDay, oldestHour, oldestMin, oldestSec) == 1){
                    unsigned short* ADate = entryStart + 0x18;
                    short ADateYear = (*ADate >> 9) + 1980;
                    short ADateMonth = (*ADate >> 5) & 0b1111;
                    short ADateDay = *ADate & 0b11111;
                    unsigned short* ATime = entryStart + 0x16;
                    short ATimeHour = *ATime >> 11;
                    short ATimeMin = (*ATime >> 5) & 0b111111;
                    short ATimeSec = (*ATime & 0b11111) * 2;

                    oldestYear = ADateYear;
                    oldestMonth = ADateMonth;
                    oldestDay = ADateDay;
                    oldestHour = ATimeHour;
                    oldestMin = ATimeMin;
                    oldestSec = ATimeSec;

                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(oldestPath, path);
                    *pathEnd = '\0';
                }
                unsigned short* contentCluster = entryStart + 0x1A;
                if(isCookieFile(dataStart, FATStart, entryStart, clusterSize) == 1){
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(cookiePath, path);
                    *pathEnd = '\0';
                    cookieCluster = (int)*contentCluster;
                }
                totFileSize += *SizeBytes;
            }
        }
        if(*specialByte == (char)0x00){
            if(dirlvl > maxDirLvl)
                maxDirLvl = dirlvl;
            return;
        }
    }
    nextCluster = FATStart + (clusterNum *2);
    if(*nextCluster < 0xFFF8){
        searchCluster(dataStart, FATStart, clusterSize, *nextCluster, DirPerCluster, dirlvl, path);
    }
}

int isOldest(void* entryA, int oYear, int oMonth, int oDay, int oHour, int oMin, int oSec){
    unsigned short* ADate = entryA + 0x18;
    short ADateYear = (*ADate >> 9) + 1980;
    short ADateMonth = (*ADate >> 5) & 0b1111;
    short ADateDay = *ADate & 0b11111;

    if(ADateYear < oYear)
        return 1;
    if(ADateYear > oYear)
        return 0;
    if(ADateMonth < oMonth)
        return 1;
    if(ADateMonth > oMonth)
        return 0;

    if(ADateDay < oDay)
        return 1;
    if(ADateDay > oDay)
        return 0;
    
    unsigned short* ATime = entryA + 0x16;
    short ATimeHour = *ATime >> 11;
    short ATimeMin = (*ATime >> 5) & 0b111111;
    short ATimeSec = (*ATime & 0b11111) * 2;

    if(ATimeHour < oHour)
        return 1;
    if(ATimeHour > oHour)
        return 0;

    if(ATimeMin < oMin)
        return 1;
    if(ATimeMin > oMin)
        return 0;

    if(ATimeSec < oSec)
        return 1;
    if(ATimeSec > oSec)
        return 0;

    return -1;
}

int isCookieFile(void* dataStart, void* FATStart, void* entry, int clusterSize){  
    unsigned short* clusterNum = entry + 0x1A;
    unsigned int* contentLength = entry + 0x1c;
    short* start = FATStart;

    void* contentStart = dataStart + (((int)*clusterNum - 2) * clusterSize);

    int partialClusterSize;
    int numClusters = *contentLength / clusterSize;
    if((*contentLength % clusterSize) > 0){
        partialClusterSize = *contentLength % clusterSize;
    }
    
    int numPrinted = 0;
    while(numPrinted < *contentLength){

        if(numClusters != 0){
            if(checkClusterForCookie(contentStart, clusterSize)){
                return 1;
            }
            numClusters--;
            numPrinted += clusterSize;
            clusterNum = start + *clusterNum;
            contentStart = dataStart + (((int)*clusterNum - 2) * clusterSize);
        }else if(partialClusterSize != 0){
            if(checkClusterForCookie(contentStart, partialClusterSize)){
                return 1;
            }
            numPrinted += partialClusterSize;
        }

    }
    
}

int checkClusterForCookie(void* contentStart, int contentLength){
    char* tempChar = contentStart;

    for(int i = 0; i < contentLength; i++){
        if(*tempChar == 'C'){
            tempChar++;
            if(*tempChar == 'O'){
                tempChar++;
                if(*tempChar == 'S'){
                    tempChar++;
                    if(*tempChar == ' '){
                        tempChar++;
                        if(*tempChar == '4'){
                            tempChar++;
                            if(*tempChar == '2'){
                                tempChar++;
                                if(*tempChar == '1'){
                                    tempChar++;
                                    if(*tempChar == ' '){
                                        tempChar++;
                                        if(*tempChar == 'c'){
                                            tempChar++;
                                            if(*tempChar == 'o'){
                                                tempChar++;
                                                if(*tempChar == 'o'){
                                                    tempChar++;
                                                    if(*tempChar == 'k'){
                                                        tempChar++;
                                                        if(*tempChar == 'i'){
                                                            tempChar++;
                                                            if(*tempChar == 'e'){
                                                                return 1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }else{
            tempChar++;
        }
    }
    return 0;

}

void testWriteFS(void* addr){
    void* firstOpenDirAddr;

    short* BPS = addr + 0x00B;
    char* SPC= addr + 0x00D;
    short* RS = addr + 0x00E;
    char* numFATs = addr + 0x010;
    short* maxRootDirEnts = addr + 0x011;
    unsigned short* numLogS = addr + 0x013;
    unsigned char* mediaDesc = addr + 0x015;
    short* SPFAT = addr + 0x016;
    int intNumLogS = (int)*numLogS;
   
    if(*numLogS == 0){
        intNumLogS = *(int*)(addr + 0x020);
    }
    
    int FATRegionSize = (int)(*numFATs) * (int)(*SPFAT) * (int)(*BPS);
    int ReservedRegionSize = (int)(*RS) * (int)(*BPS);
    int RootDirectorySize = ((int)(*maxRootDirEnts) * 32);
    int clusterSize = (int)(*BPS) * (int)(*SPC);
    int DirPerCluster = clusterSize / (32);

    int FATOffset  = ReservedRegionSize;
    int rootOffset = FATRegionSize + ReservedRegionSize;
    int dataOffset = FATRegionSize + ReservedRegionSize + RootDirectorySize;

    void* rootDir   = addr + rootOffset;
    void* dataStart = addr + dataOffset;
    void* FATStart  = addr + FATOffset;

    unsigned short* FATEntry = FATStart + 8;
    int firstEmptyCluster = -1;
    int secondEmptyCluster = -1;
    int TotNumClusters = ((intNumLogS * (int)*BPS) - ReservedRegionSize - FATRegionSize - RootDirectorySize) / clusterSize;
    for(int i = 2; i < TotNumClusters; i++){
        unsigned short* currentFATEntry = FATStart + (i*2);
        if( *currentFATEntry == 0x00 && firstEmptyCluster != -1){
            secondEmptyCluster = i;
            break;
        }
        if( *currentFATEntry == 0x00 && firstEmptyCluster == -1)
            firstEmptyCluster = i;
    }

    if(firstEmptyCluster == -1)
        return;

    //There is at least 1 free cluster for us so we can calculate the data
    //from Milestone4;

    totNumDirs= 0;
    totRootDirFiles = 0;
    totFiles = 0;
    maxDirLvl = 1;
    largestSize = 0;
    totCapacity = 0;
    totFileSize = 0;
    unusedSpace = 0;
    oldestYear = 4040;
    oldestMonth = 13;
    oldestDay = 32;
    oldestHour = 25;
    oldestMin = 61;
    oldestSec = 61;
    cookieCluster = 0;
    char* path = malloc(4096);
    largestPath = malloc(4096);
    oldestPath = malloc(4096);
    cookiePath = malloc(4096);

    void* entryStart;
    char* subDirAttr;
    char* specialByte;
    unsigned short* clusterNum;
    for(int i = 0; i < *maxRootDirEnts; i++){
        entryStart = rootDir + (i*32);
        specialByte = entryStart;
        subDirAttr = entryStart + 0x0B;
        if(*specialByte != (char)0x00 && *specialByte != (char)0xE5 && *specialByte != (char)0x2E){
            if(*subDirAttr & 0x10){
                totNumDirs++;
                unusedSpace += 2048;
                clusterNum = entryStart + 0x1A;
                char pathName[10];
                memset(pathName, 0, 10);
                pathName[0] = '/';
                int a = 1;
                for(int i = 0; i < 8; i++){
                    char* tempChar = entryStart + i;
                    if(*tempChar != ' '){
                        pathName[a] = *tempChar;
                        a++;
                    }
                }
                strcpy(path, pathName);
                searchCluster(dataStart, FATStart, clusterSize, *clusterNum, DirPerCluster, 2, path);
            }else{
                totFiles++;
                totRootDirFiles++;
                unsigned int* SizeBytes = entryStart + 0x1C;
                if( *SizeBytes > largestSize){
                    largestSize = *SizeBytes;
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    strcpy(largestPath, fileName);
                }
                if(isOldest(entryStart, oldestYear, oldestMonth, oldestDay, oldestHour, oldestMin, oldestSec) == 1){
                    unsigned short* ADate = entryStart + 0x18;
                    short ADateYear = (*ADate >> 9) + 1980;
                    short ADateMonth = (*ADate >> 5) & 0b1111;
                    short ADateDay = *ADate & 0b11111;
                    unsigned short* ATime = entryStart + 0x16;
                    short ATimeHour = *ATime >> 11;
                    short ATimeMin = (*ATime >> 5) & 0b111111;
                    short ATimeSec = (*ATime & 0b11111) * 2;

                    oldestYear = ADateYear;
                    oldestMonth = ADateMonth;
                    oldestDay = ADateDay;
                    oldestHour = ATimeHour;
                    oldestMin = ATimeMin;
                    oldestSec = ATimeSec;

                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(oldestPath, path);
                    *pathEnd = '\0';
                }
                unsigned short* contentCluster = entryStart + 0x1A;
                if(isCookieFile(dataStart, FATStart, entryStart, clusterSize) == 1){
                    char fileName[14];
                    memset(fileName, 0, 14);
                    fileName[0] = '/';
                    int a = 1;
                    for(int i = 0; i < 8; i++){
                        char* tempChar = entryStart + i;
                        if( *tempChar != ' '){
                            fileName[a] = *tempChar;
                            a++;
                        }
                        
                    }
                    char* tempChar = entryStart + 8;
                    if(*tempChar != ' '){
                        fileName[a] = '.';
                        a++;
                        for(int i = 8; i < 11; i++){
                            char* tempChar = entryStart + i;
                            if( *tempChar != ' '){
                                fileName[a] = *tempChar;
                                a++;
                            }
                        }
                    }
                    char* pathEnd = path + strlen(path);
                    strcat(path, fileName);
                    strcpy(cookiePath, path);
                    *pathEnd = '\0';
                    cookieCluster = (int)*contentCluster;
                }
                totFileSize += *SizeBytes;
            }
        }
        if(*specialByte == (char)0x00){
            firstOpenDirAddr = entryStart;
            break;
        }
    }

    ////////////////////////////////////////
    ////  Prepare data for ANSWERS.TXT  ////
    ////////////////////////////////////////
    char fsDataBuffer[4096];
    memset(fsDataBuffer, 0, 4096);

    char numTemp[100];
    memset(numTemp, 0, 100);

    strcat(fsDataBuffer, "Number of files in root directory: ");
    sprintf(numTemp, "%d", totRootDirFiles);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Number of files in the file system: ");
    sprintf(numTemp, "%d", totFiles);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");


    strcat(fsDataBuffer, "Number of directories in the file system: ");
    sprintf(numTemp, "%d", totNumDirs);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    totCapacity = intNumLogS * (int)(*BPS);
    int numAllocatedEntries = 0;

    int numFATEntries = (FATRegionSize / 2) / (int)*numFATs;
    for(int i = 2; i < numFATEntries; i++){
        unsigned short* currentFATEntry = FATStart + (i*2);
        if(*currentFATEntry != 0x00){
            numAllocatedEntries++;
        }
    }
    totAllocatedSize = numAllocatedEntries * clusterSize;
    unallocatedSpace = totCapacity - totAllocatedSize;
    unusedSpace = totAllocatedSize - totFileSize;

    strcat(fsDataBuffer, "Total capacity of the file system: ");
    sprintf(numTemp, "%d", totCapacity);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Total allocated space: ");
    sprintf(numTemp, "%d", totAllocatedSize);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Total size of files: ");
    sprintf(numTemp, "%d", totFileSize);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Unused, but allocated, space (for files): ");
    sprintf(numTemp, "%d", unusedSpace);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Unallocated space: ");
    sprintf(numTemp, "%d", unallocatedSpace);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Largest file (");
    sprintf(numTemp, "%d", largestSize);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, " bytes): ");
    strcat(fsDataBuffer, largestPath);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Path to file with cookie: ");
    strcat(fsDataBuffer, cookiePath);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Starting cluster for file with cookie: ");
    sprintf(numTemp, "%d", cookieCluster);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Directory hierarchy levels: ");
    sprintf(numTemp, "%d", maxDirLvl);
    strcat(fsDataBuffer, numTemp);
    strcat(fsDataBuffer, "\n");

    strcat(fsDataBuffer, "Oldest file: ");
    strcat(fsDataBuffer, oldestPath);
    strcat(fsDataBuffer, "\n");

    ////////////////////////////////////////////////////
    ////  Prepare Directory Header for ANSWERS.TXT  ////
    ////////////////////////////////////////////////////

    char newDirEntry[32];
    memset(newDirEntry, 0, 32);
    //File Shortname
    memset(newDirEntry, 0x41, 1);                    
    memset(newDirEntry+1, 0x4E, 1);
    memset(newDirEntry+2, 0x53, 1);
    memset(newDirEntry+3, 0x57, 1);
    memset(newDirEntry+4, 0x045, 1);
    memset(newDirEntry+5, 0x52, 1);
    memset(newDirEntry+6, 0x53, 1); 
    //File Extention
    memset(newDirEntry+7, 0x20, 1);                  
    memset(newDirEntry+8, 0x54, 1);
    memset(newDirEntry+9, 0x58, 1);  
    memset(newDirEntry+10, 0x54, 1);
    //File Attributes
    memset(newDirEntry+11, 0x20, 1);                
    //Assume 0x00
    memset(newDirEntry+12, 0x00, 1);
    //Create Time ms
    //WIP
    memset(newDirEntry+13, 0, 1);                  
    //Hrs/Min/Sec
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    short formattedCurrentTime = (tm.tm_hour << 11) | (tm.tm_min << 5) | (tm.tm_sec/2);
    memset(newDirEntry+14, formattedCurrentTime, 1);
    memset(newDirEntry+15, formattedCurrentTime >> 8, 1);
    //Year/Month/Da
    short formattedCurrentData = ((tm.tm_year-80) << 9) | ((tm.tm_mon+1) << 5) | tm.tm_mday;
    memset(newDirEntry+16, formattedCurrentData, 1);
    memset(newDirEntry+17, formattedCurrentData>>8, 1);
    //Last Access Date
    memset(newDirEntry+18, formattedCurrentData, 1);
    memset(newDirEntry+19, formattedCurrentData>>8, 1);
    //Extended Attributes
    memset(newDirEntry+20, 0x0000, 2);               
    //Last Modified Time
    memset(newDirEntry+22, formattedCurrentTime, 1);
    memset(newDirEntry+23, formattedCurrentTime >> 8, 1);
    //Last Modified Date
    memset(newDirEntry+24, formattedCurrentData, 1); 
    memset(newDirEntry+25, formattedCurrentData>>8, 1);
    //Start Cluster
    memset(newDirEntry+26, (firstEmptyCluster >> 8), 1);
    memset(newDirEntry+26, firstEmptyCluster, 1);   
    //File Size
    int newFileSize = strlen(fsDataBuffer);
    memset(newDirEntry+28, newFileSize, 1);         
    memset(newDirEntry+29, newFileSize >> 8, 1);
    memset(newDirEntry+30, newFileSize >> 16, 1);
    memset(newDirEntry+31, newFileSize >> 24, 1);

    //Make sure we have space if we need more than one cluster
    if(newFileSize > clusterSize && secondEmptyCluster == -1)
        return;

    //////////////////////
    ////  Update FAT  ////
    //////////////////////

    //printf("%d\n", firstEmptyCluster);

    short EOFVal = 0xFFFF;
    if(newFileSize > clusterSize){
        void* firstFATUpdateAddr = FATStart + (firstEmptyCluster*2);
        void* secondFATUpdateAddr = FATStart + (secondEmptyCluster*2);
        memcpy(firstFATUpdateAddr, (void*)&secondEmptyCluster, 2);
        memcpy(secondFATUpdateAddr, (void*)&EOFVal, 2);
    }else{
        void* firstFATUpdateAddr = FATStart + (firstEmptyCluster*2);
        memcpy(firstFATUpdateAddr, (void*)&EOFVal, 2);
    }

    /////////////////////////////
    ////  Store ANSWERS.TXT  ////
    /////////////////////////////

    int clusterOffset;
    if(newFileSize > clusterSize){
        clusterOffset = (firstEmptyCluster-2) * clusterSize;
        void* firstDataUpdateAddr = dataStart + clusterOffset;
        clusterOffset = (secondEmptyCluster-2) * clusterSize;
        void* secondDataUpdateAddr = dataStart + clusterOffset;
        memcpy(firstDataUpdateAddr, &fsDataBuffer, clusterSize);
        memcpy(secondDataUpdateAddr, (&fsDataBuffer + clusterSize), (newFileSize - clusterSize));
    }else{
        clusterOffset = (firstEmptyCluster-2) * clusterSize;
        void* firstDataUpdateAddr = dataStart + clusterOffset;
        memcpy(firstDataUpdateAddr, &fsDataBuffer, newFileSize);
    }

    ///////////////////////////
    ////  Store Dir Entry  ////
    ///////////////////////////
    memcpy(firstOpenDirAddr, &newDirEntry, 32);

    /////////////////////////////////
    ////  Ensure Updates Happen  ////
    /////////////////////////////////
    munmap(addr, totCapacity);

}