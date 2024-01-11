#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/printk.h> 
 
MODULE_DESCRIPTION("tsulab"); 
MODULE_AUTHOR("lina"); 
MODULE_LICENSE("GPL"); 
 
int init_module(void){   
  pr_info("Welcome to the Tomsk State University\n");
  
  return 0; 
} 
 
void cleanup_module(void){ 
    pr_info("Tomsk State University forever!\n");
}
