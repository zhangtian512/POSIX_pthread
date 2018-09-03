#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>

#define OS_THREAD_TOTAL  (256)
#define OS_OBJECT_NAME   (16)

typedef struct _ST_OS_TASK
{
    char                 s8Name[OS_OBJECT_NAME];
    pthread_t            stThread;
    pthread_cond_t       stCond;
    pthread_mutex_t      stLock;
    pid_t                stPID;
    int                  iPolicy;
    unsigned char         u8Priority;
    bool                 b8Sleep;
    void                 (*pfnFunction)(void *);
    void                 *pParam;
    unsigned int         handle;
    int                  isToBeDestroyed;
} ST_OS_TASK;

void *Task_Wrap_func(void* arg);
void taskCleanup(ST_OS_TASK *pstTask);
void *Task_Create( void (*pfnFunction)(void *), void *pParam, unsigned int u32Stack, unsigned int u8Priority, const char *pc8Name );
void Task_A(void *arg);
void Task_B(void *arg);

#endif