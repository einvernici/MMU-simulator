#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MEM_PRINCIPAL 8	//paginas em RAM 
#define MEM_VIRTUAL 128	//max paginas processo
#define PAGE_SIZE 1	//tamanho pagina
#define FRAME_SIZE 1 	//tamanho frame
#define NUM_PROCESSES 2	//quantidade de processos
#define RUNNING_TIME 1 0 //tempo que programa vai rodar

int pageCount = 0;
int pagehit = 0;
int pagemiss = 0;
int running = 1;

typedef struct Page {
    int number;
} Page;

typedef struct Process {
    int size;
    Page* process_pages[MEM_VIRTUAL];
} Process;

typedef struct MMU {
    Process* process_page_table[NUM_PROCESSES];
    Page* page_memoria[MEM_PRINCIPAL];
} MMU;

MMU mmu_global;

void set_process_pages(Process *p);
bool mmu_consulta(MMU *mmu, int pid);
void page_miss(MMU* mmu, Page* page_needed);
void* thread_processo(void* arg);

void* thread_processo(void* arg) {
    int pid = *(int*)arg;

    while(running){
        if (!mmu_consulta(&mmu_global, pid)){
            sleep(2);
        }

        printf("Processo %d executou normalmente!\n", pid);
        sleep(1);
    }

    return NULL;
}

int main(int argc, char *argv[]){
    srand(time(NULL));

    pthread_t threads[NUM_PROCESSES];
    Process processos[NUM_PROCESSES];
    int ids[NUM_PROCESSES];

    for(int i = 0; i < MEM_PRINCIPAL; i++){
        mmu_global.page_memoria[i] = NULL;
    }

    for(int i = 0; i < NUM_PROCESSES; i++){
        processos[i].size = 16;
        set_process_pages(&processos[i]);
        mmu_global.process_page_table[i] = &processos[i];
    }

    for(int i = 0; i < NUM_PROCESSES; i++){
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_processo, &ids[i]);
    }
    
    sleep(RUNNING_TIME);
    running = 0;

    for(int i = 0; i < NUM_PROCESSES; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Aconteceram %d falta de paginas\nAconteceram %d acertos de pagina\n", pagemiss, pagehit);

    return 0;
}

// Helper functions

// Processo - popula o processo com o numero de paginas que ele possui
void set_process_pages(Process *p){
		for (int i = 0; i < p->size; i++){
        Page *page = malloc(sizeof(Page));
		page->number = pageCount+1;
		p->process_pages[i] = page;
	}
	return;
}

// MMU - consulta na tabela
// hit o processo segue normalmente
// miss mmu poe a pagina na memoria e processo sai do processador
bool mmu_consulta(MMU *mmu, int pid){
    Process* p = mmu->process_page_table[pid];
    int random1 = rand() % p->size;
    Page* page_needed = p->process_pages[random1];

    printf("Processo %d requisitou pagina %d\n", pid, page_needed->number);

    for(int i = 0; i < MEM_PRINCIPAL; i++){
        if(mmu->page_memoria[i] == page_needed){
            printf("Page hit!\n");
            pagehit++;
            return true;
        }
    }

    printf("Page miss!\n");
    pagemiss++;
    page_miss(mmu, page_needed);
    return false;
}

//poe pagina na memoria
void page_miss(MMU* mmu, Page* page_needed){
    int random = rand() % MEM_PRINCIPAL;
    mmu->page_memoria[random] = page_needed;
}
