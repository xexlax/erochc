思考题1：

选择主cpu:
mpidr_el1中的值可以看作CPU的编号
代码读取mpidr_el1中的值，若为0则跳转至primary流程

其余的CPU继续后续的wait_for_bss_clear检查bss段是否清除
检查clear_bss_flag，若不为0则循环
然后执行wait_until_smp_enabled
检查secondary_boot_flag，当其他cpu被唤醒时secondary_boot_flag会被设为非0，解除阻塞，否则处于循环中

思考题2：
由于其他CPU的MMU还未初始化，secondary_boot_flag只能是物理地址
在init_c中调用start_kernel(secondary_boot_flag)，在start_kernel中作为存在x0中的第一参数被保存到栈中，刷新tlb后取回，然后跳转至main
即作为main函数的第一个参数boot_flag（物理地址）使用，调用enable_smp_cores(boot_flag)时传递

init_c中，secondary_boot_flag初始化为{NOT_BSS, 0, 0, ...}
在enable_smp 中通过 phys_to_virt(boot_flag) 转换为虚拟地址 secondary_boot_flag 再进行赋值

思考题5：
保存在栈中的是caller-saved寄存器 x0-x15，在其他函数中可以直接调用不需要考虑对其的影响，
而调用unlock之后紧接着就返回了，所以不用储存在栈中

思考题6：
若idle_thread 加入队列，则会被调度器取出并运行，而其本身并没有运行的意义，不应该被加入队列同其它线程一样对待

思考题8:
若空闲线程不获得锁而捕获异常，在异常处理函数中返回仍会释放大内核锁，导致其owner和next错位，造成阻塞

prodcon.bin  
可以通过运行测试但无法在评分脚本中获得分数，原因未知