
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <assert.h>
#include <string.h>
#include "common.h"

static ST_OS_TASK g_stTask[OS_THREAD_TOTAL];//全局Task变量，用来管理创建的线程

//线程运行函数
void *Task_Wrap_func(void* arg)
{
    if( NULL == arg )
    {
        perror("Task_Wrap_func arg is null\n");
    }
    
    ST_OS_TASK* pstTask = (ST_OS_TASK*)arg;
    
    void (*routine)(void *) = (void (*)(void*))(&taskCleanup);
    pstTask->stPID = syscall( SYS_gettid );

    printf("Task_Wrap_func in name:%s pid:%d self_tid:%ld real_tid:%d\n",
        pstTask->s8Name,getpid(),pthread_self(),pstTask->stPID);
    pthread_cleanup_push(routine, arg);
    if( NULL != pstTask->pfnFunction )
    {
        pstTask->pfnFunction( pstTask->pParam );
    }
    else
    {
        perror("func is null\n");
    }
    pthread_cleanup_pop(1);

    printf("Task_Wrap_func out\n");
    return NULL;
}

//线程资源释放函数
void taskCleanup(ST_OS_TASK *pstTask)
{
    printf("taskCleanup in name:%s\n",pstTask->s8Name);
    if( NULL != pstTask )
    {
        if( 0 == pstTask->stThread )
        {
            //do nothing because the thread is still exists
        }
        else
        {
            pthread_mutex_destroy(&pstTask->stLock);
            pthread_cond_destroy(&pstTask->stCond);
            memset( pstTask->s8Name, 0, OS_OBJECT_NAME );
            pstTask->stThread = 0;
            pstTask->handle = 0xFFFFFFFF;
            pstTask->isToBeDestroyed = false;
        }
    }
    printf("taskCleanup out\n");
}

//线程创建函数
void* Task_Create( void (*pfnFunction)(void *), void *pParam, \
    unsigned int u32Stack, unsigned int u8Priority, const char *pc8Name )
{
    int nRet = 0;
    int i = 0;
    ST_OS_TASK *pstTask = NULL;
    pthread_attr_t stThreadAttr;
    
    printf("Task_Create in name:%s\n",pc8Name);
 
    for( i = 0; i < OS_THREAD_TOTAL; i++ )
    {
        if( 0 == g_stTask[i].stThread )
        {
            pstTask = &(g_stTask[i]);
            break;
        }
    }

    memset( pstTask->s8Name, 0, OS_OBJECT_NAME );
    //线程属性初始化
    pthread_attr_init( &stThreadAttr );
    //线程scope属性，表示线程是与进程内线程竞争还是和所有线程一起竞争
    pthread_attr_setscope( &stThreadAttr, PTHREAD_SCOPE_PROCESS );
    //设置线程栈大小
    pthread_attr_setstacksize( &stThreadAttr, u32Stack );
    //设置线程是否显示指定调度策略
    pthread_attr_setinheritsched(&stThreadAttr, PTHREAD_EXPLICIT_SCHED);
    //设置调度策略
    pthread_attr_setschedpolicy( &stThreadAttr, SCHED_OTHER );
    //设置线程结束是否自动释放线程资源
    pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);

    pthread_mutex_init( &( pstTask->stLock ), NULL );//初始化互斥锁
    pthread_cond_init( &( pstTask->stCond ), NULL );//初始化条件变量

    pthread_mutex_lock(&(pstTask->stLock));

    pstTask->pfnFunction = pfnFunction;
    pstTask->pParam      = pParam;
    pstTask->b8Sleep     = false;
    pstTask->iPolicy     = SCHED_OTHER;
    pstTask->u8Priority  = u8Priority;
    pstTask->handle      = i+1000;
    memcpy( pstTask->s8Name, pc8Name, OS_OBJECT_NAME );
 
    //参数1：线程tid 2:线程属性结构体 3:线程函数入口 4:线程函数参数
    nRet = pthread_create( &(pstTask->stThread), &stThreadAttr, Task_Wrap_func, pstTask);
    assert(0 == nRet);
    printf("pthread_create success tid:%ld\n",(unsigned long)pstTask->stThread);
    
    pthread_attr_destroy(&stThreadAttr);
    pthread_mutex_unlock(&(pstTask->stLock));
    
    printf("Task_Create out name:%s\n",pc8Name);
}

//任务A
void Task_A(void *arg)
{
    printf("Task_A in\n");
    int i = 1;
    while(1)
    {
        printf("Task_A run %d time\n",i);
        i++;
        //sleep(1);
    }
}
//任务B
void Task_B(void *arg)
{
    printf("Task_B in\n");
    int i = 1;
    while(1)
    {
        printf("Task_B run %d time\n",i);
        i++;
        //sleep(1);
    }
}

int main()
{
    printf("main start\n");
    memset( &(g_stTask[0]), 0, (OS_THREAD_TOTAL*sizeof(ST_OS_TASK)) );
    
    Task_Create(Task_A, NULL, 4096, 16, "Task_A");
    Task_Create(Task_B, NULL, 4096, 16, "Task_B");

    sleep(100);
    printf("main end\n");
    return 0;
}
