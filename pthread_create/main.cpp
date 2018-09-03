
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <assert.h>
#include <string.h>
#include "common.h"

static ST_OS_TASK g_stTask[OS_THREAD_TOTAL];//ȫ��Task�������������������߳�

//�߳����к���
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

//�߳���Դ�ͷź���
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

//�̴߳�������
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
    //�߳����Գ�ʼ��
    pthread_attr_init( &stThreadAttr );
    //�߳�scope���ԣ���ʾ�߳�����������߳̾������Ǻ������߳�һ����
    pthread_attr_setscope( &stThreadAttr, PTHREAD_SCOPE_PROCESS );
    //�����߳�ջ��С
    pthread_attr_setstacksize( &stThreadAttr, u32Stack );
    //�����߳��Ƿ���ʾָ�����Ȳ���
    pthread_attr_setinheritsched(&stThreadAttr, PTHREAD_EXPLICIT_SCHED);
    //���õ��Ȳ���
    pthread_attr_setschedpolicy( &stThreadAttr, SCHED_OTHER );
    //�����߳̽����Ƿ��Զ��ͷ��߳���Դ
    pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);

    pthread_mutex_init( &( pstTask->stLock ), NULL );//��ʼ��������
    pthread_cond_init( &( pstTask->stCond ), NULL );//��ʼ����������

    pthread_mutex_lock(&(pstTask->stLock));

    pstTask->pfnFunction = pfnFunction;
    pstTask->pParam      = pParam;
    pstTask->b8Sleep     = false;
    pstTask->iPolicy     = SCHED_OTHER;
    pstTask->u8Priority  = u8Priority;
    pstTask->handle      = i+1000;
    memcpy( pstTask->s8Name, pc8Name, OS_OBJECT_NAME );
 
    //����1���߳�tid 2:�߳����Խṹ�� 3:�̺߳������ 4:�̺߳�������
    nRet = pthread_create( &(pstTask->stThread), &stThreadAttr, Task_Wrap_func, pstTask);
    assert(0 == nRet);
    printf("pthread_create success tid:%ld\n",(unsigned long)pstTask->stThread);
    
    pthread_attr_destroy(&stThreadAttr);
    pthread_mutex_unlock(&(pstTask->stLock));
    
    printf("Task_Create out name:%s\n",pc8Name);
}

//����A
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
//����B
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
