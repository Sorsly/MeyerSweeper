#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define NROW 4
#define NCOL 10
#define NBOMB 8

void Initial_board(int(*)[]);
void Initial_mask(int(*)[]);
void Create_board(int(*)[]);//set mine and set number of mines around
int GameControl(int,int,int,int(*)[],int(*)[]);//run this function when click one button
int FlagControl(int,int,int,int(*)[]);//flag function
//-1 means mine; 9 means not clicked; 10 means flag
int main()
{

  //  int nrow = 10;//size
  //  int ncol = 10;//size
  int irow = 0;//place clicked
  int icol = 9;//place clicked
  int frow = 0;//place flag
  int fcol = 1;//place flag
  //  int nbomb = 10;//number of bomb
  int nclick = 0;
  int nflag = 0;
  int fail = 0;
  int win = 0;
  int mine[NROW][NCOL];//map with mine and other things
  int mask[NROW][NCOL];//cover map
   int i;
   int j;
   printf("%ld\n",time(0));
   Initial_board(mine);
   Initial_mask(mask); 
   Create_board(mine);
       printf("Mine: \n");
   for(i = 0; i < NROW; i++){
   for(j = 0; j < NCOL; j++){
    printf("%2d ",mine[i][j]);
  	}
   printf("\n");
    }
   nclick = GameControl(irow,icol,nclick,mine,mask);
   if(nclick == -1){
     fail = 1;
   }
          printf("Mask: \n");
   for(i = 0; i < NROW; i++){
   for(j = 0; j < NCOL; j++){
    printf("%2d ",mask[i][j]);
  	}
   printf("\n");
    }
   printf("Number Click: %d\n",nclick);
   nflag = FlagControl(frow,fcol,nflag,mask);
   if((nflag + nclick) == NBOMB){
     win = 1;
   }
   printf("Number Flag: %d\n",nflag);
          printf("Flag: \n");
   for(i = 0; i < NROW; i++){
   for(j = 0; j < NCOL; j++){
    printf("%2d ",mask[i][j]);
  	}
   printf("\n");
    }
  return 0;
}
void Initial_board(int board[][NCOL])
{
  int i;
  int j;

  for(i = 0; i < NROW; i++){

    for(j = 0; j < NCOL; j++){

      board[i][j] = 0;
    }
  }
}

void Initial_mask(int board[][NCOL])
{
  int i;
  int j;

  for(i = 0; i < NROW; i++){

    for(j = 0; j < NCOL; j++){

      board[i][j] = 9;
    }
  }
}
void Create_board(int board[][NCOL])
{
  int i;
  int j;
  int nbomb = NBOMB;
  int n = time(0);
  while(nbomb != 0){
    srand(n);
    i = rand() % NROW;
    j = rand() % NCOL;
    printf("i = %d, j = %d\n",i,j);
    if(!(board[i][j] == -1)){
      board[i][j] = -1;
      if((i - 1) != -1 && (j - 1) != -1 && (!(board[i-1][j-1] == -1))){
	  board[i - 1][j - 1]++;
	}
      if((i - 1) != -1 && (!(board[i-1][j] == -1))){
	  board[i - 1][j]++;
	}
      if((i - 1) != -1 && (j + 1) != (NCOL) && (!(board[i-1][j+1] == -1))){
	  board[i - 1][j + 1]++;
	}
      if((j - 1) != -1 && (!(board[i][j-1] == -1))){
	  board[i][j - 1]++;
	}
      if((j + 1) != (NCOL) && (!(board[i][j+1] == -1))){
	  board[i][j + 1]++;
	}
      if((i + 1) != (NROW) && (j - 1) != -1 && (!(board[i+1][j-1] == -1))){
	  board[i + 1][j - 1]++;
	}
      if((i + 1) != (NROW) && (j) != -1 && (!(board[i+1][j] == -1))){
	  board[i + 1][j]++;
	}
      if((i + 1) != (NROW) && (j + 1) != (NCOL) && (!(board[i+1][j+1] == -1))){
	  board[i + 1][j + 1]++;
	  }
	nbomb--;
    }
    ++n;
    ++n;
    ++n;
  }
}

int GameControl( int irow, int icol, int nclick, int mine[][NCOL], int mask[][NCOL])
{
  int i;
  int j;
  int trow;
  int tcol;
  if((mask[irow][icol] != mine[irow][icol]) && (mask[irow][icol] != 10)){
  mask[irow][icol] = mine[irow][icol];
  nclick++;
  if (mine[irow][icol] == -1){
    printf("Fail!!!\n");
    return -1;
  }
  if (mask[irow][icol] == 0){
    for(i = 0;i < 3;i++){
      for(j = 0;j < 3;j++){
	trow = irow + i - 1;
	tcol = icol + j - 1;
	if ((trow >=0) && (trow < NROW) && (tcol >= 0) && (tcol < NCOL) && (mask[trow][tcol] == 9)){
	  nclick = GameControl(trow,tcol,nclick,mine,mask);
	}
      }
    }
  }
  }
  return nclick;
} 

int FlagControl(int frow, int fcol, int nflag, int mask[][NCOL])
{
  if(mask[frow][fcol] == 9){
    mask[frow][fcol] = 10;
    nflag++;
  }else if (mask[frow][fcol] == 10){
    mask[frow][fcol] = 9;
    nflag--;
  }
  return nflag;
}
