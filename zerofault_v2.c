#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "defns.h"
#include "segy.h"
#include "segyIO_class.h"
#include <time.h>
//#include "segyIO_class.h"

int main(int argc, char **argv)
{
FILE *fin;		/* Input file				*/
FILE *fzeros;		/* output file				*/
FILE *fmodel;
//segy  thdr;		/* holds the full trace header		*/
char  txthdr[3202];	/* The character file header		*/
//register complex *ct;   /* complex transformed trace            */
float dt;               /* sample spacing                       */
int ns;                 /* number of points on input trace      */
register int i;
int   ierr=0;
int endian = 1;
int format;
unsigned char chdr[3202];
int status=0;
bhed  bh;	
int   fmt,  itr;
int idt;
segy  thdr;
float *tr;	/* Array to hold the trace samples */
int *ptr2hdr;
int count=0;
char trhdr[242];
int delta =0;
int iline=0, xline=0,ilineStart=0,xlineStart=0,xlineEnd=0,horizon=0;
int numXlines=0;
char str[101];
int zero=0;
int  window=0;
float  fillerBelow=0,fillerAbove=0, fillerAve=0;
int fill=0;

	endian = checkEndian();
	
   if (argc == 8) {
	  fzeros = fopen(argv[1],"r");
      fin = fopen(argv[2], "r+b");
      //fout = fopen(argv[3],"wb");
      
      if (fzeros == NULL) {
         printf("Unable to open the zeros file.  Please check the name.\n");
         return -1;  
      }
      if (fin == NULL) {
         printf("Unable to open the input velocity file.  Please check the name.\n");
         return -1;  
      }
      if(sscanf(argv[3], "%d", &ilineStart) != 1){
			printf("\nStarting Inline entered not valid. Exiting...");
			return -1;
	  }
	  if(sscanf(argv[4], "%d", &xlineStart) != 1){
			printf("\nStarting Xline entered not valid. Exiting...");
			return -1;
	  }
      if(sscanf(argv[5], "%d", &xlineEnd) != 1){
			printf("\nEnding Xline entered not valid. Exiting...");
			return -1;
	  }  
	  if(sscanf(argv[6], "%d", &window) != 1){
			printf("\nStarting window entered not valid. Exiting...");
			return -1;
	  }  
	  if(sscanf(argv[7], "%d", &fill) != 1){
			printf("\nStarting fill entered not valid. Exiting...");
			return -1;
	  }
      
   } else {
	fprintf(stderr,"\n***************************************************************************\n");
      fprintf(stderr, "Program reads horizon data from text file and zeros those horizon values.\n\n");
      fprintf(stderr, "Usage: headerDump <horizon data> <input segy file> <starting inline>  \n");
      fprintf(stderr, "       <starting xline> <ending xline> <window> <fill> \n\n");
      fprintf(stderr, "window:  number samples above and below to be zeroed/filled\n\n");
      fprintf(stderr, "fill:  0 = zero values, 1 = fill previously zeroed values \n");
    fprintf(stderr,"\n***************************************************************************\n");
      return 0;
   }
   
   if (fin != NULL ) {
	  
      status = segyReadHeader(fin, chdr, &bh, endian);
        fmt = bh.format; /*format*/
        idt = bh.hdt; /*rate in microseconds*/
        ns = bh.hns; /*samples per trace*/
 
        printf("\nSamp rate is: %d  Num samps is: %d format: %d \n", idt, ns,fmt);
       	
      //New method:  Fseek to values! 
		numXlines=xlineEnd-xlineStart + 1;
	
		while ( fgets(str,100,fzeros)!=NULL){
			if(count%5000==0){
				printf("Still running...");
				}
			count++;
			if(fill==0){
				
				sscanf(str,"%d %d %d", &iline,&xline,&horizon);
				
                delta = (iline-ilineStart)*(numXlines)*(ns*4 + 240);
                delta+= (xline-xlineStart)*(ns*4+240) ;
                delta+= 240 + (horizon);
               
                fseek(fin,3600+delta,SEEK_SET);
             
                fwrite(&zero,4,1, fin);
                
                for (i=1;i<=window;++i){
					fseek(fin,3600+delta-i*4,SEEK_SET);
					fwrite(&zero,4,1, fin);
					fseek(fin,3600+delta+i*4,SEEK_SET);
					fwrite(&zero,4,1, fin);
				}
            }else {

				sscanf(str,"%d %d %d", &iline,&xline,&horizon);
                delta = (iline-ilineStart)*(numXlines)*(ns*4 + 240);
                delta+= (xline-xlineStart)*(ns*4+240) ;
                delta+= 240 + (horizon);
               
                fseek(fin,3600+delta - (window+1)*4  ,SEEK_SET);
                fread(&fillerAbove,4,1,fin);
                fseek(fin,3600+delta + (window+1)*4 ,SEEK_SET);
                fread(&fillerBelow,4,1,fin);
                if(fillerAbove==0 && fillerBelow ==0){
					printf("fillerAbove an Below both zero!!");
				}else if(fillerAbove==0){
					fillerAbove = fillerBelow;
				}else if(fillerBelow==0){
					fillerBelow = fillerAbove;
				}
                
                
                for(i=1;i<=window;++i){
					fseek(fin,3600+delta - i*4 ,SEEK_SET);
					fwrite(&fillerAbove,4,1, fin);
					fseek(fin,3600+delta + i*4 ,SEEK_SET);
					fwrite(&fillerBelow,4,1, fin);	
				}

                fseek(fin,3600+delta ,SEEK_SET);
                fwrite(&fillerBelow,4,1, fin);

			}    
     
        }
   }

  fclose(fin);
  return 0;
  
}

/*
************************************************************************
*/
void doMessage(char *str)
{
   fprintf(stderr, "%s\n", str);
}
/*
***********************************************************************
*/


