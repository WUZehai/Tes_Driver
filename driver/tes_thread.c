#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/kthread.h>
#include <linux/delay.h>

/** 启动一个内核线程 
 *  kernel_thread()
 *  kthread_create()
 *  kthread_run()
 */

static struct task_struct *dec_task;
static struct task_struct *debug_task;

static int thread_decd(void *arg)
{
	int decCnt = 10;
	while (decCnt--) {
		printk("[decd thread] current cnt is %d\n",decCnt);
		//mdelay( );							//死等，不可用

		//set_current_state(TASK_INTERRUPTIBLE);	//必须先设置线程状态
		//schedule_timeout(HZ * 5);				//HZ为1s

		if (kthread_should_stop())
			break;
		msleep(5000);							//5s打印一次
	}

	return 0;
}

static int thread_debugd(void *arg)
{
	while (!kthread_should_stop()) {
		printk("[thread_debugd] file:%s func:%s alive\n", __FILE__, __FUNCTION__);
		//mdelay( );

		//set_current_state(TASK_INTERRUPTIBLE);
		//schedule_timeout(HZ * 5);

		msleep(20 * 1000);
	}

	return 0;
}

pid_t tes_ktherad_create(void)
{
	/** "kernel_thread" 这个函数用不了,猜测是没有导出符号，要编进内核
	 *  return kernel_thread(just_print, arg, 0);
	 */
	long ret;
	
	dec_task = kthread_create(thread_decd, NULL, "tes->decd");
	if (IS_ERR(dec_task)) {
		ret = PTR_ERR(dec_task);
		dec_task = NULL;
	} else {
		wake_up_process(dec_task);
	}

	debug_task = kthread_run(thread_debugd, NULL, "tes->debugd");
	if (IS_ERR(debug_task)) {
		pr_err("hwrng_fill thread creation failed\n");
		debug_task = NULL;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(tes_ktherad_create);

void tes_kthread_destroy(void)
{
	kthread_stop(debug_task);		//只是进行flag设置,然后等待线程退出，线程中需要调用kthread_should_stop()进行判断
	kthread_stop(dec_task);
}
EXPORT_SYMBOL_GPL(tes_kthread_destroy);


