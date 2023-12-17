//
//  main.c
//  SMMarble
//
//  Created by seoyeon chu on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"



//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;


typedef struct player {
        int energy;
        int position;
        char name[MAX_CHARNAME];
        int accumCredit;
        int flag_graduate;
} player_t;


static player_t *cur_player;
//static player_t cur_player[MAX_PLAYER];

static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];

/*
//function prototypes
int isGraduated(void); //check if any player is graduated
 //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
*/

void printGrades(int player)
{
    int i;
    void *gradePtr;
    for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
    {
        gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        printf("%s : %s\n", smmObj_getNodeName(LISTNO_OFFSET_GRADE + player), smmObj_getNodeGrade(LISTNO_OFFSET_GRADE + player));
    }
}

void printPlayerStatus(void)
{
     int i;
     
     for (i=0;i<player_nr;i++)
     {
         printf("%s : credit %i, energy %i, position %i\n", 
                      cur_player[i].name,
                      cur_player[i].accumCredit,
                      cur_player[i].energy,
                      cur_player[i].position);
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     //n time loop
     for (i=0;i<n;i++)
     {
         //input name
         printf("Input player %i's name:", i); //??? ???? 
         scanf("%s", cur_player[i].name);
         fflush(stdin);
         
         //set position
         player_position[i] = 0;
         cur_player[i].position = 0;
         
         //set energy
         player_energy[i] = initEnergy;
         cur_player[i].energy = initEnergy;
         cur_player[i].accumCredit = 0;
         cur_player[i].flag_graduate = 0;
     }
}


int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);
    
#if 1
    if (c == 'g')
        printGrades(player);
#endif
    
    return (rand()%MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
    void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position );
    //int type = smmObj_getNodeType( cur_player[player].position );
    int *type = smmObj_getNodeType( boardPtr );
    char *name = smmObj_getNodeName( boardPtr );
    void *gradePtr;
    
    switch(type)
    {
        //case lecture:
        case SMMNODE_TYPE_LECTURE: // ���� ���
            if (cur_player[player].energy-smmObj_getNodeEnergy( boardPtr )<0) { // ���� �������� �ҿ信���� �̻� ���� ���
                break; // ������ �� ����
            }
            cur_player[player].accumCredit += smmObj_getNodeCredit( boardPtr );
            cur_player[player].energy -= smmObj_getNodeEnergy( boardPtr );
            
            //grade generation
            smmObjType_e grade = takeLecture(player, name, smmObj_getNodeCredit( boardPtr ));
            gradePtr = smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit( boardPtr ), 0, grade);
            smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
            
            break;
            
        case SMMNODE_TYPE_RESTAURANT: // �Ĵ�
            // ���� ��������ŭ �÷��̾��� ���� �������� ������
            cur_player[player].energy += smmObj_getNodeEnergy( boardPtr ); // ���� ��������ŭ ����
            break;

        case SMMNODE_TYPE_LABORATORY: // �����
            break;

        case SMMNODE_TYPE_HOME: // ��
            // �������� ���� ������ ���� ��������ŭ ���� �������� ������
            cur_player[player].energy += smmObj_getNodeEnergy( boardPtr ); // ���� ��������ŭ ����
            break;

        case SMMNODE_TYPE_GOTOLAB: // ����
            // ������ ���·� ��ȯ�Ǹ鼭 ����Ƿ� �̵� (�ֻ��� �� �������� ���� ���� ���ذ��� �������� ����.)
            break;

        case SMMNODE_TYPE_FOODCHANCE: // ���� ���� 
            // ����ī�带 ���� �������� ������ ���õ� ���� �������� ���� �������� ����
            break;

        case SMMNODE_TYPE_FESTIVAL: // ����
            // ����ī�带 ���� �������� ��� ���õ� �̼��� ����
            break;

        default:
            break;
    }
}

void goForward(int player, int step)
{
     void *boardPtr;
     cur_player[player].position += step;
     boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position );
     
     printf("%s go to node %i (name: %s)\n", 
                cur_player[player].name, cur_player[player].position,
                smmObj_getNodeName(boardPtr);
}

int isGraduated(void) { // print grade history of the player
// cur_player�� �ִ� ��� �������� grade�� ����ϴ� �Լ��Դϴ�.
// �Ѹ��̶� �����ϸ� 1��, �ƹ��� �������� �ʾ����� 0�� �����մϴ�.
    for(int i=0; i<sizeof(cur_player)/sizeof(player_t); i++) {
        printGrades(i);
        if (cur_player[i].flag_graduate!=0) {
            return 1;
        }
    }
    return 0;
}

float calcAverageGrade(int player) {
    int totalCredit = 0; // ��ü credit ���� �����ϴ� ����
    int totalGrade = 0; // ��ü grade�� ���� �����ϴ� ����

    for (int i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++) {
        void *boardObj = smmdb_getData(LISTNO_NODE, i);
        totalCredit += smmObj_getNodeCredit(boardObj); // credit �������� �Լ�
        totalGrade += smmObj_getNodeGrade(boardObj); // grade �������� �Լ�
    }
    return totalGrade/totalCredit;
}

smmObjType_e takeLecture(int player, char *lectureName, int credit) {
    void*  gradePtr;
    smmObjType_e grade = rand() % 9; // �������� ���� ����

    gradePtr = smmObj_genObject(lectureName, smmObjType_grade, 0, credit, 0, grade); // ��ü ����
    smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr); // �����ͺ��̽��� ���� �߰�

    return grade; // ���� ���� ��ȯ
}

void* findGrade(int player, char *lectureName) { // find the grade from the player's grade history
    void *gradePtr;
    for (int i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++) // player�� ��ü grade history�� ��ȸ�ϸ鼭
    {
        gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        if (strcmp(smmObj_getNodeName(gradePtr), lectureName)) { // lectureName�� ���� �����̸��� ���� node�� �߰��ϸ� �ش� node�� grade�� ��ȯ
            return smmObj_getNodeGrade(gradePtr); // smm_object.c�� �����߽��ϴ�.
        }
    }
}

int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn=0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        //(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
        void *boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);
        
        if (type == SMMNODE_TYPE_HOME)
           initEnergy = energy;
        board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    
    for (i = 0;i<board_nr;i++)
    {
        void *boardObj = smmdb_getData(LISTNO_NODE, i);
        
        printf("node %i : %s, %i(%s), credit %i, energy %i\n", 
                     i, smmObj_getNodeName(boardObj), 
                     smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                     smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    }
    printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
  
    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", name, &energy) == 2 ) //read a food parameter set
    {
        //store the parameter set
        void *boardObj = smmObj_genObject(name, smmObjType_card, 1, 0, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, boardObj);
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
    
    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", name) == 1 ) //read a festival card string
    {
        //store the parameter set
        void *boardObj = smmObj_genObject(name, smmObjType_card, 6, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTCARD, boardObj);
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);

    
    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("input player no.:");
        scanf("%d", &player_nr);
        fflush(stdin);
    }
    while (player_nr < 0 || player_nr >  MAX_PLAYER);
    
    cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    
    
    
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)        
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1)%player_nr;
    }
    
    
    free(cur_player);
    system("PAUSE");
    return 0;
}