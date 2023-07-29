思考题1：

mrs x8 ,mpidr_el1 
从系统寄存器mpidr_el1 复制到x8
and x8, x8 ,#0xFF 
取x8中值的后8位，其余清零

mpidr_el1 是多核处理器亲和寄存器
其中的后八位是Aff0区域，标记并了多个核

cbz x8， primary 
若x8中为0，则跳转至primary，其中只有Aff0是0 的核会执行此步骤，进入后续初始化流程，其余暂停


思考题4 ：
在反汇编结果中 <init_c> 的 前两行为
   883a8:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
   883ac:	910003fd 	mov	x29, sp
用到了sp寄存器，因此必须在启动前设置启动栈