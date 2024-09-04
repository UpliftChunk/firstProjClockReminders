
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <windows.h>


enum OperationType {UNDO, REDO};
int num=1;
int times=0;

void newLine()
{
    if(times!=0){
        printf("\n");
    }
    times++;
}

//reminders list
struct node
{
    int dt,dn,mn,yn,Th,Tm;
    char label[100];
    struct node *prev;
    struct node *next;
}*first=NULL, *last;

// file carrier
typedef struct oprs{
    char fileName[8];
    FILE *file;
    struct oprs *next;
}fc;
// normalOperationsTop and redoOperationsTop
fc *undoStackTop=NULL, *redoStackTop=NULL;


fc *push(char s[], fc *top)  // push file carrier to a desired stack
{
    fc *newFile;
    newFile= (fc*)malloc(sizeof(fc));
    strcpy(newFile->fileName,s);
    if(top==NULL)    newFile->next=NULL;
    else   newFile->next=top;
    return newFile; 
}
void pop(enum OperationType a) // undo redo stack's top file carrier is exchanged
{
    if(a==UNDO) {
        redoStackTop= push(undoStackTop->fileName, redoStackTop);
        if(undoStackTop->next!=NULL) undoStackTop=undoStackTop->next;
        else undoStackTop=NULL;
    }
    else {
        undoStackTop= push(redoStackTop->fileName, undoStackTop);
        if(redoStackTop->next!=NULL) redoStackTop=redoStackTop->next;
        else redoStackTop=NULL;
    }
}
void createfile(int op,struct node *nn)  // create new file carrier and push it to undo stack
{
    char s[8];
    sprintf(s, "file%d", num++);
    FILE* ptr = fopen(s, "w+");
    fprintf(ptr,"%d\n%s\n%d/%d/%d\n%d:%d",op,nn->label,nn->dn,nn->mn,nn->yn,nn->Th,nn->Tm);
    fclose(ptr);
    undoStackTop=push(s, undoStackTop);
}
struct node *insertIntoReminders(char S[], int D, int M, int Y, int TH, int TM)  // insert reminder node and return new node address
{
    struct node *nn,*place;
    nn =(struct node*)malloc(sizeof(struct node));
    nn->next= NULL;     nn->prev= NULL;
        strcpy(nn->label, S);
        nn->dn =D , nn->mn =M , nn->yn =Y;
        nn->Th =TH , nn->Tm =TM;
    if(first==NULL){
        first=nn;    last=nn;
    }
    else
    {
        place=first;
        while(            ( place->next!=NULL ) && (nn->yn > place->next->yn)  ) place= place->next;
        while(  ( place->next!=NULL && nn->yn == place->next->yn)  && (nn->mn > place->next->mn)  ) place= place->next;
        while(  ( place->next!=NULL && nn->mn == place->next->mn)  && (nn->dn > place->next->dn)  ) place= place->next;
        while(  ( place->next!=NULL && nn->dn == place->next->dn)  && (nn->Th > place->next->Th)  ) place= place->next;
        while(  ( place->next!=NULL && nn->Th == place->next->Th)  && (nn->Tm > place->next->Tm)  ) place= place->next;
        if( place==first){
            nn->next=first;
            first->prev=nn;
            first=nn;
        } 
        else{
            nn->next=place->next;
            nn->prev=place;
            if(place->next!=NULL) place->next->prev=nn;
            place->next=nn;
        }
        
    }
    if(last->next==nn) last=nn;
    return nn;
}
void displayAllReminders()
{
    struct node *tmp=first;
    newLine();
    while(tmp!=NULL)
    {
        printf(" %02d/%02d/%04d %02d:%02d   %s ",tmp->dn,tmp->mn,tmp->yn,tmp->Th,tmp->Tm,tmp->label);
        tmp=tmp->next;
        if(tmp!=NULL){
            printf("\n");
        }
    }
}
void del(char c,struct node *pos)  // deletes a node by taking its address
{
    // c indicates if its user called(m) or admin called(n)
    struct node *tmp=pos, *nn;
    if(tmp==last){
        nn=pos->prev;
        last=nn;
        last->next=NULL;
    }
    else if(tmp==first){
        nn=tmp->next;
        first=nn;
        tmp->next=NULL;
    }
    else{
        (tmp->prev)->next= tmp->next;
        (tmp->next)->prev= tmp->prev;
    }
    if(c=='m') createfile(2,tmp);
    free(tmp);
}
struct node *findNode(char S[]){           // finds first occuring label's address
    struct node *target=first; // target iterats on reminders list
    int found=0; // available is also the serial number
    do{
        if ( strcmp((S),target->label)==0 ){
            found=1;
            break;
        } 
        target=target->next;
    }while ( target!=last );
    return target;
}
void searchLabel(char S[])  // stored address is sent to delete
{
    struct node *tmp=first, *LabelsAddress[10]; // tmp iterats on reminders list
    int available=1, found=0; // available is also the serial number
    if(tmp==NULL) {
        printf("THERE ARE NO REMINDERS TO LOOK FOR\n");
        return;
    }
    do{
        if ( strcmp((S),tmp->label)==0 ){
            found=1;
            printf("\n%d) %s IS FOUND AT %02d/%02d/%d %02d:%02d",available,S,tmp->dn,tmp->mn,tmp->yn,tmp->Th,tmp->Tm);
            LabelsAddress[available++]=tmp;
        } 
        tmp=tmp->next;
    }while ( tmp!=last );
    if(!found){
        newLine();
        printf("NO SUCH LABEL: \" %s \" IS FOUND IN YOUR REMINDERS",S);
        return;
    }

    int dL=1;
    printf("\n\tEnter anyother number to terminate this \" option-2 \"");
    while(dL<available)
    {
        printf("\nEnter serial number of the reminder to be eliminated : ");   
        scanf("%d",&dL);
        if(dL<available){
            if(LabelsAddress[dL]!=NULL){
                del('m',LabelsAddress[dL]); // user called
                printf("THE REMINDER \"%s\" AT serial number : %d is successfully removed",S,dL);
                LabelsAddress[dL]=NULL;
            }
            else{
                printf("REMINDER AT THIS SERIAL NUMBER IS ALREADY REMOVED");
            }
        }
        else{
            printf(" option exited \n");
        }
    }

}
void alter(enum OperationType c)   // (node address to remove is sent to delete | new node is inserted) and undo redo stack's top fc is exchanged
{
    int OP,D,M,Y,TH,TM,d,m,y,th,tm;
    char S[100],s[100];
    FILE *ptr;
    if(c==UNDO) ptr = fopen(undoStackTop->fileName,"r+");
    else ptr = fopen(redoStackTop->fileName,"r+");

    struct node *tmp=first; // iterates on reminders list
    fscanf(ptr, "%d\n%s\n%d/%d/%d\n%d:%d" ,&OP,S,&D,&M,&Y,&TH,&TM);
    fclose(ptr);
    if( (OP==1 && c==UNDO) || (OP==2 && c==REDO) ){ // 1- added, 2- removed
        while(tmp!=NULL){
            strcpy(s,tmp->label);
            d=tmp->dn, m=tmp->mn, y=tmp->yn, th=tmp->Th, tm=tmp->Tm;
            if ( strcmp(S,s)==0 && D==d && M==m && Y==y && TH==th && TM==tm){
                del('n',tmp); // admin called: doesnt create new File Carrier, exisitng file carrier is exchanged 
                printf("THE REMINDER \"%s\" IS REMOVED\n",S);
                tmp=NULL;
                break;
            }
            tmp=tmp->next;
        }
    }
    else{
        tmp=insertIntoReminders(S,D,M,Y,TH,TM);
        printf("THE REMINDER \"%s\" IS ADDED AGAIN\n",S);
    }
    
    pop(c);
}


int main()
{
    
    int OP, Th,Tm,d,m,y,sec,o=0;
    char SrcLabel[100],note[100],S[100];
    struct node *p;
    printf("\n \t\t\t\t\t\t ~Clock follows 24hr format for input~\n"); 
    
    printf( 
        "Allowed options:\n"
        "1. Add reminder label and time \n"
        "2. Remove a label from the queue \n"
        "3. Undo / Redo(if possible) the most recent option\n"
        "5. View all events in your calendar\n"
        "6. Start Clock(this action can't be interrupted)\n"
    );

    
    while(o!=6)  // handle operations
    {
    // system("clear"); - clear for linux, cls for windows  
        
        printf(" \t\t\t\t\t\t ~Enter a valid Choice: ");        scanf("%d",&o);
        // performing option
            if(o==1)
            {
                printf(" mention the label: "); scanf("%s",note);
                printf(" date(dd/mm/yyyy) : "); scanf("%d/%d/%d",&d,&m,&y);
                printf(" enter time (hr:min) : "); scanf("%d:%d",&Th,&Tm);
                p=insertIntoReminders(note,d,m,y,Th,Tm);
                createfile(1,p);
                printf("\t\t\t\t ~reminder Added~\n"); 
            }
            
            else if(o==2){
                printf("Enter search label: "), scanf("%s",SrcLabel);
                searchLabel(SrcLabel);
            }
            
            else if(o==3){
                do{
                    printf("Select option:\t3.Undo    4.Redo    5.displayAllReminders\t: "); scanf(" %d",&o);
                    // printf("           %d          \n",o);
                    if(o==3){
                        if(undoStackTop==NULL) printf("reminder list is empty\n");
                        else 
                            alter(UNDO);
                    }
                    else if(o==4){
                        if(redoStackTop==NULL) printf("no UNDO operation was performed recently\n");
                        else 
                            alter(REDO);
                    }
                    else if(o==5){
                        displayAllReminders(); printf("\n");
                    }
                    else{
                        printf("Quitting option\n");
                        break;
                    }
                }while(1);
                fc *nn,*tmp=redoStackTop;
                while(tmp!=NULL)
                {
                    nn=tmp;
                    tmp=tmp->next;
                    remove(nn->fileName);
                    free(nn);
                }
                redoStackTop = NULL;    
            }
            
            else if(o==5){
                displayAllReminders(); printf("\n");
            }
            
            else if(o>6)printf("\nINVALID option\n");
        //   for(i=0; i<wait; i++);
           
            else{
                printf("\nSTARTING CLOCK IN 5 seconds.");
                int a=4;
                while(a--) {
                    sleep(1);
                    printf(".");
                }   
            }    
    }

    //run clock

    
        int min,hour,date,month,year;
        time_t currentTime;
        time(&currentTime);
        struct tm *myTime = localtime(&currentTime);
        sec=myTime->tm_sec ;
        min=myTime->tm_min ;
        hour=myTime->tm_hour ;
        date=myTime->tm_mday ;
        month= ++myTime->tm_mon ;
        year=myTime->tm_year ;
    
        struct node *nn,*tmp=first;  // tmp points to the upcoming reminder
        d=tmp->dn, m=tmp->mn, y=tmp->yn, Th=tmp->Th, Tm=tmp->Tm;

        strcpy(note, "_start_point");
        p=insertIntoReminders(note,d,m,y,Th,Tm);
        tmp= findNode(note)->next;
        if(tmp==NULL) goto finish;

set:
    int D=tmp->dn, M=tmp->mn, Y=tmp->yn, TH=tmp->Th, TM=tmp->Tm;
    strcpy(S,tmp->label);
        
        while(tmp!=NULL){
                system("cls");
                printf("upcoming remainder: %s\n", S);
                // printf(" %d/%d/%d %d:%d   %s\n" ,tmp->dn,tmp->mn,tmp->yn,tmp->Th,tmp->Tm,tmp->label);
                // printf(" %d/%d/%d %d:%d   %s\n" ,d,m,y,Th,Tm,S);
                // printf(" %d/%d/%d %d:%d   %s\n" ,d,m,y,Th,Tm,S);
                time(&currentTime);
                myTime = localtime(&currentTime);
                printf("%s\n",ctime(&currentTime));
                
                sec=myTime->tm_sec ;
                Tm=min=myTime->tm_min ;
                Th=hour=myTime->tm_hour ;
                d=date=myTime->tm_mday ;
                m=month= ++myTime->tm_mon ;
                y=year=myTime->tm_year +1900;

                if ( d==D && m==M && y==Y && Th==TH && Tm==TM){
                    printf("TIME FOR \"%s\"\n",S);
                    Beep(600, 200);
                    if(sec==59){
                        tmp=tmp->next;
                        if(tmp!=NULL) goto set;
                    } 
                }
                sleep(1);    
        }
    


    newLine();
    printf("Clock stopped, finished execution\n");  
    finish:    
    // clearing malloc data of reminders
        tmp=first;
        while(tmp!=NULL)
        {
            nn=tmp;
            tmp=tmp->next;
            free(nn);
        }
    // clearing malloc data of reminders
        fc *n,*temp=undoStackTop;
        while(temp!=NULL)
        {
            n=temp;
            temp=temp->next;
            remove(n->fileName);
            free(n);
        }
    undoStackTop = NULL;   
    return 0;
}







