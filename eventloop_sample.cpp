#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
using namespace std;

#define DEBUG(x) cout << '>' << #x << ':' << x << endl;
#define FOR(i,a,b) for(int i = (a);i < (b); ++i)

#define REDIS_HZ 250
#define run_with_period(_ms_) if (!(cronloops%((_ms_)/(1000/REDIS_HZ))))
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);

typedef struct aeTimeEvent {
    // 时间事件的唯一标识符
    long long id; /* time event identifier. */

    // 事件的到达时间
    long when_sec; /* seconds */
    long when_ms; /* milliseconds */

    // 事件处理函数
    aeTimeProc *timeProc;

    // 多路复用库的私有数据
    void *clientData;

    // 指向下个时间事件结构，形成链表
    struct aeTimeEvent *next;
} aeTimeEvent;
typedef struct aeEventLoop {
    // 目前已注册的最大描述符
    long long timeEventNextId;
    // 时间事件
    aeTimeEvent *timeEventHead;
    // 事件处理器的开关
} aeEventLoop;

/*
 * 取出当前时间的秒和毫秒，
 * 并分别将它们保存到 seconds 和 milliseconds 参数中
 */
static void aeGetTime(long *seconds, long *milliseconds)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec/1000;
}
/*
 * 为当前时间加上 milliseconds 秒。
 */
static void aeAddMillisecondsToNow(long long milliseconds, long *sec, long *ms) {
    long cur_sec, cur_ms, when_sec, when_ms;

    // 获取当前时间
    aeGetTime(&cur_sec, &cur_ms);

    // 计算增加 milliseconds 之后的秒数和毫秒数
    when_sec = cur_sec + milliseconds/1000;
    when_ms = cur_ms + milliseconds%1000;

    // 进位：
    // 如果 when_ms 大于等于 1000
    // 那么将 when_sec 增大一秒
    if (when_ms >= 1000) {
        when_sec ++;
        when_ms -= 1000;
    }
    *sec = when_sec;
    *ms = when_ms;
}

long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc,void *clientData)
{
    // 更新时间计数器
    long long id = eventLoop->timeEventNextId++;
    aeTimeEvent *te;

    te = (aeTimeEvent*)malloc(sizeof(*te));
    if (te == NULL) return -1;

    te->id = id;

    // 设定处理事件的时间
    aeAddMillisecondsToNow(milliseconds,&te->when_sec,&te->when_ms);
    te->timeProc = proc;
    te->clientData = clientData;

    // 将新事件放入表头
    te->next = eventLoop->timeEventHead;
    eventLoop->timeEventHead = te;

    return id;
}

/*
 * 初始化事件处理器状态
 */
aeEventLoop *aeCreateEventLoop() {
    aeEventLoop *eventLoop;
    int i;

    // 创建事件状态结构
    if ((eventLoop = (aeEventLoop*)malloc(sizeof(*eventLoop))) == NULL) 
        return NULL;

    // 初始化时间事件结构
    eventLoop->timeEventHead = NULL;
    eventLoop->timeEventNextId = 0;

    /* Events with mask == AE_NONE are not set. So let's initialize the
     * vector with it. */
    return eventLoop;
}

/*
 * 删除事件处理器
 */
void aeDeleteEventLoop(aeEventLoop *eventLoop) {
    free(eventLoop);
}

/*
 * Time Process Function 
 * */
long long cronloops = 0;
int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    run_with_period(1000000) {
        cout << "this is 1000 ms running" << cronloops << endl;
    };
    run_with_period(2000000) {
        cout << "this is 2000 ms running" << cronloops << endl;
    };
    cronloops++;
    if(cronloops > 80000000)//define max
        cronloops = 0;
    return 1000/REDIS_HZ;
}

/* Process time events
 *
 * 处理所有已到达的时间事件
 */
static int processTimeEvents(aeEventLoop *eventLoop) {
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;
    time_t now = time(NULL);

    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId-1;
    while(te) {
        long now_sec, now_ms;
        long long id;

        // 获取当前时间
        aeGetTime(&now_sec, &now_ms);

        // 如果当前时间等于或等于事件的执行时间，那么执行这个事件
        if (now_sec > te->when_sec ||
                (now_sec == te->when_sec && now_ms >= te->when_ms))
        {
            int retval;

            id = te->id;
            retval = te->timeProc(eventLoop, id, te->clientData);
            processed++;
            // 因为执行事件之后，事件列表可能已经被改变了
            // 因此需要将 te 放回表头，继续开始执行事件
            te = eventLoop->timeEventHead;
        } else {
            te = te->next;
        }
    }
    return processed;
}

int main(){
    aeEventLoop* el;
    el = aeCreateEventLoop();//initialize
    aeCreateTimeEvent(el, 100, serverCron, NULL);//register
    cout << "processing....." << endl;
    while(1) {
        processTimeEvents(el); //process
    }
    return 0;
}