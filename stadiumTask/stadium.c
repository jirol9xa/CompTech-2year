#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

enum 
{
    RUNNER_READY = 0xDEAD,
    SEND_STICK,
    RCVD_STICK,
    MSG_TYPE,
    JUDGE_STICK,
    LAST_STICK,
};

typedef struct
{
    long type;
    char run_status[1];
} Package;

#define SND_FAILED              \
{                               \
    perror("Msgsnd failed");    \
    return -1;                  \
}

#define RCV_FAILED              \
{                               \
    perror("Msgrcv failed");    \
    return -1;                  \
}


int judge_dump(int msg_type)
{
    switch (msg_type)
    {
    case SEND_STICK:
        printf("I'm judge, I gave stick to the first runners\n");
        break;
    case RCVD_STICK:
        printf("I'm judge, I recieved stick from the last runner\n");
        break;
    case RUNNER_READY:
        printf("I'm judge and every runner saied to me, that he is ready to start now\n");
        break;
    default:
        return -1;
    }

    return 0;
}

int runner_dump(int msg_type, int run_num)
{
    if (run_num < 0)
        return -1;

    switch (msg_type)
    {
        case SEND_STICK:
            printf("I' m runner number %d, I gave stick to the next runner or judge\n", run_num);
            break;
        case RCVD_STICK:
            printf("I'm runner numer %d, I recieved stick from prev runner or judge\n", run_num);
            break;
        case RUNNER_READY:
            printf("I'm runner numer %d, I am ready for running now\n", run_num);
            break;
        default:
            return -1;
            break;
    }

    return 0;
}

void clearBuff()
{
    while (getchar() != '\n')
        continue;
}

int main(void)
{
    // FIFO buffer for messages
    int id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);

    int run_amnt = -1;
    printf("Enter the runners amount\n");
    while (!(scanf("%d", &run_amnt) || run_amnt > 0))  
    {
        clearBuff();
        printf("Please, enter the runner amount correctly\n");    
    }

    long start_time = clock();
    int pid, proc_num = 0;
    for (; proc_num < run_amnt; ++proc_num)
    {
        pid = fork();

        if (pid == 0)
        {
            runner_dump(RUNNER_READY, proc_num);
            Package pkg = {RUNNER_READY, (char) RUNNER_READY};
            if (msgsnd(id, &pkg, sizeof(Package), 0) == -1)
            {
                SND_FAILED;
            }
            
            break;
        }
    }

    if (pid != 0)
    {
        // Waiting for runners preparing
        Package pkg;
        for (int i = 0; i < run_amnt; ++i)
            if (msgrcv(id, &pkg, sizeof(Package), RUNNER_READY, 0) == -1)
            {
                RCV_FAILED;
            }
        judge_dump(RUNNER_READY);

        judge_dump(SEND_STICK);
        // Giving a stick to the first runner
        Package judge_pkg = {JUDGE_STICK, (char) JUDGE_STICK};
        if (msgsnd(id, &judge_pkg, sizeof(Package), 0) == -1)
        {
            SND_FAILED;
        } 
       
        // Recieving a stick from the last runner
        if (msgrcv(id, &judge_pkg, sizeof(Package), SEND_STICK | (run_amnt - 1), 0) == -1)
        {
            RCV_FAILED;
        }
        judge_dump(RCVD_STICK);
    
        printf("The time of the race is %ld\n", clock() - start_time);
    }
    else
    {
        Package pkg;
        if (proc_num == 0)
        {
            if (msgrcv(id, &pkg, sizeof(Package), JUDGE_STICK, 0) == -1)
            {
                RCV_FAILED;
            }
        }
        else
        {
            if (msgrcv(id, &pkg, sizeof(Package), SEND_STICK | (proc_num - 1), 0) == -1)
            {
                RCV_FAILED;
            }
        }    
        runner_dump(RCVD_STICK, proc_num);

        runner_dump(SEND_STICK, proc_num);
        Package runner_pkg = {SEND_STICK | proc_num, SEND_STICK | proc_num};
        if (msgsnd(id, &runner_pkg, sizeof(Package), 0) == -1)
        {
            SND_FAILED;
        }
    }

    if (pid != 0)
    {
    int status;
    for (int i = 0; i < run_amnt; ++i)
           while (wait(&status) != -1)
                continue;
    }

    return 0; 
}

