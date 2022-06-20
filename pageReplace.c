#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

int bytes;
long int pgMiss;
int counter;
int numFrames;
int numEntries;
char* policy;
unsigned long int addr;

int FIFORep();
int randomRep();
int queueContains();
int arrayContains();
void enQueue();
void deQueue();
void FIFOAddwSpace();
void randomAddwSpace();
void FIFOAddwoutSpace();
void randomAddwoutSpace();

struct node { 
    long int key; 
    struct node* next; 
}; 

struct LLQueue { 
    struct node *start, *end; 
}; 

struct node* newNode(int k){ 
    struct node* temp = (struct node*)malloc(sizeof(struct node)); 
    temp->key = k; 
    temp->next = NULL; 
    return temp; 
} 
  
struct LLQueue* createLLQueue(){ 
    struct LLQueue* q = (struct LLQueue*)malloc(sizeof(struct LLQueue)); 
    q->start = q->end = NULL; 
    return q; 
} 

void main(int argc, char **argv){
    counter = 0;    
    numFrames = 0;
    policy = (char *) malloc(7);

    int opt;
    int digit_optind = 0;

    while(1){
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"policy",     required_argument,  NULL,  'p'},
            {"num-frames", required_argument,  NULL,  'n'},
            {0,            0,                  0,       0},
        };

        opt = getopt_long(argc, argv, "p: n:", long_options, &option_index);

        if( opt == -1)
            break;

        switch (opt){
            case 'p':
                strcpy( policy, optarg);
                break;
            case 'n':
                numFrames = atoi(optarg);
                break;
            case '?':
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", opt);
        }
    }

    if (optind < argc){
        printf("non-option ARGV-elements : ");
        while( optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    if(strcmp(policy, "random") == 0){
        policy = "Random";
        counter = randomRep();
    }else if(strcmp(policy, "FIFO") == 0){
        counter = FIFORep();
    }else{
        fprintf(stderr, "Unknown Policy: %s\n", policy);
        exit(EXIT_FAILURE);
    }

    printf("%s %d: %d\n", policy, numFrames, counter);
    exit(EXIT_SUCCESS);
}

int randomRep(){
    numEntries = 0;
    size_t bufferSize = 0;
    int pageContained = 0;
    long int pageStartNum = 0;
    long int pageEndNum = 0;
    long int PT[numFrames]; 
    char type;
    char* instruc;

    for(int i = 0; i < numFrames; i++){
        PT[i] = -1;
    }

    while(getline(&instruc, &bufferSize, stdin) > 0){
        if(instruc[0] == ' '){
            sscanf(instruc, " %c %lx,%d", &type, &addr, &bytes);
        }else{
            sscanf(instruc, "%c  %lx,%d", &type, &addr, &bytes);
        }

        if( (type != 'I') && (type != 'L') && (type != 'M') && (type != 'S') )
            continue; 

        pageStartNum = addr >> 12;
        pageEndNum = ((addr + (bytes)) - 1) >> 12;

        if(pageStartNum != pageEndNum){
            pageContained = arrayContains(PT, pageStartNum);
            if(!pageContained){
                if(numEntries == numFrames){
                    randomAddwoutSpace(PT, pageStartNum);
                }else{
                    randomAddwSpace(PT, pageStartNum);
                }
            }
        }

        pageContained = arrayContains(PT, pageEndNum);
        if(!pageContained){
            if(numEntries == numFrames){
                randomAddwoutSpace(PT, pageEndNum);
            }else{
                randomAddwSpace(PT, pageEndNum);
            }
        }
    }
    return pgMiss;
}

void randomAddwSpace(long int a[numFrames], long int page){
    a[numEntries] = page;
    pgMiss++;
    numEntries++;
}

void randomAddwoutSpace(long int a[numFrames], long int page){
    int temp = rand() % numFrames;
    a[temp] = page;
    pgMiss++;
}

int arrayContains(long int a[numFrames], long int pageNum){
    int temp = 0;
    while(temp != numEntries){
        if(a[temp] == pageNum)
            return 1;
        temp++;
    }
    return 0;
}

int FIFORep(){
    numEntries = 0;
    size_t bufferSize = 0;
    int pageContained = 0;
    long int pageStartNum = 0;
    long int pageEndNum = 0;
    char type;
    char* instruc;
    struct LLQueue* queue = createLLQueue(numFrames); 

    while( getline(&instruc, &bufferSize, stdin) > 0){
        if(instruc[0] == ' '){
            sscanf(instruc, " %c %lx,%d", &type, &addr, &bytes);
        }else{
            sscanf(instruc, "%c  %lx,%d", &type, &addr, &bytes);
        }

        if( (type != 'I') & (type != 'M') & (type != 'S') & (type != 'L') )
               continue; 

        pageStartNum = addr >> 12;
        pageEndNum = ((addr + (bytes)) - 1) >> 12;

        if(pageStartNum != pageEndNum){
            pageContained = queueContains(queue, pageStartNum);
            if(!pageContained){
                if(numEntries == numFrames){
                    FIFOAddwoutSpace(queue, pageStartNum);
                }else{
                    FIFOAddwSpace(queue, pageStartNum);
                }
            }
        }

        pageContained = queueContains(queue, pageEndNum);
        if(!pageContained){
            if(numEntries == numFrames){
                FIFOAddwoutSpace(queue, pageEndNum);
            }else{
                FIFOAddwSpace(queue, pageEndNum);
            }
        }
    }
    return pgMiss;
}

void FIFOAddwSpace(struct LLQueue* q, long int page){
    enQueue(q, page);
    pgMiss++;
    numEntries++;
}

void FIFOAddwoutSpace(struct LLQueue* q, long int page){
    deQueue(q);
    enQueue(q, page);
    pgMiss++;
}

int queueContains(struct LLQueue* q, long int pageNum){
    struct node* temp = q->start;
    for(int i = 0; i < numEntries; i++){
        if(temp->key == pageNum){
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}
  
void enQueue(struct LLQueue* q, long k){ 
    struct node* temp = newNode(k); 
  
    if (q->end == NULL) { 
        q->start = q->end = temp; 
        return; 
    } 
  
    q->end->next = temp; 
    q->end = temp; 
} 
  
void deQueue(struct LLQueue* q){ 
    if (q->start == NULL) 
        return; 
  
    struct node* temp = q->start; 
  
    q->start = q->start->next; 
  
    if (q->start == NULL) 
        q->end = NULL; 
    free(temp); 
} 
