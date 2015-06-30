#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "defns.h"
#include "segy.h"
#include "segyIO_class.h"
#include <time.h>
#include <sys/stat.h>
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
int iline=0, xline=0,ilineStart=0,xlineStart=0,xlineEnd=0,horizon=0,ilineEnd=0;
int numXlines=0;
char str[101];
int zero=0;
int  window=0;
float  velAve=0;
int *pointAve;
int fill=0;
float sumVels=0,read=0;
int numVels=0;
long int fileBeg=0, fileEnd=0,size;
int choice;
char buff[80];
fpos_t position;

struct stat st;


	endian = checkEndian();
	
   if (argc == 9) {
	  fzeros = fopen(argv[1],"r");
      fin = fopen(argv[2], "r+b");
     
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
	  if(sscanf(argv[5], "%d", &ilineEnd) != 1){
			printf("\Ending iline entered not valid. Exiting...");
			return -1;
	  }
      if(sscanf(argv[6], "%d", &xlineEnd) != 1){
			printf("\nEnding Xline entered not valid. Exiting...");
			return -1;
	  }  
	  if(sscanf(argv[7], "%d", &window) != 1){
			printf("\nStarting window entered not valid. Exiting...");
			return -1;
	  }  
	  if(sscanf(argv[8], "%d", &fill) != 1){
			printf("\nStarting fill entered not valid. Exiting...");
			return -1;
	  }
      
   } else {
	fprintf(stderr,"\n***************************************************************************\n");
      fprintf(stderr, "Program reads horizon data from text file and zeros those horizon values.\n\n");
      fprintf(stderr, "Usage: headerDump <horizon data> <input segy file> <starting inline>  \n");
      fprintf(stderr, "       <starting xline> <ending iline> <ending xline> <window> <fill> \n\n");
      fprintf(stderr, "window:  number samples above and below to be zeroed/filled\n\n");
      fprintf(stderr, "fill:  0 = zero values, 1 = fill previously zeroed values \n");
    fprintf(stderr,"\n***************************************************************************\n");
      return 0;
   }
   
   
   stat(fin, &st);
  size = st.st_size;
   fseek(fin,0,SEEK_END);
   fileEnd = fgetpos(fin, &position);
   //fileEnd=ftell(fin);
   fseek(fin,0,SEEK_SET);
   fileBeg=ftell(fin);
	
	printf("\n\nHorizon: %s", argv[1]);
	printf("\nInput: %s", argv[2]);
	printf("\niline Start: %d", ilineStart);
	printf("\nxline Start: %d", xlineStart);
	printf("\niline End: %d", ilineEnd);
	printf("\nxline End: %d", xlineEnd);
	printf("\nWindow %d", window);
	printf("\nFill: %d", fill);
	printf("\nfileBeg: %ld", fileBeg);
	printf("\nfileEnd: %ld", size);
	printf("\nfileEnd: %ld", position);
	
	printf("\n\nAre these correct? (-1: exit)");
	fgets(buff,80,stdin);
	sscanf(buff,"%d",&choice);
	if(choice == -1){ return 0;} 
  

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
                if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
					printf("\n\n Check the inline and xline numbers including those in the headers\n");
					return -1;
				}
             
                fwrite(&zero,4,1, fin);
                
                for (i=1;i<=window;++i){
					fseek(fin,3600+delta-i*4,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}	
					fwrite(&zero,4,1, fin);
					fseek(fin,3600+delta+i*4,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fwrite(&zero,4,1, fin);
				}
            }else {
				sumVels=0;
				numVels=0;
				velAve=0;
				sscanf(str,"%d %d %d", &iline,&xline,&horizon);
				
				if(iline - 1 >= ilineStart){
					delta = (iline-ilineStart-1)*(numXlines)*(ns*4 + 240);
					delta+= (xline-xlineStart)*(ns*4+240) ;
					delta+= 240 + (horizon);
					fseek(fin,3600+delta  ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels;
				}
				
				if(iline + 1 <= ilineEnd){
					delta = (iline-ilineStart+1)*(numXlines)*(ns*4 + 240);
					delta+= (xline-xlineStart)*(ns*4+240) ;
					delta+= 240 + (horizon);
					fseek(fin,3600+delta  ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels;
				}

				if(xline-1 >= xlineStart){
					delta = (iline-ilineStart)*(numXlines)*(ns*4 + 240);
					delta+= (xline-xlineStart-1)*(ns*4+240) ;
					delta+= 240 + (horizon);
					fseek(fin,3600+delta  ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels;
				}
				
				if(xline+1 <= xlineEnd){
					delta = (iline-ilineStart)*(numXlines)*(ns*4 + 240);
					delta+= (xline-xlineStart+1)*(ns*4+240) ;
					delta+= 240 + (horizon);
					fseek(fin,3600+delta  ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels;
				}
				
				delta = (iline-ilineStart)*(numXlines)*(ns*4 + 240);
				delta+= (xline-xlineStart)*(ns*4+240) ;
				delta+= 240 + (horizon);
				
				fseek(fin,3600+delta ,SEEK_SET);
				if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
				}
                fread(&read,4,1,fin);
                sumVels+=read;
                ++numVels;
                
                if( !((240+ns*4)%(delta) == 0)){
					fseek(fin,3600+delta + 1*4 ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels; 
				}
				
				if(!(delta==240 || (240+ ns*4)%(delta-240)==0)){
					fseek(fin,3600+delta - 1*4 ,SEEK_SET);
					if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
					}
					fread(&read,4,1,fin);
					sumVels+=read;
					++numVels;
				}
				
				
				fseek(fin,3600+delta ,SEEK_SET);
				if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
				}
                fread(&read,4,1,fin);
                sumVels+=read;
                ++numVels;

				velAve=sumVels/numVels;
				
                fseek(fin,3600+delta ,SEEK_SET);
                if(ftell(fin)<fileBeg || ftell(fin)>fileEnd){
						printf("\n\n Check the inline and xline numbers including those in the headers\n");
						return -1;
				}
                fwrite(&velAve,4,1, fin);
			
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


