#break run_tasks
break init_PS2
break long_mode_start
#break ps2_rx_task
#break vga_task
#break hw_serial_task
break hw_serial_init
break serial_tx_handler
break asm_int_handler

disp vga_channel
disp ps2_ipc

c
