/* sys_call.c */

#include "sys_call.h"
#include "x86_desc.h"
#include "file_drivers.h"
#include "terminal.h"
#include "rtc_drivers.h"
#include "process.h"
#include "lib.h"
#include "scheduling.h"

#include "paging.h"

/* File operations for all the different kinds of file types */
static int32_t std_in_fops[] = {(int32_t) terminal_open, (int32_t) terminal_read, NULL, (int32_t) terminal_close};
static int32_t std_out_fops[] = {(int32_t) terminal_open, NULL, (int32_t) terminal_write, (int32_t) terminal_close};
static int32_t file_fops[] = {(int32_t) open_file, (int32_t) read_file, (int32_t) write_file, (int32_t) close_file};
static int32_t directory_fops[] = {(int32_t) open_directory, (int32_t) read_directory, (int32_t) write_directory, (int32_t) close_directory};
static int32_t rtc_fops[] = {(int32_t) open_rtc, (int32_t) read_rtc, (int32_t) write_rtc, (int32_t) close_rtc};

/* sys_halt
 * Description: The halt system call terminates a proccess, returning the specified value to its
 * parent proccess. The system call handler itself is responsible for expanding 
 * the 8-bit argument from BL into the 32-bit return value to the parent program's
 * execute system call. Be careful not to return all 32 bits from EBX. This 
 * call should never return to the caller.
 * Inputs: 8-bit argument from BL
 * Outputs: Should never return to caller
 */
int32_t sys_halt(uint8_t status)
{
    /* Local variables */
    pcb_t* pcb; /* Pointer to the current PCB */
    uint32_t i; /* Iteration variable (for closing FDs) */
    uint32_t retval;
    uint32_t new_phys_addr;
    uint32_t offset;
    
    
    /***   1. Close any relevant FDs ***/
    /* Loop through FDs and close if in use */
    for (i = FIRST_FD; i < FD_ARRAY_SIZE; i++)
    {
        sys_close(i);
    }
    
    
    /***   2. Restore parent data ***/ /* NOTE: Not sure exactly what should go here... */
    /* Setting the current PCB */
    pcb = CURRENT_PCB_ADDRESS;
    
    /* Restoring the process information */
    REMOVE_PID(global_pid);
    
    /* Restarting shell if we try to quit out of a base shell */
    if (pcb->parent_pid == -1)
    {
        reset_screen();
        sys_execute((uint8_t*)"shell");
    }
    
    /* Restoring the former ESP0 based on parent pid */
    global_pid = pcb->parent_pid;
    tss.esp0 = pcb->parent_esp0;
    
    
    /***   3. Restore parent paging ***/
    offset = global_pid;
    new_phys_addr = (PID_OFFSET + offset) * FOUR_M;
    page_modify(VIRTUAL_BEGIN, new_phys_addr, USER_PRIV);
    
    
    /*** 3.5. Restore task's PCB ***/
    CURRENT_TASK.pcb = pcb->parent_pcb;
    CURRENT_TASK.pid = pcb->parent_pid;
    
    
    /***   4. Jump to execute return ***/
    /* Determining the proper return value based on status code */
    retval = (uint32_t) status;
    
    /* Restoring parent's values of ESP and EBP and doing a hacky jump to parent */
    asm volatile ("        \n\
            movl %0, %%eax \n\
            movl %1, %%esp \n\
            movl %2, %%ebp \n\
            leave          \n\
            ret            \n\
            "
            : 
            : "r"(retval), "r"(pcb->parent_esp), "r"(pcb->parent_ebp)
            : "esp", "ebp"
    );
    /* Return */
    /* In theory should never reach here */
    return -1;
}

/* sys_execute
 * Description: attempts to load and execute a new program, 
 * handing off the proccessor to the new program until it terminates. 
 * Inputs: command is a space-separated sequence of words. 
 * The first word is the file name of the program to be executed, 
 * and the rest of the command stripped of leading spaces should be provided to the new
 * program on request via the getargs system call. 
 * Returns: -1 if the command cannot be executed,
 * for example, if the program does not exist or the filename specified is not 
 * an executable, 256 if the program dies by an exception, 
 * or a value in the range 0 to 255 if the program executes a halt system 
 * call, in which case the value returned is that given by the program's 
 * call to halt.
 */
int32_t sys_execute(const uint8_t* command)
{
    /* Local variables */
    uint32_t eip, cs, eflags, esp, ss; /* Values for IRET stack */
    uint32_t new_pid;    /* PID values for the new and parent processes */
    uint32_t parent_pid; /* See above ^ */
    uint8_t parsed_cmd[MAX_CMD_SIZE];
    uint8_t parsed_arg[MAX_CMD_SIZE];
    uint32_t cmd_start_index;
    uint32_t cmd_end_index;
    uint32_t arg_start_index;
    uint32_t arg_end_index;
    uint32_t exec_fd;
    uint32_t exec_header;
    pcb_t* pcb;
    
    
    /***   0. See if process is available ***/
    /* Assign the new pid to the first available one */
    FIRST_AVAILABLE_PID(new_pid);
    
    /* Return if there's no more processes available */
    if (!(~new_pid))
    {
        return -1;
    }
    
    /* Assign parent pid based on whether or not it's the one of the first processes */
    if (new_pid < NUM_TERMINALS)
    {
        parent_pid = -1;
    }
    else
    {
        parent_pid = global_pid;
    }
    
    /***   1. Parse args ***/
    /* Returning error if command is invalid or just a null character */
    if (!command || !(*command))
    {
        REMOVE_PID(new_pid);
        return -1;
    }
    
    /* Clearing out the parsed command and arg */
    memset(parsed_cmd, NULL, MAX_CMD_SIZE);
    memset(parsed_arg, NULL, MAX_CMD_SIZE);
    
    /* Finding the first non-space character index (AKA start of command) */
    cmd_start_index = 0;
    while (command[cmd_start_index] == SPACE_CHAR)
    {
        cmd_start_index++;
    }
    
    /* Finding the last non-space character index and copying command (last character can be null) */
    cmd_end_index = cmd_start_index;
    while (command[cmd_end_index] && (command[cmd_end_index] != SPACE_CHAR) && (command[cmd_end_index] != NEWLINE_CHAR))
    {
        parsed_cmd[cmd_end_index - cmd_start_index] = command[cmd_end_index];
        cmd_end_index++;
    }
    
    /* Finding the first non-space character index after the command (AKA start of args) */
    arg_start_index = cmd_end_index;
    while (command[arg_start_index] == SPACE_CHAR)
    {
        arg_start_index++;
    }
    
    /* Finding the last character index and copying args (last character can be null) */
    arg_end_index = arg_start_index;
    while (command[arg_end_index] && (command[arg_end_index] != NEWLINE_CHAR))
    {
        parsed_arg[arg_end_index - arg_start_index] = command[arg_end_index];
        arg_end_index++;
    }
    
    /***   2. Check file validity ***/
    /* Checking if file exists */
    if ((exec_fd = sys_open(parsed_cmd)) == -1)
    {
        REMOVE_PID(new_pid);
        return -1;
    }
    
    /* Get file header */
    read_file(exec_fd, (uint8_t*) &exec_header, sizeof(uint32_t));

    /* Check to see if it matches the ELF header, if not return error */
    if (exec_header != ELF_HEADER)
    {
        REMOVE_PID(new_pid);
        return -1;
    }
    
    /***   3. Set up paging ***/
    /* Change page directory to one with virtual address set for user program to use */
    uint32_t new_phys_addr = (PID_OFFSET + new_pid) * FOUR_M;
    page_modify(VIRTUAL_BEGIN, new_phys_addr, USER_PRIV);

    /***   4. Load file into memory ***/
    /* Reading the file into memory, and then closing the file */
    uint32_t length = file_size((uint8_t*)parsed_cmd);
    read_data(CURRENT_PCB_ADDRESS->fd_array[exec_fd].inode, 0, (uint8_t*)PROGRAM_START_ADDRESS, length);
    sys_close(exec_fd);
    
    /***   5. Create PCB/Open FDs ***/
    /* Calculating the relevant addresses */
    pcb = PCB_ADDRESS(new_pid);
    
    /* Assign the parent pid number in the PCB */
    pcb->parent_pid = parent_pid;
    pcb->terminal = CURRENT_TASK.terminal;
    pcb->parent_pcb = PCB_ADDRESS(parent_pid);
    strncpy((int8_t*) pcb->args, (int8_t*) parsed_arg, MAX_ARG_SIZE);
    
    /* Getting the values of eflags, esp, and ebp */
    asm volatile ("        \n\
            pushfl         \n\
            popl %0        \n\
            movl %%esp, %1 \n\
            movl %%ebp, %2 \n\
            "
            : "=r"(eflags), "=r"(pcb->parent_esp), "=r"(pcb->parent_ebp)
            : 
    );
    
    /* Index 0 and 1 are stdin and stdout respectively */
    pcb->fd_array[0].file_ops = std_in_fops;
    pcb->fd_array[0].flags |= FD_IN_USE;
    pcb->fd_array[1].file_ops = std_out_fops;
    pcb->fd_array[1].flags |= FD_IN_USE;
    
    /* Assigning the kernel stack address to the TSS's ESP0 */
    pcb->parent_esp0 = tss.esp0;
    tss.esp0 = KERNEL_STACK_ADDRESS(new_pid);
    
    
    /*** 5.5. Populate task structure ***/
    CURRENT_TASK.pcb = pcb;
    CURRENT_TASK.pid = new_pid;
    
    /***   6. Prepare for context switch ***/
    /* Assign/calculate the relevant values for the context switch */
    eip = *((uint32_t*)(PROGRAM_START_ADDRESS + ENTRY_POINT_LOCATION));
    cs = USER_CS;
    eflags |= ENABLE_INTERRUPTS;
    esp = VIRTUAL_STACK_START;
    ss = USER_DS;
    
    /* Update to indicate that there's a new process running now */
    global_pid = new_pid;

    /* Initialize the pcb_started varfor use with the virtual RTC */

    pcb_all_started = pit_interrupt_counter;
    
    
    /*** 7/8. Push IRET context to stack and switch context ***/
    /* Assembly function that calls IRET with args as IRET stack */
    context_switch(eip, cs, eflags, esp, ss);

    
    /***   9. Return ***/
    /* In theory should never reach here */
    return -1;
}

/* sys_read
 * Description: Links the read syscall with the fd array fops pointer of
 * the current process. 
 * Inputs: A file descriptor, a buffer to read into, and the number of bytes to read
 * Returns: -1 on failure; the number of bytes read for a file; 
 * always 0 for RTC after an interrupt has occurred
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
{
    //bounds check
    if (fd != 0 && (fd < FIRST_FD || fd > FD_ARRAY_SIZE -1))
    {
        return -1;
    }

    if (nbytes < 0)
    {
        return -1;
    }

    if (!buf)
    {
        return -1;
    }
    
    if (!((CURRENT_PCB_ADDRESS)->fd_array[fd].flags & FD_IN_USE))
    {
        return -1;
    }

    //keep track of state inside of file object
    return ((read_t)((CURRENT_PCB_ADDRESS)->fd_array[fd].file_ops[FOPS_READ]))(fd, buf, nbytes);
}

/* sys_write
 * Description: Links the write syscall with the fd array fops pointer of
 * the current process. 
 * Inputs: A file descriptor, a buffer to write into, and the number of bytes to read
 * Returns: Writes to regular files should always return -1 to indicate failure since
 * the file system is read-only. The call returns the number of bytes
 * written, or -1 on failure.
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
    if (fd != 1 && (fd < FIRST_FD || fd > FD_ARRAY_SIZE -1))
    {
        return -1;
    }

    if (nbytes < 0)
    {
        return -1;
    }

    if (!buf)
    {
        return 0;
    }
    
    if (!((CURRENT_PCB_ADDRESS)->fd_array[fd].flags & FD_IN_USE))
    {
        return -1;
    }

    return ((write_t)((CURRENT_PCB_ADDRESS)->fd_array[fd].file_ops[FOPS_WRITE]))(fd, buf, nbytes);
}

/* sys_open
 * Description: The open system call provides access to the filesystem. 
 * The call should find the directory entry corresponding to the
 * named file,allocate an unused file descriptor, 
 * and set up any data neccessary to handle the given type of file (directory,
 * RTC, device, or regular file). 
 * Input: A filename.
 * Returns: If the named file does not exist or no 
 * descriptors are free, the call returns -1.
 */
int32_t sys_open(const uint8_t* filename)
{

    // Find directory entry corresponding to the named file
    d_entry_t working_dentry;
    int32_t i;
    int32_t filetype;
    
    if (read_dentry_by_name(filename, &working_dentry) == -1)
    {
        return -1;
    }
    
    // Find an unused file descriptor and allocate it
    for (i = FIRST_FD; i < FD_ARRAY_SIZE; i++){
        if (!((CURRENT_PCB_ADDRESS)->fd_array[i].flags & FD_IN_USE))
        {
            (CURRENT_PCB_ADDRESS)->fd_array[i].flags |= FD_IN_USE;
            filetype = working_dentry.filetype;
            //Invole actual syscall for that type, and reset position if relevant
            switch(filetype)
            {
                case FILETYPE_RTC: 
                    open_rtc(filename);
                    CURRENT_PCB_ADDRESS->fd_array[i].file_ops = rtc_fops;
                    break;
                case FILETYPE_DIRECTORY: 
                    open_directory(filename);
                    CURRENT_PCB_ADDRESS->fd_array[i].file_ops = directory_fops;
                    CURRENT_PCB_ADDRESS->fd_array[i].position = 0;
                    break;
                case FILETYPE_FILE: 
                    CURRENT_PCB_ADDRESS->fd_array[i].inode = open_file(filename);
                    CURRENT_PCB_ADDRESS->fd_array[i].file_ops = file_fops;
                    CURRENT_PCB_ADDRESS->fd_array[i].position = 0;
                    break;
            }
            return i;
        }
    }
    
    // At this point, no file descriptor was free
    return -1;
}

/* sys_close
 * Description: The close system call closes the specified file descriptor and 
 * makes it available for return from later calls to open.
 * You should not allow the user to close the default descriptors
 * (0 for input and 1 for output). 
 * Input: The file descriptor to close.
 * Returns: Trying to close an invalid
 * descriptor should result in a return value of -1;
 * successful closes should return 0.
 */
int32_t sys_close(int32_t fd)
{
    if (fd < FIRST_FD || fd > FD_ARRAY_SIZE - 1)
    {
        return -1;
    }

    // Check if the target fd is already closed 
    if (!((CURRENT_PCB_ADDRESS)->fd_array[fd].flags & FD_IN_USE))
    {
        return -1;
    }

    (CURRENT_PCB_ADDRESS)->fd_array[fd].flags &= ~FD_IN_USE;
    return 0;
}

/* sys_getargs
 * Description: Transfers the arguments that were populated into the current
 * process during execute into the passed in buffer.
 * Input: The buffer to populate, and the number of bytes.
 * Returns: -1 for invalid parameters; 0 on success
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{
    /* Local variables */
    int i;               /* Iteration variable */
    uint8_t* args;
    
    /* Checking for invalid params */
    // Check that pointer address is in user memory
    if ( ((uint32_t)buf < VIRTUAL_BEGIN) || ((uint32_t)buf > VIRTUAL_END) )
    {
        return -1;
    }
    
    if (nbytes < 0)
    {
        return -1;
    }
    
    args = (CURRENT_PCB_ADDRESS)->args;
    
    if (!(*args))
    {
        return -1;
    }
    
    /* Copying what's inside the arg buf */
    for (i = 0; i < nbytes; i++)
    {
        buf[i] = args[i];
        if (!args[i])
        {
            break;
        }
    }
    
    /* Return the number written */
    return 0;
}

/* sys_vidmap
 * Description: Allocates a page that allows user programs to write to the graphics buffer
 * Input: The address to the pointer of the allocated page
 * Output: Sets screen start to a new page
 * Returns: -1 for invalid parameters; 0 on success
 */
int32_t sys_vidmap(uint8_t** screen_start)
{
    //check that pointer address is in user memory
    if ( ((uint32_t)screen_start < VIRTUAL_BEGIN) || ((uint32_t)screen_start > VIRTUAL_END) )
    {
        return -1;
    }
    //give user programs a 4kB page starting at 132MB. This is totally arbitrary.
    *screen_start = (uint8_t*)VIRTUAL_END;
    
    return 0;        
}

/* sys_set_handler
 * Description: Used for consistency with syscalls documentation
 */
int32_t sys_set_handler(int32_t signum, void* handler_address)
{
    return -1;
}

/* sys_sigreturn
 * Description: Used for consistency with syscalls documentation
 */
int32_t sys_sigreturn(void)
{
    return -1;
}


