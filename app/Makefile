APP_CC := arm-buildroot-linux-gnueabihf-gcc

sys_notify:sys_notify.c
	@echo "[start] build sys_notify ..."
	@${APP_CC} $^ -o out/$@
	@echo "[end] sys_notify!"

all:sys_notify

clean:
	@rm -rf out/sys_notify
