
extern int  emmc_model_init(void);
extern void emmc_model_exit(void);
extern long emmc_model_ioctl(unsigned int cmd, void *arg, unsigned long len);

int  emmc_init(void) { return emmc_model_init(); }
void emmc_exit(void) { emmc_model_exit(); }
long emmc_send_cmd(unsigned int cmd, void *arg, unsigned long len)
{
    return emmc_model_ioctl(cmd, arg, len);
}
