#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define M_PI 3.14159265358979323846

typedef struct HSVclr{
	unsigned int h;
    unsigned char s;
    unsigned char v;
}HSV;

typedef struct Pixels{
        unsigned char b;
        unsigned char g;
        unsigned char r;
        int inSegment;
        }Pix;

typedef struct BitMaps{
		//fileheader
        short signature;
        int file_size;
        short reserved1;
        short reserved2;
        int f_offset;
        
        //v5header
        int DIB_Header;
        int img_w;
        int img_h;
        short planes;
        short bits_per_pixel;
        int compression;
        int img_size;
        int x_per_meter;
        int y_per_meter;
        int colors_in_table;
        int important_clrs;
        unsigned char * junk;
        Pix** PixelArray;
        }BitMap;
      
typedef struct pairVal{
	int x;
	int y;
}pair;
      
typedef struct searchColor{
	HSV desired;
	int plusH;
	int minusH;
	int plusS;
	int minusS;
	int plusV;
	int minusV;
}color;

//typedef struct searchEdge{
//	Pix** PixelArray;
//}edge;

typedef struct segmentation{
	color clr;
	//edge* edges;
	int width;
	int height;
	int x;
	int y;
	int n;
}segment;

typedef struct Bbox{
	segment *smlist;
}boundary;

BitMap readBitMap(FILE *BMPFile){
       BitMap newBitMap;

		//load file header   
        fread(&newBitMap.signature,sizeof(newBitMap.signature),1,BMPFile);               
        fread(&newBitMap.file_size,sizeof(newBitMap.file_size),1,BMPFile);      
        fread(&newBitMap.reserved1,sizeof(newBitMap.reserved1),1,BMPFile);       
        fread(&newBitMap.reserved2,sizeof(newBitMap.reserved2),1,BMPFile);       
        fread(&newBitMap.f_offset,sizeof(newBitMap.f_offset),1,BMPFile);
        
        //load v5 header
        fread(&newBitMap.DIB_Header,sizeof(newBitMap.DIB_Header),1,BMPFile);               
        fread(&newBitMap.img_w,sizeof(newBitMap.img_w),1,BMPFile);      
        fread(&newBitMap.img_h,sizeof(newBitMap.img_h),1,BMPFile);       
        fread(&newBitMap.planes,sizeof(newBitMap.planes),1,BMPFile);       
        fread(&newBitMap.bits_per_pixel,sizeof(newBitMap.bits_per_pixel),1,BMPFile);
        fread(&newBitMap.compression,sizeof(newBitMap.compression),1,BMPFile);               
        fread(&newBitMap.img_size,sizeof(newBitMap.img_size),1,BMPFile);      
        fread(&newBitMap.x_per_meter,sizeof(newBitMap.x_per_meter),1,BMPFile);       
        fread(&newBitMap.y_per_meter,sizeof(newBitMap.y_per_meter),1,BMPFile);       
        fread(&newBitMap.colors_in_table,sizeof(newBitMap.colors_in_table),1,BMPFile);
        fread(&newBitMap.important_clrs,sizeof(newBitMap.important_clrs),1,BMPFile);
        
			if ((newBitMap.f_offset-newBitMap.DIB_Header-14) > 0){
		newBitMap.junk = (unsigned char*)malloc(sizeof(unsigned char)*(newBitMap.f_offset-newBitMap.DIB_Header-14));
		fread(newBitMap.junk, sizeof(unsigned char), (newBitMap.f_offset-newBitMap.DIB_Header-14), BMPFile);
	}
		
		/*      
        int tellmewhereiam = ftell(BMPFile);
        printf("%d      ",tellmewhereiam);
        
        //allocate memory for junk
        newBitMap.junk = malloc(newBitMap.f_offset - tellmewhereiam);              
        
        //load junk bitmap
        fread(newBitMap.junk,(newBitMap.f_offset - tellmewhereiam),1,BMPFile);
        
        tellmewhereiam = ftell(BMPFile);
        printf("%d %d  %d\n  ",tellmewhereiam,newBitMap.f_offset,BMPFile);
      */  //printf("%d \n  ",newBitMap.DIB_Header-40); 
        //read array of pixels
        int i=0;
        int j=0;
        int Padding = 4-(newBitMap.img_w * 3)%4; //subject to change
        
        char tmpb;
        char tmpg;
        char tmpr;
        
        printf("%d ", newBitMap.img_h);
        printf("%d ", newBitMap.img_w);
        printf("\n");
        printf("\n");
        
        int tmp;
        
        newBitMap.PixelArray = (Pix**) malloc((sizeof(Pix*))*newBitMap.img_w);
        
        for(tmp=0;tmp<newBitMap.img_w;tmp++){
        	newBitMap.PixelArray[tmp] = (Pix*)malloc(sizeof(Pix)*newBitMap.img_h);
        }
        
        for(i=0;i<newBitMap.img_h;i++){                                     
        	for(j=0;j<newBitMap.img_w;j++){   
                                                                               
        		fread(&tmpb,1,1,BMPFile);
        		fread(&tmpg,1,1,BMPFile);
        		fread(&tmpr,1,1,BMPFile);
        		
        		newBitMap.PixelArray[j][i].b = tmpb;
        		newBitMap.PixelArray[j][i].g = tmpg;
        		newBitMap.PixelArray[j][i].r = tmpr;

        		//printf("%02X,%02X,%02X ",tmpb,tmpg,tmpr);
			}
			if(Padding!=4){
				fseek(BMPFile, Padding, SEEK_CUR);// ominiecie Padding bajtów
			}
        }

        return newBitMap;
       }
       
void writeBitMap(FILE *BMPFile, BitMap bm){
        
        //Write file header   
        fwrite(&bm.signature,sizeof(bm.signature),1,BMPFile);               
        fwrite(&bm.file_size,sizeof(bm.file_size),1,BMPFile);      
        fwrite(&bm.reserved1,sizeof(bm.reserved1),1,BMPFile);       
        fwrite(&bm.reserved2,sizeof(bm.reserved2),1,BMPFile);       
        fwrite(&bm.f_offset,sizeof(bm.f_offset),1,BMPFile);
        
        //Write v5 header
        fwrite(&bm.DIB_Header,sizeof(bm.DIB_Header),1,BMPFile);               
        fwrite(&bm.img_w,sizeof(bm.img_w),1,BMPFile);      
        fwrite(&bm.img_h,sizeof(bm.img_h),1,BMPFile);       
        fwrite(&bm.planes,sizeof(bm.planes),1,BMPFile);       
        fwrite(&bm.bits_per_pixel,sizeof(bm.bits_per_pixel),1,BMPFile);
        fwrite(&bm.compression,sizeof(bm.compression),1,BMPFile);               
        fwrite(&bm.img_size,sizeof(bm.img_size),1,BMPFile);      
        fwrite(&bm.x_per_meter,sizeof(bm.x_per_meter),1,BMPFile);       
        fwrite(&bm.y_per_meter,sizeof(bm.y_per_meter),1,BMPFile);       
        fwrite(&bm.colors_in_table,sizeof(bm.colors_in_table),1,BMPFile);
        fwrite(&bm.important_clrs,sizeof(bm.important_clrs),1,BMPFile);
        
        int tellmewhereiam = ftell(BMPFile);
        //printf("\n \n %d    \n  ",tellmewhereiam);
        
        //write junk back again
        fwrite(&bm.junk, sizeof(bm.junk),1,BMPFile);
        
        fseek(BMPFile,bm.f_offset,SEEK_SET);
        
        //int i=0;
        //int j=0;
        
        //for(i=0;i<bm.img_h;i++){                                     
        //	for(j=0;j<bm.img_w;j++){   
        //                                                                       
        //		printf("%d,%d,%d ",bm.PixelArray[j][i].b ,bm.PixelArray[j][i].g ,bm.PixelArray[j][i].r );
		//	}printf("\n");
        //}
        
        //write array of pixels
        //fwrite(bm.PixelArray,sizeof(bm.PixelArray),1,BMPFile);
        
        int i=0;
        int j=0;
        int y=0;
        int Padding = 4-(bm.img_w * 3)%4;
        
        int tmp = bm.img_w;
        
        char tmpb;
        char tmpg;
        char tmpr;

        for(i=0;i<bm.img_h;i++){                                     
        	for(j=0;j<bm.img_w;j++){   
        		tmpb = bm.PixelArray[j][i].b;
        		tmpg = bm.PixelArray[j][i].g;
        		tmpr = bm.PixelArray[j][i].r;
        		//printf("%02X,%02X,%02X ",(unsigned char)tmpb,(unsigned char)tmpg,(unsigned char)tmpr);
                fwrite(&tmpb,1,1,BMPFile);    
				fwrite(&tmpg,1,1,BMPFile);   
				fwrite(&tmpr,1,1,BMPFile);        
			}//printf("\n");
			if(Padding!=4){
				fwrite(&tmpr,1,Padding,BMPFile);// wstawienie Padding bajtów
			}
        }

        /*
        for(i=0;i<bm.img_h;i++){                                     
        	for(j=0;j<tmp*3;j++){   
                fwrite(bm.PixelArray+j+i*tmp*3,1,1,BMPFile);                                                               
        		
			}//printf("\n");
			int x=0;
			for(y=0;y<Padding;y++){
			fwrite(&x,1,1,BMPFile);
			}
        }*/
        
		
		}
		
//get pixel data from array
Pix getPix(int x, int y, BitMap bm){
	Pix pix;
	pix.b = bm.PixelArray[x][y].b;
	pix.g = bm.PixelArray[x][y].g;
	pix.r = bm.PixelArray[x][y].r;
	//printf("\n\n\n %02X,%02X,%02X \n" , pix.b, pix.g,pix.r);
	return pix;
}

//put specific pixel data to array
void putPix(int x, int y, BitMap bm, Pix p){
	bm.PixelArray[x][y].b = p.b;
	bm.PixelArray[x][y].g = p.g;
	bm.PixelArray[x][y].r = p.r;
	//printf("\n\n\n pixel data insertet %02X,%02X,%02X , at %d %d \n" , p.b, p.g,p.r,x,y);
}




//
//HISTOGRAMS
//

//lightness histogram
int* histogramL(BitMap bm){
	int* histogram = (int*)malloc(256*3*sizeof(Pix));
	
	int i;
	
	for(i=0;i<256;i++){
		histogram[i]=0;
	}
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			int light = (bm.PixelArray[x][y].b + bm.PixelArray[x][y].g + bm.PixelArray[x][y].r)/3;
			histogram[light]++;
		}
	}
	
	//debug
	//for(i=0;i<256;i++){
	//	printf(" %d", histogram[i]);
	//}
	
	return histogram;
}

//blue color histogram
int* histogramB(BitMap bm){
	int* histogram = (int*)malloc(256*3*sizeof(Pix));
	
	int i;
	
	for(i=0;i<256;i++){
		histogram[i]=0;
	}
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			histogram[bm.PixelArray[x][y].b]++;
		}
	}
	
	//for(i=0;i<256;i++){
	//	printf(" %d", histogram[i]);
	//}
	
	return histogram;
}

//green color histogram
int* histogramG(BitMap bm){
	int* histogram = (int*)malloc(256*3*sizeof(Pix));
	
	int i;
	
	for(i=0;i<256;i++){
		histogram[i]=0;
	}
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			histogram[bm.PixelArray[x][y].g]++;
		}
	}
	
	//for(i=0;i<256;i++){
	//	printf(" %d", histogram[i]);
	//}
	
	return histogram;
}

//red color histogram
int* histogramR(BitMap bm){
	int* histogram = (int*)malloc(256*3*sizeof(Pix));
	
	int i;
	
	for(i=0;i<256;i++){
		histogram[i]=0;
	}
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			histogram[bm.PixelArray[x][y].r]++;
		}
	}
	
	//for(i=0;i<256;i++){
	//	printf(" %d", histogram[i]);
	//}
	
	return histogram;
}




//
//SINGLE BITMAP ARITHMETICS
//

//adding const
BitMap addConst(BitMap bm, int a){
	BitMap added;
	added = bm;
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			//add to blue
			if(bm.PixelArray[x][y].b+a > 255){
				bm.PixelArray[x][y].b = 255;
			}
			else if(bm.PixelArray[x][y].b+a < 0){
				bm.PixelArray[x][y].b = 0;
			}
			else{
				bm.PixelArray[x][y].b = bm.PixelArray[x][y].b+a;
			}
			
			//add to green
			if(bm.PixelArray[x][y].g+a > 255){
				bm.PixelArray[x][y].g = 255;
			}
			else if(bm.PixelArray[x][y].g+a < 0){
				bm.PixelArray[x][y].g = 0;
			}
			else{
				bm.PixelArray[x][y].g = bm.PixelArray[x][y].g+a;
			}
			
			//add to red
			if(bm.PixelArray[x][y].r+a > 255){
				bm.PixelArray[x][y].r = 255;
			}
			else if(bm.PixelArray[x][y].r+a < 0){
				bm.PixelArray[x][y].r = 0;
			}
			else{
				bm.PixelArray[x][y].r = bm.PixelArray[x][y].r+a;
			}
		}
	}
	
	return added;
}

//gray out
BitMap gray(BitMap bm){
	BitMap grayed;
	grayed = bm;
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			int gray = (bm.PixelArray[x][y].b + bm.PixelArray[x][y].g + bm.PixelArray[x][y].r)/3;
			bm.PixelArray[x][y].b = gray;
			bm.PixelArray[x][y].g = gray;
			bm.PixelArray[x][y].r = gray;			
		}
	}
	
	return grayed;
}

//multiplying by const
BitMap multiplyConst(BitMap bm, float a){
	BitMap multiplied;
	multiplied = bm;
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			//multiply in blue
			if(bm.PixelArray[x][y].b*a > 255){
				bm.PixelArray[x][y].b = 255;
			}
			else if(bm.PixelArray[x][y].b*a < 0){
				bm.PixelArray[x][y].b = 0;
			}
			else{
				bm.PixelArray[x][y].b = bm.PixelArray[x][y].b*a;
			}
			
			//multiply in  green
			if(bm.PixelArray[x][y].g*a > 255){
				bm.PixelArray[x][y].g = 255;
			}
			else if(bm.PixelArray[x][y].g*a < 0){
				bm.PixelArray[x][y].g = 0;
			}
			else{
				bm.PixelArray[x][y].g = bm.PixelArray[x][y].g*a;
			}
			
			//multiply in red
			if(bm.PixelArray[x][y].r*a > 255){
				bm.PixelArray[x][y].r = 255;
			}
			else if(bm.PixelArray[x][y].r*a < 0){
				bm.PixelArray[x][y].r = 0;
			}
			else{
				bm.PixelArray[x][y].r = bm.PixelArray[x][y].r*a;
			}
		}
	}
	
	return multiplied;
}

//negative
BitMap negative(BitMap bm){
	BitMap neg;
	neg = bm;
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			
			neg.PixelArray[x][y].b = 255 - bm.PixelArray[x][y].b;
			neg.PixelArray[x][y].g = 255 - bm.PixelArray[x][y].g;
			neg.PixelArray[x][y].r = 255 - bm.PixelArray[x][y].r;			
		}
	}
	
	return neg;
}

//threshold
BitMap threshold(BitMap bm, int level){
	BitMap thr;
	thr = gray(bm);
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			if(thr.PixelArray[x][y].b < level){
				thr.PixelArray[x][y].b = 0;
			}
			else{
				thr.PixelArray[x][y].b = 255;
			}
			
			if(thr.PixelArray[x][y].g < level){
				thr.PixelArray[x][y].g = 0;
			}
			else{
				thr.PixelArray[x][y].g = 255;
			}
			
			if(thr.PixelArray[x][y].r < level){
				thr.PixelArray[x][y].r = 0;
			}
			else{
				thr.PixelArray[x][y].r = 255;
			}			
		}
	}
	
	return thr;
}




//
//STATISTICS
//

//maximum
int maxStats(BitMap bm)
{
	int x;
	int y;
	int max = 0;
	
	for (x=0; x<bm.img_w; x++)
	{
		for (y=0; y<bm.img_h; y++)
		{
			int average = (bm.PixelArray[x][y].b + bm.PixelArray[x][y].g + bm.PixelArray[x][y].r)/3;
			if (average > max)
				max = average;
		}
	}
	
	return max;
}

//minimum
int minStats(BitMap bm)
{
	int x; 
	int y; 
	int min = 255;
	
	for (x=0; x<bm.img_w; x++)
	{
		for (y=0; y<bm.img_h; y++)
		{
			int average = (bm.PixelArray[x][y].b + bm.PixelArray[x][y].g + bm.PixelArray[x][y].r)/3;
			if (average < min)
				min = average;
		}
	}
	
	return min;
}



//
//DOUBLE BITMAP ARITHMETICS
//

//adding bitmaps
BitMap add(BitMap bm1, BitMap bm2){
	BitMap joined;
	joined = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			if(bm1.PixelArray[x][y].b + bm2.PixelArray[x][y].b > 255){
				joined.PixelArray[x][y].b = 255;
			}
			else{
				joined.PixelArray[x][y].b = bm1.PixelArray[x][y].b + bm2.PixelArray[x][y].b;
			}
			if(bm1.PixelArray[x][y].g + bm2.PixelArray[x][y].g > 255){
				joined.PixelArray[x][y].g = 255;
			}
			else{
				joined.PixelArray[x][y].g = bm1.PixelArray[x][y].g + bm2.PixelArray[x][y].g;
			}
			if(bm1.PixelArray[x][y].r + bm2.PixelArray[x][y].r > 255){
				joined.PixelArray[x][y].r = 255;
			}
			else{
				joined.PixelArray[x][y].r = bm1.PixelArray[x][y].r + bm2.PixelArray[x][y].r;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return joined;
}

//maximum
BitMap max(BitMap bm1, BitMap bm2){
	BitMap maxed;
	maxed = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			int max1 = (bm1.PixelArray[x][y].b + bm1.PixelArray[x][y].g + bm1.PixelArray[x][y].r)/3;
			int max2 = (bm2.PixelArray[x][y].b + bm2.PixelArray[x][y].g + bm2.PixelArray[x][y].r)/3;
			
			if(max1 > max2){
				maxed.PixelArray[x][y].b = bm1.PixelArray[x][y].b;
				maxed.PixelArray[x][y].g = bm1.PixelArray[x][y].g;
				maxed.PixelArray[x][y].r = bm1.PixelArray[x][y].r;
			}
			else if(max1 < max2){
				maxed.PixelArray[x][y].b = bm2.PixelArray[x][y].b;
				maxed.PixelArray[x][y].g = bm2.PixelArray[x][y].g;
				maxed.PixelArray[x][y].r = bm2.PixelArray[x][y].r;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return maxed;
}

//minimum
BitMap min(BitMap bm1, BitMap bm2){
	BitMap minimized;
	minimized = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			int min1 = (bm1.PixelArray[x][y].b + bm1.PixelArray[x][y].g + bm1.PixelArray[x][y].r)/3;
			int min2 = (bm2.PixelArray[x][y].b + bm2.PixelArray[x][y].g + bm2.PixelArray[x][y].r)/3;
			
			if(min1 < min2){
				minimized.PixelArray[x][y].b = bm1.PixelArray[x][y].b;
				minimized.PixelArray[x][y].g = bm1.PixelArray[x][y].g;
				minimized.PixelArray[x][y].r = bm1.PixelArray[x][y].r;
			}
			else if(min1 > min2){
				minimized.PixelArray[x][y].b = bm2.PixelArray[x][y].b;
				minimized.PixelArray[x][y].g = bm2.PixelArray[x][y].g;
				minimized.PixelArray[x][y].r = bm2.PixelArray[x][y].r;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return minimized;
}

//difference of two bitmaps
BitMap difference(BitMap bm1, BitMap bm2){
	BitMap diff;
	diff = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			if(bm1.PixelArray[x][y].b - bm2.PixelArray[x][y].b < 0){
				diff.PixelArray[x][y].b = 0;
			}
			else{
				diff.PixelArray[x][y].b = bm1.PixelArray[x][y].b - bm2.PixelArray[x][y].b;
			}
			if(bm1.PixelArray[x][y].g - bm2.PixelArray[x][y].g < 0){
				diff.PixelArray[x][y].g = 0;
			}
			else{
				diff.PixelArray[x][y].g = bm1.PixelArray[x][y].g - bm2.PixelArray[x][y].g;
			}
			if(bm1.PixelArray[x][y].r - bm2.PixelArray[x][y].r < 0){
				diff.PixelArray[x][y].r = 0;
			}
			else{
				diff.PixelArray[x][y].r = bm1.PixelArray[x][y].r - bm2.PixelArray[x][y].r;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return diff;
}

//multiply two bitmaps
BitMap multiply(BitMap bm1, BitMap bm2){
	BitMap multi;
	multi = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			float multib = (bm1.PixelArray[x][y].b * bm2.PixelArray[x][y].b)/255.0;
			float multig = (bm1.PixelArray[x][y].g * bm2.PixelArray[x][y].g)/255.0;
			float multir = (bm1.PixelArray[x][y].r * bm2.PixelArray[x][y].r)/255.0;
			
			
			//multiply in blue
			if(multib > 255){
				multi.PixelArray[x][y].b = 255;
			}
			else if(multib < 0){
				multi.PixelArray[x][y].b = 0;
			}
			else{
				multi.PixelArray[x][y].b = multib;
			}
			
			//multiply in  green
			if(multig > 255){
				multi.PixelArray[x][y].g = 255;
			}
			else if(multig < 0){
				multi.PixelArray[x][y].g = 0;
			}
			else{
				multi.PixelArray[x][y].g = multig;
			}
			
			//multiply in red
			if(multir > 255){
				multi.PixelArray[x][y].r = 255;
			}
			else if(multir < 0){
				multi.PixelArray[x][y].r = 0;
			}
			else{
				multi.PixelArray[x][y].r = multir;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return multi;
}

//linear combination of two bitmaps
BitMap linearCombination(BitMap bm1, BitMap bm2, float weight1, float weight2){
	BitMap linear;
	linear = bm1;
	
	int x;
	int y;
	
	if(bm1.img_w == bm2.img_w && bm1.img_h == bm2.img_h){
		
	for(x=0;x<bm1.img_w;x++){
		for(y=0;y<bm1.img_h;y++){
			float linb = (bm1.PixelArray[x][y].b * weight1 + bm2.PixelArray[x][y].b) * weight2;
			float ling = (bm1.PixelArray[x][y].g * weight1 + bm2.PixelArray[x][y].g) * weight2;
			float linr = (bm1.PixelArray[x][y].r * weight1 + bm2.PixelArray[x][y].r) * weight2;
			
			
			//multiply in blue
			if(linb > 255){
				linear.PixelArray[x][y].b = 255;
			}
			else if(linb < 0){
				linear.PixelArray[x][y].b = 0;
			}
			else{
				linear.PixelArray[x][y].b = linb;
			}
			
			//multiply in  green
			if(ling > 255){
				linear.PixelArray[x][y].g = 255;
			}
			else if(ling < 0){
				linear.PixelArray[x][y].g = 0;
			}
			else{
				linear.PixelArray[x][y].g = ling;
			}
			
			//multiply in red
			if(linr > 255){
				linear.PixelArray[x][y].r = 255;
			}
			else if(linr < 0){
				linear.PixelArray[x][y].r = 0;
			}
			else{
				linear.PixelArray[x][y].r = linr;
			}
		}
	}
	}
	else{
		printf("adding failed, different image sizes \n");
	}
	
	return linear;
}



//
//GEOMETRICS
//

//mod function from:
//https://stackoverflow.com/questions/4003232/how-to-code-a-modulo-operator-in-c-c-obj-c-that-handles-negative-numbers
int mod (int a, int b)
{
   if(b < 0) //you can check for b == 0 separately and do what you want
     return mod(a, -b);   
   int ret = a % b;
   if(ret < 0)
     ret+=b;
   return ret;
}

//move bitmap by vector
BitMap move(BitMap bm, int wl, int hl, int cycle){
	BitMap moved;
	moved = bm;
	
	int x;
	int y;
	
	for(x=0;x<bm.img_w;x++){
		for(y=0;y<bm.img_h;y++){
			if(cycle == 1){
				int modx = mod(x+wl, bm.img_w);
				int mody = mod(y-hl, bm.img_h);
				moved.PixelArray[x][y] = bm.PixelArray[modx][mody];
			}
			else if (x + wl >= 0 && x + wl < bm.img_w && y - hl >= 0 && y - hl < bm.img_h){
				moved.PixelArray[x][y] = bm.PixelArray[x+wl][y-hl];
			}
		}
	}
	
	return moved;
}

//new bitmap with different size
BitMap newBitMapSize(int new_w, int new_h,BitMap oldBitMap)
{
	BitMap newBitMap = oldBitMap;
			
	newBitMap.img_w = new_w;
	newBitMap.img_h = new_h;
    
    int padding = 4-(newBitMap.img_w * 3)%4;

    newBitMap.img_size = (new_w*3+padding)*new_h;

    int tmp;
        
    newBitMap.PixelArray = (Pix**) malloc((sizeof(Pix*))*newBitMap.img_w);
        
    for(tmp=0;tmp<newBitMap.img_w;tmp++){
        newBitMap.PixelArray[tmp] = (Pix*)malloc(sizeof(Pix)*newBitMap.img_h);
        memset(newBitMap.PixelArray[tmp],0,sizeof(Pix)*newBitMap.img_h);
    }
	
	int junksize = newBitMap.f_offset-newBitMap.DIB_Header-14;
	
	newBitMap.file_size = 54 + junksize + newBitMap.img_size;

	return newBitMap;
}

//crop bitmap
BitMap crop(BitMap bm, int w, int h, int x, int y){
	BitMap cropped = newBitMapSize(w,h,bm);
	
	int i;
	int j;
	
	for(i=0; i<cropped.img_w; i++){
		for(j=0; j<cropped.img_h; j++){
			cropped.PixelArray[i][j] = bm.PixelArray[i+x][j+y];
		}
	}
	
	return cropped;
}

//resize bitmap
BitMap resize(BitMap bm, int w, int h, const char * method){
	BitMap resized = newBitMapSize(w,h,bm);
	
	float ratioX = (float)bm.img_w / (float)w;
	float ratioY = (float)bm.img_h / (float)h;
	
	int x;
	int y;
	
	if(method == "n"){
		for(x=0; x<resized.img_w;x++){
			for(y=0; y<resized.img_h;y++){
				resized.PixelArray[x][y] = bm.PixelArray[x*(int)ratioX][y*(int)ratioY];
			}
		}
	}else if(method == "b"){
		
	}
	
	return resized;
}

//rotate bitmap by 90,180 or 270 degrees 
BitMap rotate(BitMap bm, int deg){
	BitMap rotated = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	if(deg == 90 || deg == 270){
		rotated = newBitMapSize(bm.img_h,bm.img_w,bm);
	}
	
	int x;
	int y;

	for(x=0; x<rotated.img_w;x++){
		for(y=0; y<rotated.img_h;y++){
			if(deg==90){
				rotated.PixelArray[x][y] = bm.PixelArray[bm.img_w-y-1][x];				
			}
			else if(deg == 270){
				rotated.PixelArray[x][y] = bm.PixelArray[y][bm.img_h-x-1];
			}
			else if(deg == 180){
				rotated.PixelArray[x][y] = bm.PixelArray[bm.img_w-x-1][bm.img_h-y-1];
			}
		}
	}
	
	return rotated;
}

//flip bitmap 0-vertically 1-horizontaly
BitMap flip(BitMap bm, int direct){
	BitMap flipped = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	int x;
	int y;
	
	for(x=0; x<bm.img_w; x++){
		for(y=0; y<bm.img_h; y++){
			if(direct == 0){
				flipped.PixelArray[x][y] = bm.PixelArray[x][bm.img_h-y-1];
			}else if(direct == 1){
				flipped.PixelArray[x][y] = bm.PixelArray[bm.img_w-x-1][y];
			}
		}
	}
	
	return flipped;
}



//
//MORPHOLOGICAL FILTERS
//

//function generating matrix of ints values of 1
int** newMatrix(int w, int h){
	int ** matrix = (int**)malloc(w*sizeof(int*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (int*)malloc(h*sizeof(int));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
				matrix[i][j] = 1;
		}
	}
	return matrix;
}

//BitMap dilate
BitMap dilate(BitMap bm, int** matrix, int w, int h){
	BitMap dilated = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	int x;
	int y;
	
	int i;
	int j;
	
	for(x=w/2; x<bm.img_w-(w-1); x++)
	{
		for(y=h/2; y<bm.img_h-(h-1); y++)
		{
			int max=0;
			
			for(i=0; i<w;i++)
			{
				for(j=0; j<h;j++)
				{
					if(matrix[i][j] == 1)
					{
						Pix pix = bm.PixelArray[x+(i-w/2)][y+(j-h/2)];
						if(((pix.b+pix.g+pix.r)/3) > max)
							max = (pix.b+pix.g+pix.r)/3;
					}
				}
			}
			
			 dilated.PixelArray[x][y].b = max;
			 dilated.PixelArray[x][y].g = max;
			 dilated.PixelArray[x][y].r = max;		
		}
	}
	
	return dilated;
}

//Bitmap erode
BitMap erode(BitMap bm, int** matrix, int w, int h){
	BitMap eroded = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	int x;
	int y;
	
	int i;
	int j;
	
	for(x=w/2; x<bm.img_w-(w-1); x++)
	{
		for(y=h/2; y<bm.img_h-(h-1); y++)
		{
			int min=255;
			
			for(i=0; i<w;i++)
			{
				for(j=0; j<h;j++)
				{
					if(matrix[i][j] == 1)
					{
						Pix pix = bm.PixelArray[x+(i-w/2)][y+(j-h/2)];
						if(((pix.b+pix.g+pix.r)/3) < min)
							min = (pix.b+pix.g+pix.r)/3;
					}
				}
			}
			
			 eroded.PixelArray[x][y].b = min;
			 eroded.PixelArray[x][y].g = min;
			 eroded.PixelArray[x][y].r = min;		
		}
	}
	
	return eroded;
}

//Bitmap opening
BitMap openMor(BitMap bm, int** matrix,int w, int h)
{
	BitMap tmp = erode(bm,matrix,w,h);
	BitMap opened = dilate(tmp,matrix,w,h);
	return opened;
}

//Bitmap closing
BitMap closeMor(BitMap bm, int** matrix,int w, int h)
{
	BitMap tmp = dilate(bm,matrix,w,h);
	BitMap closed = erode(tmp,matrix,w,h);
	return closed;
}

//Bitmap white Top-Hat
BitMap whiteTH(BitMap bm, int** matrix,int w, int h)
{
	BitMap tmp = openMor(bm,matrix,w,h);
	BitMap whitetophat = difference(bm,tmp);
	return whitetophat;
}

//Bitmap black Top-Hat
BitMap blackTH(BitMap bm, int** matrix,int w, int h)
{
	BitMap tmp = closeMor(bm,matrix,w,h);
	BitMap blacktophat = difference(tmp,bm);
	return blacktophat;
}



//
//UPPER AND LOWER FILTERS +NOISE
//

//Add noise to Bitmap
BitMap addNoiseRect(BitMap bm, int level, float density){
	BitMap withNoise = bm;
	
	int x;
	int y;
	
	for(x=0; x<bm.img_w;x++){
		for(y=0; y<bm.img_h;y++){
			if ((double)rand() / RAND_MAX <= density)
			{
				int variation = -level + 2 * level * (double)rand() / RAND_MAX;
				
				if(bm.PixelArray[x][y].b + variation > 255){
					withNoise.PixelArray[x][y].b = 255;
				}else if(bm.PixelArray[x][y].b + variation < 0){
					withNoise.PixelArray[x][y].b = 0;
				}else{
					withNoise.PixelArray[x][y].b = bm.PixelArray[x][y].b + variation;
				}
				
				if(bm.PixelArray[x][y].g + variation > 255){
					withNoise.PixelArray[x][y].g = 255;
				}else if(bm.PixelArray[x][y].g + variation < 0){
					withNoise.PixelArray[x][y].g = 0;
				}else{
					withNoise.PixelArray[x][y].g = bm.PixelArray[x][y].g + variation;
				}
				
				if(bm.PixelArray[x][y].r + variation > 255){
					withNoise.PixelArray[x][y].r = 255;
				}else if(bm.PixelArray[x][y].r + variation < 0){
					withNoise.PixelArray[x][y].r = 0;
				}else{
					withNoise.PixelArray[x][y].r = bm.PixelArray[x][y].r + variation;
				}
			}
		}
	}
	
	return withNoise;
}

//Add normal noise to Bitmap
BitMap addNoiseNor(BitMap bm, int mean, float sigma, float density){
	BitMap withNoise = bm;
	
	int x;
	int y;
	
	for(x=0; x<bm.img_w;x++){
		for(y=0; y<bm.img_h;y++){
			if ((double)rand() / RAND_MAX <= density)
			{
				int variation = sqrt(-2.0 * log((double)rand() / RAND_MAX)) * cos(2.0 * M_PI * ((double)rand() / RAND_MAX)) * sigma + mean;
				
				if(bm.PixelArray[x][y].b + variation > 255){
					withNoise.PixelArray[x][y].b = 255;
				}else if(bm.PixelArray[x][y].b + variation < 0){
					withNoise.PixelArray[x][y].b = 0;
				}else{
					withNoise.PixelArray[x][y].b = bm.PixelArray[x][y].b + variation;
				}
				
				if(bm.PixelArray[x][y].g + variation > 255){
					withNoise.PixelArray[x][y].g = 255;
				}else if(bm.PixelArray[x][y].g + variation < 0){
					withNoise.PixelArray[x][y].g = 0;
				}else{
					withNoise.PixelArray[x][y].g = bm.PixelArray[x][y].g + variation;
				}
				
				if(bm.PixelArray[x][y].r + variation > 255){
					withNoise.PixelArray[x][y].r = 255;
				}else if(bm.PixelArray[x][y].r + variation < 0){
					withNoise.PixelArray[x][y].r = 0;
				}else{
					withNoise.PixelArray[x][y].r = bm.PixelArray[x][y].r + variation;
				}
			}
		}
	}
	
	return withNoise;
}

//Add Salt & Pepper Noise
BitMap addNoiseSP(BitMap bm, float density)
{
	BitMap withNoise = bm;
	
	int x;
	int y;
	
	for(x=0; x<bm.img_w;x++){
		for(y=0; y<bm.img_h;y++){
			if ((double)rand() / RAND_MAX <= density)
			{
				int channel = rand() % 3;
				
				if (channel == 0){
					if(rand() % 2 == 0 ){
						withNoise.PixelArray[x][y].b = 0;
					}
					else{
						withNoise.PixelArray[x][y].b = 255;
					}
				}
				else if (channel == 1){
					if(rand() % 2 == 0 ){
						withNoise.PixelArray[x][y].g = 0;
					}
					else{
						withNoise.PixelArray[x][y].g = 255;
					}
				}
				else if (channel == 2){
					if(rand() % 2 == 0 ){
						withNoise.PixelArray[x][y].r = 0;
					}
					else{
						withNoise.PixelArray[x][y].r = 255;
					}
				}
			}
		}
	}

	return withNoise;
}

//filter funtion
BitMap filter(BitMap bm, float ** matrix, int w, int h)
{
	BitMap filtered = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	int x;
	int y;
	
	int i;
	int j;

	for(x=0; x<bm.img_w;x++){
		for(y=0; y<bm.img_h;y++){
			
			double b = 0.0;
			double g = 0.0;
			double r = 0.0;
			
			for(i=0; i<w;i++){
				for(j=0; j<h;j++){
					
					int o = w / 2;
					if (x - o + i >= 0 && y - o + j >= 0 && x - o + i < bm.img_w && y - o + j < bm.img_h)
					{
						b += bm.PixelArray[x - o + i][ y - o + j].b * matrix[i][j];
						g += bm.PixelArray[x - o + i][ y - o + j].g * matrix[i][j];
						r += bm.PixelArray[x - o + i][ y - o + j].r * matrix[i][j];
					}
				}
			}
			if(b > 255){
				filtered.PixelArray[x][y].b = 255;	
			}
			else if(b < 0 ){
				filtered.PixelArray[x][y].b = 0;	
			}
			else{
				filtered.PixelArray[x][y].b = b;	
			}
			
			if(g > 255){
				filtered.PixelArray[x][y].g = 255;	
			}
			else if(g < 0 ){
				filtered.PixelArray[x][y].g = 0;	
			}
			else{
				filtered.PixelArray[x][y].g = g;	
			}
			
			if(r > 255){
				filtered.PixelArray[x][y].r = 255;	
			}
			else if(r < 0 ){
				filtered.PixelArray[x][y].r = 0;	
			}
			else{
				filtered.PixelArray[x][y].r = r;	
			}
		}
	}

	return filtered;
}


//FILTER MATRIXES

//LOWER

//usredniajacy
float** usr(int w, int h){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
				matrix[i][j] = 1/9.0;
		}
	}
	return matrix;
}

//usredniajacy wzmocniony
float** usrPlus(int w, int h){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			if(i==w/2 && j==h/2){
				matrix[i][j] = 2/10.0;
			}else{
				matrix[i][j] = 1/10.0;
			}	
		}
	}
	return matrix;
}

//usredniajacy oslabiony
float** usrMinus(int w, int h){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			if(i==w/2 && j==h/2){
				matrix[i][j] = 0/8.0;
			}else{
				matrix[i][j] = 1/8.0;
			}	
		}
	}
	return matrix;
}

//Gauss
float** gauss(int w, int h){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			if(i==w/2 && j==h/2){
				matrix[i][j] = 4/16.0;
				//printf("4");
			}
			else if(i==w/2 || j==h/2){
				matrix[i][j] = 2/16.0;
				//printf("2");
			}
			else{
				matrix[i][j] = 1/16.0;
				//printf("1");
			}	
		}
	}
	return matrix;
}


//UPPER

//Roberts, mx:my -1 coords, px:py +1 coords
float** roberts(int w, int h, int mx, int my, int px, int py){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			matrix[i][j] = 0.0;		
		}
	}
	
	matrix[mx][my] = -1.0;
	matrix[px][py] = 1.0;
	
	return matrix;
}

//Prewitt, mode 0-3, 0: up, 1: down, 2: right, 3: left
float** prewitt(int w, int h, int mode){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			matrix[i][j] = 0.0;
			
			switch(mode){
				case 0:
					if(i==0){
						matrix[i][j] = 1.0;
					}
					else if(i==(w-1)){
						matrix[i][j] = -1.0;
					}
					else{
						matrix[i][j] = 0.0;
					}
					break;
				case 1:
					if(i==0){
						matrix[i][j] = -1.0;
					}
					else if(i==(w-1)){
						matrix[i][j] = 1.0;
					}
					else{
						matrix[i][j] = 0.0;
					}
					break;
				case 2:
					if(j==0){
						matrix[i][j] = -1.0;
					}
					else if(j==(h-1)){
						matrix[i][j] = 1.0;
					}
					else{
						matrix[i][j] = 0.0;
					}
					break;
				case 3:
					if(j==0){
						matrix[i][j] = 1.0;
					}
					else if(j==(h-1)){
						matrix[i][j] = -1.0;
					}
					else{
						matrix[i][j] = 0.0;
					}
					break;
				default:
					printf("this mode doesnt exist");
					break;
			}
		}
	}
	
	return matrix;
}

//Sobel, mode 0-3, 0: up, 1: down, 2: right, 3: left
float** sobel(int w, int h, int mode){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			matrix[i][j] = 0.0;
			
			switch(mode){
				case 0:
					if(i==0 && j==h/2){
						matrix[i][j] = 2.0;
						printf("2");
					}
					else if(i==(w-1) && j==h/2){
						matrix[i][j] = -2.0;
						printf("-2");
					}
					else if(i==0){
						matrix[i][j] = 1.0;
						printf("1");
					}
					else if(i==(w-1)){
						matrix[i][j] = -1.0;
						printf("-1");
					}
					else{
						matrix[i][j] = 0.0;
						printf("0");
					}
					break;
				case 1:
					if(i==0 && j==h/2){
						matrix[i][j] = -2.0;
						printf("-2");
					}
					else if(i==(w-1) && j==h/2){
						matrix[i][j] = 2.0;
						printf("2");
					}
					else if(i==0){
						matrix[i][j] = -1.0;
						printf("-1");
					}
					else if(i==(w-1)){
						matrix[i][j] = 1.0;
						printf("1");
					}
					else{
						matrix[i][j] = 0.0;
						printf("0");
					}
					break;
				case 2:
					if(j==0 && i==w/2){
						matrix[i][j] = -2.0;
						printf("-2");
					}
					else if(j==(h-1) && i==w/2){
						matrix[i][j] = 2.0;
						printf("2");
					}
					else if(j==0){
						matrix[i][j] = -1.0;
						printf("-1");
					}
					else if(j==(h-1)){
						matrix[i][j] = 1.0;
						printf("1");
					}
					else{
						matrix[i][j] = 0.0;
						printf("0");
					}
					break;
				case 3:
					if(j==0 && i==w/2){
						matrix[i][j] = 2.0;
						printf("2");
					}
					else if(j==(h-1) && i==w/2){
						matrix[i][j] = -2.0;
						printf("-2");
					}
					else if(j==0){
						matrix[i][j] = 1.0;
						printf("1");
					}
					else if(j==(h-1)){
						matrix[i][j] = -1.0;
						printf("-1");
					}
					else{
						matrix[i][j] = 0.0;
						printf("0");
					}
					break;
				default:
					printf("this mode doesnt exist");
					break;
			}
		}
	}
	
	return matrix;
}

//Laplace
float** laplace(int w, int h){
	float ** matrix = (float**)malloc(w*sizeof(float*));
	
	int i;
	int j;
	
	for(i=0;i<w;i++)
	{
		matrix[i] = (float*)malloc(h*sizeof(float));
	}
	for(i=0; i<w;i++)
	{
		for(j=0; j<h; j++)
		{
			if(i==w/2 && j==h/2){
				matrix[i][j] = 4.0;
				printf("4");
			}
			else if((i==0 || i==(w-1)) && j==h/2){
				matrix[i][j] = -1.0;
				printf("-1");	
			}
			else if((j==0 || j==(h-1)) && i==w/2){
				matrix[i][j] = -1.0;
				printf("-1");	
			}
			else{
				matrix[i][j] = 0.0;	
				printf("0");
			}				
		}
	}
	
	return matrix;
}

//filtr medianowy

BitMap filterMedian(BitMap bm, int w, int h)
{
	BitMap filtered = newBitMapSize(bm.img_w,bm.img_h,bm);
	
	int* resultMatrix[3];
	
	resultMatrix[0] = (int*)malloc(w*h*sizeof(int));
	resultMatrix[1] = (int*)malloc(w*h*sizeof(int));
	resultMatrix[2] = (int*)malloc(w*h*sizeof(int));
	
	int x;
	int y;
	
	int i;
	int j;
	
	int count;
	int number;
	
	for(x=w/2;x<filtered.img_w-w/2;x++)
	{
		for(y=h/2;y<filtered.img_h-h/2;y++)
		{
			for(i=0;i<w;i++)
			{
				for(j=0;j<h;j++)
				{
					int counter = i*h+j;
					resultMatrix[0][counter] = bm.PixelArray[x+(i-w/2)][y+(j-h/2)].b;
					resultMatrix[1][counter] = bm.PixelArray[x+(i-w/2)][y+(j-h/2)].g;
					resultMatrix[2][counter] = bm.PixelArray[x+(i-w/2)][y+(j-h/2)].r;					 
				}
			}
			
			for(i=0; i<3;i++)
			{
				for(count=0; count<w*h;count++)
				{
					for(number=0; number<w*h-1;number++)
					{
						if(resultMatrix[i][number] < resultMatrix[i][number+1])
						{
							int tmp = resultMatrix[i][number];
							resultMatrix[i][number] = resultMatrix[i][number+1];
							resultMatrix[i][number+1]=tmp;
						}
					}
				}
			}
			
		    filtered.PixelArray[x][y].b = resultMatrix[0][(int)((w*h)/2 +1)];
		    filtered.PixelArray[x][y].g = resultMatrix[1][(int)((w*h)/2 +1)];
		    filtered.PixelArray[x][y].r = resultMatrix[2][(int)((w*h)/2 +1)];
		}
	}
	
	return filtered;
}

//Convert rgb to hsv
HSV RgbToHsv(Pix rgb){
	HSV HueSatVal;
	
	unsigned char rgbMin, rgbMax;
	//minimal and maxilmal rgb values
	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    HueSatVal.v = rgbMax;
    
    if (rgbMin == HueSatVal.v)
    {
        HueSatVal.h = 0;
    }
    else{
    	if(rgb.r == HueSatVal.v){
    		HueSatVal.h = 0 + ((rgb.g-rgb.b)*60 / (HueSatVal.v - rgbMin));
    	}
    	if(rgb.g == HueSatVal.v){
    		HueSatVal.h = 120 + ((rgb.b-rgb.r)*60 / (HueSatVal.v - rgbMin));
    	}
    	if(rgb.b == HueSatVal.v){
    		HueSatVal.h = 240 + ((rgb.r-rgb.g)*60 / (HueSatVal.v - rgbMin));
    	}
    }
    
    if(HueSatVal.h < 0){
    	HueSatVal.h = HueSatVal.h + 360;
    }

	if (HueSatVal.v == 0){
		HueSatVal.s = 0;	
	}
	else{
		HueSatVal.s = (HueSatVal.v-rgbMin)*100 	/ HueSatVal.v;
	}
	 
	HueSatVal.v = (100*HueSatVal.v)/255;
	 
    return HueSatVal;
}

segment initSegment(color colorData, int w, int h){
	segment sm;
	//set found color values for segment from stepped pixel
	sm.clr.desired.h = colorData.desired.h;
	sm.clr.desired.s = colorData.desired.s;
	sm.clr.desired.v = colorData.desired.v;
	
	//hue range
	sm.clr.plusH = colorData.plusH;
	sm.clr.minusH = colorData.minusH;
	//saturation range
	sm.clr.plusS = colorData.plusS;
	sm.clr.minusS = colorData.minusS;
	//value range
	sm.clr.plusV = colorData.plusV;
	sm.clr.minusV = colorData.minusV;
    //initial segment size
    sm.width = 0;
    sm.height = 0;
    //maximal possible size
    sm.x = w;
    sm.y = h;
    //amount of pixels in segment
    sm.n = 0;
    
    return sm;
}

int checkColor(BitMap *bm, color colorData, int x, int y){
	int minhue;//min hue
	int maxhue;//max hue
	//get hsv values from pixel rgb
	HSV pixHSV = RgbToHsv(bm->PixelArray[x][y]);
	//if range below 0, circle to 360
	if(colorData.desired.h - colorData.minusH < 0){
		minhue = (360 + colorData.desired.h - colorData.minusH);
	}
	else{
		minhue = colorData.desired.h - colorData.minusH;
	}
	//if range above 360, circle to 0
	if(colorData.desired.h + colorData.plusH > 360){
		maxhue = (colorData.desired.h + colorData.plusH - 360);
	}
	else{
		maxhue = colorData.desired.h + colorData.plusH;
	}
	
	//printf("|%d %d %d|/|%d %d %d| \n", pixHSV.h, pixHSV.s, pixHSV.v, colorData.desired.h, colorData.desired.s, colorData.desired.v);
	//check color of pixel with segment pixel
	if(maxhue > minhue){
		if(pixHSV.h > minhue && pixHSV.h < maxhue && pixHSV.s > colorData.desired.s - colorData.minusS && pixHSV.s < colorData.desired.s + colorData.plusS && pixHSV.v > colorData.desired.v - colorData.minusV && pixHSV.v < colorData.desired.v + colorData.plusV) {
			return 1;			
		}
	}
	if(minhue > maxhue){
		if(pixHSV.h < minhue && pixHSV.h > maxhue && pixHSV.s > colorData.desired.s - colorData.minusS && pixHSV.s < colorData.desired.s + colorData.plusS && pixHSV.v > colorData.desired.v - colorData.minusV && pixHSV.v < colorData.desired.v + colorData.plusV) {
			return 1;			
		}
	}

	return 0;
}

segment addPixToSegment(BitMap *bm, segment sm, pair xy){
	//printf("|%d %d, %d %d [%d]|", xy.x,xy.y, sm.x,sm.y,bm.PixelArray[xy.x][xy.y].inSegment);
	//check pixel within segment size
	if(xy.x < sm.x){
		sm.x = xy.x;
		sm.width++;
	}
	if(xy.y < sm.y){
		sm.y = xy.y;
		sm.height++;
	}
	//if pixel out of segment, increase it's size
	if(xy.x > sm.x + sm.width){
		sm.width++;
	}
	if(xy.y > sm.y + sm.height){
		sm.height++;
	}
	
	sm.n++;
	
	bm->PixelArray[xy.x][xy.y].inSegment = 1;
	//printf("powinno byc 1");
	return sm;
}

segment checkNeighbours(BitMap *bm, segment sm, pair xy){
        //check if already in segment
	if(bm->PixelArray[xy.x][xy.y].inSegment == 1){

		return sm;
	}
	//check color of neighbour
	if(checkColor(bm,sm.clr,xy.x,xy.y) == 1){
		segment newSegment = addPixToSegment(bm,sm,xy);
		//check neighbours
		if((xy.x)+1 < bm->img_w){
			pair neighPix;
			neighPix.x = (xy.x)+1;
			neighPix.y = xy.y;
			segment neighbour = checkNeighbours(bm,newSegment,neighPix);
			newSegment = neighbour;
		}
		
		if((xy.x)-1 > 0){
			pair neighPix;
			neighPix.x = (xy.x)-1;
			neighPix.y = xy.y;
			segment neighbour = checkNeighbours(bm,newSegment,neighPix);
			newSegment = neighbour;
		}
		
		if((xy.y)+1 < bm->img_h){
			pair neighPix;
			neighPix.x = xy.x;
			neighPix.y = (xy.y)+1;
			segment neighbour = checkNeighbours(bm,newSegment,neighPix);
			newSegment = neighbour;
		}
		
		if((xy.y)-1 > 0){
			pair neighPix;
			neighPix.x = xy.x;
			neighPix.y = (xy.y)-1;
			segment neighbour = checkNeighbours(bm,newSegment,neighPix);
			newSegment = neighbour;
		}
		
		return newSegment;
	}

	return sm;
}

boundary createBox(BitMap *bm, int step, int plusH, int minusH,int plusS, int minusS,int plusV, int minusV){
//	int x;
//	int y;
//	
//	for(x=0; x<bm.img_w;x++){
//		for(y=0;y<bm.img_h;y++){
//			bm.PixelArray[x][y].inSegment = 0;
//		}
//	}
	
	//max size of segment
	int maxSegmentSize = (bm->img_w*bm->img_h)/ step;

	boundary newBoundary;
	
	int i;
	int j;
	printf("start\n ",i,j);
	newBoundary.smlist = (segment*)malloc(maxSegmentSize * sizeof(segment)); // new list of segments	
	int counter = 0;
		printf("ss %d\n ",bm->img_w/step);
	for(i=0; i< bm->img_w/step; i++){printf("i\n ");
		for(j=0; j< bm->img_h/step; j++){	printf("j\n ");
                 printf("%d%d ",i,j);	
			if(bm->PixelArray[i*step][j*step].inSegment == 0){
				//new color data			
				HSV newPixHSV = RgbToHsv(bm->PixelArray[i*step][j*step]);
				//printf(" |%d %d| ", i*step,j*step);
				color clr;
				clr.desired.h = newPixHSV.h;
				clr.desired.s = newPixHSV.s;
				clr.desired.v = newPixHSV.v;
				
				clr.plusH = plusH;
				clr.minusH = minusH;
				
				clr.plusS = plusS;
				clr.minusS = minusS;
				
				clr.plusV = plusV;
				clr.minusV = minusV;			
				//printf("| %d %d %d|", newPixHSV.h, newPixHSV.s, newPixHSV.v);
				//starting segment
				segment sm = initSegment(clr, bm->img_w,bm->img_h);
				
				pair pixCoords;
				pixCoords.x = i*step;
				pixCoords.y = j*step;
			    //new segment of neighbours in 
				segment newSegment = checkNeighbours(bm,sm,pixCoords);
				//printf(" %d ", newSegment.width);
				newBoundary.smlist[counter] = newSegment;
				//printf("%d", newBoundary.smlist[counter].width);
				counter++;
			}
		}
	}
	return newBoundary;
}

BitMap drawBoundary(BitMap bm, boundary box, Pix p){
	BitMap result = bm;
	
	int i;
	int x;
	int y;
	
	for(i=0; i<bm.img_w+bm.img_h;i++){
		//printf("%d",sizeof(box.smlist));
		for(x=0; x<box.smlist[i].width;x++){
			//printf("%d",box.smlist[i].width);
			for(y=0; y<box.smlist[i].height;y++){
				if (x == 0 || x == box.smlist[i].width -1|| y == box.smlist[i].height -1 || y == 0)
				{
					result.PixelArray[box.smlist[i].x + x][box.smlist[i].y + y].b = p.b;
					result.PixelArray[box.smlist[i].x + x][box.smlist[i].y + y].g = p.g;
					result.PixelArray[box.smlist[i].x + x][box.smlist[i].y + y].r = p.r;
				}
			}
		}
	}
	
	return result;
}



//MAIN FUNCTION
int main(int argc, char *argv[])
{	
	BitMap bm1;
	BitMap bm2;
	
	BitMap bmout;
	
	char *FN1="input_bitmap.bmp";
	//char *FN1="16x16x24b-K-gwiazda.bmp";
	char *FN2="new_bitmap.bmp";
	
	char *FN3="second_bitmap.bmp";
		

	FILE * BMPFile1 = fopen (FN1, "rb");
	printf("\n----\n! Try to open for read: %s  :[%s]\n",FN1,BMPFile1);

	FILE * BMPFile2 = fopen (FN2, "wb");
	printf("\n----\n! Try to open for write: %s  :[%s]\n",FN2,BMPFile2);
	
	FILE * BMPFile3 = fopen (FN3, "rb");
	printf("\n----\n! Try to open for read: %s  :[%s]\n",FN3,BMPFile3);	
	
	if (BMPFile1 == NULL){ 
		printf("! No file: %s\n----\n",FN1);
	}
	else if(BMPFile3 == NULL){
		printf("! No Second file: %s\n----\n",FN3);
	}
	else {
		
		bm1 = readBitMap(BMPFile1); 
		bm2 = readBitMap(BMPFile3); 
		
		//getting and inserting pixels         
		//Pix pixie = getPix(13,13,bm1);
		//putPix(0,0,bm1,pixie);
		
		//int* hist = histogramL(bm1);
		
		//histogram debug
//		int i;
//		printf(" %n");
//		printf(" %n");
//		for(i=0;i<256;i++){
//			printf(" %d ", hist[i]);
//		}
		
		//SINGLE
		//bmout = addConst(bm1,50);
		//bmout = gray(bm1);
		//bmout = multiplyConst(bm1,0.9);
		//=bmout = negative(bm1);
		//bmout = threshold(bm1,128);
		
		//DOUBLE
		//bmout = add(bm1,bm2);
		//bmout = max(bm1,bm2);
		//bmout = min(bm1,bm2);
		//bmout = difference(bm1,bm2);
		//bmout = multiply(bm1,bm2);
		//bmout = linearCombination(bm1,bm2, 0.8, 0.2);
				
		//STATS		
		//printf("%d \n", maxStats(bm1));	
		//printf("%d \n", minStats(bm1));	
				
		//GEOMETRICS
		//bmout = move(bm1,50,50,0);
		//bmout = crop(bm1,300,315,0,0);		
		//bmout = newBitMapSize(50,50,bm1);
		//bmout = resize(bm1,30,60,"n");
		//bmout = rotate(bm1,180);		
		//bmout = flip(bm1,1);
		
		//MORPHOLOGIC FILTERS
		int** matrix = newMatrix(3,3);
		
//		int i,j;
//		for(i=0;i<3;i++){
//			for(j=0;j<3;j++){
//			printf(" %d ", matrix[i][j]);
//			}
//		}
		
		
		//bmout = dilate(bm1,matrix,3,3);
		//bmout = erode(bm1,matrix,3,3);
		
		//bmout = openMor(bm1,matrix,3,3);
		//bmout = closeMor(bm1,matrix,3,3);
		//bmout = whiteTH(bm1,matrix,3,3);
		//bmout = blackTH(bm1,matrix,3,3);
		
		//UPPER AND LOWER FILTERS + NOISE
		//bmout = addNoiseRect(bm1,50,0.25);
		//bmout = addNoiseNor(bm1,50,0.5,0.5);
		//bmout = addNoiseSP(bm1,0.5);
		
		//usredniajacy
//		float** usred = usr(3,3);
//		bmout = filter(bm1,usred,3,3);
		
		//usredniajacy wzmocniony
//		float** usredPlus = usrPlus(3,3);
//		bmout = filter(bm1,usredPlus,3,3);
		
		//usredniajacy oslabiony
//		float** usredMinus = usrMinus(3,3);
//		bmout = filter(bm1,usredMinus,3,3);
		
		//Gauss
//		float** gaussMat = gauss(3,3);
//		bmout = filter(bm1,gaussMat,3,3);
		
		//Roberts
//		float** rob = roberts(3,3,1,0,2,1);
//		bmout = filter(bm1,rob,3,3);
		
		//Prewitt
//		float** prew = prewitt(3,3,3);
//		bmout = filter(bm1,prew,3,3);
		
		//Sobel
//		float** sob = sobel(3,3,3);
//		bmout = filter(bm1,sob,3,3);

		//Laplace
//		float** lap = laplace(3,3);
//		bmout = filter(bm1,lap,3,3);
		
		//filtr medianowy
		//bmout = filterMedian(bm1,3,3);
		
		//HSV nowyHSV = RgbToHsv(bmout.PixelArray[0][0]);
		//printf("%d %d %d : %d %d %d", bmout.PixelArray[0][0].r,bmout.PixelArray[0][0].g,bmout.PixelArray[0][0].b,nowyHSV.h,nowyHSV.s,nowyHSV.v);
		
		boundary newBoundary = createBox(&bm1, 5 , 25,25,40,40,40,40);
		Pix p;
		p.b =128;
		p.g =128;
		p.r =128;
		
		bmout = drawBoundary(bm1,newBoundary,p);
		
		writeBitMap(BMPFile2,bmout);
		
		fclose(BMPFile1);		
		fclose(BMPFile2);
	}
  
  system("PAUSE");	
  return 0;
}
