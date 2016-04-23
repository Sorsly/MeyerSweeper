#include<stdio.h>
#include<stdlib.h>
#include<time.h>
void Initial_board(int,int,int(*)[]);
void Initial_mask(int,int,int(*)[]);
void Create_board(int,int,int,int(*)[]);
void GameControl(int,int,int,int,int(*)[],int(*)[]);


int main()
{

   int nrow = 10;
   int ncol = 10;
   int irow = 0;
   int icol = 9;
   int nbomb = 10;
   int mine[nrow][ncol];
   int mask[nrow][ncol];
   int i;
   int j;
   printf("%ld\n",time(0));
   Initial_board(nrow,ncol,mine);
   Initial_mask(nrow,ncol,mask); 
    Create_board(nrow,ncol,nbomb,mine);
       printf("Mine: \n");
   for(i = 0; i < nrow; i++){
   for(j = 0; j < ncol; j++){
    printf("%2d ",mine[i][j]);
  	}
   printf("\n");
    }
       printf("Mask: \n");
   for(i = 0; i < nrow; i++){
   for(j = 0; j < ncol; j++){
    printf("%2d ",mask[i][j]);
  	}
   printf("\n");
    }
   GameControl(irow,icol,nrow,ncol,mine,mask);
          printf("Mask: \n");
   for(i = 0; i < nrow; i++){
   for(j = 0; j < ncol; j++){
    printf("%2d ",mask[i][j]);
  	}
   printf("\n");
    }
  return 0;
}
void Initial_board(int nrow, int ncol, int board[][ncol])
{
  int i;
  int j;

  for(i = 0; i < nrow; i++){

    for(j = 0; j < ncol; j++){

      board[i][j] = 0;
    }
  }
}

void Initial_mask(int nrow, int ncol, int board[][ncol])
{
  int i;
  int j;

  for(i = 0; i < nrow; i++){

    for(j = 0; j < ncol; j++){

      board[i][j] = 9;
    }
  }
}
void Create_board(int nrow, int ncol, int nbomb, int board[][ncol])
{
  int i;
  int j;
  int n = time(0);
  while(nbomb != 0){
    srand(n);
    i = rand() % nrow;
    j = rand() % ncol;
    printf("i = %d, j = %d\n",i,j);
    if(!(board[i][j] == -1)){
      board[i][j] = -1;
      if((i - 1) != -1 && (j - 1) != -1 && (!(board[i-1][j-1] == -1))){
	  board[i - 1][j - 1]++;
	}
      if((i - 1) != -1 && (!(board[i-1][j] == -1))){
	  board[i - 1][j]++;
	}
      if((i - 1) != -1 && (j + 1) != (ncol) && (!(board[i-1][j+1] == -1))){
	  board[i - 1][j + 1]++;
	}
      if((j - 1) != -1 && (!(board[i][j-1] == -1))){
	  board[i][j - 1]++;
	}
      if((j + 1) != (ncol) && (!(board[i][j+1] == -1))){
	  board[i][j + 1]++;
	}
      if((i + 1) != (nrow) && (j - 1) != -1 && (!(board[i+1][j-1] == -1))){
	  board[i + 1][j - 1]++;
	}
      if((i + 1) != (nrow) && (j) != -1 && (!(board[i+1][j] == -1))){
	  board[i + 1][j]++;
	}
      if((i + 1) != (nrow) && (j + 1) != (ncol) && (!(board[i+1][j+1] == -1))){
	  board[i + 1][j + 1]++;
	  }
	nbomb--;
    }
    ++n;
  }
}

void GameControl(int irow, int icol, int nrow, int ncol, int mine[][ncol], int mask[][ncol])
{
  int i;
  int j;
  int trow;
  int tcol;
  mask[irow][icol] = mine[irow][icol];
  if (mine[irow][icol] == -1){
    printf("Fail!!!\n");
  }
  if (mask[irow][icol] == 0){
    for(i = 0;i < 3;i++){
      for(j = 0;j < 3;j++){
	trow = irow + i - 1;
	tcol = icol + j - 1;
	if ((trow >=0) && (trow < nrow) && (tcol >= 0) && (tcol < ncol) && (mask[trow][tcol] == 9)){
	  GameControl(trow,tcol,nrow,ncol,mine,mask);
	}
      }
    }
  }
    
} 
