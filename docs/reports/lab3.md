
思考题1：
在init_c 的最后，head.S 中的 start_kernel 跳转至 main.c中的main函数，
调用arch_interrupt_init初始化异常向量表，调用create_root_thread
其中create_root_cap_group创建根进程，然后在其中创建根线程并将当前线程设置为该线程
最后使用 eret_to_thread(switch_context()) 切换上下文，完成到用户态的过渡。


思考题8：


irq_entry.S中异常向量中的el0_syscall处理方法为：
保存寄存器x0-x15
将syscall_table的地址加载到x27
将syscall编号从w8加载到x16（
chcore中syscall的调用会将其编号传至x8寄存器，例如：

static inline long __chcore_syscall0(long n)
{
        register long x8 __asm__("x8") = n;
        register long x0 __asm__("x0");
        __asm_syscall("r"(x8));
}
将n写入x8
）

计算得到对应syscall handler的地址 [x27, x16, lsl #3]	，放入x16，然后跳转
