 #include <stdio.h>  
 #include <mqueue.h>
 
 #define MAX_MSG_LEN     10000
  
 int main(int argc,char **argv) {  
        mqd_t msgq_id;
        char msgcontent[MAX_MSG_LEN];
        int msgsz;
        unsigned int sender;      
        FILE *fp;
    
        msgq_id = mq_open("/test.mq", O_RDWR);
            
        msgsz = mq_receive(msgq_id, msgcontent, MAX_MSG_LEN, &sender);
        
        fp = fopen("/home/box/message.txt", "wb");
        fwrite(msgcontent, 1, msgsz, fp);
        fclose(fp);
            
        mq_close(msgq_id);
     
        return 0;  
 }  