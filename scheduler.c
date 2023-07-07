/**********************************************************************/
/*                                                                    */
/* Program Name: program1 - Scheduler                                 */
/* Author:       Michael J. Garton                                    */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: April 17, 2020                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge this assignment is my own first time work.                */
/* I pledge I did not copy or try to copy work from the Internet.     */
/* I pledge I did not copy or try to copy work from any student.      */
/* I pledge I did not copy or try to copy work from any where else.   */
/* I pledge the only person I asked for help from was my teacher.     */
/* I pledge I did not attempt to help any student on this assignment. */
/* I understand if I violate this pledge I will receive a 0 grade.    */
/*                                                                    */
/*                                                                    */
/*                      Signed: ___________Michael Garton____________ */
/*                                           (signature)              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* This program simulates a scheduler by scheduling and running one   */
/* hundred processes.  Processes will be terminated after reaching    */
/* their maximum CPU time.  After running a process is preempted, in  */
/* which, it might block, will have its priority recalculated, and be */
/* added to a the end of a process queue based on its new priority.   */
/* New processes are scheduled when running processes stop running.   */
/* Blocked processes have a chance to become unblocked every clock    */
/* tick.  New processes can arrive into the process table.            */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>  /* printf function                                */
#include <stdlib.h> /* abs function, rand function                    */

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define MINIMUM_PID       1   /* The minimum possible process ID      */
#define MAXIMUM_PID       100 /* The maximum possible process ID      */
#define LIST_HEADER       MINIMUM_PID-1
                              /* The header of the ID list            */
#define LIST_TRAILER      MAXIMUM_PID+1
                              /* The trailer of the ID list           */
#define HEADER_ALLOC_ERR  1   /* Header memory allocation error       */
#define TRAILER_ALLOC_ERR 2   /* Trailer memory allocation error      */
#define PID_ALLOC_ERR     3   /* Process ID memory allocation error   */

/**********************************************************************/
/*                         Program Structures                         */
/**********************************************************************/
/* Process table                                                      */
struct process_table
{
   int  block_time,     /* Amount of ticks before the process blocks  */
        cpu_used,       /* Time the process has run on the CPU        */
        max_time,       /* Maximum CPU time needed by the process     */
        pid,            /* The process ID                             */
        priority,       /* The priority of the process                */
        quantum_used,   /* Amount of quantum used                     */
        wait_ticks;     /* Ticks the process has been waiting         */
   char state;          /* Process state, ruNning, Blocked, or Ready  */
   struct process_table *p_next_process;
                        /* Points to the next process                 */
};
typedef struct process_table PROCESS_TABLE;

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
PROCESS_TABLE *create_table();
   /* Create a process table                                          */
int process_insert(PROCESS_TABLE *p_process_table, int pid);
   /* Attempt to insert a new process into the process table          */
void pid_sort_process(PROCESS_TABLE *p_process_table);
   /* Sort process IDs in ascending order                             */
void sort_table(PROCESS_TABLE *p_process_table);
   /* Sort processes by their priority, highest to lowest             */
void invert_negative_priorities(PROCESS_TABLE *p_process_table);
   /* Invert negative priorities; higher is lower and vice versa      */
void run_process(PROCESS_TABLE *p_process_table);
   /* Calculate the running process's new values                      */
void update_wait_ticks(PROCESS_TABLE *p_process_table, int total_ticks);
   /* Update the wait ticks on all ready processes                    */
int get_count(PROCESS_TABLE *p_process_table);
   /* Get the amount of processes on the process table                */
void print_process_table(PROCESS_TABLE *p_process_table);
   /* Print the process table                                         */
void update_priority(PROCESS_TABLE *p_process_table);
   /* Recalculate the priority of a process after it runs             */
void update_state(PROCESS_TABLE *p_process_table);
   /* Update the state of a previously running process                */
void terminate(PROCESS_TABLE *p_process_table);
   /* Terminate a process if it process has used all of its CPU time  */
void reset_quantum(PROCESS_TABLE *p_process_table);
   /* Reset the quantum for all processes                             */
void unblock_processes(PROCESS_TABLE *p_process_table);
   /* Give all blocked processes a chance to unblock                  */
void schedule_new_process(PROCESS_TABLE *p_process_table);
   /* Schedule a new process to run                                   */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{
   int           pid = 0;          /* The highest process ID added    */
   PROCESS_TABLE *p_process_table; /* Points to the process table     */

   /* Create, initialize, and sort (by process ID) the process table  */
   p_process_table = create_table();
   while(pid <= 4)
      pid = process_insert(p_process_table, pid);
   pid_sort_process(p_process_table);

   /* Loops running processes until one hundred are scheduled         */
   while(pid = process_insert(p_process_table, pid), pid < MAXIMUM_PID)
   {
      /* Sort the process table by priority                           */
      sort_table(p_process_table);

      /* Run the process on the CPU                                   */
      run_process(p_process_table);

      /* Print the process table before scheduling a process          */
      printf("\n\nBEFORE SCHEDULING: Next PID =  %d,  ", pid + 1);
      printf("Number of Processes =  %d", get_count(p_process_table));
      print_process_table(p_process_table);

      /* Update the previously running process's priority and state   */
      update_priority(p_process_table);
      update_state(p_process_table);

      /* Terminate a process that has reached its maximum CPU time    */
      terminate(p_process_table);

      /* Reset the quantum for the next clock tick                    */
      reset_quantum(p_process_table);

      /* Attempt to unblock each blocked process                      */
      unblock_processes(p_process_table);

      /* Schedule the next Ready process to run                       */
      schedule_new_process(p_process_table);

      /* Sort the process table by priority                           */
      sort_table(p_process_table);

      /* Print the process table after scheduling a process           */
      printf("\n\nAFTER SCHEDULING: Next PID =  %d,  ", pid + 1);
      printf("Number of Processes =  %d", get_count(p_process_table));
      print_process_table(p_process_table);

   }
   return 0;
}

/**********************************************************************/
/*                       Create a process table                       */
/**********************************************************************/
PROCESS_TABLE *create_table()
{
   PROCESS_TABLE *p_new_process_table; /* Points to newly created     */
                                       /* process table               */

   /* Get a new node and make it the list header                      */
   if((p_new_process_table = (PROCESS_TABLE *)
      malloc(sizeof(PROCESS_TABLE))) == NULL)
   {
      printf("\nError #%d occurred in create_table().", HEADER_ALLOC_ERR);
      printf("\nCannot allocate memory for the list header.");
      printf("\nThe program is aborting.");
      exit(HEADER_ALLOC_ERR);
   }
   p_new_process_table->pid = LIST_HEADER;

   /* Get a new node and attach it to end of list as trailer          */
   if((p_new_process_table->p_next_process = (PROCESS_TABLE *)
      malloc(sizeof(PROCESS_TABLE))) == NULL)
   {
      printf("\nError #%d occurred in create_table().", TRAILER_ALLOC_ERR);
      printf("\nCannot allocate memory for the list trailer.");
      printf("\nThe program is aborting.");
      exit(TRAILER_ALLOC_ERR);
   }
   p_new_process_table->p_next_process->pid            = LIST_TRAILER;
   p_new_process_table->p_next_process->p_next_process = NULL;
   return p_new_process_table;
}

/**********************************************************************/
/*       Attempt to insert a new process into the process table       */
/**********************************************************************/
int process_insert(PROCESS_TABLE *p_process_table, int pid)
{
   PROCESS_TABLE *p_new_process; /* Points to newly created PID       */

   if(get_count(p_process_table) < 10)
   {
      if(rand() % 5 == 0)
      {
         if((p_new_process = (PROCESS_TABLE *)
            malloc(sizeof(PROCESS_TABLE))) == NULL)
         {
            printf("\nError #%d in process_insert().", PID_ALLOC_ERR);
            printf("\nCannot allocate memory for a new PID.");
            printf("\nThe program is aborting.");
            exit(PID_ALLOC_ERR);
         }

         pid                            += 1;
         p_new_process->pid              = pid;
	   	p_new_process->cpu_used         = 0;
         p_new_process->max_time         = ((rand() % 18) + 1);
         p_new_process->state            = 'R';
         p_new_process->priority         = 0;
         p_new_process->quantum_used     = 0;
         if((rand() % 2) == 0)
            p_new_process->block_time    = 6;
         else
            p_new_process->block_time    = ((rand() % 4) + 1);
         p_new_process->wait_ticks       = 0;
         p_new_process->p_next_process   = p_process_table->p_next_process;
         p_process_table->p_next_process = p_new_process;
      }
   }
   return pid;
}

/**********************************************************************/
/*                Sort process IDs in ascending order                 */
/**********************************************************************/
void pid_sort_process(PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_sort_process,      /* Points to each process      */
                 *p_temporary_process; /* Temporary process for swaps */
   int           sort_count;           /* Loops to sort the list      */

   for(sort_count = 1; sort_count <= get_count(p_process_table);
      sort_count++)
   {
      p_sort_process = p_process_table;
      while(p_sort_process->p_next_process->p_next_process->pid !=
         LIST_TRAILER)
      {
         if(p_sort_process->p_next_process->pid >
            p_sort_process->p_next_process->p_next_process->pid)
         {
            p_temporary_process                            =
               p_sort_process->p_next_process->p_next_process;
            p_sort_process->p_next_process->p_next_process =
               p_temporary_process->p_next_process;
            p_temporary_process->p_next_process            =
               p_sort_process->p_next_process;
            p_sort_process->p_next_process                 =
               p_temporary_process;
         }
         p_sort_process = p_sort_process->p_next_process;
      }
   }
   return;
}

/**********************************************************************/
/*        Sort processes by their priority, highest to lowest         */
/**********************************************************************/
void sort_table(PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_sort_process,      /* Points to each process      */
                 *p_temporary_process; /* Temporary process for swaps */
   int           sort_count;           /* Loops to sort the list      */

   invert_negative_priorities(p_process_table);
   for(sort_count = 1; sort_count <= get_count(p_process_table);
      sort_count++)
   {
      p_sort_process = p_process_table;
      while(p_sort_process->p_next_process->p_next_process->pid !=
         LIST_TRAILER)
      {
         if(p_sort_process->p_next_process->priority >
            p_sort_process->p_next_process->p_next_process->priority)
         {
            p_temporary_process                            =
               p_sort_process->p_next_process->p_next_process;
            p_sort_process->p_next_process->p_next_process =
               p_temporary_process->p_next_process;
            p_temporary_process->p_next_process            =
               p_sort_process->p_next_process;
            p_sort_process->p_next_process                 =
               p_temporary_process;
         }
         p_sort_process = p_sort_process->p_next_process;
      }
   }
   invert_negative_priorities(p_process_table);
   return;
}

/**********************************************************************/
/*     Invert negative priorities; higher is lower and vice versa     */
/**********************************************************************/
void invert_negative_priorities(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
   {
      switch(p_process_table->priority)
      {
         case -1:
            p_process_table->priority = -5;
         break;
         case -2:
            p_process_table->priority = -4;
         break;
         case -4:
            p_process_table->priority = -2;
         break;
         case -5:
            p_process_table->priority = -1;
         break;
      }
   }
   return;
}
/**********************************************************************/
/*             Calculate the running process's new values             */
/**********************************************************************/
void run_process(PROCESS_TABLE *p_process_table)
{
   int update_amount; /* The time the CPU will actually run           */

   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
   {
      if(p_process_table->state == 'N')
      {
         update_amount = p_process_table->block_time;
         if(p_process_table->cpu_used + p_process_table->block_time >
            p_process_table->max_time)
         {
            update_amount = p_process_table->max_time;
            p_process_table->cpu_used = update_amount;
         }
         else
            p_process_table->cpu_used += update_amount;
         update_wait_ticks(p_process_table, update_amount);
         p_process_table->quantum_used = update_amount;
      }
   }
   return;
}

/**********************************************************************/
/*            Update the wait ticks on all ready processes            */
/**********************************************************************/
void update_wait_ticks(PROCESS_TABLE *p_process_table, int total_ticks)
{
   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
   {
      if(p_process_table->state == 'R')
         p_process_table->wait_ticks += total_ticks;
   }
   return;
}

/**********************************************************************/
/*          Get the amount of processes on the process table          */
/**********************************************************************/
int get_count(PROCESS_TABLE *p_process_table)
{
   int amount_of_processes = 0; /* The amount of processes            */

   while(p_process_table = p_process_table->p_next_process,
         p_process_table->pid != LIST_TRAILER)
      amount_of_processes += 1; 

   return amount_of_processes;
}

/**********************************************************************/
/*                       Print the process table                      */
/**********************************************************************/
void print_process_table(PROCESS_TABLE *p_process_table)
{
   printf("\n PID   CPU Used   MAX Time   STATE   PRI   QUANTUM USED");
   printf("   BLK TIME   WAIT TKS");

   while(p_process_table = p_process_table->p_next_process,
         p_process_table->pid != LIST_TRAILER)
   {
      printf("\n%4d   %5d   %8d        %c     %3d   %7d    %9d      %6d",
         p_process_table->pid, p_process_table->cpu_used,
         p_process_table->max_time, p_process_table->state,
         p_process_table->priority, p_process_table->quantum_used,
         p_process_table->block_time, p_process_table->wait_ticks);
   }
   return;
}

/**********************************************************************/
/*        Recalculate the priority of a process after it runs         */
/**********************************************************************/
void update_priority(PROCESS_TABLE *p_process_table)
{
   int new_priority; /* The new priority of the running process       */

   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
   {
      if(p_process_table->state == 'N')
      {
         new_priority = ((abs(p_process_table->priority) +
            p_process_table->quantum_used) / 2);
         if(p_process_table->block_time < 6)
         {
            new_priority *= -1;
            if(new_priority == 0)
               new_priority = -1;
         }
         p_process_table->priority = new_priority;
      }
   }
   return;
}

/**********************************************************************/
/*          Update the state of a previously running process          */
/**********************************************************************/
void update_state(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
   {
      if(p_process_table->state == 'N')
      {
         if(p_process_table->block_time == 6)
            p_process_table->state = 'R';
         else
            p_process_table->state = 'B';
      }
   }
   return;
}

/**********************************************************************/
/*   Terminate a process if it process has used all of its CPU time   */
/**********************************************************************/
void terminate(PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_current  = p_process_table,
                                  /* Points to each PID               */
                 *p_previous = p_process_table;
                                  /* Points to previous PID           */

   while(p_current = p_current->p_next_process,
      p_current->pid != LIST_TRAILER)
   {
      if(p_current->max_time <= p_current->cpu_used)
      {
         p_previous->p_next_process = p_current->p_next_process;
         free(p_current);
      }
      else
         p_previous = p_current;
   }
   return;
}

/**********************************************************************/
/*                 Reset the quantum for all processes                */
/**********************************************************************/
void reset_quantum(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
      p_process_table->pid != LIST_TRAILER)
         p_process_table->quantum_used = 0;
   return;
}

/**********************************************************************/
/*           Give all blocked processes a chance to unblock           */
/**********************************************************************/
void unblock_processes(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
         p_process_table->pid != LIST_TRAILER)
   {
      if(p_process_table->state == 'B')
      {
         if(rand()%20 == 0)
            p_process_table->state = 'R';
      }
   }
   return;
}

/**********************************************************************/
/*                   Schedule a new process to run                    */
/**********************************************************************/
void schedule_new_process(PROCESS_TABLE *p_process_table)
{
   int is_process_running = 0; /* Boolean for if a process is running */
      
   while(p_process_table = p_process_table->p_next_process,
         p_process_table->pid != LIST_TRAILER)
      if(is_process_running == 0 && p_process_table->state == 'R')
      {
         p_process_table->state = 'N';
         is_process_running = 1;
      }
         
   return;
}
