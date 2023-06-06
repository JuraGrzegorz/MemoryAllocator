#include <stdio.h>
#include <stdlib.h>
#include "heap.h"



int heap_setup(void){
    memory_manager.memory_start= sbrk(0);
    if(memory_manager.memory_start==(void *)-1){
        return 1;
    }
    memory_manager.first_memory_chunk=0;
    memory_manager.flag=1928;
    return 0;
}
void heap_clean(void){
    if(memory_manager.flag!=1928){
        return;
    }
    if(memory_manager.first_memory_chunk==0){
        memory_manager.flag=0;
        memory_manager.memory_start=0;
        return;
    }

    struct memory_chunk_t *tmp = memory_manager.first_memory_chunk;
    while (1) {
        if (tmp->next == 0){
            break;
        }
        tmp=tmp->next;
    }
    long size=((char *)tmp+44+tmp->size)-((char *)(memory_manager.memory_start));
    sbrk(-size);
    memory_manager.first_memory_chunk=0;
    memory_manager.flag=0;
    memory_manager.memory_start=0;
}
void* heap_malloc(size_t size){
    if(size==0 || memory_manager.flag!=1928){
        return 0;
    }
    if(memory_manager.first_memory_chunk==0){
        struct memory_chunk_t *new= sbrk(size + sizeof(struct memory_chunk_t) + 4);
        if(new==(void *)-1){
            return 0;
        }
        memory_manager.first_memory_chunk=new;
        new->size=size;
        new->free=0;
        new->next=0;
        new->prev=0;
        flag_create(&new,&new->flag);
        for(int i=0;i<2;i++){
            *((char *)new+sizeof(struct memory_chunk_t)+i)='#';
            *((char *)new+sizeof(struct memory_chunk_t)+2+size+i)='#';
        }
        return (void *)((char *)new+sizeof(struct memory_chunk_t)+2);
    }else{
        struct memory_chunk_t *tmp = memory_manager.first_memory_chunk;
        while (1) {
            if (tmp->next == 0 || (tmp->free == 1 && tmp->size >= size+4)){
                break;
            }
            tmp=tmp->next;
        }

        if (tmp->next == 0){
            struct memory_chunk_t *new= sbrk(size + 44);
            if(new==(void *)-1){
                return 0;
            }
            new->size=size;
            new->next=0;
            new->prev=tmp;
            new->free=0;
            tmp->next=new;
            flag_create(&tmp,&tmp->flag);
            flag_create(&new,&new->flag);
            for(int i=0;i<2;i++){
                *(((char *)new+40+i))='#';
                *(((char *)new+new->size+42+i))='#';
            }
            return ((char *) new + sizeof(struct memory_chunk_t)+2);
        }else {
            tmp->size=size;
            tmp->free=0;
            flag_create(&tmp,&tmp->flag);
            for(int i=0;i<2;i++){
                *((char *)tmp+42+size+i)='#';
            }
            return ((char *)tmp+sizeof(struct memory_chunk_t)+2);
        }
    }
}
void* heap_calloc(size_t number, size_t size){
    if(number<1 || size<1){
        return 0;
    }
    void *ptr=heap_malloc(number*size);
    if(ptr==0){
        return 0;
    }
    for(unsigned i=0;i<(number*size);i++){
        *((char *)ptr+i)=0;
    }
    return ptr;
}
void* heap_realloc(void* memblock, size_t count){
    if(count<1 && memblock==0) {
        return 0;
    }

    if(memblock==0){
        return heap_malloc(count);
    }else{

        int error=get_pointer_type(memblock);
        if(error!=6){
            return 0;
        }

       if(count==0){
           heap_free(memblock);
           return 0;
       }
        struct memory_chunk_t *new_tmp= (struct memory_chunk_t *) ((char *) memblock - sizeof(struct memory_chunk_t)-2);

       if(new_tmp->size>count){
           new_tmp->size=count;
           flag_create(&new_tmp,&new_tmp->flag);
           for(int i=0;i<2;i++){
               *((char *)new_tmp+42+count+i)='#';
           }
           return memblock;
       }
       if(new_tmp->size==count){
           return memblock;
       }
       if(new_tmp->next==0){
           struct memory_chunk_t *new = sbrk(count - (new_tmp->size));
           if(new==(void *)-1){
               return 0;
           }
           new_tmp->size=count;
           for(int i=0;i<2;i++){
               *((char *)new_tmp+42+count+i)='#';
           }
           flag_create(&new_tmp,&new_tmp->flag);
           return memblock;
       }

       if(new_tmp->next!=0){
           size_t size=((unsigned long long)new_tmp->next)-(unsigned long long)new_tmp-42;

           if(new_tmp->next->next!=0){
               if(size<count && new_tmp->next->free==1){
                   size=((unsigned long long)new_tmp->next->next)-(unsigned long long)new_tmp-44;
               }
           }
           if(size>=count){
               if(new_tmp->next->next==0 || new_tmp->next->free==0){
                   new_tmp->size=count;
                   for(int i=0;i<2;i++){
                       *((char *)new_tmp+42+count+i)='#';
                   }
                   flag_create(&new_tmp,&new_tmp->flag);
                   return memblock;
               }else{
                   new_tmp->next->next->prev=new_tmp;
                   new_tmp->next=new_tmp->next->next;
                   new_tmp->size=count;
                   for(int i=0;i<2;i++){
                       *((char *)new_tmp+42+count+i)='#';
                   }
                   flag_create(&new_tmp,&new_tmp->flag);
                   flag_create(&new_tmp->next,&new_tmp->next->flag);
                   return memblock;
               }
           }else{
               char * char_new=heap_malloc(count);
               if(char_new==0){
                   return 0;
               }
               for(unsigned int i=0;i<new_tmp->size;i++){
                   *(char_new+i)=*((char*)memblock + i);
               }
               heap_free(memblock);
               return char_new;
           }
       }
    }
    return 0;
}
void  heap_free(void* memblock){
    if(memblock==0){
        return;
    }
    int error=get_pointer_type(memblock);

    if(error!=6){
        return;
    }
    struct memory_chunk_t *new_tmp= (struct memory_chunk_t *) ((char *) memblock - sizeof(struct memory_chunk_t)-2);

    new_tmp->free=1;
    new_tmp->size=new_tmp->size+4;


    if(new_tmp->next!=0){
        new_tmp->size=((unsigned long long)new_tmp->next)-(unsigned long long)new_tmp-40;
    }
    long size=new_tmp->size;
    flag_create(&new_tmp,&new_tmp->flag);
    if(new_tmp->next==0 && new_tmp->prev==0){
        size+=40;
        sbrk(-size);
        memory_manager.first_memory_chunk=0;
        return;
    }

    if(new_tmp->next==0 && new_tmp->prev!=0){
        if(new_tmp->prev->free==0){
            size=new_tmp->size+40;
            new_tmp=new_tmp->prev;
            new_tmp->next=0;
            flag_create(&new_tmp,&new_tmp->flag);
            sbrk(-size);
            return;
        }
        if(new_tmp->prev->free==1){
            if(new_tmp->prev->prev==0){
                size=new_tmp->size+40+new_tmp->prev->size+40;
                sbrk(-size);
                memory_manager.first_memory_chunk=0;
            }else{
                struct memory_chunk_t *prev=new_tmp->prev->prev;
                size=((char *)new_tmp+new_tmp->size+40)-((char *)prev+prev->size+44);
                new_tmp=prev;
                new_tmp->next=0;
                flag_create(&new_tmp,&new_tmp->flag);
                sbrk(-size);
            }
            return;
        }
    }
    if(new_tmp->prev!=0 && new_tmp->next!=0){
        if(new_tmp->prev->free==1 && new_tmp->next->free==1){
            struct memory_chunk_t *prev=new_tmp->prev;
            struct memory_chunk_t *next=new_tmp->next->next;
            prev->size+=new_tmp->size+40+new_tmp->next->size+40;
            prev->next=next;

            if(next!=0){
                next->prev=prev;
                flag_create(&next,&next->flag);
            }
            flag_create(&prev,&prev->flag);
            return;
        }
    }

    if(new_tmp->prev!=0 && new_tmp->next!=0) {
        if (new_tmp->prev->free == 1 && new_tmp->next->free==0) {
            new_tmp = new_tmp->prev;
            new_tmp->size +=new_tmp->next->size+40;
            new_tmp->next->next->prev = new_tmp;
            new_tmp->next = new_tmp->next->next;
            flag_create(&new_tmp,&new_tmp->flag);
            flag_create(&new_tmp->next,&new_tmp->next->flag);
            return;
        }
    }

    if(new_tmp->next!=0){
        if(new_tmp->next->free==1){
            new_tmp->size+=new_tmp->next->size+40;
            new_tmp->next->next->prev=new_tmp;
            new_tmp->next=new_tmp->next->next;
            flag_create(&new_tmp,&new_tmp->flag);
            flag_create(&new_tmp->next,&new_tmp->next->flag);
            return;
        }
    }
}

int heap_validate(void){
    if(memory_manager.flag!=1928){
        return 2;
    }
    if(memory_manager.first_memory_chunk==0){
        return 0;
    }
    struct memory_chunk_t *tmp=memory_manager.first_memory_chunk;
    int sum=0;
    long long tmp_flag;
    while (1){
        sum=0;
        if(tmp==0){
            break;
        }
        flag_create(&tmp,&tmp_flag);
        if(tmp_flag!=tmp->flag){
            return 3;
        }
        for(int i=0;i<2;i++){
            if(*((char *)tmp+40+i)=='#' && *((char *)tmp+42+tmp->size+i)=='#'){
                sum++;
            }
        }
        if(tmp->free==0){
            if(sum!=2){
                return 1;
            }
        }
        tmp=tmp->next;
    }
    return 0;
}
enum pointer_type_t get_pointer_type(const void* const pointer){
    if(pointer==0){
        return 0;
    }
    struct memory_chunk_t *tmp=memory_manager.first_memory_chunk;

    while (1){
        if(tmp==0){
            break;
        }

        if((char *)pointer==(char *)tmp+sizeof(struct memory_chunk_t)+2 && tmp->free==0){
            return 6;
        }

        if((char *)pointer>=(char *)tmp && (char *)pointer<(char *)tmp+40){
            return 2;
        }

        if((char *)pointer>=(char *)tmp+40 && (char *)pointer<(char *)tmp+42 && tmp->free==0){
            return 3;
        }
        if((char *)pointer>=(char *)tmp+tmp->size+42 && (char *)pointer<(char *)tmp+tmp->size+44 && tmp->free==0){
            return 3;
        }

        if((char *)pointer>(char *)tmp+42 && (char *)pointer<(char *)tmp+tmp->size+42 && tmp->free==0){
            return 4;
        }
        if(tmp->free==1 && (char *)pointer>=(char*)tmp +40 && (char *)pointer<(char*)tmp +40+tmp->size){
            return 5;
        }
        tmp=tmp->next;
    }

    return 5;
}
size_t   heap_get_largest_used_block_size(void){
    if(memory_manager.flag!=1928 || 0!=heap_validate() || memory_manager.first_memory_chunk==0){
        return 0;
    }

    size_t res=0;
    struct memory_chunk_t *tmp=memory_manager.first_memory_chunk;

    while (1){
        if(tmp==0){
            break;
        }
        if(tmp->size>res && tmp->free==0){
            res=tmp->size;
        }
        tmp=tmp->next;
    }
    return res;
}
void flag_create(struct memory_chunk_t **val,long long *flag){
    if(val==0 || flag==0){
        return;
    }
    struct memory_chunk_t *tmp;
    tmp=*val;
    if(tmp==0){
        return;
    }
    *flag=0;
    for(int i=0;i<32;i++){
        *flag+=*((char *)tmp+i);
    }
}

