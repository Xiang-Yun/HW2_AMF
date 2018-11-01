/*
原始資料
============== Bitmap File Header ==============
Identifier : 19778
File Size : 230454
Reserve : 0
Bitmap Data Offset : 54

============== Bitmap Info Header ==============
Bitmap Header Size : 40 Bytes
Width : 320
Height : 240
Planes : 1
Bits Per Pixel : 24
Compression : 0
Bitmap Data Size : 230400 Bytes
Color Table Size(Palette)：0 Bytes
H-Rosolution : 0
U-Rosolution : 0
Used Colors Size : 0
Important Colors: 0
*/

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <direct.h>         // 創建資料夾的函式庫
#include "my_AMF.h"

#define NC 320
#define NR 240

int main(int argc, char *argv[])
{
	unsigned char **ima;
	int r, g, b;
	int Nr, Nc;
	int depth;       // BMP影像bits數

	int tmp = 0;
	int i = 0, j = 0;
	int threshold = 30;
	int  fileIndex = 0;     // 172張camImage
	int C[2] = { NULL };    // 物體中心點座標

	char filename[128];
	char *SrcPath, *bgPath, *drawPath, *fgPath;

	// 影像內容(第一張影像) 
	cam_img camSrc;
	unsigned char *camImage = (unsigned char *)malloc(NC * NR * 3 * sizeof(unsigned char));
	camSrc.nr = NC;
	camSrc.nc = NR;
	camSrc.m = camImage;

	// 初始化
	cam_img nowSrc = uc2D_Initial(NC, NR);
	cam_img bgSrc = uc2D_Initial(NC, NR);
	cam_img drawSrc = uc2D_Initial(NC, NR);

	// 初始化物件，產生txt檔案
	initializeList();

	// create dir for
	_mkdir("AMF_result");
	_mkdir("AMF_result/fg_img");
	_mkdir("AMF_result/bg_img");
	_mkdir("AMF_result/draw_img");

	ifstream fin("AMF_img/filelist.txt");
	while (fin >> filename)
	{
		
		char SrcPath[128] = "AMF_img/";
		char result_position[128] = "AMF_result/";
		strcat(SrcPath, filename);        // AMF_img/filename

		char fgPath[128];
		char bgPath[128];
		char drawPath[128];

		// copypath
		strcpy(fgPath, result_position);
		strcpy(bgPath, result_position);
		strcpy(drawPath, result_position);

		// add filename to path
		strcat(fgPath, "fg_img/");
		strcat(fgPath, filename);
		strcat(bgPath, "bg_img/");
		strcat(bgPath, filename);
		strcat(drawPath, "draw_img/");
		strcat(drawPath, filename);


		//第一張背景初始化
		if (fileIndex == 0)
		{
			float R[NR][NC];
			float G[NR][NC];

			camSrc.m = Load_bmp_24bit(SrcPath, &Nr, &Nc, &depth);

			for (i = 0; i < Nr; i++)
				for (j = 0; j<Nc; j++)
				{
					tmp = i*Nc + j;                   // bmp彩色圖像為左下到右上，依序為BGR
					b = camSrc.m[tmp * 3];
					g = camSrc.m[tmp * 3 + 1];
					r = camSrc.m[tmp * 3 + 2];
	
					if (r + g + b != 0) {
						R[i][j] = (float)r / (b + g + r);
						G[i][j] = (float)g / (b + g + r);
					}
					else {
						R[i][j] = 0;
						G[i][j] = 0;
					}
					// char*類型(ASCII代表的數字，例如'T' = 84
					bgSrc.m[tmp] = nowSrc.m[tmp] = (r + g + b) / 3;      // 大小為NR*NC(灰階)
				}
			fileIndex += 1;
		} // end if
		
		
		//第二張之後進行AMF近似中值濾波法
 		else
		{
			drawSrc.m = Load_bmp_24bit(SrcPath, &Nr, &Nc, &depth);    // 彩色圖標示結果
			drawSrc.nr = Nr;
			drawSrc.nc = Nc;

			nowSrc.m = Load_bmp_8bit(SrcPath, &Nr, &Nc, &depth);
			for (i = 0; i < Nr; i++)
				for (j = 0; j < Nc; j++)
				{
					tmp = i*Nc + j;
					if (nowSrc.m[tmp] > bgSrc.m[tmp])
						bgSrc.m[tmp]++;
					else if (nowSrc.m[tmp] < bgSrc.m[tmp])
						bgSrc.m[tmp]--;

					if (abs(nowSrc.m[tmp] - bgSrc.m[tmp]) > threshold)
						nowSrc.m[tmp] = 255;
					else
						nowSrc.m[tmp] = 0;
				}
		
			erosion(nowSrc);    //侵蝕
			erosion(nowSrc);
			dilation(nowSrc);   //膨脹
			dilation(nowSrc);
		
			DoSearch(nowSrc, C);       // 找到框出的點
			drawImage(drawSrc, C);     // 框出原始影像的物件

			Save_bmp_8bit(bgPath, bgSrc.m, Nr, Nc);
			Save_bmp_8bit(fgPath, nowSrc.m, Nr, Nc);
			Save_bmp_24bit(drawPath, drawSrc.m, Nr, Nc);
			cout << "Load "<< SrcPath <<" is successful." << endl;
			cout << "Image Width = " << Nc << " pixels" << endl;
			cout << "Image Height = " << Nr << " pixels" << endl;
			cout << "============================================================= " << endl;
		} // end else
	} // end while
	fin.close();
	system("pause");
	return 0;
}